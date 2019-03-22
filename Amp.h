/*
	Amp.h
	
	Copyright 2002-14 Tim Goetze <tim@quitte.de>
	
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

#ifndef AMP_H
#define AMP_H

#include "dsp/util.h"
#include "dsp/IIR1.h"
#include "dsp/IIR2.h"

#include "dsp/Oversampler.h"

#include "dsp/RBJ.h"

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

		DSP::IIR2<sample_t> lp, biaslp;
		DSP::HP1b<sample_t> hp1, dc1, dc2; /* dc blockers */

		int model;
		DSP::ToneStack tonestack;

		uint remain;
		DSP::CompressRMS compress;

		void setratio (int r);

		void cycle (uint frames);
		template <class Over> void subcycle (uint frames, Over & over);

	public:
		static PortInfo port_info[];

		void init();
		void activate();
};

#endif /* AMP_H */
