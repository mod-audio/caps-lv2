/*
	White.cc
	
	Copyright 2004-13 Tim Goetze <tim@quitte.de>
	
	http://quitte.de/dsp/

	Approximating white pseudonoise generation.

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

#include "White.h"
#include "Descriptor.h"

void
White::activate()
{ 
	gain = getport(0); 
	white.init (frandom());
	cream.init (frandom());
	hp.set_f (.48);
}

void
White::cycle (uint frames)
{
	double g = (gain == *ports[0]) ? 
		1 : pow (getport(0) / gain, 1. / (double) frames);

	sample_t * d = ports[1];

	for (uint i = 0; i < frames; ++i)
	{
		sample_t x = .4*white.get();
		x += hp.process(cream.get());
		d[i] = gain*x;
		gain *= g;
	}

	gain = getport(0);
}

/* //////////////////////////////////////////////////////////////////////// */

PortInfo
White::port_info [] =
{
	{ "volume", INPUT | CONTROL, {DEFAULT_HIGH, MIN_GAIN, 1} }, 
	{	"out", OUTPUT | AUDIO}
};

template <> void
Descriptor<White>::setup()
{
	Label = "White";
	Name = CAPS "White - Noise generator";
	autogen();
}

