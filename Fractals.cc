/*
	Fractals.cc
	
	Copyright 2002-12 Tim Goetze <tim@quitte.de>
	
	http://quitte.de/dsp/

	Lorenz and Roessler attractors made audible.

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

#include <stdlib.h>

#include "basics.h"

#include "Fractals.h"
#include "Descriptor.h"

void
Lorenz::init()
{
	lorenz.init (h = .001, 0.1 * frandom());
	gain = 0;
}

template <yield_func_t F>
void
Lorenz::cycle (uint frames)
{
	lorenz.set_rate (2.268e-05 *fs * getport(0));

	double g = (gain == *ports[4]) ? 
		1 : pow (getport(4) / gain, 1. / (double) frames);

	sample_t * d = ports[5];

	sample_t x, sx = getport(1), sy = getport(2), sz = getport(3);
	
	for (uint i = 0; i < frames; ++i)
	{
		lorenz.step();

		x = sx * lorenz.get_x() + sy * lorenz.get_y() + sz * lorenz.get_z();

		x = hp.process (x + normal);
		F (d, i, gain * x, adding_gain);
		gain *= g;
	}

	gain = getport(4);
}

/* //////////////////////////////////////////////////////////////////////// */

PortInfo
Lorenz::port_info [] =
{
	{
		"rate", INPUT | CONTROL,
		{DEFAULT_LOW, 0, 1}
	}, {
		"x", INPUT | CONTROL | GROUP,
		{DEFAULT_1, 0, 1}
	}, {
		"y", INPUT | CONTROL,
		{DEFAULT_0, 0, 1}
	}, {
		"z", INPUT | CONTROL,
		{DEFAULT_0, 0, 1}
	}, {
		"volume", INPUT | CONTROL | GROUP,
		{DEFAULT_MID, MIN_GAIN, 1}
	}, {
		"out", OUTPUT | AUDIO
	}
};

template <> void
Descriptor<Lorenz>::setup()
{
	Label = "Lorenz";

	Name = CAPS "Lorenz - Audio stream from a Lorenz attractor";
	Maker = "Tim Goetze <tim@quitte.de>";
	Copyright = "2004-12";

	/* fill port info and vtable */
	autogen();
}

/* //////////////////////////////////////////////////////////////////////// */

void
Roessler::init()
{
	roessler.init (h = .001, frandom());
	gain = 0;
}

template <yield_func_t F>
void
Roessler::cycle (uint frames)
{
	roessler.set_rate (2.268e-05 *fs * getport(0));

	double g = (gain == getport(4)) ? 
		1 : pow (getport(4) / gain, 1. / (double) frames);

	sample_t * d = ports[5];

	sample_t x,
			sx = .043 * getport(1), 
			sy = .051 * getport(2), 
			sz = .018 * getport(3);
	
	for (uint i = 0; i < frames; ++i)
	{
		roessler.get();

		x = 
				sx * (roessler.get_x() - .515) + 
				sy * (roessler.get_y() + 2.577) + 
				sz * (roessler.get_z() - 2.578);

		x = hp.process (x + normal);
		F (d, i, gain * x, adding_gain);
		gain *= g;
	}

	gain = getport(4);
}

/* //////////////////////////////////////////////////////////////////////// */

PortInfo 
Roessler::port_info [] =
{
	{
		"rate", INPUT | CONTROL,
		{DEFAULT_LOW, 0, 1}
	}, {
		"x", INPUT | CONTROL | GROUP,
		{DEFAULT_1, 0, 1}
	}, {
		"y", INPUT | CONTROL,
		{DEFAULT_MID, 0, 1}
	}, {
		"z", INPUT | CONTROL,
		{DEFAULT_0, 0, 1}
	}, {
		"volume", INPUT | CONTROL | GROUP,
		{DEFAULT_MID, MIN_GAIN, 1}
	}, {
		"out", OUTPUT | AUDIO
	}
};

template <> void
Descriptor<Roessler>::setup()
{
	Label = "Roessler";

	Name = CAPS "Roessler - Audio stream from a Roessler attractor";
	Maker = "Tim Goetze <tim@quitte.de>";
	Copyright = "2004-12";

	/* fill port info and vtable */
	autogen();
}

