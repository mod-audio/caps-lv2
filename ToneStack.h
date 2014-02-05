/*
	ToneStack.h
	
	Copyright 2006-7
		David Yeh <dtyeh@ccrma.stanford.edu> 
		Tim Goetze <tim@quitte.de> (cosmetics)
	
	Tone Stack emulation.

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

#ifndef TONESTACK_H
#define TONESTACK_H

#include "dsp/util.h"
#include "dsp/windows.h"
#include "dsp/ToneStack.h"

class ToneStack 
: public Plugin
{
	private:
		int model;

		DSP::ToneStack tonestack;
		DSP::ToneStackLT tonestacklt;

		template <yield_func_t F>
			void cycle (uint frames);

	public:
		static PortInfo port_info [];

		void init()
			{ 
				tonestack.init (fs);
				tonestacklt.init (fs);
			}

		void activate();

		void run (uint n) { cycle<store_func> (n); }
		void run_adding (uint n) { cycle<adding_func> (n); }
};

#endif /* TONESTACK_H */
