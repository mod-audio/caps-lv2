/*
	dsp/polynomials.h
	
	Copyright 2012 Tim Goetze <tim@quitte.de>
	
	http://quitte.de/dsp/

	Prototypes for polynoms.cc

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

#ifndef DSP_POLYNOMIALS_H
#define DSP_POLYNOMIALS_H

namespace DSP {
namespace Polynomial {

extern float power_clip_7 (float x);
extern float power_clip_11 (float x);

extern float atan (float x);
extern float atan1 (float x);
extern float atan15 (float x);
extern float one5 (float x);
extern float sin1 (float x);
extern float clip3 (float x);
extern float clip9 (float x);
extern float one53 (float x);
extern float tanh (float x);

}; /* namespace Polynomial */
}; /* namespace DSP */


#endif /* DSP_POLYNOMIALS_H */
