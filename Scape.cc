/*
	Scape.cc
	
	Copyright 2004-13 Tim Goetze <tim@quitte.de>
	
	http://quitte.de/dsp/

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

#include "Scape.h"
#include "Descriptor.h"

static double 
dividers [] = {
	1 /* not used, 0 sentinel */, 
	1, 0.5, 0.66666666666666666667,	0.75
};

static float 
frandom2()
{
	float f = frandom();
	return f*f*f;
}

void 
Scape::init()
{
	delay.init ((int) (2.01 * fs)); /* two seconds = 30 bpm + */
	for (int i = 0; i < 2; ++i)
	{
		lfo[i].lorenz.init(),
		lfo[i].lorenz.set_rate (.00000001*fs);
		lfo[i].lp.set_f (3*over_fs);
	}
}

void
Scape::activate()
{
	time = 0;
	fb = 0;

	for (int i = 0; i < 4; ++i)
		svf[i].reset(),
		svf[i].set_out (1), /* band pass */
		hipass[i].set_f (250*over_fs);
	svf[3].set_out (0); /* low pass */

	delay.reset();
	period = 0;
}

inline double
pick_f (float range, float tune)
{
	static double over_12 = 1./12;
	int n = 48 + (int) (4*12*range*frandom());
	return tune * pow (2, (n - 69) * over_12);
}


void
Scape::cycle (uint frames)
{
	/* delay times */
	double t1 = fs * 60. / getport(0);
	int div = (int) getport(1);
	double t2 = t1 * dividers[div];

	fb = .94*getport(2);

	double dry = getport(3);
	dry = dry * dry;
	double blend = getport(4);
	float tune = getport(5);

	sample_t * s = ports[6];
	sample_t * dl = ports[7];
	sample_t * dr = ports[8];

	DSP::FPTruncateMode truncate;

	while (frames)
	{
		/* retune filters */
		if (period <= 1)
		{
			period = t2 * .5;
			float f;

			f = frandom2();
			svf[0].set_f_Q (pick_f(.3,tune)*over_fs, .3);
			svf[3].set_f_Q (pick_f(.5,tune)*over_fs, .6); /* LP */
			
			f = frandom2();
			svf[1].set_f_Q (pick_f(.8,tune)*over_fs, .9*f);
			svf[2].set_f_Q (pick_f(.9,tune)*over_fs, .5*f);
		}
		
		uint n = min((uint) period, frames);
		for (uint i=0; i < n; ++i)
		{
			sample_t x = s[i] + normal;

			sample_t x1 = delay.get_linear (t1);
			sample_t x2 = delay.get_linear (t2);

			delay.put (x + fb*x1);
			x = dry*x + .2*svf[0].process (x) + .6*svf[3].process(x);

			x1 = svf[1].process (x1 - normal);
			x2 = svf[2].process (x2 - normal);

			x1 = hipass[1].process (x1);
			x2 = hipass[2].process (x2);

			sample_t x1l, x1r, x2l, x2r;
			x1l = fabs (lfo[0].lp.process(lfo[0].lorenz.get()));
			x1r = 1 - x1l;
			x2r = fabs (lfo[1].lp.process(lfo[1].lorenz.get()));
			x2l = 1 - x2r;

			dl[i] = x + blend*(x1*x1l + x2*x2l);
			dr[i] = x + blend*(x1*x1r + x2*x2r);
		}

		frames -= n;
		period -= n;
		s += n;
		dl += n;
		dr += n;
	}
}

/* //////////////////////////////////////////////////////////////////////// */

PortInfo
Scape::port_info [] =
{
	{ "bpm", CTRL_IN, {DEFAULT_MID, 30, 164} }, 
	{	"divider", CTRL_IN, {INTEGER | DEFAULT_MID, 2, 4}, 
		"{2:'eighths',3:'triplets',4:'sixteenths'}" }, 
	{ "feedback", CTRL_IN | GROUP, {DEFAULT_HIGH, 0, 1} }, 
	{ "dry", CTRL_IN | GROUP, {DEFAULT_MID, 0, 1} }, 
	{ "blend", CTRL_IN, {DEFAULT_1, 0, 1} }, 
	{ "tune (Hz)", CTRL_IN | GROUP, {DEFAULT_440, 415, 467} }, 

	{ "in", AUDIO_IN }, 
	{ "out.l", AUDIO_OUT }, 
	{ "out.r", AUDIO_OUT }
};

template <> void
Descriptor<Scape>::setup()
{
	Label = "Scape";

	Name = CAPS "Scape - Stereo delay with chromatic resonances";
	Maker = "Tim Goetze <tim@quitte.de>";
	Copyright = "2004-12";

	/* fill port info and vtable */
	autogen();
}

