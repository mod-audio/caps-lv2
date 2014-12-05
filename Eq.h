/*
	Eq.h
	
	Copyright 2004-13 Tim Goetze <tim@quitte.de>
	
	http://quitte.de/dsp/

	IIR equalisation filters.

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

#ifndef EQ_H
#define EQ_H

#include "dsp/util.h"
#include "dsp/Eq.h"
#include "dsp/IIR2.h"
#include "dsp/RBJ.h"
#include "dsp/v4f.h"
#include "dsp/v4f_IIR2.h"

/* octave-band variants, mono and stereo */
class Eq10
: public Plugin
{
	public:
		sample_t gain[10];
		DSP::Eq<10> eq;

		int block;
			enum { BlockSize = 64 };

		void cycle (uint frames);

	public:
		static PortInfo port_info [];

		void init();
		void activate();
};

class Eq10X2
: public Plugin
{
	public:
		sample_t gain[10];
		DSP::Eq<10> eq[2];

		void cycle (uint frames);

	public:
		static PortInfo port_info [];

		void init();
		void activate();
};

/* 4-way parametric, parallel implementation */
class Eq4p
: public Plugin
{
	public:
		struct {sample_t mode,gain,f,Q;} state[4]; /* parameters */

		DSP::IIR2v4 filter[2];

		bool xfade;
		void updatestate();

		void cycle (uint frames);

	public:
		static PortInfo port_info [];

		void init();
		void activate();
};

/* 4-way parametric, parallel implementation */
class EqFA4p
: public Plugin
{
	public:
		struct {sample_t mode,gain,f,bw;} state[4]; /* parameters */

		DSP::MREqv4 filter[2];

		bool xfade;
		void updatestate();
		sample_t gain;

		void cycle (uint frames);

	public:
		static PortInfo port_info [];

		void init();
		void activate();
};

#endif /* EQ_H */
