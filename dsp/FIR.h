/*
	dsp/FIR.h
	
	Copyright 2003-12 Tim Goetze <tim@quitte.de>
	
	http://quitte.de/dsp/

	finite impulse response filters, with options for up- and down-sampling.

*/
/* 
	Somewhat surprisingly, exploiting the symmetry of linear-phase filter
	kernels by only storing one half of the kernel does not improve 
	performance on this Core2 chip, on the contrary.  apparently,
	memory bandwidth (the full kernel likely being read into the CPU 
	cache as soon as it is accessed) is not an issue, but additional index 
	arithmetic becomes one.
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

#ifndef FIR_H
#define FIR_H

#include "util.h"

namespace DSP {
	
/* 
	Brute-force FIR filter with downsampling method (decimating). 
*/
class FIR
{
	public:
		/* kernel length, history length - 1 */
		uint n, m;
		
		/* coefficients, history */
		sample_t * c, * x;

		/* history index */
		int h; 
		
		FIR() { c = x = 0; }
		~FIR() { free(c); free(x); }
		
		void init (uint N)
			{
				n = N;

				/* keeping history size a power of 2 makes it possible to wrap the
				 * history pointer by & instead of %, saving a few cpu cycles. */
				m = next_power_of_2 (n);

				c = (sample_t *) malloc (n * sizeof (sample_t));
				x = (sample_t *) malloc (m * sizeof (sample_t));

				m -= 1;

				reset();
			}
	
		void reset()
			{
				h = 0;
				memset (x, 0, n * sizeof (sample_t));
			}
		
		inline sample_t process (sample_t s)
			{
				x[h] = s;
				
				s *= c[0];

				for (uint Z = 1, z = h - 1; Z < n; --z, ++Z)
					s += c[Z] * x[z & m];

				h = (h + 1) & m;

				return s;
			}

		/* Z is the time, in samples, since the last non-zero sample.
		 * OVER is the oversampling factor. just here for documentation, use
		 * a FIRUpsampler instead.
		 */
		template <uint Z, uint OVER>
		inline sample_t upsample (sample_t s)
			{
				x[h] = s;
				
				s = 0;

				/* for the interpolation, iterate over the history in z ^ -OVER
				 * steps -- all the samples between are 0.
				 */
				for (uint j = Z, z = h - Z; j < n; --z, j += OVER)
					s += c[j] * x[z & m];

				h = (h + 1) & m;

				return s;
			}

		/* used in downsampling */
		inline void store (sample_t s)
			{
				x[h] = s;
				h = (h + 1) & m;
			}
};

/* FIR upsampler, optimised not to store the 0 samples */
template <int N, int Oversample>
class FIRUpsampler
{
	public:
		uint m; /* history length - 1 */
		int h; /* history index */

		sample_t * c, * x; /* coefficients, history */

		FIRUpsampler()
			{
				c = x = 0;
				init();
			}

		~FIRUpsampler()
			{
				if (c) free (c);
				if (x) free (x);
			}
		
		void init()
			{
				/* FIR kernel length must be a multiple of the oversampling ratio */
				assert (N % Oversample == 0);

				/* like FIR, keep the history buffer a power of 2; additionally,
				 * don't store the 0 samples inbetween. */
				m = next_power_of_2 ((N + Oversample - 1) / Oversample);

				c = (sample_t *) malloc (N * sizeof (sample_t));
				x = (sample_t *) malloc (m * sizeof (sample_t));

				m -= 1;

				reset();
			}
	
		void reset()
			{
				h = 0;
				memset (x, 0, (m + 1) * sizeof (sample_t));
			}
		
		/* upsample the given sample */
		inline sample_t upsample (sample_t s)
			{
				x[h] = s;
				
				s = 0;

				for (uint Z = 0, z = h; Z < N; --z, Z += Oversample)
					s += c[Z] * x[z & m];

				h = (h + 1) & m;

				return s;
			}

		/* upsample a zero sample (interleaving), Z being the time, in samples,
		 * since the last non-0 sample. */
		inline sample_t pad (uint Z)
			{
				sample_t s = 0;

				for (uint z = h-1; Z < N; --z, Z += Oversample)
					s += c[Z] * x[z & m];

				return s;
			}

};

/* templating for kernel length allows g++ to optimise aggressively
 * resulting in appreciable performance gains. */
template <int N>
class FIRn
{
	public:
		/* history length - 1 */
		uint m;
		
		/* coefficients, history */
		sample_t c[N], x[N];

		/* history index */
		int h; 
		
		FIRn()
			{
				/* keeping history size a power of 2 makes it possible to wrap the
				 * history pointer by & instead of %, saving a few cpu cycles. */
				m = next_power_of_2 (N) - 1;

				reset();
			}
	
		void reset()
			{
				h = 0;
				memset (x, 0, N * sizeof (sample_t));
			}
		
		inline sample_t process (sample_t s)
			{
				x[h] = s;
				
				s *= c[0];

				for (uint Z = 1, z = h - 1; Z < N; --z, ++Z)
					s += c[Z] * x[z & m];

				h = (h + 1) & m;

				return s;
			}

		/* used in downsampling */
		inline void store (sample_t s)
			{
				x[h] = s;
				h = (h + 1) & m;
			}
};

} /* namespace DSP */

#endif /* FIR_H */
