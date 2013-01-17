/*
	Phaser.h
	
	Copyright 2002-12 Tim Goetze <tim@quitte.de>
	
	http://quitte.de/dsp/

	Fractal-modulated phaser units.

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

#ifndef _PHASER_H_
#define _PHASER_H_

#include "dsp/Sine.h"
#include "dsp/Lorenz.h"
#include "dsp/Delay.h"
#include "dsp/OnePole.h"

class PhaserAP
{
	public:
		sample_t a, m;
		
		PhaserAP() { a=m=0.; }

		void set (sample_t delay)
			{ a = (1-delay) / (1+delay); } 

		sample_t process (sample_t x)
			{
				sample_t y = -a*x + m;
				m = a*y + x;
				return y;
			}
};

class PhaserII
: public Plugin
{
	public:
		enum {Notches = 12};
		PhaserAP ap[Notches];
		DSP::Lorenz lorenz;
		DSP::OnePoleLP<sample_t> lfo_lp;

		sample_t rate;
		sample_t y0;

		struct {
			double bottom, range;
		} delay;

		template <yield_func_t>
			void cycle (uint frames);
	
		uint blocksize, remain;

	public:
		static PortInfo port_info [];

		void init()
			{
				blocksize = 16;
				if (fs > 32000) blocksize *= 2;
				if (fs > 64000) blocksize *= 2;
				if (fs > 128000) blocksize *= 2;
				lorenz.init();
			}

		void activate()
			{
				y0 = 0.;
				remain = 0;

				delay.bottom = 400*over_fs;
				delay.range = 2200*over_fs;

				lfo_lp.set_f(.000001*over_fs);
				rate = -1; /* force lfo reset in cycle() */
			}

		void run (uint n) { cycle<store_func> (n); }
		void run_adding (uint n) { cycle<adding_func> (n); }
};

class StereoPhaserII
: public Plugin
{
	public:
		enum {Notches = 12};
		PhaserAP apl[Notches], apr[Notches];
		DSP::Lorenz lorenz;
		DSP::OnePoleLP<sample_t> lfo_lp;

		sample_t y0l, y0r;

		struct {
			double bottom, range;
		} delay;

		template <yield_func_t, bool StereoIn>
			void cycle (uint frames);
	
		uint blocksize, remain;

	public:
		static PortInfo port_info [];

		void init()
			{
				blocksize = 16;
				if (fs > 32000) blocksize *= 2;
				if (fs > 64000) blocksize *= 2;
				if (fs > 128000) blocksize *= 2;
				lorenz.init();
			}

		void activate()
			{
				y0l = y0r = 0;
				remain = 0;

				delay.bottom = 400*over_fs;
				delay.range = 2200*over_fs;

				lfo_lp.set_f(.000001*over_fs);
			}

		void run (uint n) { cycle<store_func, false> (n); }
		void run_adding (uint n) { cycle<adding_func, false> (n); }
};

class StereoPhaserII2x2
: public StereoPhaserII
{
	public:
		static PortInfo port_info [];

		void run (uint n) { cycle<store_func, true> (n); }
		void run_adding (uint n) { cycle<adding_func, true> (n); }
};

#endif /* _PHASER_H_ */
