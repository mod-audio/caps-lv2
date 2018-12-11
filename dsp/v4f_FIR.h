/*
	dsp/v4f_FIR.h
	
	Copyright 2003-14 Tim Goetze <tim@quitte.de>
	
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

#if 0
template <int N>
struct FIRv4b
{
	enum {M=(N*4)-1};
	v4f_t x4[N];
	v4f_t c4[4][N];
	int z;

	FIRv4b()
		{
			z = 0;
			reset();
		}

	void halfband()
		{
			float *c0 = (float*) c4[0];
			for(int i=0; i<2*N; ++i)
			{
				double x = i-2*N+.5;
				double w = 1;
				c0[i] = c0[M-i] = w*w*sin(3*x*M_PI)/(x*M_PI);
			}
			DSP::kaiser<DSP::apply_window> (c0, 4*N, 6.4);
			double s = 0; 
			for(int i=0; i<4*N; ++i) 
				s += c0[i];
			s = 1./s; /* normalise */
			for(int i=0; i<4*N; ++i)
				c0[i] = 0 ? i : s*c0[i];
			setkernel(c0);
			reset();
		}

	void setkernel(float *k)
		{
			float *c0 = (float*) c4[0];
			for(int i=0; i<4*N; ++i)
				c0[i] = k[i];
			for(int j=1; j<4; ++j)
			{
				float *c = (float*) c4[j];
				for (int i=0; i<4*N; ++i)
					c[i] = c0[(j+i)&M];
			}
		}

	void reset() {for(int i=0; i<N; ++i) x4[i]=v4f_0;}
	
	float hb_put(float a)
		{
			float *x = (float*) x4;
			x[(-z)&M] = a;
			return x[(2*N-z)&M];
		}
	float hb_get()
		{
			v4f_t y = v4f_0;
			v4f_t *c = c4[z&3];
			int j0 = z>>2;
			for(int j=0; j<N; ++j)
				y += c[(j+j0)&(N-1)]*x4[j];
			y += v4f_shuffle(y,2,3,0,0);
			y += v4f_shuffle(y,1,0,0,0);
			z = (z+1)&M;
			return v4fa(y)[0];
		}
} __attribute__ ((packed, aligned(16)));
#endif

} /* namespace DSP */

#endif /* DSP_V4F_FIR_H */

