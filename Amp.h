/*
	Amp.h
	
	Copyright 2002-12 Tim Goetze <tim@quitte.de>
	
	http://quitte.de/dsp/

	Guitar amplification.

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

#ifndef _AMP_H_
#define _AMP_H_

#include "dsp/util.h"
#include "dsp/OnePole.h"
#include "dsp/BiQuad.h"

#include "dsp/Oversampler.h"

#include "dsp/RBJ.h"
#include "dsp/Eq.h"

#include "dsp/ToneStack.h"
#include "dsp/Compress.h"

/* TODO: mega wookie bands 80 240 750 2200 6600 */

class AmpVTS
: public Plugin
{
	public:
		int ratio;
		DSP::Oversampler<2,32> over2;
		DSP::Oversampler<4,32> over4;
		DSP::Oversampler<8,64> over8;

		DSP::BiQuad<sample_t> hp, lp, biaslp;
		DSP::OnePoleHP<sample_t> dcblock;

		int model;
		DSP::ToneStack tonestack;

		uint remain;
		DSP::CompressRMS compress;

		void setratio (int r);

		template <yield_func_t F> void cycle (uint frames);
		template <yield_func_t F, class Over> void subcycle (uint frames, Over & over);

		float bias;

	public:
		static PortInfo port_info[];

		void init();
		void activate();

		void run (uint n) { cycle<store_func> (n); }
		void run_adding (uint n) { cycle<adding_func> (n); }
};

#endif /* _AMP_H_ */
