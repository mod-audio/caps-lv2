/*
	dsp/Roessler.h
	
	Copyright 2003-13 Tim Goetze <tim@quitte.de>
	
	http://quitte.de/dsp/

	Roessler attractor.

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

#ifndef DSP_ROESSLER_H
#define DSP_ROESSLER_H

namespace DSP {

class Roessler
{
	public:
		double x[2], y[2], z[2];	
		double h, a, b, c;
		int I;

	public:
		Roessler()
			{
				h = 0.001;
				a = .2;
				b = .2;
				c = 5.7;
			}

		/* rate is normalized (0 .. 1) */
		void set_rate (double r)
			{
				h = max (.000001, r * .096);
			}

		void init (double _h = .001, double seed = .0)
			{
				h = _h;

				I = 0;

				x[0] = -0.327732 + .0001 * seed;
				y[0] = 2.569375;
				z[0] = 0.036099;
			}

		void step()
			{
				int J = I ^ 1;

				x[J] = x[I] + h*(-y[I] - z[I]);
				y[J] = y[I] + h*(x[I] + a*y[I]);
				z[J] = z[I] + h*(b + z[I]*(x[I] - c));

				I = J;
			}

		sample_t get()
			{
				step();	
				return x[I]*.01725 + z[I]*.015;
			}

#if 0
		double get_x() { return x[I]; }
		double get_y() { return y[I]; }
		double get_z() { return z[I]; }
#else
		double get_x() {return (x[I]-0.22784)*-.08;}
		double get_y() {return (y[I]+1.13942)*-.09;}
		double get_z() {return (z[I]-1.13929)*.055;}
#endif
};

} /* namespace DSP */

#endif /* DSP_ROESSLER_H */
