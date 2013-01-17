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

#ifndef _CHORUS_H_
#define _CHORUS_H_

#include "dsp/Sine.h"
#include "dsp/Roessler.h"
#include "dsp/Lorenz.h"
#include "dsp/Delay.h"
#include "dsp/OnePole.h"
#include "dsp/BiQuad.h"
#include "dsp/RBJ.h"

class ChorusStub
: public Plugin
{
	public:
		DSP::OnePoleHP<sample_t> hp;
		sample_t time, width, rate;
};

#define FRACTAL_RATE 0.004

class FracTap 
{
	public:
		DSP::Roessler lfo;
		DSP::OnePoleLP<sample_t> lp;
		
		void init (double fs)
			{	
				lp.set_f (.0001/fs); 
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

class ChorusI
: public ChorusStub
{
	public:
		DSP::Sine lfo;
		DSP::Delay delay;
		DSP::DelayTapA tap;

		template <yield_func_t>
				void one_cycle (int frames);
	
	public:
		static PortInfo port_info [];

		void init()
			{
				rate = .15;
				delay.init ((int) (.040 * fs));
			}

		void activate()
			{
				time = 0;
				width = 0;
				
				rate = *ports[3];
				
				delay.reset();
				tap.reset();

				lfo.set_f (rate, fs, 0);
			}

		void run (int n)
			{
				one_cycle<store_func> (n);
			}
		
		void run_adding (int n)
			{
				one_cycle<adding_func> (n);
			}
};

/* ///////////////////////////////////////////////////////////////////////// */

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

class StereoChorusII
: public ChorusStub
{
	public:
		sample_t rate;
		sample_t phase;

		DSP::Delay delay;

		struct {
			DSP::Roessler fractal;
			DSP::OnePoleLP<sample_t> lfo_lp;
			DSP::DelayTapA tap;
		} left, right;

		enum {Mono=0,Stereo=1};
		template <yield_func_t>
			void cycle (uint frames, int mode=Mono);
	
		void set_rate (sample_t r);

	public:
		static PortInfo port_info [];
		sample_t adding_gain;

		void init()
			{
				hp.set_f (350*over_fs);
				delay.init ((int) (.025 * fs));

				left.fractal.init (.001, frandom());
				right.fractal.init (.001, frandom());
			}

		void activate()
			{
				time = 0;
				width = 0;

				hp.reset();
				delay.reset();

				left.tap.reset();
				right.tap.reset();

				set_rate (*ports[3]);
			}

		void run (uint n) { cycle<store_func> (n,Mono); }
		void run_adding (uint n) { cycle<adding_func> (n,Mono); }
};

class StereoChorusII2x2
: public StereoChorusII
{
	public:
		static PortInfo port_info [];

		void run (uint n) { cycle<store_func> (n,Stereo); }
		void run_adding (uint n) { cycle<adding_func> (n,Stereo); }
};

#endif /* _CHORUS_H_ */
