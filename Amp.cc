/*
	Amp.cc
	
	Copyright 2003-18 Tim Goetze <tim@quitte.de>
	
	http://quitte.de/dsp/

	Idealised guitar amplification.

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
	
	dc2.set_f (25*over_fs); 
	DSP::RBJ::LP (1*over_fs, .7, biaslp);
	/* compress is initialised in activate() */
}

void
AmpVTS::activate()
{
	hp1.reset();
	lp.reset();

	remain = 0;
	compress.init(fs,16);
	compress.set_threshold(0);
	compress.set_release(.0);

	/* dc1 is reset in setratio() */
	dc2.reset();

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
	dc1.set_f (72./(ratio*fs)); 
	dc1.reset();

	over2.reset();
	over4.reset();
	over8.reset();
}

void
AmpVTS::cycle (uint frames)
{
	int r = 2 << (int) getport(0);
	setratio(r);
	if (r == 8) subcycle (frames, over8);
	else if (r == 4) subcycle (frames, over4);
	else subcycle (frames, over2);
}

/* saturating function selection */
#define preamp DSP::Polynomial::one5
#define poweramp DSP::Polynomial::atan

/* rough correction for tonestack model gain differences */
static float tsgain[] = {.639, 1.290, .534, 1.008, .542, .936, .605, 1.146, .211};

template <class Over>
void
AmpVTS::subcycle (uint frames, Over & over)
{
	int m = getport(4);
	if (m != model) tonestack.setmodel (model = m);
	tonestack.updatecoefs (getport(5),getport(6),getport(7));

	float x=getport(1), y=getport(3); /* = gain,powa :: shorthand for gain calc */
	float bright = getport(2)*(1-x*.5);
	DSP::RBJ::LP ((500+6500*bright*bright)/(over.Ratio*fs), .7, lp); 

	float gain = x*x;
	float powa = .2*gain + (1-.2*gain)*y; /* ramp up powa with gain */

	compress.set_attack (.6*(1-.5*x)*getport(8));
	float squash = .0 + .8*getport(9)*(1 - .2*powa*gain);

	float bias = .62*powa;

	/* roughly correcting for loudness increase TODO: revise */
	float makeup = (.086-.06*y)/(11.6+exp((12.1-5*y)*(.81-.08*y-x)))+0.00032+.0026*y;
	makeup = 0 ? 1 : .0006/makeup; /* debug switch for makeup */

	float lowcut = .1 + 392*getport(10); 
	hp1.set_f (1.5*lowcut*over_fs); 

	gain = pow (200, x) * -tsgain[model];
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
			sample_t a = s[i];
			sample_t b = biaslp.process (bias*compress.power.current - .00002);

			a = hp1.process(a);
			a *= gain*compress.get();
			a = tonestack.process(a + normal);
			a += .5*b;

			a = over.upsample(a);
			a = preamp(a);
			a = dc1.process(a);
			a = lp.process(a);
			a = poweramp(powa*a - b);
			a = over.downsample(a);

			for (int o=1; o < over.Ratio; ++o)
			{
				sample_t a = over.uppad(o);
				a = preamp(a);
				a = dc1.process(a);
				a = lp.process(a);
				a = poweramp(powa*a - b);
				over.downstore(a);
			}

			a = dc2.process(a+normal);
			compress.store(a);

			a *= makeup;

			d[i] = a;
		}

		s+=n;d+=n;
		frames-=n;
		remain-=n;
	}
}

/* //////////////////////////////////////////////////////////////////////// */

PortInfo
AmpVTS::port_info [] = 
{
	{	"over", CTRL_IN, {DEFAULT_1 | INTEGER, 0, 2}, "{0:'2x',1:'4x',2:'8x'}"},

	/* 1 */
	{ "gain", CTRL_IN | GROUP, {DEFAULT_LOW, 0, 1} }, 
	{ "bright", CTRL_IN, {DEFAULT_HIGH, 0, 1} }, 
	{ "power", CTRL_IN, {DEFAULT_MID, 0, 1} },
	
	/* 4 */
	{	"tonestack", CTRL_IN | GROUP, {DEFAULT_0 | INTEGER, 0, 8}, DSP::ToneStack::presetdict},

	{ "bass", CTRL_IN | GROUP, {DEFAULT_LOW, 0, 1} }, 
	{ "mid", CTRL_IN, {DEFAULT_HIGH, 0, 1} }, 
	{ "treble", CTRL_IN, {DEFAULT_1, 0, 1} }, 

	/* 8 */
	{ "attack", CTRL_IN | GROUP, {DEFAULT_LOW, 0, 1} },
	{ "squash", CTRL_IN, {DEFAULT_HIGH, 0, 1} },

	/* 10 */
	{ "lowcut", CTRL_IN | GROUP, {DEFAULT_HIGH, 0, 1} },

	{ "in", INPUT | AUDIO }, 
	{	"out", OUTPUT | AUDIO }, 
};

template <> void
Descriptor<AmpVTS>::setup()
{
	Label = "AmpVTS";
	Name = CAPS "AmpVTS - Idealised guitar amplification";
	autogen();

	Maker = "Tim Goetze <tim@quitte.de>, David Yeh <dtyeh@ccrma.stanford.edu>";
}

