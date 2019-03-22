/*
	Phaser.cc
	
	Copyright 2002-13 Tim Goetze <tim@quitte.de>
	
	http://quitte.de/dsp/

	Phaser effect

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

void
PhaserII::cycle (uint frames)
{
	sample_t * s = ports[5];
	sample_t * dst = ports[6];

	rate = getport(0);
	lfo.sine.set_f (max(.001,rate*blocksize), fs, lfo.sine.get_phase());
	lfo.lp.set_f (5*(1+rate)*over_fs);
	lfo.roessler.set_rate(.05*rate);

	float mode = getport(1);

	sample_t depth = getport(2);
	sample_t spread = 1 + .5*M_PI*getport(3);
	sample_t fb = FBSCALE * getport(4);

	while (frames)
	{
		if (remain == 0) remain = blocksize;

		int n = min (remain, frames);

		float d;
		if (mode < .5)
			d = fabs (lfo.sine.get()), 
			d *= d;
		else
			d = min (.99,fabs (lfo.lp.process(4.3*lfo.roessler.get())));
		d = delay.bottom + d*delay.range;

		for (int j = 0; j < Notches; ++j)
		{
			ap[j].set(d);
			d *= spread;
		}
		
		for (int i = 0; i < n; ++i)
		{
			sample_t x = .5 * s[i];
			sample_t y = x + y0 * fb + normal;

			for (int j = 0; j < Notches; ++j)
				y = ap[j].process (y);
			
			y0 = y;

			dst[i] = x + y*depth;
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
	{ "rate", CTRL_IN, {DEFAULT_LOW, 0, 1} }, 
	{ "lfo", CTRL_IN, {DEFAULT_0 | INTEGER, 0, 1}, "{0:'sine',1:'fractal'}"}, 

	{ "depth", CTRL_IN | GROUP, {DEFAULT_HIGH, 0, 1} }, 
	{ "spread", CTRL_IN, {DEFAULT_HIGH, 0, 1} }, 
	{ "resonance", CTRL_IN, {DEFAULT_LOW, 0, 1} }, 

	{ "in", INPUT | AUDIO }, 
	{ "out", OUTPUT | AUDIO }
};

template <> void
Descriptor<PhaserII>::setup()
{
	Label = "PhaserII";
	Name = CAPS "PhaserII - Mono phaser";
	autogen();
}

