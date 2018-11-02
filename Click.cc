/*
	Click.cc
	
	Copyright 2002-18 Tim Goetze <tim@quitte.de>
	
	http://quitte.de/dsp/

	Plugins playing a sound snippet in regular intervals.

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
#include "dsp/RBJ.h"
#include "dsp/White.h"

#include "Click.h"
#include "Descriptor.h"

template <int Waves>
void
ClickStub<Waves>::initwave(int i, int16 * _wave, uint _N)
{
	wave[i].data = _wave;
	wave[i].N = _N;
}

template <int Waves>
void
ClickStub<Waves>::cycle(uint frames)
{
	static double scale16 = 1./32768;
	int p = Waves>1 ? 1 : 0; /* port */
	int w = p ? getport(0) : 0; /* wave */
	bpm = getport(p);
	int div = Waves>1 ? (int) getport(p+1) : 1;
	div = div>0 ? div : 1;
	sample_t gain = getport(p+p+1);
	gain *= scale16 * gain;
	lp.set(1 - getport(p+p+2));
	
	sample_t * d = ports[p+p+3];

	while(frames)
	{
		if(period == 0)
		{
			period = (int) (fs * 60 / (div*bpm));
			played = 0;
		}

		uint n = min(frames, period);
				
		if(played < wave[w].N)
		{
			n = min(n, wave[w].N - played);

			for(uint i = 0; i < n; ++i)
			{
				double x = gain * wave[w].data[played+i];
				x = lp.process(x);
				d[i] = x;
			}

			played += n;
		}
		else 
		{
			for(uint i = 0; i < n; ++i)
				d[i] = lp.process(normal);
		}

		period -= n;
		frames -= n;
		d += n;
	}
}

/* //////////////////////////////////////////////////////////////////////// */

/* initialising the wavetable for the click sound from a few oscillation modes */
void
Click::initsimple()
{
	enum { Peaks = 7 };
	/* {f, gain} pairs */
	static float _peaks[Peaks][2]  = {
		{897, 1.0},
		{1423, 0.20},
		{2054, 0.92},
		{2353, 0.244},
		{2843, 0.719},
		{3567, 0.191},
		{5117, 0.091},
	};

	DSP::LP1<sample_t> lp1;
	lp1.set_f(800*over_fs);
	DSP::IIR2<sample_t> lp;
	DSP::RBJ::LP(8000*over_fs, .2, lp);

	DSP::IIR2<sample_t> peaks[Peaks];
	for(int i = 0; i < Peaks; ++i)
	{
		/* tune to g' = 784 Hz */
		float f = .8740245*_peaks[i][0]*over_fs;
		float g = _peaks[i][1];
		DSP::RBJ::BP(f, 22*g, peaks[i]);
	}

	DSP::IIR2<sample_t> bp;
	DSP::RBJ::BP(150*over_fs, 3.8, bp);

	DSP::IIR2<sample_t> post;
	DSP::RBJ::PeakingEQ(1000*over_fs, 1.8, 24, post);

	int n = (int) (fs * 2800. / 44100.);
	int16 * click = new int16[n];

	DSP::White white;
	int m = 8;
	sample_t mi = 1./m;
	sample_t x;
	for(int i = 0; i < n; ++i)
	{
		if(i < m) /* simplistic noise excitation signal */
			x = .5 * white.get() * (m-i)*mi;
		x = lp.process(x);
		double a = x;
		for(int j = 0; j < Peaks; ++j)
			a += peaks[j].process_bp(x);
		a = post.process(a);
		/* add some ring-modulated noisz */
		a += a*bp.process(white.get());
		click[i] = (int16) (a * 32767.);
		x = 0;
	}

	initwave(0, click, n);
}

/* using parfilt models to generate the click */
#include "Cabinet.h"
#include "waves/click.h"

void
Click::initparfilt()
{
	DSP::IIR2v4Bank<128> bank;

	ParModel<128,1> * model;
	if(fs > 120000) model = &waves_click_wav_176000;
	else if(fs > 60000) model = &waves_click_wav_88200;
	else model = &waves_click_wav_44100;

	bank.set_a(1, model->a1);
	bank.set_a(2, model->a2);
	bank.set_b(1, model->b1);
	bank.set_b(2, model->b2);
	bank.reset();
	
	int n = (int) (fs*2800/44100);
	int16 * click = new int16[n];

	DSP::IIR2<sample_t> hp;
	DSP::RBJ::HP(1520*over_fs, .7, hp);

	DSP::White white;
	int m = 3;
	sample_t mi = 1./m;
	sample_t x;
	for(int i = 0; i < n; ++i)
	{
		if(i < m) /* simplistic noise excitation signal */
			x = .5 * white.get() * (m-i)*mi;
		x = v4f_sum(bank.process_bp(v4f(x)));
		x = hp.process(x);
		click[i] = (int16) (x * 32767.);
		x = 0;
	}

	initwave(1, click, n);
}

void
Click::initsine()
{
	float f = 2*784;
	DSP::Sine sin(2*M_PI*f*over_fs);

	int n = (int) (12*fs/f);
	int m = 6*n/4;
	int16 * click = new int16[m];

	DSP::IIR2<sample_t> lp;
	DSP::RBJ::BP(f*over_fs,2.5,lp);

	float a = .4 * 32767;
	for(int i = 0; i < n; ++i)
	{
		sample_t x = a*sin.get();
		x = lp.process(x);
		click[i] = (int16) (x);
	}
	for(int i = n; i < m; ++i)
	{
		sample_t x = lp.process(NOISE_FLOOR);
		click[i] = (int16) (x);
	}

	initwave(2, click, m);
}

void
Click::initdirac()
{ 
	int16 * dirac = new int16[1];
	*dirac = 32767;
	initwave(3, dirac, 1);
}

template <> void
Descriptor<Click>::setup()
{
	Label = "Click";
	Name = CAPS "Click - Metronome";
	autogen();
}

/* //////////////////////////////////////////////////////////////////////// */

PortInfo
Click::port_info[] =
{
	{ "model", CTRL_IN, {INTEGER | DEFAULT_1, 0, 3}, 
		"{0:'box',1:'stick',2:'beep',3:'dirac'}" }, 
	{ "bpm", CTRL_IN | GROUP, {DEFAULT_LOW, 4, 240}, },
	{ "div", CTRL_IN | GROUP, {INTEGER | DEFAULT_MIN, 1, 4},
		"{1:'♩',2:'♪♪',3:'♪♪♪',4:'♬♬'}" }, 
	{	"vol", CTRL_IN | GROUP, {DEFAULT_LOW, 0, 1} }, 
	{	"tone", CTRL_IN, {DEFAULT_HIGH, 0, 1} }, 

	{ "out", OUTPUT | AUDIO}
};

/* //////////////////////////////////////////////////////////////////////// */

PortInfo
CEO::port_info[] =
{
	{ "ppm", CTRL_IN, {DEFAULT_LOW, 30, 232} }, 
	{	"volume", CTRL_IN | GROUP, {DEFAULT_HIGH, 0, 1}}, 
	{	"damping", CTRL_IN, {DEFAULT_0, 0, 1} }, 

	{ "out", OUTPUT | AUDIO, {0} }
};

#include "waves/profit.h"

void
CEO::init()
{ 
	int m = sizeof(profit)/sizeof(*profit);

	float s = fs/8000., dx = 1/s;

	int n = (int) (s*m);
	int16 * wave = new int16[n];

	DSP::IIR2<sample_t> lp;
	/* suppress aliasing with an additional lowpass; also slight gain at 3 kHz */
	DSP::RBJ::LP(3000*over_fs,1.5,lp);
	#if 1 /* linear */
	float x = 0;
	for(int i = 0; i < n-1; ++i)
	{
		int j = (int) x;
		float a = x-j;
		a = (1-a)*profit[j] + a*profit[j+1];
		a = lp.process(a);
		wave[i] = (int16) a;
		x += dx;
	}
	#else /* cubic, unneeded */
	float x = 0;
	for(int i=0; i < n; ++i, x+=dx)
	{
		int j = (int) x;
		float f = x-j;
		sample_t x_1 = profit[j-1];
		sample_t x0 = profit[j];
		sample_t x1 = profit[j+1];
		sample_t x2 = profit[j+2];

		sample_t a = 
				.5*(3*(x0 - x1) - x_1 + x2);
		sample_t b =
				2*x1 + x_1 - .5*(5*x0 + x2);
		sample_t c = 
				.5*(x1 - x_1);

		a = x0 + (((a * f) + b) * f + c) * f;
		wave[i] = (int16) a;
	}
	#endif

	initwave(0, wave, n-1);
}

template <> void
Descriptor<CEO>::setup()
{
	Label = "CEO";
	Name = CAPS "CEO - Chief Executive Oscillator";
	autogen();
}


