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
#include "dsp/OnePole.h"
#include "dsp/BiQuad.h"
#include "dsp/RBJ.h"

class ChorusI
: public Plugin
{
	public:
		DSP::OnePoleHP<sample_t> hp;
		float time, width, rate;

		struct { DSP::Sine sine; } lfo;

		DSP::Delay delay;

		template <yield_func_t>
				void one_cycle (int frames);
	
		void setrate (float r);

	public:
		static PortInfo port_info [];

		void init()
			{
				rate = .15;
				lfo.sine.set_f (rate, fs, 0);
				delay.init ((int) (.050 * fs));
			}

		void activate();

		void run (int n)
			{ one_cycle<store_func> (n); }
		
		void run_adding (int n)
			{ one_cycle<adding_func> (n); }
};

/* ///////////////////////////////////////////////////////////////////////// */

#if 0
#define FRACTAL_RATE 0.004

class FracTap 
{
	public:
		DSP::Roessler lfo;
		DSP::OnePoleLP<sample_t> lp;
		
		void init (double fs)
			{	
				lp.set_f (1/fs); 
				lfo.init (.001, frandom());
			}
		
		void set_rate (sample_t r)
			{
				lfo.set_rate (r);
			}

		/* t = time, w = width, should inline nicely */
		sample_t get (DSP::Delay & d, double t, double w)
			{
				double m = lp.process (lfo.get());
				return d.get_cubic (t + w * m);
			}
};

class ChorusII
: public ChorusStub
{
	public:
		enum {
			Taps = 3
		};

		FracTap taps[Taps];
		DSP::Delay delay;

		template <yield_func_t>
			void cycle (uint frames);
	
		void set_rate (sample_t r)
			{
				rate = r;
				r *= FRACTAL_RATE*44100*over_fs;
				for (int i = 0; i < Taps; ++i)
				{
					taps[i].set_rate (r);
					r *= 1.2;
				}
			}

	public:
		static PortInfo port_info [];

		void init()
			{
				hp.set_f (350*over_fs);
				delay.init ((int) (.025*fs));
				for (int i = 0; i < Taps; ++i)
					taps[i].init (fs);
			}

		void activate()
			{
				time = 0;
				width = 0;
				
				set_rate (*ports[3]);
				
				hp.reset();
				delay.reset();
			}

		void run (uint n) { cycle<store_func> (n); }
		void run_adding (uint n) { cycle<adding_func> (n); }
};

#endif
#endif /* CHORUS_H */
