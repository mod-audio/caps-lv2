/*
	dsp/complex.h
	
	Copyright 2003-12 tim goetze <tim@quitte.de>

	http://quitte.de/dsp/

	complex algebra

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

#ifndef COMPLEX_H
#define COMPLEX_H

namespace DSP {
	
class complex
{ 
	public:
		double re, im;
		
    complex() { }	
		
    complex (double r, double i=0) { re = r; im = i; }
		void operator = (double r) { re = r; im = 0; }

		double abs() { return sqrt(_squared()); }
		double _squared() { return re*re + im*im; }

		static inline complex polar (double phi, double mag=1)
			{ return complex (mag*cos(phi), mag*sin(phi)); }

		inline complex exp()
			{
				double r = ::exp(re);
				return complex (r*cos(im), r*sin(im));
			}
		inline complex conj()
			{ return complex (re,-im); }
};

inline complex 
operator * (double a, complex z)
{
	z.re *= a; 
	z.im *= a;

	return z;
}

inline complex 
operator * (complex z1, complex z2)
{
	return complex (
			z1.re * z2.re - z1.im * z2.im,
			z1.re * z2.im + z1.im * z2.re);
}

inline complex 
operator / (complex z1, complex z2)
{
	double m = z2.re * z2.re + z2.im * z2.im;
	return complex (
			((z1.re * z2.re) + (z1.im * z2.im)) / m,
			((z1.re * z2.im) - (z1.im * z2.re)) / m);
}
						
inline complex 
operator / (complex z, double a)
{ 
	z.re /= a; 
	z.im /= a;

	return z;
}

inline void 
operator /= (complex &z, double a)
{ 
	z = z / a;
}

inline complex 
operator + (complex z1, complex z2)
{ 
	z1.re += z2.re;
	z1.im += z2.im;
	
	return z1;
}

inline complex 
operator - (complex z1, complex z2)
{ 
	z1.re -= z2.re;
	z1.im -= z2.im;
	
	return z1;
}

inline complex 
operator - (complex z)
{ 
	return 0.0 - z;
}

/* */
inline complex 
expj (double theta)
{
	return complex (cos (theta), sin (theta));
}

inline double 
hypot (complex z)
{
	return ::hypot (z.im, z.re);
}

} /* namespace DSP */

#endif /* COMPLEX_H */
