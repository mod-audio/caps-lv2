/*
	Compress.h
	
	Copyright 2013 Tim Goetze <tim@quitte.de>
	
	http://quitte.de/dsp/

	Dynamic range processing.

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

#ifndef _COMPRESS_H_
#define _COMPRESS_H_

#include "dsp/util.h"
#include "dsp/Compress.h"
#include "dsp/polynomials.h"
#include "dsp/FIR.h"
#include "dsp/sinc.h"
#include "dsp/windows.h"

template <int Over, int FIRSize>
class CompSaturate
{
	public:
		/* antialias filters */
		DSP::FIRUpsampler<FIRSize, Over> up;
		DSP::FIRn<FIRSize> down;

	public:
		void init (double fs)
			{
				/* going a bit lower than nominal with fc */
				double f = .7 * M_PI / Over;
				
				DSP::sinc (f, up.c, FIRSize);
				DSP::kaiser<DSP::apply_window> (up.c, FIRSize, 6.4);

				/* copy upsampler filter kernel for downsampler, make sum */
				double s = 0;
				for (uint i = 0; i < FIRSize; ++i)
					down.c[i] = up.c[i],
					s += up.c[i];
				
				s = 1 / s;

				/* scale kernels for unity gain */
				for (uint i = 0; i < FIRSize; ++i)
					down.c[i] *= s;

				s *= Over;
				for (uint i = 0; i < FIRSize; ++i)
					up.c[i] *= s;
			}


		sample_t process (sample_t x)
			{
				x = up.upsample (x);
				x = DSP::Polynomial::tanh(x);
				x = down.process (x);

				for (int o = 1; o < Over; ++o)
					down.store (DSP::Polynomial::atan1 (up.pad (o)));

				return x;
			}
};

typedef CompSaturate<2,32> CompSat2;
typedef CompSaturate<4,64> CompSat4;
typedef CompSaturate<4,128> CompSat8;

template <int Channels>
class CompressStub
: public Plugin
{
	public:
		uint remain;
		DSP::CompressPeak compress;

		enum { Stereo = (Channels == 2) };

		struct {
			CompSat2 two; 
			CompSat4 four; 
			CompSat8 eight;
		} saturate [Channels];

		template <yield_func_t F>
				void cycle (uint frames);
		template <yield_func_t F, class Sat>
				void subcycle (uint frames, Sat & satl, Sat & satr);

	public:
		static PortInfo port_info [];

		void init() 
			{
				for (int i=0; i < Channels; ++i)
				{
					saturate[i].two.init(fs); 
					saturate[i].four.init(fs); 
					saturate[i].eight.init(fs); 
				}
			}
		void activate();

		void run (uint n) { cycle<store_func> (n); }
		void run_adding (uint n) { cycle<adding_func> (n); }
};

class Compress
: public CompressStub<1>
{
	public:
		static PortInfo port_info [];
};

class Compress2x2
: public CompressStub<2>
{
	public:
		static PortInfo port_info [];
};

#endif /* _COMPRESS_H_ */
