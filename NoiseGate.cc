/*
	NoiseGate.cc
	
	Copyright 2011-12 Tim Goetze <tim@quitte.de>
	
	http://quitte.de/dsp/

	Noise reduction measures

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
#include <stdio.h>

#include "NoiseGate.h"
#include "Descriptor.h"

void
NoiseGate::init()
{
	N = 882*fs/44100; /* 20 ms RMS accumulation when open */
	over_N = 1./N;
	gain.quiet = db2lin (-60);
}

void
NoiseGate::activate()
{
	rms.reset();
	remain = 0;
	gain.current = gain.quiet;
	gain.delta = 0;
	f_mains = -1; /* make sure filters are updated when processing */
}

template <yield_func_t F>
void
NoiseGate::cycle (uint frames)
{
	sample_t * s = ports[0];
	sample_t * d = ports[1]; 

	float open = db2lin (getport (2));
	float attack = max (.005*N*getport(3), 2); 
	float close = db2lin (getport (4));

	float f = getport (5);
	if (f != f_mains)
	{
		f_mains = f;
		if (!f) /* no mains hum filtering: set filters to identity */
		{
			humfilter[0].unity();
			humfilter[1].unity();
		}
		else
		{
			DSP::RBJ::BP (f_mains*over_fs, 5, humfilter[0]);
			DSP::RBJ::BP (f_mains*over_fs, 1, humfilter[1]);
		}
		humfilter[0].reset();
		humfilter[1].reset();
	}

	while (frames)
	{
		if (remain == 0)
		{
			remain = N;
			if (gain.delta) /* just opened or closed */
				gain.delta = 0;
			else if (rms.get() < close) /* otherwise, need to close? */
			{
				//fprintf (stderr, "%.3f < %.3f\n", rms.get(), close);
				gain.delta = (gain.quiet - gain.current) * over_N;
			}
		}

		uint i = 0;
		uint n = min (frames, remain);
		//fprintf (stderr, "%d %.3f (%.3f)\n", n, gain.current, gain.delta);
		if (gain.delta) for (  ; i < n; ++i) /* opening or closing */
		{
			register sample_t a = s[i];
			store (a);
			F (d, i, a * gain.current, adding_gain);
			gain.current += gain.delta;
		}
		else if (gain.current == 1) for (  ; i < n; ++i) /* currently open */
		{
			register sample_t a = s[i];
			store (a);
			F (d, i, a, adding_gain);
		}
		else for (  ; i < n; ++i) /* currently closed */
		{
			register sample_t a = s[i];
			store(a);
			if (fabs(a) < open)
				F (d, i, a * gain.current, adding_gain);
			else 
			{
				remain = (int) attack;
				gain.delta = (1-gain.current) / remain;
				/* correct for later subtraction of i */
				remain += i;
				break;
			}
		}
		
		s += i, d += i;
		frames -= i;
		remain -= i;
	}
}

/* //////////////////////////////////////////////////////////////////////// */

PortInfo
NoiseGate::port_info [] = 
{
	{ "in", INPUT | AUDIO },
	{	"out", OUTPUT | AUDIO },

	/* 2 */
	{ "open (dB)", CTRL_IN, {DEFAULT_LOW, -60, 0} }, 
	{ "attack (ms)", CTRL_IN, {DEFAULT_LOW, 0, 3} }, 
	{ "close (dB)", CTRL_IN, {DEFAULT_LOW, -90, 0} }, 

	/* mains */
	{ "mains (Hz)", CTRL_IN|GROUP, {INTEGER|DEFAULT_MID, 0, 100},
		"{0:'off',50:'global',60:'imperial'}"}, 
};

template <> void
Descriptor<NoiseGate>::setup()
{
	Label = "NoiseGate";

	Name = CAPS "NoiseGate - Attenuate noise resident in silence";
	Maker = "Tim Goetze <tim@quitte.de>";
	Copyright = "2011-12";

	/* fill port info and vtable */
	autogen();
}


