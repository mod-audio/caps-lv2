/*
	Saturate.cc
	
	Copyright 2003-16 Tim Goetze <tim@quitte.de>
	
	http://quitte.de/dsp/

	Oversampled saturating waveshaping/clipping.

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

#include "Saturate.h"
#include "Descriptor.h"

#include "dsp/polynomials.h"
#include "dsp/RBJ.h"
#include "dsp/Butterworth.h"

/* a few clipping functions in addition to those provided in
 * dsp/polynomials.cc */
inline sample_t
_noclip(sample_t a)
{
	return a;
}

#if 0
/* branchless clamping, works but is slower than the branching version */
inline int 
getsign(float x, int shift = 31)
{
	union { float f; uint32 i; } u;
	u.f = x;
	return(u.i & 0x80000000) >> shift;
}

inline sample_t
_hardclip(sample_t a)
{
	/* set bit 1 if a < -.9 and bit 2 if a > .9 */
	int i = getsign(a+.9,31) | getsign(.9-a,30); 
	float r[4] = {a,-.9,.9,1};
	return r[i];
}
#else
inline sample_t
_hardclip(sample_t a)
{
	if(a < -.9) return -.9;
	if(a > +.9) return +.9;
	return a;
}
#endif

float _power_clip_7 (float x)
{
	union { float f; uint32 i; } u;
	u.f = x;
	float a = x, x2 = x*x;

	x *= x2;
	u.i &= 0xFF800000;
	a -= x * .3333333333333333333333333;
	u.i += 0x800000;
	x *= x2;
	u.i >>= 30;
	a += x * .2;
	x *= x2;
	a -= x * .142857142857143;
	
	float aa[4] = {a, .7238095403, a, -.7238095403};
	return aa[u.i];
}

void
Saturate::init()
{
	hp.set_f(40*over_fs);
	gain.linear = 1;
}

void
Saturate::activate()
{
	hp.reset();
	over.reset();
	bias = 0;
}

/* templated for waveshaping function */
template <clip_func_t C>
void
Saturate::subcycle(uint frames)
{
	sample_t * s = ports[3];
	sample_t * d = ports[4];

	/* apply inverse gain after saturation for a rough approximation of
	 * constant loudness */
	sample_t 
		ig  = .07 + .8/gain.linear, /* loop gain */
		igt = .07 + .8/(gain.linear + frames*gain.delta), /* target */
		igd = (igt - ig)/frames; /* delta */

	/* we have tried a chebyshev-poly added first harmonic instead of the
	 * DC bias and it wasn't as useful. */
	for(uint i = 0; i < frames; ++i)
	{
		sample_t x = s[i] + bias;
		x *= gain.linear;
		x = over.downsample(C(over.upsample(x)));

		for(int o=1; o < over.Ratio; ++o)
			over.downstore(C(over.uppad(o)));

		d[i] = ig*hp.process(x);

		ig += igd;
		gain.linear += gain.delta;
	}
}

static const char * SaturateModes = 
	"{0:'bypass', 1:'atan', 2:'atan15', 3:'clip', 4:'one5', \
		5:'one53', 6:'clip3', 7:'clip9', 8:'sin1', 9:'pow7', 10:'tanh', 11:'rectify',}";

/* correcting for the differences in sign and gain
 * between the individual waveshaping modes */
static float preamp[] = 
{0.998, 1.195, 1.142, 0.998, -1.092, -1.092, -0.446, -0.165, -0.696, 1.250, 1.195, 1};

void
Saturate::cycle(uint frames)
{
	int mode = (int) getport(0);
	double g = getport(1);

	if(!mode || mode == 11)
		g = 0;

	g = preamp[mode] * db2lin(g);
	gain.delta = (g-gain.linear) / frames;

	bias = .5*getport(2);
	bias = bias*bias;

	if(mode == 1) 
		subcycle<DSP::Polynomial::atan> (frames);
	else if(mode == 2) 
		subcycle<DSP::Polynomial::atan15> (frames);
	else if(mode == 3) 
		subcycle<_hardclip> (frames);
	else if(mode == 4) 
		subcycle<DSP::Polynomial::one5> (frames);
	else if(mode == 5) 
		subcycle<DSP::Polynomial::one53> (frames);
	else if(mode == 6) 
		subcycle<DSP::Polynomial::clip3> (frames);
	else if(mode == 7) 
		subcycle<DSP::Polynomial::clip9> (frames);
	else if(mode == 8) 
		subcycle<DSP::Polynomial::sin1> (frames);
	else if(mode == 9) 
		subcycle<DSP::Polynomial::power_clip_7> (frames);
	else if(mode == 10) 
		subcycle<DSP::Polynomial::tanh> (frames);
	else if(mode == 11) 
		subcycle<fabsf> (frames);
	else 
		subcycle<_noclip> (frames);
}

/* //////////////////////////////////////////////////////////////////////// */

PortInfo
Saturate::port_info [] = 
{
	{ "mode", CTRL_IN, {INTEGER | DEFAULT_1, 0, 11}, SaturateModes }, 
	{ "gain (dB)", CTRL_IN | GROUP, {DEFAULT_0, -24, 72} }, 
	{ "bias", CTRL_IN, {DEFAULT_0, 0, 1} },

	{ "in", INPUT | AUDIO }, 
	{ "out", OUTPUT | AUDIO }
};

template <> void
Descriptor<Saturate>::setup()
{
	Label = "Saturate";
	Name = CAPS "Saturate - Various static nonlinearities, 8x oversampled";
	autogen();
}

/* //////////////////////////////////////////////////////////////////////// */

/* 12 dB/oct. */
/* 56 / 83 / 160 ... 425 / 2k / 4.9k */
void
Spice::init()
{
	float amps[] = {0,0,1,.3,.01};
	cheby.calculate(amps);
}

void
Spice::activate()
{
	remain = 0;

	for(int i=0; i < 2; ++i)
	{
		split[i].reset();
		shape[i].reset();
	}
	compress.init(fs,32);
	compress.set_threshold(0);
	compress.set_attack(0);
	compress.set_release(0);
}

void
Spice::cycle(uint frames)
{
	struct { float f, squash, gain, vol; } 
			lo = {getport(0)*over_fs, getport(1), getport(2), getport(3)},
			hi = {getport(4)*over_fs, 0, getport(5), getport(6)};

	if(split[0].f != lo.f || lo.vol != vol.lo)
	{
		vol.lo = lo.vol;
		split[0].set_f(lo.f);
		DSP::RBJ::BP(2*lo.f,.7,shape[0]);
		shape[0].scale(db2lin(vol.lo));
	}
	if(split[1].f != hi.f || hi.vol != vol.hi)
	{
		vol.hi = hi.vol;
		split[1].set_f(hi.f);
		DSP::RBJ::HP(2*hi.f,.7,shape[1]);
		shape[1].scale(db2lin(vol.hi));
	}

	lo.gain = pow(8,lo.gain) - 1;
	hi.gain = pow(3,hi.gain) - 1;

	sample_t dc = cheby.process(0);

	sample_t * s = ports[7];
	sample_t * d = ports[8];

	while(frames)
	{
		if(remain == 0)
		{
			remain = compress.blocksize;
			compress.start_block(lo.squash); 
		}

		uint n = min(frames, remain);
		for(uint i = 0; i < n; ++i)
		{
			sample_t x,a,b,c;

			/* lo */
			x = s[i];
			a = split[0].low(x);
			b = split[0].high(x);
			x = a;
			x *= compress.get();
			x *= lo.gain;
			x = cheby.process(x)-dc;
			x = shape[0].process(x);
			compress.store(x);
			c = x;

			/* hi */
			x = a+b;
			a = split[1].low(x);
			b = split[1].high(x);
			x = b;
			x = cheby.process(x)-dc;
			x *= hi.gain;
			x = shape[1].process(x);
			x = x+a+b+c;

			d[i] = x;
		}

		s+=n, d+=n;
		remain-=n, frames-=n;
	}
}

/* //////////////////////////////////////////////////////////////////////// */

PortInfo
Spice::port_info [] = 
{
	{	"lo.f (Hz)",   CTRL_IN,	{LOG | DEFAULT_LOW, 50, 800}}, 
	{	"lo.compress", CTRL_IN,	{DEFAULT_MID, 0, 1}}, 
	{	"lo.gain",     CTRL_IN,	{DEFAULT_LOW, 0, 1}}, 
	{	"lo.vol (dB)",	     CTRL_IN,	{DEFAULT_0, -60, 60}}, 
	{	"hi.f (Hz)",   CTRL_IN | GROUP,	{LOG | DEFAULT_LOW, 400, 5000}}, 
	{	"hi.gain",     CTRL_IN,	{DEFAULT_LOW, 0, 1}}, 
	{	"hi.vol (dB)",	     CTRL_IN,	{DEFAULT_0, -60, 60}}, 

	{ "in", INPUT | AUDIO }, 
	{ "out", OUTPUT | AUDIO }, 
};

template <> void
Descriptor<Spice>::setup()
{
	Label = "Spice";
	Name = CAPS "Spice - Not an exciter";
	autogen();
}

/* //////////////////////////////////////////////////////////////////////// */

void
SpiceX2::init()
{
	float amp[] = {0,0,1,.3,.01};
	cheby.calculate(amp);
}

void
SpiceX2::activate()
{
	remain = 0;

	for(int c=0; c < 2; ++c)
		for(int i=0; i < 2; ++i)
		{
			chan[c].split[i].reset();
			chan[c].shape[i].reset();
		}
	compress.init(fs,64);
	compress.set_threshold(0);
	compress.set_attack(0);
	compress.set_release(0);
}

void
SpiceX2::cycle(uint frames)
{
	struct { float f, squash, gain, vol; } 
			lo = {getport(0)*over_fs, getport(1), getport(2), getport(3)},
			hi = {getport(4)*over_fs, 0, getport(5), getport(6)};

	if(chan[0].split[0].f != lo.f || lo.vol != vol.lo)
	{
		vol.lo = lo.vol;
		for(int c=0; c<2; ++c)
		{
			chan[c].split[0].set_f(lo.f);
			DSP::RBJ::BP(2*lo.f,.7,chan[c].shape[0]);
			chan[c].shape[0].scale(db2lin(vol.lo));
		}
	}
	if(chan[0].split[1].f != hi.f || hi.vol != vol.hi)
	{
		vol.hi = hi.vol;
		for(int c=0; c<2; ++c)
		{
			chan[c].split[1].set_f(hi.f);
			DSP::RBJ::BP(2*hi.f,.7,chan[c].shape[1]);
			chan[c].shape[1].scale(db2lin(vol.hi));
		}
	}

	lo.gain = pow(8,lo.gain) - 1;
	hi.gain = pow(3,hi.gain) - 1;

	sample_t dc = cheby.process(0);

	sample_t * s[2] = {ports[7],ports[8]};
	sample_t * d[2] = {ports[9],ports[10]};

	while(frames)
	{
		if(remain == 0)
		{
			remain = compress.blocksize;
			compress.start_block(lo.squash); 
		}

		uint n = min(frames, remain);
		for(uint i = 0; i < n; ++i)
		{
			sample_t x,a,b,bass[2];

			float comp = compress.get();
			for(int c=0; c<2; ++c)
			{
				/* lo */
				x = s[c][i];
				a = chan[c].split[0].low(x);
				b = chan[c].split[0].high(x);
				x = a;
				x *= comp;
				x *= lo.gain;
				x = cheby.process(x)-dc;
				x = chan[c].shape[0].process(x);
				bass[c] = x;

				/* hi */
				x = a+b;
				a = chan[c].split[1].low(x);
				b = chan[c].split[1].high(x);
				x = b;
				x *= hi.gain;
				x = DSP::Polynomial::atan(x);
				x = chan[c].shape[1].process(x);
				x = x+a+b+bass[c];

				d[c][i] = x;
			}
			compress.store(bass[0],bass[1]);
		}

		for(int c=0; c<2; ++c)
			s[c]+=n, d[c]+=n;
		remain-=n, frames-=n;
	}
}

/* //////////////////////////////////////////////////////////////////////// */

PortInfo
SpiceX2::port_info [] = 
{
	{	"lo.f (Hz)",   CTRL_IN,	{LOG | DEFAULT_LOW, 50, 800}}, 
	{	"lo.compress", CTRL_IN,	{DEFAULT_MID, 0, 1}}, 
	{	"lo.gain",     CTRL_IN,	{DEFAULT_LOW, 0, 1}}, 
	{	"lo.vol (dB)",	     CTRL_IN,	{DEFAULT_0, -60, 60}}, 
	{	"hi.f (Hz)",   CTRL_IN | GROUP,	{LOG | DEFAULT_LOW, 400, 5000}}, 
	{	"hi.gain",     CTRL_IN,	{DEFAULT_LOW, 0, 1}}, 
	{	"hi.vol (dB)",	     CTRL_IN,	{DEFAULT_0, -60, 60}}, 

	{ "in:l", INPUT | AUDIO }, 
	{ "in:r", INPUT | AUDIO }, 
	{ "out:l", OUTPUT | AUDIO }, 
	{ "out:r", OUTPUT | AUDIO }, 
};

template <> void
Descriptor<SpiceX2>::setup()
{
	Label = "SpiceX2";
	Name = CAPS "SpiceX2 - Not an exciter either";
	autogen();
}


