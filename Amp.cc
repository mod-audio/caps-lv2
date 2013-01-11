/*
	Amp.cc
	
	Copyright 2003-13 Tim Goetze <tim@quitte.de>
	
	http://quitte.de/dsp/

	Idealised tube amplifer emulation.

*/
/*
	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 3
	of the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
	02111-1307, USA or point your web browser to http://www.gnu.org.
*/

#include "basics.h"

#include "Amp.h"
#include "Descriptor.h"
#include "dsp/polynomials.h"

void
AmpVTS::init()
{
	tonestack.init (fs);
	
	DSP::RBJ::LP (1/fs, .7, biaslp);
	/* compress is initialised in activate() */
}

void
AmpVTS::activate()
{
	hp.reset();
	lp.reset();

	bias = 0;

	remain = 0;
	compress.init (fs);
	compress.set_threshold(0);
	compress.set_release(0);

	/* dcblock is reset in setratio() */

	model = -1;
	ratio = -1;
}

/* set oversampling ratio */
void
AmpVTS::setratio (int r)
{
	if (r == ratio) 
		return;
	ratio = r;
	dcblock.set_f (15./(ratio*fs)); 
	dcblock.reset();
	over2.reset();
	over4.reset();
	over8.reset();
}

template <yield_func_t yield>
void
AmpVTS::cycle (uint frames)
{
	int r = 2 << (int) getport(0);
	setratio(r);
	if (r == 8)
		subcycle<yield> (frames, over8);
	else if (r == 4)
		subcycle<yield> (frames, over4);
	else
		subcycle<yield> (frames, over2);
}

/* saturating function selection */
#define preamp DSP::Polynomial::one5
#define poweramp DSP::Polynomial::atan

/* correction for tonestack model gain differences */
static float tsgain[] = {.764, .615, 1.240, 1.599, 1.132, .628, .715, .393, 1.218};

template <yield_func_t yield, class Over>
void
AmpVTS::subcycle (uint frames, Over & over)
{
	int m = getport(1);
	if (m != model)
		tonestack.setmodel (model = m);
	tonestack.updatecoefs (getport(5),getport(6),getport(7));

	compress.set_attack (.25*getport(8));
	
	float x=getport(2), y=getport(4); /* = gain,powa :: shorthand for gain calc */
	float gain = x*x;
	float powa = .9*gain + (1-.9*gain)*y; /* ramp up powa with gain */
	float squash = .01+.21*getport(9)*(1 - powa*gain);

	bias = .12*powa;

	/* rough correction for loudness increase with gain */
	/* TODO: check 1/gain * 1/powa as alternative */
	float makeup = (.086-.06*y)/(11.6+exp((12.1-5*y)*(.81-.08*y-x)))+0.00032+.0026*y;
	makeup = 0 ? 1 : .0005/makeup;

	float lowcut = .1 + 372*getport (10); 
	DSP::RBJ::HP (lowcut/fs, .7, hp);

	float bright = .59*getport(3)*(1-x*.81);
	DSP::RBJ::LP ((2000*pow(10,bright))/(over.Ratio*fs), .7, lp);

	/* negated to correct for amplitude inversion */
	gain = tsgain[model] * -pow (200, gain);
	powa = pow (125, powa);

	sample_t * s = ports[11];
	sample_t * d = ports[12]; 

	while (frames)
	{
		if (remain == 0)
		{
			remain = compress.blocksize;
			compress.start_block (squash);
		}

		uint n = min (frames, remain);
		for (uint i = 0; i < n; ++i)
		{
			double a = s[i];

			a = hp.process (a);
			a *= gain * compress.get();
			a = tonestack.process (a + normal);
			a += biaslp.process (bias*compress.power.current - .00002);

			a = over.upsample (a);
			a = preamp (a);
			a = dcblock.process (a);
			a = lp.process (a);
			a = poweramp (powa * a);
			a = over.downsample (a);

			for (int o = 1; o < over.Ratio; ++o)
			{
				sample_t a = over.uppad (o);
				a = preamp (a);
				a = dcblock.process (a);
				a = lp.process (a);
				a = poweramp (powa * a);
				over.downstore (a);
			}

			compress.store (a);

			a *= makeup;
			yield (d, i, a, adding_gain);
		}

		s += n; d += n;
		frames -= n;
		remain -= n;
	}
}

/* //////////////////////////////////////////////////////////////////////// */

PortInfo
AmpVTS::port_info [] = 
{
	{	"ratio", CTRL_IN, {DEFAULT_MAX | INTEGER, 0, 2}, "{0:'2x',1:'4x',2:'8x'}"},
	{	"model", CTRL_IN | GROUP, {DEFAULT_0 | INTEGER, 0, 8}, DSP::ToneStack::presetdict},

	/* 2 */
	{ "gain", CTRL_IN | GROUP, {DEFAULT_0, 0, 1} }, 
	{ "bright", CTRL_IN, {DEFAULT_MID, 0, 1} }, 
	{ "power", CTRL_IN, {DEFAULT_MID, 0, 1} },
	
	/* 5 */
	{ "bass", CTRL_IN | GROUP, {DEFAULT_LOW, 0, 1} }, 
	{ "mid", CTRL_IN, {DEFAULT_1, 0, 1} }, 
	{ "treble", CTRL_IN, {DEFAULT_HIGH, 0, 1} }, 

	/* 8 */
	{ "attack", CTRL_IN | GROUP, {DEFAULT_LOW, 0, 1} },
	{ "squash", CTRL_IN, {DEFAULT_MID, 0, 1} },

	/* 10 */
	{ "low cut", CTRL_IN | GROUP, {DEFAULT_HIGH, 0, 1} },

	{ "in", INPUT | AUDIO }, 
	{	"out", OUTPUT | AUDIO }, 
};

template <> void
Descriptor<AmpVTS>::setup()
{
	Label = "AmpVTS";

	Name = CAPS "AmpVTS - Idealised guitar amplification";
	Maker = "Tim Goetze <tim@quitte.de>, David Yeh <dtyeh@ccrma.stanford.edu>";
	Copyright = "2002-13";

	/* fill port info and vtable */
	autogen();
}

