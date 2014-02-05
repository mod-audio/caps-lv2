/*
	dsp/RMS.h
	
	Copyright 2004-13 Tim Goetze <tim@quitte.de>
	
	http://quitte.de/dsp/

	Sliding window, running sum root-mean-square.

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

#ifndef DSP_RMS_H
#define DSP_RMS_H

namespace DSP {

template <int N>
class RMS
{
	protected:
		sample_t buffer[N];
		int write;

	public:
		double sum, over_N;

		RMS()
			{
				/* g++ doesn't reliably optimise the division in get() below into 
				 * a multiplication with 1/N */
				over_N = 1./N;
				write = 0;
				reset();
			}

		void reset()
			{
				sum = 0.;
				memset (buffer, 0, sizeof (buffer));
			}

		/* caution: pass in the SQUARED! sample value */
		void store (sample_t x)
			{
				sum -= buffer[write];
				sum += (buffer[write] = x);
				write = (write+1) & (N-1);
			}

		sample_t get()
			{
				/* lack of running sum accuracy necessitates fabs() */
				return sqrt(fabs(sum * over_N));
			}
};

} /* namespace DSP */

#endif /* DSP_RMS_H */
