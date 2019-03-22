/*
	Compress.h
	
	Copyright 2013-18 Tim Goetze <tim@quitte.de>
	
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

#ifndef COMPRESS_H
#define COMPRESS_H

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
		/* take off some edge */
		DSP::LP1<sample_t> lp;

	public:
		void init(double fs)
			{
				/* going a bit lower than half fs with fc */
				double f = .7*M_PI/Over;
				
				DSP::sinc(f, up.c, FIRSize);
				DSP::kaiser<DSP::apply_window> (up.c, FIRSize, 6.4);
				lp.set_f(5000/fs);

				/* copy upsampler filter kernel for downsampler, make sum */
				double s = 0;
				for(uint i = 0; i < FIRSize; ++i)
					down.c[i] = up.c[i],
					s += up.c[i];
				
				s = 1/s;

				/* scale kernels for unity gain */
				for(uint i=0; i<FIRSize; ++i)
					down.c[i] *= s;

				s *= Over;
				for(uint i=0; i<FIRSize; ++i)
					up.c[i] *= s;
			}


		sample_t process(sample_t x)
			{
				x = up.upsample(x);
				x = DSP::Polynomial::tanh(x);
				x = down.process(x);
				x = lp.process(x);

				for(int o = 1; o < Over; ++o)
					down.store(DSP::Polynomial::tanh(up.pad(o)));

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
		struct {
			DSP::CompressPeak peak;
			DSP::CompressRMS rms;
		} compress;

		enum { Stereo = (Channels == 2) };

		struct {
			CompSat2 two; 
			CompSat4 four; 
		} saturate [Channels];

		void cycle(uint frames);
		template <class Comp>
				void subcycle(uint frames, Comp & comp);
		template <class Comp, class Sat>
				void subsubcycle(uint frames, Comp & comp, Sat & satl, Sat & satr);

	public:
		static PortInfo port_info [];

		void init() 
			{
				for(int i=0; i < Channels; ++i)
				{
					saturate[i].two.init(fs); 
					saturate[i].four.init(fs); 
				}
			}
		void activate();
};

class Compress
: public CompressStub<1>
{
	public:
		static PortInfo port_info [];
};

class CompressX2
: public CompressStub<2>
{
	public:
		static PortInfo port_info [];
};

#endif /* COMPRESS_H */
