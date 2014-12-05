/*
	dsp/v4f_FIR.h
	
	Copyright 2003-12 Tim Goetze <tim@quitte.de>
	
	http://quitte.de/dsp/

	FIR filter, SIMD version

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

#ifndef DSP_V4F_FIR_H
#define DSP_V4F_FIR_H

#include "v4f.h"

namespace DSP {

/* template parameter is number of taps and must be a power of two */
template <int N>
class FIRv4
{
	public:
		enum { 
			/* data is laid out

				{c[0-3] c[4-7] ... c[N-4 - N-1]}  = filter kernel

				{x[0-3] x[4-7] ... x[N-4 - N-1]}  = input history
				{x[1-4] x[5-8] ... x[N-3 - N]}    =   "      "    offset by one sample
				{x[2-5] x[6-9] ... x[N-2 - N+1]}  =   "      "      "     " two samples
				{x[3-6] x[7-10] ... x[N-1 - N+2]} =   "      "      "     " three  "

			Past input samples are stored fourfold in order to avoid unaligned accesses.  
			Storing the current sample into the four correct locations
			is a bit of a convoluted process.  However, the simpler alternative 
			of processing four consecutive samples at a time occupies exactly as much 
			memory and introduces an undesirable three-sample delay.

			*/
			DataSize = 5 * (N/4) * sizeof (v4f_t)
		};
		char _data [DataSize + sizeof (v4f_t)];

		int h;

	public:
		FIRv4()
			{
				h = 0;
				reset();
			}

		v4f_t process (float s)
			{
				v4f_t * c = data();

				/* current row of x[] according to the data layout laid out above ^^ */
				v4f_t * x = c+N/4;
				/* store current sample */
				int j = h&3;
				int i = N*j + (h&~3);
				for (  ; j < 4; ++j, i += N+1)
					((float *) x) [i] = s;
				i -= 4*N - 4;
				if (h > N-4) i -= N;
				for (j = (h&3); j > 0; --j, i += N+1)
					((float *) x) [i] = s;

				x += (N/4) * (h&3);
				i = h>>2;

				#if 0
				{
					v4f_t a = v4f_0;
					for (j = 0; i >= 0; ++j, --i)
					{
						//msg(), j, " (", i, ") ", a, " += ", c[j], " * ", x[i];
						a += c[j] * x[i];
					}
					i = N/4-1;
					for (  ; j < N/4; ++j, --i)
					{
						//msg(), j, " (", i, ") ", a, " += ", c[j], " * ", x[i];
						a += c[j] * x[i];
					}
					msg(), "after x=", s, " h=", h, " at row ", (h&3), " col ", (h>>2);
					dump();
				}
				#else
				v4f_t a = v4f_0;
				for (j = 0; i >= 0; ++j, --i)
					a += c[j] * x[i];
				i = N/4-1;
				for (  ; j < N/4; ++j, --i)
					a += c[j] * x[i];
				#endif

				h = (h+1) & (N-1);

				return a;
			}

		/* take care of alignment issues */
		inline v4f_t * data()
			{
				uint64 p = ((uint64) _data + 16) & ~15ll;
				return (v4f_t *) p;
			}
	
		#ifdef __CREAM__
		void dump()
			{
				float * f = (float *) data();
				f += N;
				for (int i = 1; i < 5; ++i)
				{
					message m;
					if (i) m, "x[",i-1,"] ";
					else m, "   c ";
					for (int j = 0; j < N; ++j, ++f)
					{
						char s[20];
						sprintf (s, "%.1f", *f);
						if (*f < 1)
							s[0] = ' ';
						if (*f < .1)
							s[2] = ' ';
						m, s, ((j+1)&3) ? " " : "  ";
					}
				}
			}
		#endif

		void set_kernel (float * f, int n = N)
			{
				float * c = (float *) data();
				int i = 0;
				for (  ; i < n; ++i)
					c[i] = f[i];
				for (  ; i < N; ++i)
					c[i] = 0;
			}

		void reset()
			{
				/* set all x[] to 0 */
				memset (data() + N/4, 0, 4*N * sizeof (float));
			}
};

} /* namespace DSP */

#endif /* DSP_V4F_FIR_H */

