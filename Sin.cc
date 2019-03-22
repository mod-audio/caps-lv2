/*
	Sin.cc
	
	Copyright 2002-13 Tim Goetze <tim@quitte.de>
	
	http://quitte.de/dsp/

	simple sin() generator.

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

#include "Sin.h"
#include "Descriptor.h"

void
Sin::activate()
{ 
	gain = getport(1); 
	f = getport(0);
	sin.set_f (f, fs, 0);
}

void
Sin::cycle (uint frames)
{
	sample_t * d = ports[2];
	double g = getport(1);
	g = (g == gain ? 1 : pow (g/gain, 1./(double) frames));

	float ff = getport(0);
	if (ff == f)
	{
		for (uint i = 0; i < frames; ++i)
		{
			d[i] = gain*sin.get();
			gain *= g;
		}
	}
	else /* crossfade old and new frequency */
	{
		sample_t g0=1, g1=0, dg=1./frames;
		DSP::Sine sin0 = sin;
		sin.set_f (f = ff, fs, sin.get_phase());
		for (uint i = 0; i < frames; ++i)
		{
			sample_t x = g0*sin0.get() + g1*sin.get();
			g0 -= dg, g1 += dg;
			d[i] = gain*x;
			gain *= g;
		}
	}

	gain = getport(1);
}

/* //////////////////////////////////////////////////////////////////////// */

PortInfo
Sin::port_info [] =
{
	{ "f (Hz)", CTRL_IN, {LOG | DEFAULT_440, 0.0001, 20000} }, 
	{ "volume", CTRL_IN, {DEFAULT_MID, MIN_GAIN, 1}	}, 

	{ "out", OUTPUT | AUDIO, {0} }
};

template <> void
Descriptor<Sin>::setup()
{
	Label = "Sin";
	Name = CAPS "Sin - Sine wave generator";
	autogen();
}

