/*
	AutoFilter.cc
	
	Copyright 2002-13 Tim Goetze <tim@quitte.de>
	
	http://quitte.de/dsp/

	AutoFilter, a Lorenz fractal modulating the cutoff frequency of a 
	state-variable (ladder) filter.

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

#include "basics.h"

#include "AutoFilter.h"
#include "Descriptor.h"

#include "dsp/RBJ.h"
#include "dsp/polynomials.h"

inline sample_t sq (sample_t x) { return x * x; }

void
AutoFilter::init()
{
	blocksize = (uint) (fs/1200); /* samples to process with constant f and Q */
	f = .1;
	Q = .1;
	lorenz.init();

	/* for envelope RMS calculation */
	hp.set_f (250*over_fs);

	smooth.lfo.set(.86);
	DSP::RBJ::LP (.001, .5, smooth.env);
}

void 
AutoFilter::activate()
{ 
	f=getport(4)*over_fs, Q=getport(5);

	svf1.reset();
	svf1.set_f_Q (f,Q);
	svf2.reset();
	svf2.set_f_Q (f,Q);

	rms.reset();
	hp.reset();
	smooth.lfo.reset();
	smooth.env.reset();

	oversampler.two.reset();
	oversampler.four.reset();
	oversampler.eight.reset();
}

template <yield_func_t F, class SVF, class Over>
void
AutoFilter::subsubcycle (uint frames, SVF & svf, Over & over)
{
	div_t qr = div (frames, blocksize);
	int blocks = qr.quot;
	if (qr.rem) ++blocks;
	float over_blocks = 1./blocks;

	svf.set_out ((int) getport(1));
	float over_ratio = 1./over.Ratio;

	float g = svf.gainfactor() * db2lin(getport(3));

	/* f,Q sweep */
	float df = (getport(4)*over_fs - f) * over_blocks;
	float dQ = (getport(5) - Q) * over_blocks;

	float range = getport(6);
	float env = getport(7);

	lorenz.set_rate (3e-05*fs * .6*sq(getport(8)));
	float x=.9, z=.1;;

	sample_t * s = ports[9];
	sample_t * d = ports[10];

	while (frames)
	{
		lorenz.step();

		float fmod = 2.5*(x*lorenz.get_x() + z*lorenz.get_z());
		fmod = smooth.lfo.process(fmod);
		float fenv = smooth.env.process (rms.get()+normal);
		fenv = 64*fenv*fenv;

		#if 0 
		static int _turn = 0;
		static float ffm = 0;
		ffm = max(fabs(fmod),ffm);
		if (_turn++ % 100 == 0)
			fprintf (stderr, "%.4f %.4f %.4f\n", fmod, env, ffm);
		#endif
		fmod = fmod*(1-env) + fenv*env;
		fmod *= range;

		fmod = max (.001, f*(1 + fmod));
		uint n = min (frames, blocksize);
		
		/* envelope */
		for (uint i = 0; i < n; ++i)
		{
			sample_t x = hp.process (s[i]);
			rms.store (x*x);
		}

		svf.set_f_Q (fmod*over_ratio, Q);
		for (uint i = 0; i < n; ++i)
		{
			sample_t x = s[i] + normal;

			x = over.upsample (x);
			x = svf.process_clip(x,g);
			x = over.downsample (x);

			F (d,i,.5*x,adding_gain);

			for (int o=1; o < over.Ratio; ++o)
			{
				x = over.uppad (o);
				x = svf.process_clip(x,g);
				over.downstore (x);
			}
		}

		s += n;
		d += n;
		frames -= n;

		f += df;
		Q += dQ;
	}
}

/* //////////////////////////////////////////////////////////////////////// */

template <yield_func_t F, class SVF>
void
AutoFilter::subcycle (uint frames, SVF & svf)
{
	int r = getport(0);
	if (r == 3)
		subsubcycle<F,SVF,DSP::Oversampler<8,64> > (frames, svf, oversampler.eight);
	else if (r == 2)
		subsubcycle<F,SVF,DSP::Oversampler<4,64> > (frames, svf, oversampler.four);
	else if (r == 1)
		subsubcycle<F,SVF,DSP::Oversampler<2,32> > (frames, svf, oversampler.two);
	else
		subsubcycle<F,SVF,DSP::NoOversampler> (frames, svf, oversampler.one);
}

template <yield_func_t F>
void
AutoFilter::cycle (uint frames)
{
	if (getport(2) == 4)
		subcycle<F,SVF5> (frames, svf5);
	else if (getport(2) == 3)
		subcycle<F,SVF4> (frames, svf4);
	else if (getport(2) == 2)
		subcycle<F,SVF3> (frames, svf3);
	else if (getport(2) == 1)
		subcycle<F,SVF2> (frames, svf2);
	else
		subcycle<F,SVF1> (frames, svf1);
}

/* //////////////////////////////////////////////////////////////////////// */

PortInfo
AutoFilter::port_info [] =
{
	{	"over", CTRL_IN, {DEFAULT_1 | INTEGER, 0, 3}, 
		"{0:'none',1:'2x',2:'4x',3:'8x'}"},
	{ "mode", CTRL_IN | GROUP, {DEFAULT_1 | INTEGER, 0, 1},
		"{0:'low pass',1:'band pass',}" },
	{ "filter", CTRL_IN | GROUP, {DEFAULT_1 | INTEGER, 0, 4}, 
		"{0:'breathy',1:'fat A',2:'fat B',3:'fat C',4:'fat D'}", }, 
	{ "gain (dB)", CTRL_IN, {DEFAULT_LOW, 0, 24} }, 
	/* 4 */
	{ "f (Hz)", CTRL_IN | GROUP, {LOG | DEFAULT_HIGH, 20, 3000} }, 
	{ "Q", CTRL_IN, {DEFAULT_0, 0, 1} }, 
	/* 6 */
	{ "range", CTRL_IN | GROUP, {DEFAULT_HIGH, 0, 1} }, 
	{ "lfo/env", CTRL_IN, {DEFAULT_LOW, 0, 1} }, 
	{ "rate", CTRL_IN, {DEFAULT_LOW, 0, 1} }, 
	/* 9 */
	{ "in", AUDIO_IN}, 
	{	"out", AUDIO_OUT}
};

template <> void
Descriptor<AutoFilter>::setup()
{
	Label = "AutoFilter";

	Name = CAPS "AutoFilter - Modulated filter cascade";
	Maker = "Tim Goetze <tim@quitte.de>";
	Copyright = "2004-13";

	/* fill port info and vtable */
	autogen();
}

