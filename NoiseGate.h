/*
	NoiseGate.h
	
	Copyright 2011 Tim Goetze <tim@quitte.de>
	
	http://quitte.de/dsp/

	Noise reduction measures.

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

#ifndef _NOISE_GATE_H_
#define _NOISE_GATE_H_

#include "dsp/util.h"
#include "dsp/BiQuad.h"

#include "dsp/BiQuad.h"
#include "dsp/RBJ.h"
#include "dsp/RMS.h"
#include "dsp/Delay.h"
#include "dsp/OnePole.h"

class NoiseGate
: public Plugin
{
	public:
		int N;
		float over_N;
		DSP::RMS<2048> rms;
		uint remain;
		struct {
			sample_t current, delta, quiet;
		} gain;

		float f_mains;
		DSP::BiQuad<sample_t> humfilter[2];

		template <yield_func_t F>
			void cycle (uint frames);

	public:
		static PortInfo port_info[];

		void init();
		void activate();

		inline void store (sample_t x)
			{
				sample_t y;
				y = humfilter[0].process(x);
				y = humfilter[1].process(y);
				rms.store (x - .3 * y);
			}

		void run (uint n) { cycle<store_func> (n); }
		void run_adding (uint n) { cycle<adding_func> (n); }
};

#endif /* _NOISE_GATE_H_ */
