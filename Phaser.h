/*
	Phaser.h
	
	Copyright 2002-13 Tim Goetze <tim@quitte.de>
	
	http://quitte.de/dsp/

	Phaser effect plugin.

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

#ifndef PHASER_H
#define PHASER_H

#include "dsp/Sine.h"
#include "dsp/Roessler.h"
#include "dsp/Delay.h"
#include "dsp/IIR1.h"

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
		struct {
			DSP::Sine sine;
			DSP::Roessler roessler;
			DSP::LP1<sample_t> lp;
		} lfo;

		sample_t rate;
		sample_t y0;

		struct {
			double bottom, range;
		} delay;

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

				lfo.roessler.init();
				lfo.sine.set_f (300*over_fs,0);
			}

		void activate()
			{
				y0 = 0.;
				remain = 0;

				delay.bottom = 400*over_fs;
				delay.range = 2200*over_fs;
			}
};

#endif /* PHASER_H */
