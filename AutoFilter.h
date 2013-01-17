/*
	AutoFilter.h
	
	Copyright 2004-12 Tim Goetze <tim@quitte.de>
	
	http://quitte.de/dsp/

	AutoFilter - selectable SVF modulated by a Lorenz attractor and
	the input signal envelope.

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

#ifndef _AUTO_FILTER_H_
#define _AUTO_FILTER_H_

#include "dsp/SVF.h"

#include "dsp/Lorenz.h"
#include "dsp/Roessler.h"

#include "dsp/RMS.h"
#include "dsp/BiQuad.h"
#include "dsp/OnePole.h"
#include "dsp/Oversampler.h"

#include "dsp/polynomials.h"

/* g++ does not want StackedSVF::process to be called with a template parameter
 * from within AutoFilter::subsubcycle so we hardcode it here and while we do,
 * we use the opportunity to diversify things a bit */
struct SVF1
: public DSP::StackedSVF <1, DSP::SVFI<2> >
{
	double gainfactor() { return 1.3; }
	sample_t process_clip (sample_t x, double g)
		{ return process<DSP::Polynomial::atan> (x,g); }
};

struct SVF2
: public DSP::StackedSVF <2, DSP::SVFII>
{
	double gainfactor() { return 1; }
	sample_t process_clip (sample_t x, double g)
		{ return process<DSP::Polynomial::atan1> (x,g); }
};

struct SVF3
: public DSP::StackedSVF <2, DSP::SVFII>
{
	double gainfactor() { return .4; }
	sample_t process_clip (sample_t x, double g)
		{ return process<DSP::Polynomial::tanh> (x,g); }
};

struct SVF4
: public DSP::StackedSVF <3, DSP::SVFII>
{
	double gainfactor() { return .9; }
	sample_t process_clip (sample_t x, double g)
		{ return process<DSP::Polynomial::atan1> (x,g); }
};

struct SVF5
: public DSP::StackedSVF <4, DSP::SVFII>
{
	double gainfactor() { return .9; }
	sample_t process_clip (sample_t x, double g)
		{ return process<DSP::Polynomial::atan1> (x,g); }
};

/* nn, newport, duet */
class AutoFilter
: public Plugin
{
	public:
		uint blocksize;

		sample_t f, Q;

		SVF1 svf1;
		SVF2 svf2;
		SVF3 svf3;
		SVF4 svf4;
		SVF5 svf5;

		DSP::Lorenz lorenz;

		/* rms calculation and smoothing */
		DSP::OnePoleHP<sample_t> hp;
		DSP::RMS<256> rms;
		DSP::BiQuad<sample_t> smoothenv; 

		struct {
			DSP::NoOversampler one;
			DSP::Oversampler<2,32> two;
			DSP::Oversampler<4,64> four;
			DSP::Oversampler<8,64> eight;
		} oversampler;

		template <yield_func_t F>
				void cycle (uint frames);
		template <yield_func_t F, class SVF>
				void subcycle (uint frames, SVF &);
		template <yield_func_t F, class SVF, class Over>
				void subsubcycle (uint frames, SVF & svf, Over & over);

	public:
		static PortInfo port_info [];

		void init();
		void activate();

		void run (uint n) { cycle<store_func> (n); }
		void run_adding (uint n) { cycle<adding_func> (n); }
};

#endif /* _AUTO_FILTER_H_ */
