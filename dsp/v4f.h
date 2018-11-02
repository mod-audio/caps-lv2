/*
	v4f.h
	
	Copyright 2011-14 Tim Goetze <tim@quitte.de>
	
	http://quitte.de/dsp/

	Vectors of four 32-bit floats for SIMD arithmetic, gcc style.

	Implementation is evolving so the code is not maximally elegant yet, sorry.

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

#ifndef DSP_V4F_H
#define DSP_V4F_H

#ifdef __SSE__
#include <xmmintrin.h>
#endif

/* caution, gcc apparently doesnt always honour the alignment: segfault */
typedef float v4f_t __attribute__ ((vector_size(16), aligned(16))); 

inline v4f_t v4f (float x) 
	{ v4f_t v = {x,x,x,x}; return v; }
inline v4f_t v4f (float x0, float x1, float x2, float x3) 
	{ v4f_t v = {x0,x1,x2,x3}; return v; }
inline v4f_t v4f (float *x)
	{ v4f_t v = {x[0],x[1],x[2],x[3]}; return v; }


#define v4fa(x) ((float *) &x)

/* gcc's __builtin_shuffle is useless */
#ifdef __SSE__
#define v4f_shuffle(x,s3,s2,s1,s0) \
	_mm_shuffle_ps(x,x,((s0)<<6|(s1)<<4|(s2)<<2|s3))
#define v4f_shuffle2(x,y,s3,s2,s1,s0) \
	_mm_shuffle_ps(x,y,((s0)<<6|(s1)<<4|(s2)<<2|s3))
#define v4f_max(x,y) _mm_max_ps(x,y)
#define v4f_min(x,y) _mm_min_ps(x,y)
#else
inline v4f_t v4f_shuffle(v4f_t x, int s3, int s2, int s1, int s0) 
{
	v4f_t y=x; 
	float*px=(float*)&x; float*py=(float*)&y; 
	px[3]=py[s0]; px[2]=py[s1]; px[1]=py[s2]; px[0]=py[s3];
	return x;
}
#endif

inline float v4f_sum (v4f_t v)
{
	float * f = (float *) &v;
	return f[0]+f[1]+f[2]+f[3];
}

/* mapping a float to float function [e.g. sinf() e.a.] to a vector */
typedef float (*f2f_fn) (float f);

template <f2f_fn fn>
v4f_t v4f_map (v4f_t x)
{
	v4f_t y;
	float * s = (float *) &x;
	float * d = (float *) &y;
	for (uint i = 0; i < 4; ++i)
		d[i] = fn(s[i]);
	return y;
}

#define v4f_0 ((v4f_t) {0, 0, 0, 0})
#define v4f_half ((v4f_t) {.5, .5, .5, .5})
#define v4f_1 ((v4f_t) {1, 1, 1, 1})
#define v4f_2 ((v4f_t) {2, 2, 2, 2})
#define v4f_halfpi ((v4f_t) {.5*M_PI, .5*M_PI, .5*M_PI, .5*M_PI})
#define v4f_pi ((v4f_t) {M_PI, M_PI, M_PI, M_PI})
#define v4f_2pi ((v4f_t) {2*M_PI, 2*M_PI, 2*M_PI, 2*M_PI})

template <int N>
class V4fArray
{
	private:
		/* sufficient space to align actual array to 16-byte boundary */
		char _data [(N+1) * sizeof(v4f_t)];

	public:
		v4f_t * v;

		V4fArray() 
			{
				uint64 p = (uint64) ((uint64) _data + 16) & ~15ll;
				v = (v4f_t*) p;
			}

		void reset() { memset (_data, 0, sizeof (_data)); }

		v4f_t & operator [] (int i) {return v[i];}
};

class V4fData
{
	private:
		void * _data;

	public:
		v4f_t * v;

		V4fData() {_data = 0;}
		~V4fData() {free(_data);}

		operator v4f_t * () {return v;}
		operator void * () {return (void *) v;}

		void init (int N)
			{
				_data = calloc(N+1, sizeof(v4f_t));
				uint64 p = (16 + (uint64) _data) & ~15ll;
				v = (v4f_t*) p;
			}
};

/* four parallel sine oscillators */
class Sin4f
{
	public:
		char _data [4 * sizeof (v4f_t)];
		int z;

	public:
		/* get 16-byte aligned data base */
		inline v4f_t * data()
			{
				uint64 p = ((uint64) _data + 16) & ~15ll;
				return (v4f_t *) p;
			}

		inline void set_f (v4f_t f)
			{
				v4f_t *y = data();
				v4f_t w = -v4f_pi * f;
				y[0] = v4f_map<__builtin_sinf> (w);
				y[1] = v4f_map<__builtin_sinf> (v4f_2 * w);
				/* b in above scalar implementation is y[2] in the flat data */
				y[2] = v4f_2 * v4f_map<__builtin_cosf> (w); /* b */
				z = 0;
			}

		inline v4f_t get()
			{
				v4f_t *y = data();
				register v4f_t s = y[2] * y[z]; 
				z ^= 1;
				s -= y[z];
				return y[z] = s;
			}
};

#endif /* DSP_V4F_H */
