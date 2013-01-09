/*
	Cabinet.h
	
	Copyright 2002-13 Tim Goetze <tim@quitte.de>
	
	http://quitte.de/dsp/

	CabinetII - 32nd order IIR filters modeled after existing impulse responses
	for 44.1 / 48 / 88.2 / 96 kHz sample rates, switched at runtime.

	CabinetIV - IIR/FIR combination filters for cabinet emulation, following
	Bank's parfilt approach, see

		Balázs Bank, 
		"Direct Design of Parallel Second-Order Filters for Instrument Body Modeling",
		Proc. of the Int. Computer Music Conference (ICMC07) 
		Copenhagen, Denmark, Aug. 2007, vol. I., pp. 458-465.

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

#ifndef _CABINET_H_
#define _CABINET_H_

#include "dsp/util.h"
#include "dsp/BiQuad.h"
#include "dsp/Oversampler.h"
#include "dsp/v4f.h"
#include "dsp/v4f_FIR.h"
#include "dsp/v4f_BiQuad.h"

typedef double cabinet_float;

typedef struct {
	int n;
	cabinet_float a[32], b[32];
	float gain;
} Model32;

/* Second version with 32nd order filters precalculated for
 * 44.1 / 48 / 88.2 / 96 kHz sample rates */

class CabinetII
: public Plugin
{
	public:
		sample_t gain;

		static Model32 models44100 [];
		static Model32 models48000 [];
		static Model32 models88200 [];
		static Model32 models96000 [];

		Model32 * models;
		int model;
		void switch_model (int m);

		int n, h;
		cabinet_float * a, * b;
		cabinet_float x[32], y[32];
		
		template <yield_func_t F>
			void cycle (uint frames);

	public:
		static PortInfo port_info [];

		void init();
		void activate();

		void run (uint n) { cycle<store_func> (n); }
		void run_adding (uint n) { cycle<adding_func> (n); }
};

/* /////////////////////////////////////////////////////////////////////// */

/* parfilt-based cabinet: banks of parallel 2nd-order filters */

template <uint N, uint FIR>
class ParModel {
	public:
		float gain;
		float a1[4*N], a2[4*N], b1[4*N], b2[4*N];
		float fir[FIR];
};

class CabinetIV
: public Plugin
{
	public:
		uint remain; /* when oversampling, an oversampling cycle may be incomplete */

		int over; /* oversampling ratio */
		DSP::Oversampler<2,32> over2;
		DSP::Oversampler<4,64> over4;

		int model;
		void switch_model (int m);

		enum { 
			N = 64/4, /* number of bands/4 */
			FIR = 128 /* FIR filter taps */
		}; 
		DSP::BiQuad4fBank<N> bank;
		DSP::FIR4f<FIR> fir;

		double gain;

		template <yield_func_t F>
				void cycle (uint frames);
		template <yield_func_t F, class O, int Ratio>
				void cycle (uint frames, O & Over);

	public:
		static PortInfo port_info [];

		void init();
		void activate();

		void run (uint n) { cycle<store_func> (n); }
		void run_adding (uint n) { cycle<adding_func> (n); }
};

#endif /* _CABINET_H_ */
