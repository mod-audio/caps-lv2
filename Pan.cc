/*
	Pan.cc
	
	Copyright 2002-11 Tim Goetze <tim@quitte.de>
	
	http://quitte.de/dsp/

	Stereo image synthesis (inspired by the Orban 245F unit)
	and manipulation.

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
#include "dsp/RBJ.h"

#include "Pan.h"
#include "Descriptor.h"

void
Wider::init()
{
	pan = FLT_MAX; /* invalid setting to make sure gain is updated */
}

void
Wider::activate()
{ 
	set_pan (getport(1));
	float fc[3] = { 150, 900, 5000 };
	for (int i = 0; i < 3; ++i)
		DSP::RBJ::AllPass (fc[i]*over_fs, .707, ap[i]);
}

inline void
Wider::set_pan (sample_t p)
{
	if (pan == p) 
		return;

	pan = p;
	
	double phi = (pan + 1)*M_PI*.25;

	gain_l = cos (phi);
	gain_r = sin (phi);
}

template <yield_func_t F>
void
Wider::cycle (uint frames)
{
	sample_t * src = ports[0];

	sample_t p = getport(1);
	if (p != pan) set_pan (p);

	sample_t width = getport(2); 

	/* need to limit width as pan increases in order to prevent
	 * excessive phase cancellation */
	width *= 1 - fabs (p);
	width *= width;

	sample_t * dl = ports[3];
	sample_t * dr = ports[4];

	for (uint i = 0; i < frames; ++i)
	{
		sample_t m = .707*src[i] + normal;
		sample_t s = m;
		s = ap[0].process (s);
		s = ap[1].process (s);
		s = ap[2].process (s);

		s *= width;

		sample_t l = m - s;
		sample_t r = m + s;
		F (dl, i, gain_l * l, adding_gain);
		F (dr, i, gain_r * r, adding_gain);
	}
}

/* //////////////////////////////////////////////////////////////////////// */

PortInfo
Wider::port_info [] =
{
	{ "in", INPUT | AUDIO }, 
	{ "pan", INPUT | CONTROL, {DEFAULT_0, -1, 1} }, 
	{ "width", INPUT | CONTROL, {DEFAULT_1, 0, 1} }, 
	{	"out.l", OUTPUT | AUDIO	}, 
	{ "out.r", OUTPUT | AUDIO }
};

template <> void
Descriptor<Wider>::setup()
{
	Label = "Wider";

	Name = CAPS "Wider - Stereo image synthesis";
	Maker = "Tim Goetze <tim@quitte.de>";
	Copyright = "2011-13";

	/* fill port info and vtable */
	autogen();
}

/* //////////////////////////////////////////////////////////////////////// */

template <yield_func_t F>
void
Narrower::cycle (uint frames)
{
	sample_t * sl = ports[0];
	sample_t * sr = ports[1];

	float mode = getport(2);
	strength = getport(3);

	sample_t * dl = ports[4];
	sample_t * dr = ports[5];

	if (mode)
	{
		for (uint i = 0; i < frames; ++i)
		{
			sample_t xl = sl[i];
			sample_t xr = sr[i];
			
			sample_t m = xl + xr, s = xl - xr;

			m += strength * s;
			s *= (1 - strength);

			xl = .5 * (m + s);
			xr = .5 * (m - s);

			F (dl, i, xl, adding_gain);
			F (dr, i, xr, adding_gain);
		}
	}
	else 
	{
		sample_t dry = 1 - strength, wet = strength;

		for (uint i = 0; i < frames; ++i)
		{
			sample_t xl = sl[i];
			sample_t xr = sr[i];
			
			sample_t m = wet * (xl + xr) * .5;
			
			xl *= dry;
			xr *= dry;
			
			xl += m;
			xr += m;

			F (dl, i, xl, adding_gain);
			F (dr, i, xr, adding_gain);
		}
	}
}

/* //////////////////////////////////////////////////////////////////////// */

PortInfo
Narrower::port_info [] =
{
	{ "in.l", INPUT | AUDIO }, { "in.r", INPUT | AUDIO }, 
	{	"mode", INPUT | CONTROL, {INTEGER | DEFAULT_0, 0, 1}, 
		"{0:'crossfeed mixing',1:'mid/side processing',}" }, 
	{ "strength", INPUT | CONTROL | GROUP, {DEFAULT_LOW, 0, 1} }, 
	{ "out.l", OUTPUT | AUDIO }, 
	{	"out.r", OUTPUT | AUDIO	}
};

template <> void
Descriptor<Narrower>::setup()
{
	Label = "Narrower";

	Name = CAPS "Narrower - Stereo image width reduction";
	Maker = "Tim Goetze <tim@quitte.de>";
	Copyright = "2011-12";

	/* fill port info and vtable */
	autogen();
}

