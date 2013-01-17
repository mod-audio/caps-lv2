/*
	Phaser.cc
	
	Copyright 2002-12 Tim Goetze <tim@quitte.de>
	
	http://quitte.de/dsp/

	Assortment of phaser units.

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

#include "Phaser.h"
#include "Descriptor.h"

#define FBSCALE .9 /* feedback scale from 0..1 parameter range */

template <yield_func_t F>
void
PhaserII::cycle (uint frames)
{
	sample_t * s = ports[0];

	lorenz.set_rate (2.268e-05*fs * getport(1) * .02);

	sample_t depth = getport(2);
	sample_t spread = 1 + .5*M_PI*getport(3);
	sample_t fb = FBSCALE * getport(4);

	sample_t * dst = ports[5];

	while (frames)
	{
		if (remain == 0) remain = blocksize;

		int n = min (remain, frames);

		double d = lfo_lp.process(.3*lorenz.get());
		d = delay.bottom + d*delay.range;

		for (int j = 0; j < Notches; ++j)
		{
			ap[j].set (d);
			d *= spread;
		}
		
		for (int i = 0; i < n; ++i)
		{
			sample_t x = .5 * s[i];
			sample_t y = x + y0 * fb + normal;

			for (int j = 0; j < Notches; ++j)
				y = ap[j].process (y);
			
			y0 = y;

			F (dst, i, x + y * depth, adding_gain);
		}

		s += n;
		dst += n;
		frames -= n;
		remain -= n;
	}
}

/* //////////////////////////////////////////////////////////////////////// */

PortInfo
PhaserII::port_info [] =
{
	{ "in", INPUT | AUDIO }, 
	{ "rate", INPUT | CONTROL, {DEFAULT_LOW, 0, 1} }, 
	{ "depth", INPUT | CONTROL, {DEFAULT_1, 0, 1} }, 
	{ "spread", INPUT | CONTROL | GROUP, {DEFAULT_MID, 0, 1} }, 
	{ "resonance", INPUT | CONTROL, {DEFAULT_0, 0, 1} }, 
	{ "out", OUTPUT | AUDIO }
};

template <> void
Descriptor<PhaserII>::setup()
{
	Label = "PhaserII";

	Name = CAPS "PhaserII - Mono phaser modulated by a Lorenz attractor";
	Maker = "Tim Goetze <tim@quitte.de>";
	Copyright = "2002-12";

	/* fill port info and vtable */
	autogen();
}

/* //////////////////////////////////////////////////////////////////////// */

template <yield_func_t F, bool StereoIn>
void
StereoPhaserII::cycle (uint frames)
{
	sample_t * sl = ports[0];
	int port = 0;
	if (StereoIn) ++port;
	sample_t * sr = ports[port];
		
	lorenz.set_rate (2.268e-05*fs * getport(++port) * .02);

	sample_t depth = getport(++port);
	sample_t spread = 1 + .5*M_PI*getport(++port);
	sample_t fb = FBSCALE * getport(++port);

	sample_t * dstl = ports[++port];
	sample_t * dstr = ports[++port];

	while (frames)
	{
		if (remain == 0) remain = blocksize;

		uint n = min (remain, frames);

		sample_t d = .3 * lorenz.get();
		d = lfo_lp.process(d);
		sample_t dl = delay.bottom + delay.range * d;
		sample_t dr = delay.bottom - delay.range * d;

		for (uint j = 0; j < Notches; ++j)
		{
			apl[j].set (dl);
			apr[j].set (dr);
			dl *= spread;
			dr *= spread;
		}
		
		for (uint i = 0; i < n; ++i)
		{
			sample_t xl = .5 * sl[i];
			sample_t xr = .5 * sr[i];
			sample_t yl = xl + y0l * fb + normal;
			sample_t yr = xr + y0r * fb + normal;

			for (int j = 0; j < Notches; ++j)
				yl = apl[j].process (yl),
				yr = apr[j].process (yr);
			
			y0l = yl;
			y0r = yr;

			F (dstl, i, xl + yl * depth, adding_gain);
			F (dstr, i, xr + yr * depth, adding_gain);
		}

		sl += n;
		sr += n;
		dstl += n;
		dstr += n;
		frames -= n;
		remain -= n;
	}
}

/* //////////////////////////////////////////////////////////////////////// */

PortInfo
StereoPhaserII::port_info [] =
{
	{ "in", INPUT | AUDIO }, 
	{ "rate", INPUT | CONTROL, {DEFAULT_LOW, 0, 1} }, 
	{ "depth", INPUT | CONTROL, {DEFAULT_1, 0, 1} }, 
	{ "spread", INPUT | CONTROL | GROUP, {DEFAULT_MID, 0, 1} }, 
	{ "resonance", INPUT | CONTROL, {DEFAULT_0, 0, 1} }, 
	{ "out.l", OUTPUT | AUDIO }, 
	{ "out.r", OUTPUT | AUDIO }
};

template <> void
Descriptor<StereoPhaserII>::setup()
{
	Label = "StereoPhaserII";

	Name = CAPS "StereoPhaserII - Two-channel phaser modulated by a Lorenz attractor";
	Maker = "Tim Goetze <tim@quitte.de>";
	Copyright = "2002-12";

	/* fill port info and vtable */
	autogen();
}

/* //////////////////////////////////////////////////////////////////////// */

PortInfo
StereoPhaserII2x2::port_info [] =
{
	{ "in.l", INPUT | AUDIO }, 
	{ "in.r", INPUT | AUDIO }, 
	{ "rate", INPUT | CONTROL, {DEFAULT_LOW, 0, 1} }, 
	{ "depth", INPUT | CONTROL, {DEFAULT_1, 0, 1} }, 
	{ "spread", INPUT | CONTROL | GROUP, {DEFAULT_MID, 0, 1} }, 
	{ "resonance", INPUT | CONTROL, {DEFAULT_0, 0, 1} }, 
	{ "out.l", OUTPUT | AUDIO }, 
	{ "out.r", OUTPUT | AUDIO }
};

template <> void
Descriptor<StereoPhaserII2x2>::setup()
{
	Label = "StereoPhaserII2x2";

	Name = CAPS "StereoPhaserII2x2 - Two-channel phaser modulated by a Lorenz attractor";
	Maker = "Tim Goetze <tim@quitte.de>";
	Copyright = "2002-12";

	/* fill port info and vtable */
	autogen();
}

