/*
	Cabinet.cc
	
	Copyright 2002-12 Tim Goetze <tim@quitte.de>
	
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

#include "basics.h"

#include "dsp/RBJ.h"
#include "Cabinet.h"
#include "Descriptor.h"

#include "CabIIModels.h"

void
CabinetII::init()
{
	if (fs < 46000)
		models = models44100;
	else if (fs < 72000)
		models = models48000;
	else if (fs < 92000)
		models = models88200;
	else 
		models = models96000;

	h = 0;
	model = 0;
}

void
CabinetII::switch_model (int m)
{
	model = m;

	n = models[m].n;
	a = models[m].a;
	b = models[m].b;

	gain = models[m].gain * db2lin (getport(2));

	memset (x, 0, sizeof (x));
	memset (y, 0, sizeof (y));
}

void
CabinetII::activate()
{
	switch_model ((int) getport(1));
}

template <yield_func_t F>
void
CabinetII::cycle (uint frames)
{
	sample_t * s = ports[0];

	int m = (int) getport (1);
	if (m != model) switch_model (m);

	sample_t g = models[model].gain * db2lin (getport(2));
	double gf = pow (g / gain, 1 / (double) frames);
	sample_t * d = ports[3];

	for (uint i = 0; i < frames; ++i)
	{
		register cabinet_float out = s[i] + normal;
		
		x[h] = out;
		
		out *= a[0];

		for (int j = 1, z = h - 1; j < n; --z, ++j)
		{
			z &= 31;
			out += a[j] * x[z];
			out += b[j] * y[z];
		}

		y[h] = out;

		h = (h + 1) & 31;
		
		F (d, i, gain * out, adding_gain);
		gain *= gf;
	}
}

/* //////////////////////////////////////////////////////////////////////// */

PortInfo
CabinetII::port_info [] =
{
	{
		"in",
		INPUT | AUDIO,
		{BOUNDED, -1, 1}
	}, {
		"model",
		CTRL_IN,
		{INTEGER | DEFAULT_1, 0, 7},
		"{1:'unmatched A', 2:'unmatched B', 3:'supertramp', \
			4:'little wing 68',	5:'martial', 6:'mega wookie', 7:'pro sr',}"
	}, {
		"gain (dB)",
		CTRL_IN | GROUP,
		{DEFAULT_0, -24, 24}
	}, {
		"out",
		OUTPUT | AUDIO,
		{0}
	}
};

/* //////////////////////////////////////////////////////////////////////// */

template <> void
Descriptor<CabinetII>::setup()
{
	Label = "CabinetII";

	Name = CAPS "CabinetII - Simplistic loudspeaker cabinet emulation";
	Maker = "Tim Goetze <tim@quitte.de>";
	Copyright = "2002-12";

	/* fill port info and vtable */
	autogen();
}

/* //////////////////////////////////////////////////////////////////////// */

#include "CabIIIModels.h"

void
CabinetIII::init()
{
	model = -1;
}

void
CabinetIII::switch_model (int m)
{
	model = m;

	if (model < 0) return;
	
	gain = CabIIIModels[m].gain;

	bank.set_a (1, CabIIIModels[m].a1);
	bank.set_a (2, CabIIIModels[m].a2);
	bank.set_b (1, CabIIIModels[m].b1);
	bank.set_b (2, CabIIIModels[m].b2);
	bank.reset();

	fir.set_kernel (CabIIIModels[m].fir);
	fir.reset();
}

void
CabinetIII::activate()
{
	switch_model ((int) getport(1));
}

template <yield_func_t F>
void
CabinetIII::cycle (uint frames)
{
	sample_t * s = ports[0];

	int m = (int) getport (1);
	if (m != model) switch_model (m);

	sample_t * d = ports[3];

	double g = gain * db2lin (getport(2));

	for (uint i = 0; i < frames; ++i)
	{
		sample_t x = s[i];
		/* process */ 
		x = g*x + normal;
		v4f_t a = (v4f_t) {x,x,x,x};
		a = bank.process_no_a0 (a);
		x = v4f_sum(a + fir.process(x));
		F (d, i, x, adding_gain);
	}
}

/* //////////////////////////////////////////////////////////////////////// */

PortInfo
CabinetIII::port_info [] =
{
	{ "in", INPUT | AUDIO }, 
	{ "model", CTRL_IN, {INTEGER | DEFAULT_1, 0, 2}, 
		"{0:'wookie A', 1:'wookie B', 2:'wookie C'}" },
	{ "gain (dB)", CTRL_IN | GROUP, {DEFAULT_0, -24, 24} }, 
	{ "out", OUTPUT | AUDIO } 
};


template <> void
Descriptor<CabinetIII>::setup()
{
	Label = "CabinetIII";

	Name = CAPS "CabinetIII - Idealised loudspeaker cabinet emulation";
	Maker = "Tim Goetze <tim@quitte.de>";
	Copyright = "2012-13";

	/* fill port info and vtable */
	autogen();
}

/* //////////////////////////////////////////////////////////////////////// */

#include "CabIV_64_128.h"

void
CabinetIV::init()
{
	model = 0;

	/* choose oversampling ratio and 44.1 vs 48 k model */
	int f = (int) (fs/1000 + .5);
	over = 1;
	while (f > 48)
		over <<= 1,
		f >>= 1;

	if (over >= 4) over4.init(.75);
	else if (over == 2) over2.init(.75);
}

void
CabinetIV::switch_model (int m)
{
	model = m;

	if (model < 0) return;
	
	gain = CabIVModels[m].gain;

	bank.set_a (1, CabIVModels[m].a1);
	bank.set_a (2, CabIVModels[m].a2);
	bank.set_b (1, CabIVModels[m].b1);
	bank.set_b (2, CabIVModels[m].b2);
	bank.reset();

	fir.set_kernel (CabIVModels[m].fir);
	fir.reset();
}

void
CabinetIV::activate()
{
	switch_model ((int) getport(1));
	remain = 0;
}

template <yield_func_t F>
void
CabinetIV::cycle (uint frames)
{
	static DSP::NoOversampler over1;

	if (over == 1)
		cycle<F,DSP::NoOversampler,1>(frames,over1);
	else if (over == 2)
		cycle<F,DSP::Oversampler<2,32>,2>(frames,over2);
	else if (over == 4)
		cycle<F,DSP::Oversampler<4,64>,4>(frames,over4);
}

template <yield_func_t F, class O, int Ratio>
void
CabinetIV::cycle (uint frames, O & Over)
{
	sample_t * s = ports[0];

	int m = (int) getport (1);
	if (m != model) switch_model (m);

	sample_t * d = ports[3];

	double g = gain * db2lin (getport(2));
	uint n;

	/* now we have to jump through some hoops to support odd block sizes
	 * while maintaining the processing rhythm of the undersampling ratio 
	 * continuing over block borders.
	 */
	int rshift = Ratio == 4 ? 2 : (Ratio == 2 ? 1 : 0);

	if (Ratio == 1)
		n = frames;
	else /* undersampling */
	{
		n = min (remain,frames);
		for (uint i = 0; i < n; ++i)
		{
			Over.downstore(s[i]);
			sample_t x = Over.uppad(i+Ratio-remain);
			F (d, i, x, adding_gain);
		}

		remain -= n;
		frames -= n;
		s += n;
		d += n;

		if (!frames) return;

		n = (frames >> rshift) << rshift;
		remain = frames - n;
	}

	/* at 1:1 sample ratios, the following loop is all that's needed */
	uint i;
	for (i = 0; i < n; ++i)
	{
		sample_t x = s[i];
		x = Over.downsample (x);

		/* process */ 
		x = g*x + normal;
		v4f_t a = (v4f_t) {x,x,x,x};
		a = bank.process_no_a0 (a);
		x = v4f_sum(a + fir.process(x));

		x = Over.upsample (x);
		F (d, i, x, adding_gain);

		for (int o=1; o < Ratio; ++o)
		{
			Over.downstore(s[++i]);
			x = Over.uppad(o);
			F (d, i, x, adding_gain);
		}
	}
		
	if (Ratio==1 || !remain) return;

	/* at least 1 sample remaining in block, process it */
	remain = Ratio-1;
	frames -= n+1;

	sample_t x = s[i];
	x = Over.downsample (x);

	x = g*x + normal;
	v4f_t a = (v4f_t) {x,x,x,x};
	a = bank.process_no_a0 (a);
	x = v4f_sum(a + fir.process(x));

	x = Over.upsample (x);
	F (d, i, x, adding_gain);

	n = 1+min(frames,remain);

	/* downsample the remainder of the block */
	for (uint o=1; o < n; ++o, --remain)
	{
		Over.downstore(s[++i]);
		x = Over.uppad(o);
		F (d, i, x, adding_gain);
	}
}

/* //////////////////////////////////////////////////////////////////////// */

PortInfo
CabinetIV::port_info [] =
{
	{ "in", INPUT | AUDIO }, 
	{ "model", CTRL_IN, {INTEGER | DEFAULT_MID, 0, NCabIVModels-1}, CabIVModelDict },
	{ "gain (dB)", CTRL_IN | GROUP, {DEFAULT_0, -24, 24} }, 
	{ "out", OUTPUT | AUDIO } 
};


template <> void
Descriptor<CabinetIV>::setup()
{
	Label = "CabinetIV";

	Name = CAPS "CabinetIV - Idealised loudspeaker cabinet";
	Maker = "Tim Goetze <tim@quitte.de>";
	Copyright = "2012";

	/* fill port info and vtable */
	autogen();
}

