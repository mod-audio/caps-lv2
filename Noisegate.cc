/*
	Noisegate.cc
	
	Copyright 2011-13 Tim Goetze <tim@quitte.de>
	
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

#include "Noisegate.h"
#include "Descriptor.h"

void
Noisegate::init()
{
	N = 3*882*fs/44100; /* 60 ms RMS accumulation when open */
	hysteresis.threshold = (uint) (.08*fs); /* opening for at least 80 ms */
	over_N = 1./N;
	gain.quiet = db2lin (-60);
	gain.lp.set_f (80*over_fs);
	lp.set_f (20*over_fs);
	delay.init (fs*.001); /* 1 ms maximum lookahead */
}

void
Noisegate::activate()
{
	rms.reset();
	remain = 0;
	hysteresis.age = 0;
	gain.current = gain.quiet;
	gain.delta = 0;
	gain.lp.reset();
	lp.reset();
	f_mains = -1; /* make sure filters are updated when processing */
}

template <yield_func_t yield>
void
Noisegate::cycle (uint frames)
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

	bool opennow = false;
	while (frames)
	{
		if (remain == 0)
		{
			remain = N;
			if (opennow)
			{
				remain = (int) attack;
				gain.delta = (1 - gain.current) / remain;
				hysteresis.age = 0;
				opennow = false;
			}
			else if (gain.delta > 0) 
				gain.current = 1,
				gain.delta = 0;
			else if (gain.delta < 0) 
				gain.delta = 0;
			else if (rms.get() < close && hysteresis.age > hysteresis.threshold) 
			{
				//fprintf (stderr, "%.3f < %.3f\n", rms.get(), close);
				gain.delta = (gain.quiet - gain.current)*over_N;
			}
		}

		uint i = 0;
		uint n = min (frames, remain);
		//fprintf (stderr, "%d %.3f (%.3f)\n", n, gain.current, gain.delta);
		if (gain.delta > 0 || gain.current == 1) for (  ; i < n; ++i) /* opening or open */
		{
			register sample_t a = s[i];
			lp.process(fabs(a+normal));
			delay.put(a);
			store(a);
			yield (d, i, 0 ? lp.y1-open : a*gain.get(), adding_gain);
		}
		else for (  ; i < n; ++i) 
		{
			register sample_t a = s[i];
			delay.put(a);
			store(a);
			if (lp.process(fabs(a+normal)) < open)
				yield (d, i, 0 ? lp.y1-open : a*gain.get(), adding_gain);
			else 
			{
				opennow = true;
				remain = i;
				break;
			}
		}
		
		hysteresis.age += i;
		s += i, d += i;
		frames -= i;
		remain -= i;
	}
}

/* //////////////////////////////////////////////////////////////////////// */

PortInfo
Noisegate::port_info [] = 
{
	{ "in", INPUT | AUDIO },
	{	"out", OUTPUT | AUDIO },

	/* 2 */
	{ "open (dB)", CTRL_IN, {DEFAULT_MIN, -60, 0} }, 
	{ "attack (ms)", CTRL_IN, {DEFAULT_0, 0, 5} }, 
	{ "close (dB)", CTRL_IN, {DEFAULT_MIN, -90, 0} }, 

	/* mains */
	{ "mains (Hz)", CTRL_IN|GROUP, {INTEGER|DEFAULT_MID, 0, 100},
		"{0:'off',50:'global',60:'imperial'}"}, 
};

template <> void
Descriptor<Noisegate>::setup()
{
	Label = "Noisegate";

	Name = CAPS "Noisegate - Attenuate hum and noise";
	Maker = "Tim Goetze <tim@quitte.de>";
	Copyright = "2011-13";

	/* fill port info and vtable */
	autogen();
}


