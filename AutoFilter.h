/*
	AutoFilter.h
	
	Copyright 2004-12 Tim Goetze <tim@quitte.de>
	
	http://quitte.de/dsp/

	AutoFilter - selectable SVF modulated by a Lorenz attractor and
	the input signal envelope.

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

#ifndef _AUTO_FILTER_H_
#define _AUTO_FILTER_H_

#include "dsp/SVF.h"

#include "dsp/Lorenz.h"
#include "dsp/Roessler.h"

#include "dsp/RMS.h"
#include "dsp/IIR2.h"
#include "dsp/IIR1.h"

class AutoFilter
: public Plugin
{
	public:
		uint blocksize;

		sample_t f, Q;

		DSP::StackedSVF< 1, DSP::SVFI<2> > svf1;
		DSP::StackedSVF< 2, DSP::SVFII > svf2;

		DSP::Lorenz lorenz;

		/* rms calculation and smoothing */
		DSP::HP1<sample_t> hp;
		DSP::RMS<128> rms;
		DSP::IIR2<sample_t> smoothenv; 

		void cycle (uint frames);

	public:
		static PortInfo port_info [];

		void init();
		void activate();
};

#endif /* _AUTO_FILTER_H_ */
