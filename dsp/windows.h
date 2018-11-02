/*
	dsp/windows.h
	
	Copyright 2004-14 Tim Goetze <tim@quitte.de>
	
	http://quitte.de/dsp/

	Window functions.

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

#ifndef DSP_WINDOWS_H 
#define DSP_WINDOWS_H

#include "Sine.h"

namespace DSP {
	
/* prototypes for window value application ... */
typedef void (*window_sample_func_t) (sample_t &, sample_t);

/* ... which go as template parameters for the window calculation below */
inline void store_sample (sample_t &d, sample_t s) { d = s; }
inline void apply_window (sample_t &d, sample_t s) { d *= s; }

template <window_sample_func_t F>
void
hann (sample_t * s, int n, double step = 1)
{
	step = M_PI*step/n;
	
	double f = 0;
	for (int i=0; i<n; ++i)
	{
		f = i*step;
		float x = sin(f);
		F(s[i], x*x);
	}
}

/* faster but less accurate version */
template <window_sample_func_t F>
void
hann2 (sample_t *s, int n)
{
	double phi = M_PI/(n-1);
	DSP::Sine sin(phi, 0);
	for (int i=0; i<n; ++i)
	{
		float x = sin.get();
		F(s[i], x*x);
	}
}


template <window_sample_func_t F>
void
hamming (sample_t * s, int n)
{
	float in = 1. / n;
	
	for (int i = 0; i < n; ++i)
	{
		 double f = i*in;
		F (s[i], .54 - .46*cos (2*M_PI*f));
	}
}

template <window_sample_func_t F>
void
blackman (sample_t *s, int n)
{
	float in = 1. / n;

	for (int i = 0; i < n; ++i)
	{
		 float f = (float) i;

		 double b = .42f - 
						.5f*cos (2.f*f*M_PI*in) + 
						.08*cos (4.f*f*M_PI*in);

		F (s[i], b);
	}
}

template <window_sample_func_t F>
void
blackman_harris (sample_t *s, int n)
{
	double w1 = 2.f*M_PI / (n - 1);
	double w2 = 2.f*w1;
	double w3 = 3.f*w1;

	for (int i = 0; i < n; ++i)
	{
		 double f = (double) i;

		 double bh = .35875f - 
				.48829f*cos (w1*f) + 
				.14128f*cos (w2*f) - 
				.01168f*cos (w3*f);

		F (s[i], bh);
	}
}

/* by way of dobson and csound */
inline double 
besseli (double x)
{
	double a = fabs(x);
	if (a < 3.75)     
	{
		double y = x/3.75;
		y *= y;
		return 1. + y*(3.5156229 + y*(3.0899424 + y*(1.2067492 +
					y*(.2659732 + y*(.0360768 + y*.0045813)))));
	}
	double y = 3.75/a;
	return (exp(a)/sqrt(a)) * (.39894228 + y*(.01328592 + 
			y*(.00225319 + y*(-.00157565 + y*(.00916281 + y*(-.02057706 + 
			y*(.02635537 + y*(-.01647633 + y*.00392377))))))));
}

/* step = .5 : window [-n to 0] */
template <window_sample_func_t F>
void
kaiser (sample_t *s, int n, double beta, double step = 1)
{
	double bb = besseli(beta);
	int si = 0;

	for(double i = -n/2.+.5; si < n; ++si, i += step)
	{
		double a = 1 - pow((2*i / (n - 1)), 2);
		double k = besseli((beta*(a < 0 ? 0 : sqrt(a))) / bb);
		F(s[si], k);
	}
}

template <window_sample_func_t F>
void
xfade (sample_t *s, int n, int dir) /* dir [-1,1] */
{
	DSP::Sine cos(.5*M_PI/n, dir>0 ? .5*M_PI : 0);
	for (int i=0; i<n; ++i)
	{
		float c = cos.get();
		F(s[i], c*c);
	}
}

} /* namespace DSP */

#endif /* DSP_WINDOWS_H */
