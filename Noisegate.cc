/*
	Noisegate.cc
	
	Copyright 2011-16 Tim Goetze <tim@quitte.de>
	
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
	N = 3*960*fs/48000; /* 60 ms RMS accumulation when open */
	over_N = 1./N;
	hysteresis.threshold = (uint) (.180*fs); /* opening for at least 180 ms */
	gain.quiet = db2lin (-60);
	gain.lp.set_f (120*over_fs);
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
	f_mains = -1; /* make sure filters are updated when processing */
}

void 
Noisegate::process (sample_t x)
{
	x += normal;
	normal = -normal;
	sample_t y = humfilter[0].process(x);
	y = humfilter[1].process(y);
	rms.store (x - .3*y);
}

void
Noisegate::cycle (uint frames)
{
	static int frame = 0;

	float open = db2lin(getport(0)-10);
	float attack = max(.005*N*getport(1), 2); /* in samples */
	float close = db2lin(getport(2));

	float f = getport(3);
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

	sample_t * s = ports[4];
	sample_t * d = ports[5]; 
	bool opennow = false;
	while (frames)
	{
		if (remain == 0)
		{
			remain = N;
			if (opennow)
			{
				remain = (int) attack;
				gain.delta = (1 - gain.current)/remain;
				hysteresis.age = 0;
				opennow = false;
			}
			else if (gain.delta > 0)
				gain.current = 1,
				gain.delta = 0;
			else if (gain.delta < 0) 
				gain.delta = 0;
			else if (gain.current > gain.quiet+.001 && rms.get() < close && hysteresis.age > hysteresis.threshold) 
			{
				//fprintf (stderr, "%.3f < %.3f\n", rms.get(), close);
				gain.delta = (gain.quiet - gain.current)*over_N;
			}
		}

		#define OUT (gain.current+0*gain.get())
		uint i = 0;
		uint n = min (frames, remain);
		//fprintf (stderr, "%d %.3f (%.3f)\n", n, gain.current, gain.delta);
		if (gain.delta > 0 || gain.current == 1) for (  ; i < n; ++i) /* opening or open */
		{
			sample_t a = s[i];
			process(a);
			d[i] = a*gain.get();
		}
		else for (  ; i < n; ++i) /* closed */
		{
			sample_t a = s[i];
			process(a);
			if (fabs(a) < open)
				d[i] = a*gain.get();
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
		frame += i;
	}
}

/* //////////////////////////////////////////////////////////////////////// */

PortInfo
Noisegate::port_info [] = 
{
	{ "open (dB)", CTRL_IN, {DEFAULT_LOW, -55, 0} }, 
	{ "attack (ms)", CTRL_IN, {DEFAULT_0, 0, 5} }, 
	{ "close (dB)", CTRL_IN, {DEFAULT_LOW, -80, 0} }, 

	/* mains */
	{ "mains (Hz)", CTRL_IN|GROUP, {INTEGER|DEFAULT_MID, 0, 100},
		"{0:'off',50:'global',60:'imperial'}"}, 

	{ "in", INPUT | AUDIO },
	{	"out", OUTPUT | AUDIO }
};

template <> void
Descriptor<Noisegate>::setup()
{
	Label = "Noisegate";
	Name = CAPS "Noisegate - Attenuating hum and noise";
	autogen();
}


