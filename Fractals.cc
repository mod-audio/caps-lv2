/*
	Fractals.cc
	
	Copyright 2002-13 Tim Goetze <tim@quitte.de>
	
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
Fractal::init()
{
	lorenz.init (h = .001, 0.1*frandom());
	roessler.init (h = .001, frandom());
	gain = 1;
}

void
Fractal::activate()
{ 
	gain = getport(6); 
	hp.reset();
}

void
Fractal::cycle (uint frames)
{
	float mode = getport(1);
	if (mode < .5) subcycle<0> (frames);
	else subcycle<1> (frames);
}

template <int Mode>
void
Fractal::subcycle (uint frames)
{
	float r = 2.268e-05*fs*getport(0);
	lorenz.set_rate(r);
	roessler.set_rate(r);

	float f = getport(5);
	if (f) hp.set_f (f*200*over_fs);
	else hp.identity();

	float _gain = getport(6);
	float g = _gain*_gain;
	g = (gain == g) ? 1 : pow (g/gain, 1./(double)frames);

	sample_t * d = ports[7];

	sample_t sx=getport(2), sy=getport(3), sz=getport(4);
	for (uint i = 0; i < frames; ++i)
	{
		sample_t x;

		if (Mode == 0)
		{
			lorenz.step();
			x = sx*lorenz.get_x() + sy*lorenz.get_y() + sz*lorenz.get_z();
		}
		else /* Mode == 1 */
		{
			roessler.step();
			x = sx*roessler.get_x() + sy*roessler.get_y() + sz*roessler.get_z();
		}

		x = hp.process (x + normal);
		d[i] = gain*x;
		gain *= g;
	}

	gain = _gain;
}

/* //////////////////////////////////////////////////////////////////////// */

PortInfo
Fractal::port_info [] =
{
	{ "rate", CTRL_IN, {DEFAULT_LOW, 0, 1} }, 
	{ "mode", CTRL_IN, {DEFAULT_0 | INTEGER, 0, 1}, 
		"{0:'lorenz',1:'roessler'}" }, 
	{ "x", CTRL_IN | GROUP, {DEFAULT_1, 0, 1} }, 
	{ "y", CTRL_IN, {DEFAULT_0, 0, 1} }, 
	{ "z", CTRL_IN, {DEFAULT_MID, 0, 1} }, 
	{ "hp", CTRL_IN | GROUP, {DEFAULT_MID, 0, 1} }, 
	{ "volume", CTRL_IN, {DEFAULT_MID, MIN_GAIN, 1} }, 

	{ "out", OUTPUT | AUDIO }
};

template <> void
Descriptor<Fractal>::setup()
{
	Label = "Fractal";
	Name = CAPS "Fractal - Audio stream from deterministic chaos";
	autogen();
}


