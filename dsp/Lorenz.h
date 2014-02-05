/*
	dsp/Lorenz.h
	
	Copyright 2001-13 Tim Goetze <tim@quitte.de>
	
	http://quitte.de/dsp/

	Lorenz fractal.

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

#ifndef DSP_LORENZ_H
#define DSP_LORENZ_H

namespace DSP {

class Lorenz
{
	public:
		double x[2], y[2], z[2];	
		double h, a, b, c;
		int I;

	public:
		Lorenz()
			{
				h = 0.001;
				a = 10.0;
				b = 28.0;
				c = 8.0 / 3.0;
			}

		/* rate is normalized (0 .. 1) */
		void set_rate (double r)
			{
				h = max (.0000001, r * .015);
			}

		void init (double _h = .001, double seed = .0)
			{
				I = 0;

				x[0] = -2.884960 + seed;
				y[0] = -5.549104;
				z[0] = 7.801511;

				h = _h;
			}

		sample_t get()
			{
				step();
				return .5 * get_y() + get_z();
			}

		void step()
			{
				int J = I ^ 1;

				x[J] = x[I] + h*a*(y[I] - x[I]);
				y[J] = y[I] + h*(x[I] * (b - z[I]) - y[I]);
				z[J] = z[I] + h*(x[I] * y[I] - c * z[I]);

				I = J;
			}
#if 0
		double get_x() {return x[I];}
		double get_y() {return y[I];}
		double get_z() {return z[I];}
#else
		double get_x() {return (x[I]+0.01661)*-.04;}
		double get_y() {return (y[I]-0.02379)*-.03;}
		double get_z() {return (z[I]-24.1559)*.03;}
#endif
};

} /* namespace DSP */

#endif /*DSP_LORENZ_H */
