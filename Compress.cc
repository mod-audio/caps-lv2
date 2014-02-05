/*
	Compress.cc
	
	Copyright 2011-13 Tim Goetze <tim@quitte.de>
	
	http://quitte.de/dsp/
	
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

#include "Compress.h"
#include "Descriptor.h"

template <int Channels>
void
CompressStub<Channels>::activate()
{
	compress.peak.init (fs);
	compress.rms.init (fs);
	remain = 0;
}

template <int Channels>
template <yield_func_t F>
void
CompressStub<Channels>::cycle (uint frames)
{
	int c = getport(0);
	if (c == 0) 
		subcycle<F,DSP::CompressPeak> (frames, compress.peak);
	else 
		subcycle<F,DSP::CompressRMS> (frames, compress.rms);
}

/* do-nothing 'saturator' as template for linear instead of saturating mode */
struct NoSat { sample_t process(sample_t x) { return x; } };

template <int Channels>
template <yield_func_t F, class Comp>
void
CompressStub<Channels>::subcycle (uint frames, Comp & comp)
{
	static NoSat none;

	int s = getport(1);
	if (s == 1) subsubcycle<F,Comp,CompSat2> 
			(frames, comp, saturate[0].two, saturate[1].two);
	else if (s == 2) subsubcycle<F,Comp,CompSat4> 
			(frames, comp, saturate[0].four, saturate[1].four);
	else if (s == 3) subsubcycle<F,Comp,CompSat8> 
			(frames, comp, saturate[0].eight, saturate[1].eight);
	else subsubcycle<F, Comp, NoSat> 
			(frames, comp, none, none);
}

template <int Channels>
template <yield_func_t F, class Comp, class Sat>
void
CompressStub<Channels>::subsubcycle (uint frames, Comp & comp, Sat & satl, Sat & satr)
{
	comp.set_threshold (getport(2));
	sample_t strength = getport(3);
	comp.set_attack (getport(4));
	comp.set_release (getport(5));
	sample_t gain_out = db2lin (getport (6));

	sample_t * sl = ports[Stereo ?  7 : 7]; /* ;) */
	sample_t * sr = ports[Stereo ?  8 : 7];

	sample_t * dl = ports[Stereo ?  9 : 8];
	sample_t * dr = ports[Stereo ? 10 : 8];

	while (frames)
	{
		if (remain == 0)
		{
			remain = comp.blocksize;
			comp.start_block (strength);
		}

		uint n = min (frames, remain);
		
		for (uint i = 0; i < n; ++i)
		{
			sample_t xl = sl[i], xr = sr[i];

			if (Stereo)
				comp.store (xl, xr);
			else
				comp.store (xl);

			sample_t gain = gain_out * comp.get();

			xl = satl.process (xl * gain);
			if (Stereo)
				xr = satr.process (xr * gain);

			F (dl, i, xl, adding_gain);
			if (Stereo)
				F (dr, i, xr, adding_gain);
		}

		sl += n;
		sr += n;
		dl += n;
		dr += n;
		frames -= n;
		remain -= n;
	}
}

/* //////////////////////////////////////////////////////////////////////// */

PortInfo
Compress::port_info [] =
{
	{ "measure", CTRL_IN, {INTEGER | DEFAULT_0, 0, 1},
		"{0:'peak',1:'rms'}" }, 
	{ "mode", CTRL_IN | GROUP, {INTEGER | DEFAULT_1, 0, 3},
		"{0:'no limiting',1:'saturating 2x',2:'saturating 4x',3:'saturating 4x128'}" }, 
	{ "threshold", CTRL_IN | GROUP, {DEFAULT_0, 0, 1} }, 
	{ "strength", CTRL_IN, {DEFAULT_LOW, 0, 1} }, 
	{ "attack", CTRL_IN | GROUP, {DEFAULT_0, 0, 1} }, 
	{ "release", CTRL_IN, {DEFAULT_LOW, 0, 1} }, 
	{ "gain (dB)", CTRL_IN | GROUP, {DEFAULT_0, -12, 24} }, 
	{ "in", INPUT | AUDIO, {BOUNDED, -1, 1} }, 
	{ "out", OUTPUT | AUDIO, {0} }
};

template <> void
Descriptor<Compress>::setup()
{
	Label = "Compress";

	Name = CAPS "Compress - Compressor and saturating limiter";
	Maker = "Tim Goetze <tim@quitte.de>";
	Copyright = "2011-13";

	/* fill port info and vtable */
	autogen();
}

/* //////////////////////////////////////////////////////////////////////// */

PortInfo
CompressX2::port_info [] =
{
	{ "measure", CTRL_IN, {INTEGER | DEFAULT_0, 0, 1},
		"{0:'peak',1:'rms'}" }, 
	{ "mode", CTRL_IN | GROUP, {INTEGER | DEFAULT_1, 0, 2},
		"{0:'linear',1:'saturating 2x',2:'saturating 4x',3:'saturating 4x128'}" }, 
	{ "threshold", CTRL_IN | GROUP, {DEFAULT_0, 0, 1} }, 
	{ "strength", CTRL_IN, {DEFAULT_LOW, 0, 1} }, 
	{ "attack", CTRL_IN | GROUP, {DEFAULT_0, 0, 1} }, 
	{ "release", CTRL_IN, {DEFAULT_LOW, 0, 1} }, 
	{ "gain (dB)", CTRL_IN | GROUP, {DEFAULT_0, -12, 24} }, 
	{ "in.l", INPUT | AUDIO },
	{ "in.r", INPUT | AUDIO },
	{ "out.l", OUTPUT | AUDIO },
	{ "out.r", OUTPUT | AUDIO },
};

template <> void
Descriptor<CompressX2>::setup()
{
	Label = "CompressX2";

	Name = CAPS "CompressX2 - Stereo compressor and saturating limiter";
	Maker = "Tim Goetze <tim@quitte.de>";
	Copyright = "2011-13";

	/* fill port info and vtable */
	autogen();
}

