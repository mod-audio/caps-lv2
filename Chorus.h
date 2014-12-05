/*
	Chorus.h
	
	Copyright 2004-13 Tim Goetze <tim@quitte.de>
	
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

#ifndef CHORUS_H
#define CHORUS_H

#include "dsp/Sine.h"
#include "dsp/Roessler.h"
#include "dsp/Lorenz.h"
#include "dsp/Delay.h"
#include "dsp/IIR1.h"
#include "dsp/IIR2.h"
#include "dsp/RBJ.h"

class ChorusI
: public Plugin
{
	public:
		DSP::HP1<sample_t> hp;
		float time, width, rate;

		struct { DSP::Sine sine; } lfo;

		DSP::Delay delay;

		void cycle(uint frames);
	
		void setrate(float r);

	public:
		static PortInfo port_info [];

		void init()
			{
				rate = .15;
				lfo.sine.set_f (rate, fs, 0);
				delay.init ((int) (.050 * fs));
			}

		void activate();
		
};

#endif /* CHORUS_H */
