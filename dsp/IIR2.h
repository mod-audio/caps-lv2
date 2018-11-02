/*
	dsp/IIR2.h
	
	Copyright 2003-16 Tim Goetze <tim@quitte.de>
	
	http://quitte.de/dsp/

	2nd order IIR filter implementation

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

#ifndef DSP_IIR2_H
#define DSP_IIR2_H

#include "complex.h"

namespace DSP {

template <class T>
class IIR2
{
	public:
		/* coefficients */
		T a[5], * b;

		/* history */
		int h;
		T x[2], y[2];

		IIR2()
			{
				b = a + 2;
				unity();
				reset();
			}
		
		void unity()
			{
				a[0] = 1;
				a[1] = a[2] = b[1] = b[2] = 0;
			}

		double gain (double f)
			{
				complex w1 = complex::polar(2*M_PI*f);
				complex w2 = w1*w1;
				complex n = a[0]*w2 + a[1]*w1 + a[2];
				complex d = w2 - b[1]*w1 - b[2];
				return (n/d).abs();
			}

		/* scale so that the gain at f becomes g */
		void scale (double f, double g)
			{
				double is = gain(f);
				if (!is) /* I'm afraid I can't do that, Dave */
					return;
				scale(g/is);
			}

		void scale(double g)
			{
				a[0] *= g;
				a[1] *= g;
				a[2] *= g;
			}

		void dump()
			{
				#ifdef DEBUG
				fprintf (stderr, "IIR2 a[]=%.5f %.5f %.5f b[]=%.5f %.5f\n", 
						a[0],a[1],a[2],b[1],b[2]);
				#endif
			}

		void reset()
			{
				h = 0;

				x[0] = x[1] = 
				y[0] = y[1] = 0.;
			}

		/* denormal zapping */
		void flush_0()
			{
				for (int i = 0; i < 2; ++i)
					if (is_denormal (y[i]))
						y[i] = 0;
			}

		inline T process (T s)
			{
				register int z = h;

				register T r = s * a[0];
				
				r += a[1] * x[z];
				r += b[1] * y[z];

				z ^= 1;
				r += a[2] * x[z];
				r += b[2] * y[z];

				y[z] = r;
				x[z] = s;
				
				h = z;

				return r;
			}

		inline T process_bp (T s)
			{
				register int z = h;

				register T r = s * a[0];
				
				r += b[1] * y[z];

				z ^= 1;
				r += a[2] * x[z];
				r += b[2] * y[z];

				y[z] = r;
				x[z] = s;
				
				h = z;

				return r;
			}

		/* Following are additional methods for using the biquad to filter an
		 * upsampled signal with 0 padding -- some terms reduce to 0 in this
		 * case */
		inline T process_0_1()
			{
				register int z = h;

				register T r = 0;
				
				r += a[1] * x[z];
				r += b[1] * y[z];

				z ^= 1;
				r += a[2] * x[z];
				r += b[2] * y[z];

				y[z] = r;
				x[z] = 0; 
				
				h = z;

				return r;
			}

		inline T process_0_2()
			{
				register int z = h;

				register T r = 0;
				
				r += b[1] * y[z];

				z ^= 1;
				r += a[2] * x[z];
				r += b[2] * y[z];

				y[z] = r;
				x[z] = 0;
				
				h = z;

				return r;
			}

		inline T process_0_3()
			{
				register int z = h;

				register T r = 0;
				
				r += b[1] * y[z];

				z ^= 1;
				r += b[2] * y[z];

				y[z] = r;
				x[z] = 0;
				
				h = z;

				return r;
			}
};

} /* namespace DSP */

#endif /* DSP_IIR2_H */
