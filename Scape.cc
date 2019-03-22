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

	delay.reset();
	period = 0;
}

inline double
pick_f (float range, float tune)
{
	static float over_12 = 1./12;
	int n = 48 + (int) (4*12*range*frandom());
	return tune*pow(2,(n-69)*over_12);
}


void
Scape::cycle (uint frames)
{
	/* delay times */
	double t1 = 60*fs/getport(0);
	int div = (int) getport(1);
	double t2 = t1*dividers[div];


	float q = .99*getport(2);
	float blend = pow(getport(3),.2);
	float dry = sqrt(1-blend*blend);
	fb = .94*getport(4);

	float tune = 440;

	sample_t * s = ports[5];
	sample_t * dl = ports[6];
	sample_t * dr = ports[7];

	DSP::FPTruncateMode truncate;

	while (frames)
	{
		/* retune filters */
		if (period <= 1)
		{
			period = .5*t2;
			float f;

			f = frandom2();
			svf[0].set_f_Q (pick_f(.3,tune)*over_fs, q);
			svf[3].set_f_Q (pick_f(.5,tune)*over_fs, q); /* LP */
			
			f = frandom2();
			svf[1].set_f_Q (pick_f(.8,tune)*over_fs, f*q);
			svf[2].set_f_Q (pick_f(.9,tune)*over_fs, .5*f*q);
		}
		
		uint n = min((uint) period, frames);
		for (uint i=0; i < n; ++i)
		{
			sample_t x = s[i] + normal;

			sample_t x1 = delay.get_linear (t1);
			sample_t x2 = delay.get_linear (t2);

			delay.put (x + fb*x1);
			x = dry*x + .1*svf[0].process (x) + .2*svf[3].process(x);

			x1 = svf[1].process(x1 - normal);
			x2 = svf[2].process(x2 - normal);

			x1 = hipass[1].process(x1);
			x2 = hipass[2].process(x2);

			sample_t x1l, x1r, x2l, x2r;
			x1l = .7*fabs(lfo[0].get());
			x1r = 1 - x1l;
			x2r = .7*fabs(lfo[1].get());
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
	{ "bpm", CTRL_IN, {DEFAULT_MID, 30, 182} }, 
	{	"div", CTRL_IN, {INTEGER | DEFAULT_MIN, 2, 4}, 
		"{2:'♪♪',3:'♪♪♪',4:'♬♬'}" }, 

	{ "Q", CTRL_IN | GROUP, {DEFAULT_HIGH, 0, 1} }, 
	{ "blend", CTRL_IN, {DEFAULT_MID, 0, 1} }, 
	{ "feedback", CTRL_IN, {DEFAULT_LOW, 0, 1} }, 

	{ "in", AUDIO_IN }, 
	{ "out.l", AUDIO_OUT }, 
	{ "out.r", AUDIO_OUT }
};

template <> void
Descriptor<Scape>::setup()
{
	Label = "Scape";
	Name = CAPS "Scape - Stereo delay with chromatic resonances";
	autogen();
}

/* //////////////////////////////////////////////////////////////////////// */

void 
DDDelay::init()
{
	float l = 2*fs; /* one beat at 30 bpm */
	for(int i=0; i<4; ++i)
	{
		step[i].delay.init((int) (l + .5)); 
		step[i].lp.set(.001);
	}
}

void
DDDelay::activate()
{
	for(int i=0; i<4; ++i)
	{
		step[i].delay.reset();
		step[i].lp.reset();
	}
}

void
DDDelay::cycle (uint frames)
{
	/* delay times */
	int div = (int) getport(1);
	int t = -1 + (int) (60*fs/getport(0));

	sample_t * s = ports[2];
	sample_t * d = ports[3];

	sample_t g[4] = {.4,.7,.8,.7};
	for(uint i=0; i<frames; ++i)
	{
		sample_t x=s[i], y=x;

		for(int j=0; j<div; ++j)
		{
			step[j].delay.put(y);
			y = step[j].delay[t];
			x += g[j]*y;
		}

		d[i] = x;
	}
}

/* //////////////////////////////////////////////////////////////////////// */

PortInfo
DDDelay::port_info [] =
{
	{ "bpm", CTRL_IN, {DEFAULT_MID, 30, 182} }, 
	{	"div", CTRL_IN, {INTEGER | DEFAULT_HIGH, 2, 4}, 
		"{2:'♪♪',3:'♪♪♪',4:'♬♬'}" }, 

	{ "in", AUDIO_IN }, 
	{ "out", AUDIO_OUT }, 
};

template <> void
Descriptor<DDDelay>::setup()
{
	Label = "DDDelay";
	Name = CAPS "DDDelay - Delay with fixed repetition count";
	autogen();
}

