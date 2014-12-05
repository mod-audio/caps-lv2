/*
	Eq.h
	
	Copyright 2004-14 Tim Goetze <tim@quitte.de>
	
	http://quitte.de/dsp/

	Filter prototypes from a motorola paper implementing a similar circuit on a DSP56k.

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

#ifndef DSP_EQ_H
#define DSP_EQ_H

namespace DSP {

template <int Bands, class eq_sample = float>
class Eq
{
	public:
		/* recursion coefficients, 3 per band */
		eq_sample a[Bands], b[Bands], c[Bands];
		/* past outputs, 2 per band */
		eq_sample y[2][Bands];
		/* current gain and recursion factor, each 1 per band = 2 */
		eq_sample gain[Bands], gf[Bands];
		/* input history */
		eq_sample x[2];
		/* history index */
		int h;

		eq_sample normal;

		Eq()
			{
				h = 0;
				normal = NOISE_FLOOR;
			}

		void reset()
			{
				for (int z = 0; z < 2; ++z)
					for (int i = 0; i < Bands; ++i)
						y[z][i] = 0;

				for (int i = 0; i < 2; ++i)
					x[i] = 0;
			}

		void init (double fs, double Q)
			{
				double f = 31.25;
				int i = 0;

				for (i = 0; i < Bands && f < .48*fs; ++i, f *= 2)
					init_band (i, 2*f*M_PI/fs, Q);
				/* just in case, zero the remaining coefficients */
				for (  ; i < Bands; ++i)
					zero_band (i);

				reset();
			}	

		void init_band (int i, double theta, double Q)
			{
				b[i] = (Q - .5*theta)/(2*Q + theta);
				a[i] = (.5 - b[i])*.5;
				c[i] = (.5 + b[i])*cos(theta);
				/* fprintf (stderr, "%02d %f %f %f\n", i, a[i], b[i], c[i]); */
				gain[i] = 1;
				gf[i] = 1;
			}

		void zero_band (int i) { a[i] = b[i] = c[i] = 0; }

		/* per-band recursion:
		 * 	y = 2*(a*(x - x[-2]) + c*y[-1] - b*y[-2]) 
		 */
		eq_sample process (eq_sample s)
			{
				int z1 = h, z2 = h ^ 1;

				eq_sample * y1 = y[z1];
				eq_sample * y2 = y[z2];

				eq_sample x_x2 = s - x[z2];
				eq_sample r = 0;

				for (int i = 0; i < Bands; ++i)
				{
					y2[i] = normal + 2*(a[i]*x_x2 + c[i]*y1[i] - b[i]*y2[i]);
					r += gain[i] * y2[i];
					gain[i] *= gf[i];
				}

				x[z2] = s;
				h = z2;

				return r;
			}

		/* zap denormals in history */
		void flush_0()
			{
				for (int i = 0; i < Bands; ++i)
					if (is_denormal (y[0][i]))
						y[0][i] = 0;
			}
};

} /* namespace DSP */

#endif /* DSP_EQ_H */
