/*
  interface.cc

	Copyright 2004-12 Tim Goetze <tim@quitte.de>

	http://quitte.de/dsp/

	LADSPA descriptor factory, host interface.

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
/*
	LADSPA ID ranges 1761 - 1800 and 2581 - 2660
	(2541 - 2580 donated to artemio@kdemail.net)
*/

#include "version.h"
#include "basics.h"

#include "Cabinet.h"
#include "Chorus.h"
#include "Phaser.h"
#include "Sin.h"
#include "Fractals.h"
#include "Reverb.h"
#include "Compress.h"
#include "Click.h"
#include "Eq.h"
#include "Saturate.h"
#include "White.h"
#include "AutoFilter.h"
#include "Amp.h"
#include "Pan.h"
#include "Scape.h"
#include "ToneStack.h"
#include "NoiseGate.h"

#include "Descriptor.h"

#define N 34

static DescriptorStub * descriptors [N+1];
static DescriptorStub * lv2_descriptors [N+1];

extern "C" {

const LADSPA_Descriptor *
ladspa_descriptor (unsigned long i)
{
	return i < N ? descriptors[i] : 0;
}

LV2_SYMBOL_EXPORT
const LV2_Descriptor *
lv2_descriptor(uint32_t i)
{
	return i < N ? lv2_descriptors[i] : 0;
}

__attribute__ ((constructor))
void caps_so_init()
{
	DescriptorStub ** d = descriptors;
	memset (d, 0, sizeof (descriptors));

	*d++ = new Descriptor<NoiseGate>(2602);
	*d++ = new Descriptor<Compress>(1772);
	*d++ = new Descriptor<Compress2x2>(2598);

	*d++ = new Descriptor<ToneStack>(2589);
	*d++ = new Descriptor<AmpVTS>(2592);
	*d++ = new Descriptor<CabinetII>(2581);
	*d++ = new Descriptor<CabinetIII>(2601);
	*d++ = new Descriptor<CabinetIV>(2606);

	*d++ = new Descriptor<JVRev>(1778);
	*d++ = new Descriptor<Plate>(1779);
	*d++ = new Descriptor<Plate2x2>(1795);

	*d++ = new Descriptor<Saturate>(1771);
	#if 1
	*d++ = new Descriptor<Spice>(2603);
    *d++ = new Descriptor<Spice2x2>(2607);
	#endif

	*d++ = new Descriptor<ChorusI>(1767);
	*d++ = new Descriptor<ChorusII>(2583);
	*d++ = new Descriptor<StereoChorusII>(2584);
	*d++ = new Descriptor<StereoChorusII2x2>(2605);
	*d++ = new Descriptor<PhaserII>(2586);
	*d++ = new Descriptor<StereoPhaserII>(2596);
	*d++ = new Descriptor<StereoPhaserII2x2>(2597);

	*d++ = new Descriptor<AutoFilter>(2593);
	*d++ = new Descriptor<Scape>(2588);

	*d++ = new Descriptor<Pan>(1788);
	*d++ = new Descriptor<Narrower>(2595);

	*d++ = new Descriptor<Eq>(1773);
	*d++ = new Descriptor<Eq2x2>(2594);

	*d++ = new Descriptor<Sin>(1781);
	*d++ = new Descriptor<White>(1785);
	*d++ = new Descriptor<Lorenz>(1774);
	*d++ = new Descriptor<Roessler>(1780);

	*d++ = new Descriptor<Click>(1769);
	*d++ = new Descriptor<CEO>(1770);
	*d++ = new Descriptor<Dirac>(2585);

	assert (d - descriptors <= N);

	/* LV2 */
	d = lv2_descriptors;
	memset (d, 0, sizeof (lv2_descriptors));

	*d++ = new Descriptor<NoiseGate>(CAPS_URI "NoiseGate");
	*d++ = new Descriptor<Compress>(CAPS_URI "Compress");

	*d++ = new Descriptor<Compress2x2>(CAPS_URI "Compress2x2");

	*d++ = new Descriptor<ToneStack>(CAPS_URI "ToneStack");
	*d++ = new Descriptor<AmpVTS>(CAPS_URI "AmpVTS");
	*d++ = new Descriptor<CabinetII>(CAPS_URI "CabinetII");
	*d++ = new Descriptor<CabinetIII>(CAPS_URI "CabinetIII");
	*d++ = new Descriptor<CabinetIV>(CAPS_URI "CabinetIV");

	*d++ = new Descriptor<JVRev>(CAPS_URI "JVRev");
	*d++ = new Descriptor<Plate>(CAPS_URI "Plate");
	*d++ = new Descriptor<Plate2x2>(CAPS_URI "Plate2x2");

	*d++ = new Descriptor<Saturate>(CAPS_URI "Saturate");
	#if 1
	*d++ = new Descriptor<Spice>(CAPS_URI "Spice");
	*d++ = new Descriptor<Spice2x2>(CAPS_URI "Spice2x2");
	#endif

	*d++ = new Descriptor<ChorusI>(CAPS_URI "ChorusI");
	*d++ = new Descriptor<ChorusII>(CAPS_URI "ChorusII");
	*d++ = new Descriptor<StereoChorusII>(CAPS_URI "StereoChorusII");
	*d++ = new Descriptor<StereoChorusII2x2>(CAPS_URI "StereoChorusII2x2");
	*d++ = new Descriptor<PhaserII>(CAPS_URI "PhaserII");
	*d++ = new Descriptor<StereoPhaserII>(CAPS_URI "StereoPhaserII");
	*d++ = new Descriptor<StereoPhaserII2x2>(CAPS_URI "StereoPhaserII2x2");

	*d++ = new Descriptor<AutoFilter>(CAPS_URI "AutoFilter");
	*d++ = new Descriptor<Scape>(CAPS_URI "Scape");

	*d++ = new Descriptor<Pan>(CAPS_URI "Pan");
	*d++ = new Descriptor<Narrower>(CAPS_URI "Narrower");

	*d++ = new Descriptor<Eq>(CAPS_URI "Eq");
	*d++ = new Descriptor<Eq2x2>(CAPS_URI "Eq2x2");

	*d++ = new Descriptor<Sin>(CAPS_URI "Sin");
	*d++ = new Descriptor<White>(CAPS_URI "White");
	*d++ = new Descriptor<Lorenz>(CAPS_URI "Lorenz");
	*d++ = new Descriptor<Roessler>(CAPS_URI "Roessler");

	*d++ = new Descriptor<Click>(CAPS_URI "Click");
	*d++ = new Descriptor<CEO>(CAPS_URI "CEO");
	*d++ = new Descriptor<Dirac>(CAPS_URI "Dirac");

	assert (d - lv2_descriptors <= N);
}

__attribute__ ((destructor))
void caps_so_fini()
{
	DescriptorStub ** d = descriptors;
	while (*d) delete *d++;

	d = lv2_descriptors;
	while (*d) delete *d++;
}


}; /* extern "C" */
