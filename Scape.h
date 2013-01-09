/*
	Scape.h
	
	Copyright 2004-5 Tim Goetze <tim@quitte.de>
	
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

#ifndef _SCAPE_H_
#define _SCAPE_H_

#include "dsp/Sine.h"
#include "dsp/Roessler.h"
#include "dsp/Lorenz.h"
#include "dsp/Delay.h"
#include "dsp/OnePole.h"
#include "dsp/BiQuad.h"
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
			DSP::OnePoleLP<sample_t> lp;
		} lfo[2];

		DSP::Delay delay;
		DSP::SVFI<1> svf[4];
		DSP::OnePoleHP<float> hipass[4];

		template <yield_func_t>
			void cycle (int frames);
	
	public:
		static PortInfo port_info [];

		void init();
		void activate();

		void run (int n) { cycle<store_func> (n); }
		void run_adding (int n) { cycle<adding_func> (n); }
};

#endif /* _SCAPE_H_ */
