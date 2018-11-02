/*
	Scape.h
	
	Copyright 2004-16 Tim Goetze <tim@quitte.de>
	
	http://quitte.de/dsp/

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

#ifndef SCAPE_H
#define SCAPE_H

#include "dsp/Sine.h"
#include "dsp/Roessler.h"
#include "dsp/Lorenz.h"
#include "dsp/Delay.h"
#include "dsp/IIR1.h"
#include "dsp/IIR2.h"
#include "dsp/RBJ.h"
#include "dsp/SVF.h"

class Scape
: public Plugin
{
	public:
		sample_t time, fb;
		double period;

		struct {
			DSP::Lorenz lorenz;
			DSP::LP1<sample_t> lp;
			sample_t get() { return lp.process(lorenz.get()); }
		} lfo[2];

		DSP::Delay delay;
		DSP::SVFI<1> svf[4];
		DSP::HP1<float> hipass[4];

		void cycle (uint frames);
	
	public:
		static PortInfo port_info [];

		void init();
		void activate();
};

class DDDelay
: public Plugin
{
	public:
		struct {
			DSP::Delay delay;
			DSP::LP1<sample_t> lp;
		} step[4];
		void cycle (uint frames);

	public:
		static PortInfo port_info [];

		void init();
		void activate();
};

#endif /* SCAPE_H */
