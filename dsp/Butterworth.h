/*
  dsp/Butterworth.h

	Copyright 
		2012 Tim Goetze <tim@quitte.de>

	Butterworth LP/HP IIR2 setup.

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

#ifndef DSP_BUTTERWORTH_H
#define DSP_BUTTERWORTH_H

#include "IIR2.h"

namespace DSP { 
namespace Butterworth {

template <class T>
void 
LP (float f, DSP::IIR2<T> & bq)
{
	double w,g,h,n;

	w = tan(f*M_PI);
	g = 1.4142135623730951*w; 
	h = w*w;

	n = 1/(1 + g + h);

	bq.a[0] = bq.a[2] = n*h;
	bq.a[1] = 2*bq.a[0];

	bq.b[1] = -n*2*(h-1);
	bq.b[2] = -n*(1-g+h);
}

template <class T>
void 
HP (float f, DSP::IIR2<T> & bq)
{
	LP(f,bq);
	bq.a[1] = -bq.a[1];
	bq.scale(f, 0.7071067811865476); /* costly operation */
}

} /* ~namespace Butterworth */
} /* ~namespace DSP */

#endif /* DSP_BUTTERWORTH_H */
