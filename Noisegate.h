/*
	Noisegate.h
	
	Copyright 2011-13 Tim Goetze <tim@quitte.de>
	
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

#ifndef NOISE_GATE_H
#define NOISE_GATE_H

#include "dsp/util.h"
#include "dsp/IIR2.h"

#include "dsp/RBJ.h"
#include "dsp/RMS.h"
#include "dsp/Delay.h"
#include "dsp/IIR1.h"

class Noisegate
: public Plugin
{
	public:
		int N;
		float over_N;
		DSP::RMS<8192> rms;
		uint remain;
		struct {
			float current, delta, quiet;
			DSP::LP1<sample_t> lp;
			float get()
				{
					current += delta;
					/* this filter needs no normal addition because its input is always 
					 * at least the closed-gate gain (currently -60 dB) */
					return lp.process(current);
				}
		} gain;
		struct {
			uint age, threshold;
		} hysteresis;

		float f_mains;
		DSP::IIR2<sample_t> humfilter[2];

		void cycle (uint frames);

	public:
		static PortInfo port_info[];

		void init();
		void activate();

		void process (sample_t x); /* pre- and humfilter, store to rms */
};

#endif /* NOISE_GATE_H */
