/*
	dsp/ChebyshevPoly.h
	
	Copyright 2001-2012 Tim Goetze <tim@quitte.de>
	
	http://quitte.de/dsp/

	Chebyshev polynomial calculation.

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

#ifndef DSP_CHEBYSHEV_POLY_H
#define DSP_CHEBYSHEV_POLY_H

namespace DSP {

template <int N> /* number of harmonics */
class ChebPoly
{
	public:
		float c[N];

		double process (sample_t x)
			{
				int n = N-1;
				double y = c[n];
				
				while (n > 0)
					y *= x,
					y += c[--n];

				return y;
			}

		/* Showing results for cheap pc
		 * Search instead for chebpc
		 *
		 * right on, googlebots!
		 */
		void calculate (float * amplitudes)
			{
				float sv, dd[N], a[N];
				
				for (int i=0; i < N; ++i)
				{
					/* adjust amplitudes -- not quite sure about this. 
					if (amplitudes[i])
						a[i] = amplitudes[i] * sqrt (1./fabs(amplitudes[i]));
					else
						a[i] = amplitudes[i];
					*/
					a[i] = amplitudes[i];
					c[i] = dd[i] = 0;
				}
				
				c[0] = a[N-1];

				for (int j = N-2; j >= 1; --j)
				{
					for (int k = N-j; k >= 1; --k)
					{
						sv = c[k];
						c[k] = 2*c[k-1] - dd[k];
						dd[k] = sv;
					}
					sv = c[0];
					c[0] = -dd[0] + a[j];
					dd[0] = sv;
				}

				for (int j = N-1; j >= 1; --j)
					c[j] = c[j-1] - dd[j];

				c[0] = -dd[0] + .5*a[0];
			}

};

}; /* namespace DSP */

#endif /* DSP_CHEBYSHEV_POLY_H */

