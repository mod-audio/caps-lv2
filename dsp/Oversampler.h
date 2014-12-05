/*
	dsp/Oversampler.h
	
	Copyright 2004-12 Tim Goetze <tim@quitte.de>
	
	http://quitte.de/dsp/

	Polyphase FIR up- and downsampler filters in one handy package, 
	quality adjustments through template parameters.  Can be used
	for undersampling as well.

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

#ifndef DSP_OVERSAMPLER_H
#define DSP_OVERSAMPLER_H

#include "FIR.h"
#include "sinc.h"
#include "windows.h"

namespace DSP {

/* do-nothing 1:1 oversampler substitute, occasionally needed as a 
 * template parameter */
class NoOversampler 
{
	public:
		enum { Ratio = 1 };
		sample_t downsample (sample_t x) { return x; }
		sample_t upsample (sample_t x) { return x; }
		void downstore (sample_t x) { }
		sample_t uppad (uint z) { return 0; }
};

template <int Oversample, int FIRSize>
class Oversampler
{
	public:
		enum { Ratio = Oversample };
		/* antialias filters */
		struct {
			DSP::FIRUpsampler<FIRSize, Oversample> up;
			DSP::FIRn<FIRSize> down;
		} fir;

		Oversampler()
			{ init(); }

		void init (float fc = .5) 
			{
				double f = fc * M_PI / Oversample;
				
				/* construct the upsampler filter kernel */
				DSP::sinc (f, fir.up.c, FIRSize);
				DSP::kaiser<DSP::apply_window> (fir.up.c, FIRSize, 6.4);

				/* copy upsampler filter kernel for downsampler, make sum */
				double s = 0;
				for (uint i = 0; i < FIRSize; ++i)
					fir.down.c[i] = fir.up.c[i],
					s += fir.up.c[i];
				
				/* scale downsampler kernel for unity gain */
				s = 1/s;
				for (uint i = 0; i < FIRSize; ++i)
					fir.down.c[i] *= s;

				/* scale upsampler kernel for unity gain */
				s *= Oversample;
				for (uint i = 0; i < FIRSize; ++i)
					fir.up.c[i] *= s;
			}

		void reset() 
			{
				fir.up.reset();
				fir.down.reset();
			}

		inline sample_t upsample(sample_t x)
			{ return fir.up.upsample(x); }
		inline sample_t uppad(uint z)
			{ return fir.up.pad(z); }

		inline sample_t downsample(sample_t x)
			{ return fir.down.process(x); }
		inline void downstore(sample_t x)
			{ fir.down.store(x); }
};

} /* namespace DSP */

#endif /* DSP_OVERSAMPLER_H */
