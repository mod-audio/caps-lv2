/*
	Fractals.h
	
	Copyright 2004-11 Tim Goetze <tim@quitte.de>
	
	http://quitte.de/dsp/

	Lorenz and Roessler attractors made audible.

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

#ifndef FRACTALS_H
#define FRACTALS_H

#include "dsp/Lorenz.h"
#include "dsp/Roessler.h"
#include "dsp/IIR1.h"

class Fractal
: public Plugin
{
	public:
		sample_t h, gain;

		DSP::Lorenz lorenz;
		DSP::Roessler roessler;
		DSP::HP1<sample_t> hp; /* dc removal */

		void cycle (uint frames);
		template <int Mode> void subcycle (uint frames);

	public:
		static PortInfo port_info [];

		void init();
		void activate();
};

#endif /* FRACTALS_H */
