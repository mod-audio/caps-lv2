/*
	AutoFilter.cc
	
	Copyright 2002-12 Tim Goetze <tim@quitte.de>
	
	http://quitte.de/dsp/

	AutoFilter, a Lorenz fractal modulating the cutoff frequency of a 
	state-variable (ladder) filter.

	AutoFilter, RMS envelope modulating a SVF bandpass filter.

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
	hp.set_f (250/fs);
	DSP::RBJ::LP (.001, .5, smoothenv);
}

void 
AutoFilter::activate()
{ 
	f=getport(2)/fs, Q=getport(3);

	svf1.reset();
	svf1.set_f_Q (f,Q);
	svf2.reset();
	svf2.set_f_Q (f,Q);

	rms.reset();
	hp.reset();
	smoothenv.reset();
}

void
AutoFilter::cycle (uint frames)
{
	div_t qr = div (frames, blocksize);
	int blocks = qr.quot;
	if (qr.rem) ++blocks;
	double over_blocks = 1./blocks;

	svf1.set_out ((int) getport(0));
	svf2.set_out ((int) getport(0));

	int svf = 1 + (int) getport(1);

	/* f,Q sweep */
	double df = (getport(2)/fs - f) * over_blocks;
	double dQ = (getport(3) - Q) * over_blocks;

	float range = getport(4);
	float env = getport(5);

	lorenz.set_rate (2.268e-05*fs * .6*sq (getport(6)));
	float x = getport(7), z = 1-x;


	sample_t * s = ports[8];
	sample_t * d = ports[9];

	while (frames)
	{
		lorenz.step();

		double fmod = x*lorenz.get_x() + z*lorenz.get_z();
		double fenv = smoothenv.process (rms.get()+normal);
		fenv = 64*fenv*fenv;

		#if 0 
		static int _turn = 0;
		if (_turn++ % 100 == 0)
			fprintf (stderr, "%.4f %.4f\n", fmod, env);
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

		/* filter it */
		if (svf == 1)
		{
			svf1.set_f_Q (fmod, Q);
			double g = 1.8;
			for (uint i = 0; i < n; ++i)
				d[i] = svf1.process<DSP::Polynomial::tanh>(s[i]+normal,g);
		}
		else if (svf == 2)
		{
			svf2.set_f_Q (fmod, Q);
			double g = .84*(1-Q) + .21;
			for (uint i = 0; i < n; ++i)
				d[i] = svf2.process<DSP::Polynomial::tanh>(s[i]+normal,g);
		}

		s += n;
		d += n;
		frames -= n;

		f += df;
		Q += dQ;
	}
}

/* //////////////////////////////////////////////////////////////////////// */

PortInfo
AutoFilter::port_info [] =
{
	{ "mode", CTRL_IN, {DEFAULT_1 | INTEGER, 0, 1},
		"{0:'low pass',1:'band pass',}" },
	{ "filter", CTRL_IN | GROUP, {DEFAULT_1 | INTEGER, 0, 1}, 
		"{0:'breathy',1:'fat'}", }, 
	/* 2 */
	{ "f (Hz)", CTRL_IN | GROUP, {LOG | DEFAULT_HIGH, 20, 3400} }, 
	{ "Q", CTRL_IN, {DEFAULT_LOW, 0, 1} }, 
	/* 4 */
	{ "depth", CTRL_IN | GROUP, {DEFAULT_1, 0, 1} }, 
	{ "lfo/env", CTRL_IN, {DEFAULT_LOW, 0, 1} }, 
	{ "rate", CTRL_IN | GROUP, {DEFAULT_LOW, 0, 1} }, 
	{ "x/z", CTRL_IN, {DEFAULT_1, 0, 1} }, 

	/* 8 */
	{ "in", AUDIO_IN}, 
	{	"out", AUDIO_OUT}
};

template <> void
Descriptor<AutoFilter>::setup()
{
	Label = "AutoFilter";

	Name = CAPS "AutoFilter - Self-modulating resonant filter";
	Maker = "Tim Goetze <tim@quitte.de>";
	Copyright = "2004-14";

	/* fill port info and vtable */
	autogen();
}

