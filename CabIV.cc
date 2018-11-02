/*
	CabIV.cc
	
	Copyright 2002-18 Tim Goetze <tim@quitte.de>
	
	http://quitte.de/dsp/

	Loudspeaker cabinet emulation through IIR/FIR filtering.

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
	switch_model ((int) getport(0));
	remain = 0;
}

void
CabinetIV::cycle (uint frames)
{
	static DSP::NoOversampler over1;

	if (over == 1) subcycle<DSP::NoOversampler,1>(frames,over1);
	else if (over == 2) subcycle<DSP::Oversampler<2,32>,2>(frames,over2);
	else if (over == 4) subcycle<DSP::Oversampler<4,64>,4>(frames,over4);
}

template <class O, int Ratio>
void
CabinetIV::subcycle (uint frames, O & Over)
{
	int m = (int) getport (0);
	if (m != model) switch_model (m);
	double g = gain * db2lin (getport(1));

	sample_t * s = ports[2];
	sample_t * d = ports[3];

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
			d[i] = x;
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
		d[i] = x;

		for (int o=1; o < Ratio; ++o)
		{
			Over.downstore(s[++i]);
			x = Over.uppad(o);
			d[i] = x;
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
	d[i] = x;

	n = 1+min(frames,remain);

	/* downsample the remainder of the block */
	for (uint o=1; o < n; ++o, --remain)
	{
		Over.downstore(s[++i]);
		x = Over.uppad(o);
		d[i] = x;
	}
}

/* //////////////////////////////////////////////////////////////////////// */

PortInfo
CabinetIV::port_info [] =
{
	{ "model", CTRL_IN, {INTEGER | DEFAULT_MID, 0, NCabIVModels-1}, CabIVModelDict },
	{ "gain (dB)", CTRL_IN | GROUP, {DEFAULT_0, -24, 24} }, 

	{ "in", INPUT | AUDIO }, 
	{ "out", OUTPUT | AUDIO } 
};


template <> void
Descriptor<CabinetIV>::setup()
{
	Label = "CabinetIV";
	Name = CAPS "CabinetIV - Idealised loudspeaker cabinet";
	autogen();
}

