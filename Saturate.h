/*
	Saturate.h
	
	Copyright 2004-16 Tim Goetze <tim@quitte.de>
	
	http://quitte.de/dsp/

	Oversampled waveshaper and an exciter-like circuit.

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

#ifndef SATURATE_H
#define SATURATE_H

#include "dsp/util.h"
#include "dsp/Oversampler.h"
#include "dsp/RMS.h"
#include "dsp/Compress.h"
#include "dsp/IIR1.h"
#include "dsp/IIR2.h"
#include "dsp/Butterworth.h"
#include "dsp/Delay.h"
#include "dsp/ChebyshevPoly.h"

class Saturate
: public Plugin
{
	public:
		struct {
			float linear, delta;
		} gain;
		float bias;

		DSP::HP1<sample_t> hp;

		/* use <128,1024> for cleaner output */
		DSP::Oversampler<8,64> over;

		void cycle (uint frames);
		template <clip_func_t C> void subcycle (uint frames);

	public:
		static PortInfo port_info[];

		void init();
		void activate();
};

/* stacked Butterworth crossover (Linkwitz-Riley) */
struct Splitter
{
	DSP::IIR2<sample_t> lp[2], hp[2];
	float f;

	void reset()
		{ f=0; lp[0].reset(); lp[1].reset(); hp[0].reset(); hp[1].reset(); }
	void set_f (float _f)
		{
			DSP::Butterworth::LP (_f,lp[0]);
			DSP::Butterworth::LP (_f,lp[1]);
			f = _f;
			DSP::Butterworth::HP (f,hp[0]);
			DSP::Butterworth::HP (f,hp[1]);
		}
	sample_t low (sample_t x) 
		{ return lp[1].process(lp[0].process(x)); }
	sample_t high (sample_t x) 
		{ return hp[1].process(hp[0].process(x)); }
};

class Spice
: public Plugin
{
	public:
		Splitter split[2];
		DSP::IIR2<sample_t> shape[2];
		struct {float lo, hi;} vol;

		DSP::ChebPoly<5> cheby; 

		uint remain;
		DSP::CompressPeak compress;

		void cycle (uint frames);

	public:
		static PortInfo port_info[];

		void init();
		void activate();
};

class SpiceX2
: public Plugin
{
	public:
		struct {
			Splitter split[2];
			DSP::IIR2<sample_t> shape[2];
		} chan[2];
		struct {float lo, hi;} vol;

		DSP::ChebPoly<5> cheby; 

		uint remain;
		DSP::CompressPeak compress;

		void cycle (uint frames);

	public:
		static PortInfo port_info[];

		void init();
		void activate();
};

#endif /* SATURATE_H */
