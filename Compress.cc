/*
	Compress.cc
	
	Copyright 2004-12 Tim Goetze <tim@quitte.de>
	
	http://quitte.de/dsp/

	Dynamic range processors.
	
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
	compress.init (fs);
	remain = 0;
}

struct NoSat { sample_t process(sample_t x) { return x; } };

template <int Channels>
template <yield_func_t F>
void
CompressStub<Channels>::cycle (uint frames)
{
	static NoSat none;

	int s = getport(0);
	if (!s) subcycle<F,NoSat> 
		(frames, none, none);
	else if (s == 1) subcycle<F,CompSat2> 
		(frames, saturate[0].two, saturate[1].two);
	else if (s == 2) subcycle<F,CompSat4> 
		(frames, saturate[0].four, saturate[1].four);
	else if (s == 3) subcycle<F,CompSat8> 
		(frames, saturate[0].eight, saturate[1].eight);
}

template <int Channels>
template <yield_func_t F, class Sat>
void
CompressStub<Channels>::subcycle (uint frames, Sat & satl, Sat & satr)
{
	/* mode port value is Saturate template parameter */
	compress.set_threshold (getport(1));
	sample_t strength = getport(2);
	compress.set_attack (getport(3));
	compress.set_release (getport(4));
	sample_t gain_out = db2lin (getport (5));

	sample_t * sl = ports[Stereo ? 6 : 6]; /* ;) */
	sample_t * sr = ports[Stereo ? 7 : 6];

	sample_t * dl = ports[Stereo ? 8 : 7];
	sample_t * dr = ports[Stereo ? 9 : 7];

	while (frames)
	{
		if (remain == 0)
		{
			remain = compress.blocksize;
			compress.start_block (strength);
		}

		uint n = min (frames, remain);
		
		for (uint i = 0; i < n; ++i)
		{
			sample_t xl = sl[i], xr = sr[i];

			if (Stereo)
				compress.store (xl, xr);
			else
				compress.store (xl);
			sample_t gain = gain_out * compress.get();

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
	{ "mode", CTRL_IN, {INTEGER | DEFAULT_1, 0, 2},
		"{0:'linear',1:'saturating 2x',2:'saturating 4x',3:'saturating 4x128'}" }, 
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

	Name = CAPS "Compress - Mono compressor";
	Maker = "Tim Goetze <tim@quitte.de>";
	Copyright = "2011-12";

	/* fill port info and vtable */
	autogen();
}

/* //////////////////////////////////////////////////////////////////////// */

PortInfo
Compress2x2::port_info [] =
{
	{ "mode", CTRL_IN, {INTEGER | DEFAULT_1, 0, 2},
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
Descriptor<Compress2x2>::setup()
{
	Label = "Compress2x2";

	Name = CAPS "Compress2x2 - Stereo compressor";
	Maker = "Tim Goetze <tim@quitte.de>";
	Copyright = "2011-12";

	/* fill port info and vtable */
	autogen();
}

