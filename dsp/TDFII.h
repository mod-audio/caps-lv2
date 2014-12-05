/*
	TDFII.h
	
	Copyright 2006-7
		David Yeh <dtyeh@ccrma.stanford.edu> (implementation)
	2006-14
		Tim Goetze <tim@quitte.de> (cosmetics)

	transposed Direct Form II digital filter.
	Assumes order of b = order of a.
	Assumes a0 = 1.

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

#ifndef DSP_TDFII_H
#define DSP_TDFII_H

namespace DSP {

/* N is the highest power of s in the transfer function */
template <int N>
class TDFII
{
	public:
		double a[N+1];
		double b[N+1];
		double h[N+1];
	
		void reset() 
			{
				for (int i = 0; i <= N; ++i)
					h[i] = 0;   // zero state
			}

		void init (double fs)
			{
				reset();
				clear();
			}

		void clear() 
			{
				for (int i=0; i<= N; i++) 
					a[i] = b[i] = 0;
				b[0] = 1;
			}

		/* per-band recursion:
		 * 	y = 2 * (a * (x - x[-2]) + c * y[-1] - b * y[-2]) 
		 */
		sample_t process (sample_t s)
			{
				double y = h[0] + b[0]*s;

				for (int i=1; i<N; ++i) 
						h[i-1] = h[i] + b[i]*s - a[i]*y;

				h[N-1] = b[N]*s - a[N]*y;

				return (sample_t) y;
			}
};

} /* namespace DSP */

#endif /* DSP_TDFII_H */
