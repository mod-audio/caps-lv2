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
#include "dsp/Sine.h"

#include "ToneStack.h"
#include "Descriptor.h"

const char *
DSP::ToneStack::presetdict =
		"{0:'basswoman', 1:'stanford', 2:'wookie', 3:'DC 30', 4:'juice 800',"
		"5:'twin', 6:'AK 20', 7:'nihon ace', 8:'porky',}";

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
	{250 k, 250 k, 4.8 k, 100 k, 250 pF, 100 nF, 47 nF}, /* 64 Princeton AA1164 */
	{250 k, 1 M, 25 k, 47 k, 600 pF, 20 nF, 20 nF}, /* Mesa Dual Rect. 'Orange' */
	/* Vox -- R3 is fixed (circuit differs anyway) */
	{1 M, 1 M, 10 k, 100 k, 50 pF, 22 nF, 22 nF}, /* Vox "top boost" */
	
	{220 k, 1 M, 22 k, 33 k, 470 pF, 22 nF, 22 nF}, /* 59/81 JCM-800 Lead 100 2203 */
	{250 k, 250 k, 10 k, 100 k, 120 pF, 100 nF, 47 nF}, /* 69 Twin Reverb AA270 */

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

void
ToneStack::activate()
{ 
	model = -1;
}

void
ToneStack::cycle (uint frames)
{
	int m = getport(0);
	
	if (m != model)
	{
		model = m;
		tonestack.setmodel (model);
	}

	sample_t * s = ports[4];
	sample_t * d = ports[5];

	float bass=getport(1), mid=getport(2), treble=getport(3);

	tonestack.updatecoefs (bass, mid, treble);
	for (uint i = 0; i < frames; ++i) 
	{
		sample_t a = s[i];
		d[i] = tonestack.process(a + normal);
	}
}


PortInfo
ToneStack::port_info [] = 
{
	{ "model", CTRL_IN, {DEFAULT_0 | INTEGER, 0, 8}, DSP::ToneStack::presetdict }, 
	{ "bass", CTRL_IN | GROUP, {DEFAULT_MID, 0, 1} }, 
	{ "mid", CTRL_IN, {DEFAULT_MID, 0, 1} }, 
	{ "treble", CTRL_IN, {DEFAULT_MID, 0, 1} }, 

	{ "in", INPUT | AUDIO, {BOUNDED, -1, 1} }, 
	{ "out", OUTPUT | AUDIO }
};

template <> void
Descriptor<ToneStack>::setup()
{
	Label = "ToneStack";
	Name = CAPS "ToneStack - Classic amplifier tone stack emulation";
	autogen();
	Maker = "David T. Yeh <dtyeh@ccrma.stanford.edu>";
}


