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

#define N 33 
#define CAPS_URI "http://portalmod.com/plugins/caps/"

static LV2_Descriptor * descriptors [N+1];

extern "C" {

#if 0
const LADSPA_Descriptor *
ladspa_descriptor (unsigned long i)
{
	return i < N ? descriptors[i] : 0;
}
#endif 

LV2_SYMBOL_EXPORT
const LV2_Descriptor *lv2_descriptor(uint32_t i)
{
    printf("test...\n");
	return i < N ? descriptors[i] : 0;
}

__attribute__ ((constructor)) 
void caps_so_init()
{
    printf("generating descs\n");
	LV2_Descriptor ** d = descriptors;
	memset (d, 0, sizeof (descriptors));

	*d++ = new Descriptor<NoiseGate>(CAPS_URI "noisegate");
	*d++ = new Descriptor<Compress>(CAPS_URI "compress");
	*d++ = new Descriptor<Compress2x2>(CAPS_URI "compress2x2");

	*d++ = new Descriptor<ToneStack>(CAPS_URI "tonestack");
	*d++ = new Descriptor<AmpVTS>(CAPS_URI "ampvts");
	*d++ = new Descriptor<CabinetII>(CAPS_URI "cabinetii");
	#if 0
	*d++ = new Descriptor<CabinetIII>(CAPS_URI "cabinetiii");
	#endif
	*d++ = new Descriptor<CabinetIV>(CAPS_URI "cabinetiv");

	*d++ = new Descriptor<JVRev>(CAPS_URI "jvrev");
	*d++ = new Descriptor<Plate>(CAPS_URI "plate");
	*d++ = new Descriptor<Plate2x2>(CAPS_URI "plate2x2");

	*d++ = new Descriptor<Saturate>(CAPS_URI "saturate");
	#if 0
	*d++ = new Descriptor<Spice>(CAPS_URI "spice");
	#endif

	*d++ = new Descriptor<ChorusII>(CAPS_URI "chorusii");
	*d++ = new Descriptor<StereoChorusII>(CAPS_URI "stereochorusii");
	*d++ = new Descriptor<StereoChorusII2x2>(CAPS_URI "stereochorusii2x2");
	*d++ = new Descriptor<PhaserII>(CAPS_URI "phaserii");
	*d++ = new Descriptor<StereoPhaserII>(CAPS_URI "stereophaserii");
	*d++ = new Descriptor<StereoPhaserII2x2>(CAPS_URI "stereophaserii2x2");

	*d++ = new Descriptor<AutoFilter>(CAPS_URI "autofilter");
	*d++ = new Descriptor<Scape>(CAPS_URI "scape");

	*d++ = new Descriptor<Pan>(CAPS_URI "pan");
	*d++ = new Descriptor<Narrower>(CAPS_URI "narrower");

	*d++ = new Descriptor<Eq>(CAPS_URI "eq");
	*d++ = new Descriptor<Eq2x2>(CAPS_URI "eq2x2");

	*d++ = new Descriptor<Sin>(CAPS_URI "sin");
	*d++ = new Descriptor<White>(CAPS_URI "white");
	*d++ = new Descriptor<Lorenz>(CAPS_URI "lorenz");
	*d++ = new Descriptor<Roessler>(CAPS_URI "roessler");

	*d++ = new Descriptor<Click>(CAPS_URI "click");
	*d++ = new Descriptor<CEO>(CAPS_URI "ceo");
	*d++ = new Descriptor<Dirac>(CAPS_URI "dirac");
	
	assert (d - descriptors <= N);
    printf("generated descs\n");
}

__attribute__ ((destructor)) 
void caps_so_fini()
{
	LV2_Descriptor ** d = descriptors;
	while (*d) delete *d++;
}

}; /* extern "C" */
