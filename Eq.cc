/*
	Eq.cc
	
	Copyright 2002-14 Tim Goetze <tim@quitte.de>
	
	http://quitte.de/dsp/

	Various equaliser plugins.

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

/* todo: Eq5p -- 20-400, 60-1k, 150-2.5k, 500-8k, 1k-20k */

#include "basics.h"
#include "dsp/Sine.h"

#include "Eq.h"
#include "Descriptor.h"

/* slight adjustments to gain to keep response optimally flat at
 * 0 dB gain in all bands */
inline static double 
adjust_gain(int i, double g)
{
	static float adjust[] = {
		0.69238604707174034, 0.67282771124180096, 
		0.67215187672467813, 0.65768648447259315, 
		0.65988083755898952, 0.66359580101701909, 
		0.66485139160960427, 0.65890297086039662, 
		0.6493229390740376, 0.82305724539749325
	};

	return g * adjust[i];
}

#define Eq10Q .707

void
Eq10::init()
{
	eq.init(fs, Eq10Q); 
}

void
Eq10::activate()
{
	for(int i = 0; i < 10; ++i)
	{
		gain[i] = getport(i);
		eq.gain[i] = adjust_gain(i, db2lin(gain[i]));
		eq.gf[i] = 1;
	}
}

void
Eq10::cycle(uint frames)
{
	/* evaluate band gain changes and compute recursion factor to prevent
	 * zipper noise */
	double one_over_n = frames > 0 ? 1. / frames : 1;

	for(int i = 0; i < 10; ++i)
	{
		sample_t g = getport(i);
		if(g == gain[i])
		{
			/* no gain factoring */
			eq.gf[i] = 1;
			continue;
		}
		gain[i] = g;

		double want = adjust_gain(i, db2lin(g));
		eq.gf[i] = pow(want / eq.gain[i], one_over_n);
	}

	sample_t * s = ports[10];
	sample_t * d = ports[11];

	for(uint i = 0; i < frames; ++i)
	{
		sample_t x = s[i];
		x = eq.process(x);
		d[i] = x;
	}

	eq.normal = -normal;
	eq.flush_0();
}

/* //////////////////////////////////////////////////////////////////////// */

PortInfo
Eq10::port_info [] =
{
	{"31 Hz", CTRL_IN, {DEFAULT_0, -48, 24}}, 
	{"63 Hz", CTRL_IN, {DEFAULT_0, -48, 24}}, 
	{"125 Hz", CTRL_IN | GROUP, {DEFAULT_0, -48, 24}}, 
	{"250 Hz", CTRL_IN, {DEFAULT_0, -48, 24}}, 
	{"500 Hz", CTRL_IN, {DEFAULT_0, -48, 24}}, 
	{"1 kHz", CTRL_IN | GROUP, {DEFAULT_0, -48, 24}}, 
	{"2 kHz", CTRL_IN, {DEFAULT_0, -48, 24}}, 
	{"4 kHz", CTRL_IN, {DEFAULT_0, -48, 24}}, 
	{"8 kHz", CTRL_IN | GROUP, {DEFAULT_0, -48, 24}}, 
	{"16 kHz", CTRL_IN, {DEFAULT_0, -48, 24}}, 

	{"in", INPUT | AUDIO}, 
	{"out", OUTPUT | AUDIO}
};

template <> void
Descriptor<Eq10>::setup()
{
	Label = "Eq10";
	Name = CAPS "Eq10 - 10-band equaliser";
	autogen();
}

/* //////////////////////////////////////////////////////////////////////// */

void
Eq10X2::init()
{
	for(int c = 0; c < 2; ++c)
		eq[c].init(fs, Eq10Q);
}

void
Eq10X2::activate()
{
	/* Fetch current parameter settings so we won't sweep band gains in the
	 * first block to process.
	 */
	for(int i = 0; i < 10; ++i)
	{
		gain[i] = getport(i);
		double a = adjust_gain(i, db2lin(gain[i]));
		for(int c = 0; c < 2; ++c)
			eq[c].gf[i] = 1,
			eq[c].gain[i] = a;
	}
}

void
Eq10X2::cycle(uint frames)
{
	/* evaluate band gain changes and compute recursion factor to prevent
	 * zipper noise */
	double one_over_n = frames > 0 ? 1. / frames : 1;

	for(int i = 0; i < 10; ++i)
	{
		double a;

		if(*ports[i] == gain[i])
			/* still same value, no gain fade */
			a = 1;
		else
		{
			gain[i] = getport(i);
			
			/* prepare factor for logarithmic gain fade */
			a = adjust_gain(i, db2lin(gain[i]));
			a = pow(a / eq[0].gain[i], one_over_n);
		}

		for(int c = 0; c < 2; ++c)
			eq[c].gf[i] = a;
	}

	for(int c = 0; c < 2; ++c)
	{
		sample_t 
			* s = ports[10 + c],
			* d = ports[12 + c];

		for(uint i = 0; i < frames; ++i)
		{
			sample_t x = s[i];
			x = eq[c].process(x);
			d[i] = x;
		}
	}

	/* flip 'renormal' values */
	for(int c = 0; c < 2; ++c)
	{
		eq[c].normal = normal;
		eq[c].flush_0();
	}
}

PortInfo
Eq10X2::port_info [] =
{
	{"31 Hz", CTRL_IN, {DEFAULT_0, -48, 24}}, 
	{"63 Hz", CTRL_IN, {DEFAULT_0, -48, 24}}, 
	{"125 Hz", CTRL_IN | GROUP, {DEFAULT_0, -48, 24}}, 
	{"250 Hz", CTRL_IN, {DEFAULT_0, -48, 24}}, 
	{"500 Hz", CTRL_IN, {DEFAULT_0, -48, 24}}, 
	{"1 kHz", CTRL_IN | GROUP, {DEFAULT_0, -48, 24}}, 
	{"2 kHz", CTRL_IN, {DEFAULT_0, -48, 24}}, 
	{"4 kHz", CTRL_IN, {DEFAULT_0, -48, 24}}, 
	{"8 kHz", CTRL_IN | GROUP, {DEFAULT_0, -48, 24}}, 
	{"16 kHz", CTRL_IN, {DEFAULT_0, -48, 24}}, 

	{"in.l", INPUT | AUDIO}, 
	{"in.r", INPUT | AUDIO}, 
	{"out.l", OUTPUT | AUDIO}, 
	{"out.r", OUTPUT | AUDIO}
};

template <> void
Descriptor<Eq10X2>::setup()
{
	Label = "Eq10X2";
	Name = CAPS "Eq10X2 - Stereo 10-band equaliser";
	autogen();
}

/* //////////////////////////////////////////////////////////////////////// */

void
Eq4p::init()
{
	/* limit filter frequency to slightly under Nyquist to be on the safe side */
	float limit = .48*fs; 
	for(int i = 0; i < 4; ++i)
	{
		state[i].f = -1; /* ensure all coefficients updated */
		ranges[4*i + 1].UpperBound = min(ranges[4*i + 1].UpperBound, limit);
	}
}

void
Eq4p::activate()
{
	filter[0].reset();
	filter[1].reset();

	updatestate();
	filter[0] = filter[1];
	xfade = false;
}

typedef struct {sample_t a[3], b[3];} IIR2_ab;

void
Eq4p::updatestate()
{
	for(int i=0; i<4; ++i)
	{
		sample_t mode = getport(i*4);
		sample_t f = getport(i*4 + 1);
		sample_t Q = getport(i*4 + 2);
		sample_t gain = getport(i*4 + 3);

		if(mode==state[i].mode && gain==state[i].gain && f==state[i].f && Q==state[i].Q) 
			continue;

		xfade = true;

		state[i].mode = mode;
		state[i].Q = Q;
		state[i].f = f;
		state[i].gain = gain;

		IIR2_ab c; 

		f *= over_fs;
		/* Zoelzer shelve: H(s) = (A*s^2 + s*(sqrt(A)/Q) + 1) / (s^2 + s/Q + 1) */
		/* maxima: solve([a/(1-b*0)=.5,a/(1-b)=50,a/(1-b*x)=.707],[a,b,x]); */
		Q = .5/(1-.99*Q);
		if(mode < 0)
			c.a[0]=1,c.a[1]=0,c.a[2]=0,c.b[1]=0,c.b[2]=0; /* off = identity filter */
		else if(mode < 0.5)
			DSP::RBJ::LoShelve(f,Q,gain,c);
		else if(mode < 1.5)
			DSP::RBJ::PeakingEQ(f,Q,gain,c);
		else /* if(mode < 2.5) */
			DSP::RBJ::HiShelve(f,Q,gain,c);

		filter[1].set_ab(i, c.a, c.b);
	}
}

void
Eq4p::cycle(uint frames)
{
	*ports[16] = 3; /* _latency */

	sample_t * s = ports[17];
	sample_t * d = ports[18];

	updatestate(); 

	if(!xfade)
	{
		for(uint i = 0; i < frames; ++i)
		{
			sample_t x = s[i] + normal;
			x = filter[0].seriesprocess(x);
			d[i] = x;
		}
	}
	else
	{
		float over_n = frames ? 1./frames : 1;
		DSP::Sine gf0 (.5*M_PI*over_n,.5*M_PI);
		DSP::Sine gf1 (.5*M_PI*over_n,0);
		for(uint i = 0; i < frames; ++i)
		{
			sample_t x = s[i];
			sample_t g0 = gf0.get();
			sample_t g1 = gf1.get();
			x = g0*g0*filter[0].seriesprocess(x) + g1*g1*filter[1].seriesprocess(x);
			d[i] = x;
		}
		
		filter[0] = filter[1];
		filter[1].reset();
		xfade = false;
	}
}

/* //////////////////////////////////////////////////////////////////////// */

static const char * Eq4pBandModes = "{-1:'off',0:'lowshelve',1:'band',2:'hishelve'}";

PortInfo
Eq4p::port_info [] =
{
	{"a.mode", CTRL_IN, {DEFAULT_0 | INTEGER, -1,2}, Eq4pBandModes},
	{"a.f (Hz)", CTRL_IN, {DEFAULT_LOW | LOG, 20, 14000}},
	{"a.Q", CTRL_IN, {DEFAULT_LOW, 0, 1}},
	{"a.gain (dB)", CTRL_IN, {DEFAULT_0, -48, 24}},

	/* 4 */
	{"b.mode", CTRL_IN | GROUP, {DEFAULT_1 | INTEGER, -1,2}, Eq4pBandModes},
	{"b.f (Hz)", CTRL_IN, {DEFAULT_MID | LOG, 20, 14000}},
	{"b.Q", CTRL_IN, {DEFAULT_MID, 0, 1}},
	{"b.gain (dB)", CTRL_IN, {DEFAULT_0, -48, 24}},

	/* 8 */
	{"c.mode", CTRL_IN | GROUP, {DEFAULT_1 | INTEGER, -1,2}, Eq4pBandModes},
	{"c.f (Hz)", CTRL_IN, {DEFAULT_MID | LOG, 20, 14000}},
	{"c.Q", CTRL_IN, {DEFAULT_LOW, 0, 1}},
	{"c.gain (dB)", CTRL_IN, {DEFAULT_0, -48, 24}},

	/* 12 */
	{"d.mode", CTRL_IN | GROUP, {DEFAULT_MAX | INTEGER, -1,2}, Eq4pBandModes},
	{"d.f (Hz)", CTRL_IN, {DEFAULT_HIGH | LOG, 20, 14000}},
	{"d.Q", CTRL_IN, {DEFAULT_LOW, 0, 1}},
	{"d.gain (dB)", CTRL_IN, {DEFAULT_0, -48, 24}},

	/* 16 */
	{"_latency", OUTPUT|CONTROL|GROUP, {INTEGER|DEFAULT_MAX,3,3}, "{3:'3 samples'}"},

	/* 17 */
	{"in", INPUT | AUDIO, {0, -1, 1}}, 
	{"out", OUTPUT | AUDIO, {0}}
};

template <> void
Descriptor<Eq4p>::setup()
{
	Label = "Eq4p";
	Name = CAPS "Eq4p - 4-band parametric shelving equaliser";
	autogen();
}

/* //////////////////////////////////////////////////////////////////////// */

void
EqFA4p::init()
{
	/* limit filter frequency to slightly under Nyquist to be on the safe side */
	float limit = .48*fs; 
	for(int i = 0; i < 4; ++i)
	{
		state[i].f = -1; /* ensure all coefficients updated */
		ranges[4*i + 1].UpperBound = min(ranges[4*i + 1].UpperBound, limit);
	}
}

void
EqFA4p::activate()
{
	filter[0].reset();
	filter[1].reset();

	updatestate();
	filter[0] = filter[1];
	xfade = false;
	gain = db2lin(getport(16));
}

void
EqFA4p::updatestate()
{
	for(int i=0; i<4; ++i)
	{
		sample_t mode = getport(i*4 + 0);
		sample_t f = getport(i*4 + 1);
		sample_t bw = getport(i*4 + 2);
		sample_t gain = getport(i*4 + 3);

		if(mode==state[i].mode && gain==state[i].gain && f==state[i].f && bw==state[i].bw) 
			continue;

		xfade = true;

		state[i].mode = mode;
		state[i].bw = bw;
		state[i].f = f;
		state[i].gain = gain;

		if(!mode)
			filter[1].unity(i);
		else
			filter[1].set(i, f*over_fs, bw, db2lin(gain));
	}
}

void
EqFA4p::cycle(uint frames)
{
	updatestate(); 

	sample_t g = db2lin(getport(16));
	sample_t over_n = frames ? 1./frames : 1;
	sample_t gf = pow(g/gain, over_n);

	*ports[17] = 3; /* _latency */
	sample_t * s = ports[18];
	sample_t * d = ports[19];

	if(!xfade)
	{
		for(uint i = 0; i < frames; ++i)
		{
			sample_t x = s[i];
			x = filter[0].seriesprocess(x);
			x = gain*x;
			gain *= gf;
			d[i] = x;
		}
	}
	else
	{
		DSP::Sine gf0 (.5*M_PI*over_n,.5*M_PI);
		DSP::Sine gf1 (.5*M_PI*over_n,0);
		for(uint i = 0; i < frames; ++i)
		{
			sample_t x = s[i];
			sample_t g0 = gf0.get();
			sample_t g1 = gf1.get();
			x = g0*g0*filter[0].seriesprocess(x) + g1*g1*filter[1].seriesprocess(x);
			x = gain*x;
			gain *= gf;
			d[i] = x;
		}
		
		filter[0] = filter[1];
		filter[1].reset();
		xfade = false;
	}
}

/* //////////////////////////////////////////////////////////////////////// */

static const char * EqFA4pBandModes = "{0:'off',1:'on'}";

PortInfo
EqFA4p::port_info [] =
{
	{"a.act", CTRL_IN | GROUP, {DEFAULT_0 | INTEGER, 0,1}, EqFA4pBandModes},
	{"a.f (Hz)", CTRL_IN, {DEFAULT_LOW | LOG, 20, 14000}},
	{"a.bw", CTRL_IN, {DEFAULT_1, .125, 8}},
	{"a.gain (dB)", CTRL_IN, {DEFAULT_0, -24, 24}},

	/* 4 */
	{"b.act", CTRL_IN | GROUP, {DEFAULT_0 | INTEGER, 0,1}, EqFA4pBandModes},
	{"b.f (Hz)", CTRL_IN, {DEFAULT_MID | LOG, 20, 14000}},
	{"b.bw", CTRL_IN, {DEFAULT_1, .125, 8}},
	{"b.gain (dB)", CTRL_IN, {DEFAULT_0, -24, 24}},

	/* 8 */
	{"c.act", CTRL_IN | GROUP, {DEFAULT_0 | INTEGER, 0,1}, EqFA4pBandModes},
	{"c.f (Hz)", CTRL_IN, {DEFAULT_MID | LOG, 20, 14000}},
	{"c.bw", CTRL_IN, {DEFAULT_1, .125, 8}},
	{"c.gain (dB)", CTRL_IN, {DEFAULT_0, -24, 24}},

	/* 12 */
	{"d.act", CTRL_IN | GROUP, {DEFAULT_0 | INTEGER, 0,1}, EqFA4pBandModes},
	{"d.f (Hz)", CTRL_IN, {DEFAULT_HIGH | LOG, 20, 14000}},
	{"d.bw", CTRL_IN, {DEFAULT_1, .125, 8}},
	{"d.gain (dB)", CTRL_IN, {DEFAULT_0, -24, 24}},

	/* 16 */
	{"gain", CTRL_IN|GROUP, {DEFAULT_0, -24, 24}},

	/* 17 */
	{"_latency", OUTPUT|CONTROL|GROUP, {INTEGER|DEFAULT_MAX,3,3}, "{3:'3 samples'}"},

	/* 18 */
	{"in", INPUT | AUDIO}, 
	{"out", OUTPUT | AUDIO}
};

template <> void
Descriptor<EqFA4p>::setup()
{
	Label = "EqFA4p";
	Name = CAPS "EqFA4p - 4-band parametric eq";
	autogen();
}


