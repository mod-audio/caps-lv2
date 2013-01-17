/*
	ToneStack.cc
	
	Copyright 2006-7
		David Yeh <dtyeh@ccrma.stanford.edu> 

		2007-2013
			Tim Goetze <tim@quitte.de> (cosmetics)

	Tone Stack emulation.
*
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
#include "dsp/util.h"

#include "ToneStack.h"
#include "Descriptor.h"

#include "dsp/tonestack/ks_tab.h"
#include "dsp/tonestack/vs_tab.h"

const char *
DSP::ToneStack::presetdict =
		"{0:'basswoman', 1:'twin', 2:'wookie', 3:'DC 30', 4:'juice 800',"
		"5:'stanford', 6:'HK 20', 7:'nihon ace', 8:'porky',"
		"9:'5F6-A LT'}";

DSP::TSParameters 
DSP::ToneStack::presets[] = {
	/* for convenience, */
	#define k *1e3
	#define M *1e6
	#define nF *1e-9
	#define pF *1e-12
	/* parameter order is R1 - R4, C1 - C3 */
	/* R1=treble R2=Bass R3=Mid, C1-3 related caps, R4 = parallel resistor */
	/* { 250000, 1000000, 25000, 56000, 0.25e-9, 20e-9, 20e-9 }, DY */
	{250 k, 1 M, 25 k, 56 k, 250 pF, 20 nF, 20 nF},  /* 59 Bassman 5F6-A */
	{250 k, 250 k, 10 k, 100 k, 120 pF, 100 nF, 47 nF}, /* 69 Twin Reverb AA270 */
	{250 k, 1 M, 25 k, 47 k, 600 pF, 20 nF, 20 nF}, /* Mesa Dual Rect. 'Orange' */
	/* Vox -- R3 is fixed (circuit differs anyway) */
	{1 M, 1 M, 10 k, 100 k, 50 pF, 22 nF, 22 nF}, /* 59/86 Vox AC-30 */
	
	{220 k, 1 M, 22 k, 33 k, 470 pF, 22 nF, 22 nF}, /* 59/81 JCM-800 Lead 100 2203 */
	{250 k, 250 k, 4.8 k, 100 k, 250 pF, 100 nF, 47 nF}, /* 64 Princeton AA1164 */

	{500 k, 1 M, 25 k, 47 k, 150 pF, 22 nF, 22 nF}, /* Hughes & Kettner Tube 20 */
	{250 k, 250 k, 10 k, 100 k, 150 pF, 82 nF, 47 nF}, /* Roland Jazz Chorus */
	{250 k, 1 M, 50 k, 33 k, 100 pF, 22 nF, 22 nF}, /* Pignose G40V */
	#if 0
	/* R4 is a 10 k fixed + 100 k pot in series actually */
	{250 k, 1 M, 25 k, 33 k, 500 pF, 22 nF, 22 nF}, /* 67 Major Lead 200 */

	{250 k, 1 M, 25 k, 56 k, 500 pF, 22 nF, 22 nF}, /* 81 2000 Lead */
	{250 k, 250 k, 25 k, 56 k, 250 pF, 47 nF, 47 nF}, /* undated M2199 30W solid state */
	#endif
	#undef k
	#undef M
	#undef nF
	#undef pF
};

int DSP::ToneStack::n_presets = TS_N_PRESETS;

void
ToneStack::activate()
{ 
	model = -1;
}

template <yield_func_t F>
void
ToneStack::cycle (uint frames)
{
	enum {LTModel = TS_N_PRESETS};

	sample_t * s = ports[0];
	sample_t * d = ports[5];

	int m = getport(1);
	
	if (m != model)
	{
		model = m;
		if (model == LTModel) 
			tonestacklt.reset();
		else 
			tonestack.setmodel (model);
	}

	float bass = getport(2), mid = getport(3), treble = getport(4);

	if (model == LTModel) 
	{
		tonestacklt.updatecoefs (bass, mid, treble);
		for (uint i = 0; i < frames; ++i) 
		{
			sample_t a = s[i];
			a = tonestacklt.process (a + normal);
			F (d, i, a, adding_gain);
		}
	} else {
		tonestack.updatecoefs (bass, mid, treble);
		for (uint i = 0; i < frames; ++i) 
		{
			sample_t a = s[i];
			a = tonestack.process (a + normal);
			F (d, i, a, adding_gain);
		}
	}
}


PortInfo
ToneStack::port_info [] = 
{
	{ "in", INPUT | AUDIO, {BOUNDED, -1, 1} }, 
	{ "model", CTRL_IN, {DEFAULT_0 | INTEGER, 0, TS_N_PRESETS 
		/* last is lattice filter implementation */ }, DSP::ToneStack::presetdict }, 
	{ "bass", CTRL_IN | GROUP, {DEFAULT_MID, 0, 1} }, 
	{ "mid", CTRL_IN, {DEFAULT_MID, 0, 1} }, 
	{ "treble", CTRL_IN, {DEFAULT_MID, 0, 1} }, 
	{ "out", OUTPUT | AUDIO }
};

template <> void
Descriptor<ToneStack>::setup()
{
	Label = "ToneStack";

	Name = CAPS "ToneStack - Classic amplifier tone stack emulation";
	Maker = "David T. Yeh <dtyeh@ccrma.stanford.edu>";
	Copyright = "2006-12";

	/* fill port info and vtable */
	autogen();
}


