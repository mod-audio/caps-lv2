/*
	Cabinet.h
	
	Copyright 2002-14 Tim Goetze <tim@quitte.de>
	
	http://quitte.de/dsp/

	CabinetIV - IIR/FIR combination filters for cabinet emulation, following
	Bank's parfilt approach (warped Prony), see

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

#ifndef CABINET_H
#define CABINET_H

#include "dsp/util.h"
#include "dsp/Oversampler.h"
#include "dsp/v4f.h"
#include "dsp/v4f_FIR.h"
#include "dsp/v4f_IIR2.h"

typedef double cabinet_float;

typedef struct {
	sample_t gain;
	cabinet_float a[32], b[32];
} Model32;

class CabinetIII
: public Plugin
{
	public:
		sample_t gain;

		static Model32 allmodels[];

		Model32 * models;
		int model;
		void switch_model (int m);

		int h;
		cabinet_float * a, * b;
		cabinet_float x[32], y[32];
		
		void cycle (uint frames);

	public:
		static PortInfo port_info [];

		void init();
		void activate();
};

/* /////////////////////////////////////////////////////////////////////// */

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
		DSP::IIR2v4Bank<N> bank;
		DSP::FIRv4<FIR> fir;

		double gain;

		void cycle (uint frames);
		template <class O, int Ratio> void subcycle (uint frames, O & Over);

	public:
		static PortInfo port_info [];

		void init();
		void activate();
};

#endif /* CABINET_H */
