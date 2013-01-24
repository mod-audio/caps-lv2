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
#include <cstdio>

#ifdef LADSPAFLAG

#define N 33

static DescriptorStub * descriptors [N+1];

extern "C" {

const LADSPA_Descriptor *
ladspa_descriptor (unsigned long i)
{
    return i < N ? descriptors[i] : 0;
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
    #if 0
    *d++ = new Descriptor<Spice>(2603);
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
}

__attribute__ ((destructor))
void caps_so_fini()
{
    DescriptorStub ** d = descriptors;
    while (*d) delete *d++;
}

}; /* extern "C" */


#else

#define N 33
#define CAPS_URI "http://portalmod.com/plugins/caps/"

static LV2_Descriptor * descriptors [N+1];

extern "C" {

LV2_SYMBOL_EXPORT
const LV2_Descriptor *lv2_descriptor(uint32_t i)
{

    return i < N ? descriptors[i] : 0;
}

__attribute__ ((constructor))
void caps_so_init()
{



    LV2_Descriptor ** d = descriptors;
    memset (d, 0, sizeof (descriptors));



    #ifdef AMPVTS_
    *d++ = new Descriptor<AmpVTS>(CAPS_URI "ampvts");
    #endif
    #ifdef AUTOFILTER_
    *d++ = new Descriptor<AutoFilter>(CAPS_URI "autofilter");
    #endif
    #ifdef CABINETII_
    *d++ = new Descriptor<CabinetII>(CAPS_URI "cabinetii");
    #endif
    #ifdef CABINETIII_
    *d++ = new Descriptor<CabinetIII>(CAPS_URI "cabinetiii");
    #endif
    #ifdef CABINETIV_
    *d++ = new Descriptor<CabinetIV>(CAPS_URI "cabinetiv");
    #endif
    #ifdef CEO_
    *d++ = new Descriptor<CEO>(CAPS_URI "ceo");
    #endif
    #ifdef CHORUSI_
    *d++ = new Descriptor<ChorusI>(CAPS_URI "chorusi");
    #endif
    #ifdef CHORUSII_
    *d++ = new Descriptor<ChorusII>(CAPS_URI "chorusii");
    #endif
    #ifdef CLICK_
    *d++ = new Descriptor<Click>(CAPS_URI "click");
    #endif
    #ifdef COMPRESS2X2_
    *d++ = new Descriptor<Compress2x2>(CAPS_URI "compress2x2");
    #endif
    #ifdef COMPRESS_
    *d++ = new Descriptor<Compress>(CAPS_URI "compress");
    #endif
    #ifdef DIRAC_
    *d++ = new Descriptor<Dirac>(CAPS_URI "dirac");
    #endif
    #ifdef EQ2X2_
    *d++ = new Descriptor<Eq2x2>(CAPS_URI "eq2x2");
    #endif
    #ifdef EQ_
    *d++ = new Descriptor<Eq>(CAPS_URI "eq");
    #endif
    #ifdef JVREV_
    *d++ = new Descriptor<JVRev>(CAPS_URI "jvrev");
    #endif
    #ifdef LORENZ_
    *d++ = new Descriptor<Lorenz>(CAPS_URI "lorenz");
    #endif
    #ifdef NARROWER_
    *d++ = new Descriptor<Narrower>(CAPS_URI "narrower");
    #endif
    #ifdef NOISEGATE_
    *d++ = new Descriptor<NoiseGate>(CAPS_URI "noisegate");
    #endif
    #ifdef PAN_
    *d++ = new Descriptor<Pan>(CAPS_URI "pan");
    #endif
    #ifdef PHASERII_
    *d++ = new Descriptor<PhaserII>(CAPS_URI "phaserii");
    #endif
    #ifdef PLATE2X2_
    *d++ = new Descriptor<Plate2x2>(CAPS_URI "plate2x2");
    #endif
    #ifdef PLATE_
    *d++ = new Descriptor<Plate>(CAPS_URI "plate");
    #endif
    #ifdef ROESSLER_
    *d++ = new Descriptor<Roessler>(CAPS_URI "roessler");
    #endif
    #ifdef SATURATE_
    *d++ = new Descriptor<Saturate>(CAPS_URI "saturate");
    #endif
    #ifdef SCAPE_
    *d++ = new Descriptor<Scape>(CAPS_URI "scape");
    #endif
    #ifdef SIN_
    *d++ = new Descriptor<Sin>(CAPS_URI "sin");
    #endif
    #if 0
    *d++ = new Descriptor<Spice>(CAPS_URI "spice");
    #endif
    #ifdef STEREOCHORUSII2X2_
    *d++ = new Descriptor<StereoChorusII2x2>(CAPS_URI "stereochorusii2x2");
    #endif
    #ifdef STEREOCHORUSII_
    *d++ = new Descriptor<StereoChorusII>(CAPS_URI "stereochorusii");
    #endif
    #ifdef STEREOPHASERII2X2_
    *d++ = new Descriptor<StereoPhaserII2x2>(CAPS_URI "stereophaserii2x2");
    #endif
    #ifdef STEREOPHASERII_
    *d++ = new Descriptor<StereoPhaserII>(CAPS_URI "stereophaserii");
    #endif
    #ifdef TONESTACK_
    *d++ = new Descriptor<ToneStack>(CAPS_URI "tonestack");
    #endif
    #ifdef WHITE_
    *d++ = new Descriptor<White>(CAPS_URI "white");
    #endif



    assert (d - descriptors <= N);
}

__attribute__ ((destructor))
void caps_so_fini()
{
    LV2_Descriptor ** d = descriptors;
    while (*d) delete *d++;
}

}; /* extern "C" */

#endif
