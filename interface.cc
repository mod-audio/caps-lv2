/*
  interface.cc

    Copyright 2004-11 Tim Goetze <tim@quitte.de>

    http://quitte.de/dsp/

    LADSPA descriptor factory, host interface.

*/
/*
    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation; either version 2
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

#include <sys/time.h>

#include "basics.h"

#include "Cabinet.h"
#include "Chorus.h"
#include "Phaser.h"
#include "Sin.h"
#include "Lorenz.h"
#include "Roessler.h"
#include "Reverb.h"
#include "Compress.h"
#include "Click.h"
#include "Eq.h"
#include "Clip.h"
#include "White.h"
#include "SweepVF.h"
#include "VCO.h"
#include "Amp.h"
#include "HRTF.h"
#include "Pan.h"
#include "Scape.h"
#include "ToneStack.h"

#include "Descriptor.h"

#define N 39
//static DescriptorStub * descriptors [N];

static inline void
seed()
{
    static struct timeval tv;
  gettimeofday (&tv, 0);

    srandom (tv.tv_sec ^ tv.tv_usec);
}

extern "C" {

//__attribute__ ((constructor))
//void _init()
//{
//    DescriptorStub ** d = descriptors;
//
//    *d++ = new Descriptor<Eq>();
//    *d++ = new Descriptor<Eq2x2>();
//    *d++ = new Descriptor<Compress>();
//    *d++ = new Descriptor<Pan>();
//    *d++ = new Descriptor<Narrower>();
//
//    *d++ = new Descriptor<PreampIII>();
//    *d++ = new Descriptor<PreampIV>();
//    *d++ = new Descriptor<ToneStack>();
//    *d++ = new Descriptor<ToneStackLT>();
//    *d++ = new Descriptor<AmpIII>();
//    *d++ = new Descriptor<AmpIV>();
//    *d++ = new Descriptor<AmpV>();
//    *d++ = new Descriptor<AmpVTS>();
//    *d++ = new Descriptor<CabinetI>();
//    *d++ = new Descriptor<CabinetII>();
//    *d++ = new Descriptor<Clip>();
//
//    *d++ = new Descriptor<ChorusI>();
//    *d++ = new Descriptor<StereoChorusI>();
//    *d++ = new Descriptor<ChorusII>();
//    *d++ = new Descriptor<StereoChorusII>();
//    *d++ = new Descriptor<PhaserI>();
//    *d++ = new Descriptor<PhaserII>();
//    *d++ = new Descriptor<SweepVFI>();
//    *d++ = new Descriptor<SweepVFII>();
//    *d++ = new Descriptor<AutoWah>();
//    *d++ = new Descriptor<Scape>();
//
//    *d++ = new Descriptor<VCOs>();
//    *d++ = new Descriptor<VCOd>();
//    *d++ = new Descriptor<CEO>();
//    *d++ = new Descriptor<Sin>();
//    *d++ = new Descriptor<White>();
//    *d++ = new Descriptor<Lorenz>();
//    *d++ = new Descriptor<Roessler>();
//
//    *d++ = new Descriptor<JVRev>();
//    *d++ = new Descriptor<Plate>();
//    *d++ = new Descriptor<Plate2x2>();
//
//    *d++ = new Descriptor<Click>();
//    *d++ = new Descriptor<Dirac>();
//    *d++ = new Descriptor<HRTF>();
//
//    /* make sure N is correct */
//    assert (d - descriptors == N);
//
//    //seed();
//}
//
//__attribute__ ((destructor))
//void _fini()
//{
//    for (ulong i = 0; i < N; ++i)
//        delete descriptors[i];
//}

/* /////////////////////////////////////////////////////////////////////// */

//const LADSPA_Descriptor *
//ladspa_descriptor (unsigned long i)
//{
//    if (i < N)
//        return descriptors[i];
//    return 0;
//}

}; /* extern "C" */


#define CAPS_URI "http://portalmod.com/plugins/caps"
static const LV2_Descriptor Descriptors[] = {
    {
    CAPS_URI "Eq",
    Descriptor<Eq>::_instantiate,
    Descriptor<Eq>::_connect_port,
    Descriptor<Eq>::_activate,
    Descriptor<Eq>::_run,
    Descriptor<Eq>::_deactivate,
    Descriptor<Eq>::_cleanup,
    Descriptor<Eq>::_extension_data
    },
    {
    CAPS_URI "Eq2x2",
    Descriptor<Eq2x2>::_instantiate,
    Descriptor<Eq2x2>::_connect_port,
    Descriptor<Eq2x2>::_activate,
    Descriptor<Eq2x2>::_run,
    Descriptor<Eq2x2>::_deactivate,
    Descriptor<Eq2x2>::_cleanup,
    Descriptor<Eq2x2>::_extension_data
    },
    {
    CAPS_URI "Compress",
    Descriptor<Compress>::_instantiate,
    Descriptor<Compress>::_connect_port,
    Descriptor<Compress>::_activate,
    Descriptor<Compress>::_run,
    Descriptor<Compress>::_deactivate,
    Descriptor<Compress>::_cleanup,
    Descriptor<Compress>::_extension_data
    },
    {
    CAPS_URI "Pan",
    Descriptor<Pan>::_instantiate,
    Descriptor<Pan>::_connect_port,
    Descriptor<Pan>::_activate,
    Descriptor<Pan>::_run,
    Descriptor<Pan>::_deactivate,
    Descriptor<Pan>::_cleanup,
    Descriptor<Pan>::_extension_data
    },
    {
    CAPS_URI "Narrower",
    Descriptor<Narrower>::_instantiate,
    Descriptor<Narrower>::_connect_port,
    Descriptor<Narrower>::_activate,
    Descriptor<Narrower>::_run,
    Descriptor<Narrower>::_deactivate,
    Descriptor<Narrower>::_cleanup,
    Descriptor<Narrower>::_extension_data
    },
    {
    CAPS_URI "PreampIII",
    Descriptor<PreampIII>::_instantiate,
    Descriptor<PreampIII>::_connect_port,
    Descriptor<PreampIII>::_activate,
    Descriptor<PreampIII>::_run,
    Descriptor<PreampIII>::_deactivate,
    Descriptor<PreampIII>::_cleanup,
    Descriptor<PreampIII>::_extension_data
    },
    {
    CAPS_URI "PreampIV",
    Descriptor<PreampIV>::_instantiate,
    Descriptor<PreampIV>::_connect_port,
    Descriptor<PreampIV>::_activate,
    Descriptor<PreampIV>::_run,
    Descriptor<PreampIV>::_deactivate,
    Descriptor<PreampIV>::_cleanup,
    Descriptor<PreampIV>::_extension_data
    },
    {
    CAPS_URI "ToneStack",
    Descriptor<ToneStack>::_instantiate,
    Descriptor<ToneStack>::_connect_port,
    Descriptor<ToneStack>::_activate,
    Descriptor<ToneStack>::_run,
    Descriptor<ToneStack>::_deactivate,
    Descriptor<ToneStack>::_cleanup,
    Descriptor<ToneStack>::_extension_data
    },
    {
    CAPS_URI "ToneStackLT",
    Descriptor<ToneStackLT>::_instantiate,
    Descriptor<ToneStackLT>::_connect_port,
    Descriptor<ToneStackLT>::_activate,
    Descriptor<ToneStackLT>::_run,
    Descriptor<ToneStackLT>::_deactivate,
    Descriptor<ToneStackLT>::_cleanup,
    Descriptor<ToneStackLT>::_extension_data
    },
    {
    CAPS_URI "AmpIII",
    Descriptor<AmpIII>::_instantiate,
    Descriptor<AmpIII>::_connect_port,
    Descriptor<AmpIII>::_activate,
    Descriptor<AmpIII>::_run,
    Descriptor<AmpIII>::_deactivate,
    Descriptor<AmpIII>::_cleanup,
    Descriptor<AmpIII>::_extension_data
    },
    {
    CAPS_URI "AmpIV",
    Descriptor<AmpIV>::_instantiate,
    Descriptor<AmpIV>::_connect_port,
    Descriptor<AmpIV>::_activate,
    Descriptor<AmpIV>::_run,
    Descriptor<AmpIV>::_deactivate,
    Descriptor<AmpIV>::_cleanup,
    Descriptor<AmpIV>::_extension_data
    },
    {
    CAPS_URI "AmpV",
    Descriptor<AmpV>::_instantiate,
    Descriptor<AmpV>::_connect_port,
    Descriptor<AmpV>::_activate,
    Descriptor<AmpV>::_run,
    Descriptor<AmpV>::_deactivate,
    Descriptor<AmpV>::_cleanup,
    Descriptor<AmpV>::_extension_data
    },
    {
    CAPS_URI "AmpVTS",
    Descriptor<AmpVTS>::_instantiate,
    Descriptor<AmpVTS>::_connect_port,
    Descriptor<AmpVTS>::_activate,
    Descriptor<AmpVTS>::_run,
    Descriptor<AmpVTS>::_deactivate,
    Descriptor<AmpVTS>::_cleanup,
    Descriptor<AmpVTS>::_extension_data
    },
    {
    CAPS_URI "CabinetI",
    Descriptor<CabinetI>::_instantiate,
    Descriptor<CabinetI>::_connect_port,
    Descriptor<CabinetI>::_activate,
    Descriptor<CabinetI>::_run,
    Descriptor<CabinetI>::_deactivate,
    Descriptor<CabinetI>::_cleanup,
    Descriptor<CabinetI>::_extension_data
    },
    {
    CAPS_URI "CabinetII",
    Descriptor<CabinetII>::_instantiate,
    Descriptor<CabinetII>::_connect_port,
    Descriptor<CabinetII>::_activate,
    Descriptor<CabinetII>::_run,
    Descriptor<CabinetII>::_deactivate,
    Descriptor<CabinetII>::_cleanup,
    Descriptor<CabinetII>::_extension_data
    },
    {
    CAPS_URI "Clip",
    Descriptor<Clip>::_instantiate,
    Descriptor<Clip>::_connect_port,
    Descriptor<Clip>::_activate,
    Descriptor<Clip>::_run,
    Descriptor<Clip>::_deactivate,
    Descriptor<Clip>::_cleanup,
    Descriptor<Clip>::_extension_data
    },
    {
    CAPS_URI "ChorusI",
    Descriptor<ChorusI>::_instantiate,
    Descriptor<ChorusI>::_connect_port,
    Descriptor<ChorusI>::_activate,
    Descriptor<ChorusI>::_run,
    Descriptor<ChorusI>::_deactivate,
    Descriptor<ChorusI>::_cleanup,
    Descriptor<ChorusI>::_extension_data
    },
    {
    CAPS_URI "StereoChorusI",
    Descriptor<StereoChorusI>::_instantiate,
    Descriptor<StereoChorusI>::_connect_port,
    Descriptor<StereoChorusI>::_activate,
    Descriptor<StereoChorusI>::_run,
    Descriptor<StereoChorusI>::_deactivate,
    Descriptor<StereoChorusI>::_cleanup,
    Descriptor<StereoChorusI>::_extension_data
    },
    {
    CAPS_URI "ChorusII",
    Descriptor<ChorusII>::_instantiate,
    Descriptor<ChorusII>::_connect_port,
    Descriptor<ChorusII>::_activate,
    Descriptor<ChorusII>::_run,
    Descriptor<ChorusII>::_deactivate,
    Descriptor<ChorusII>::_cleanup,
    Descriptor<ChorusII>::_extension_data
    },
    {
    CAPS_URI "StereoChorusII",
    Descriptor<StereoChorusII>::_instantiate,
    Descriptor<StereoChorusII>::_connect_port,
    Descriptor<StereoChorusII>::_activate,
    Descriptor<StereoChorusII>::_run,
    Descriptor<StereoChorusII>::_deactivate,
    Descriptor<StereoChorusII>::_cleanup,
    Descriptor<StereoChorusII>::_extension_data
    },
    {
    CAPS_URI "PhaserI",
    Descriptor<PhaserI>::_instantiate,
    Descriptor<PhaserI>::_connect_port,
    Descriptor<PhaserI>::_activate,
    Descriptor<PhaserI>::_run,
    Descriptor<PhaserI>::_deactivate,
    Descriptor<PhaserI>::_cleanup,
    Descriptor<PhaserI>::_extension_data
    },
    {
    CAPS_URI "PhaserII",
    Descriptor<PhaserII>::_instantiate,
    Descriptor<PhaserII>::_connect_port,
    Descriptor<PhaserII>::_activate,
    Descriptor<PhaserII>::_run,
    Descriptor<PhaserII>::_deactivate,
    Descriptor<PhaserII>::_cleanup,
    Descriptor<PhaserII>::_extension_data
    },
    {
    CAPS_URI "SweepVFI",
    Descriptor<SweepVFI>::_instantiate,
    Descriptor<SweepVFI>::_connect_port,
    Descriptor<SweepVFI>::_activate,
    Descriptor<SweepVFI>::_run,
    Descriptor<SweepVFI>::_deactivate,
    Descriptor<SweepVFI>::_cleanup,
    Descriptor<SweepVFI>::_extension_data
    },
    {
    CAPS_URI "SweepVFII",
    Descriptor<SweepVFII>::_instantiate,
    Descriptor<SweepVFII>::_connect_port,
    Descriptor<SweepVFII>::_activate,
    Descriptor<SweepVFII>::_run,
    Descriptor<SweepVFII>::_deactivate,
    Descriptor<SweepVFII>::_cleanup,
    Descriptor<SweepVFII>::_extension_data
    },
    {
    CAPS_URI "AutoWah",
    Descriptor<AutoWah>::_instantiate,
    Descriptor<AutoWah>::_connect_port,
    Descriptor<AutoWah>::_activate,
    Descriptor<AutoWah>::_run,
    Descriptor<AutoWah>::_deactivate,
    Descriptor<AutoWah>::_cleanup,
    Descriptor<AutoWah>::_extension_data
    },
    {
    CAPS_URI "Scape",
    Descriptor<Scape>::_instantiate,
    Descriptor<Scape>::_connect_port,
    Descriptor<Scape>::_activate,
    Descriptor<Scape>::_run,
    Descriptor<Scape>::_deactivate,
    Descriptor<Scape>::_cleanup,
    Descriptor<Scape>::_extension_data
    },
    {
    CAPS_URI "VCOs",
    Descriptor<VCOs>::_instantiate,
    Descriptor<VCOs>::_connect_port,
    Descriptor<VCOs>::_activate,
    Descriptor<VCOs>::_run,
    Descriptor<VCOs>::_deactivate,
    Descriptor<VCOs>::_cleanup,
    Descriptor<VCOs>::_extension_data
    },
    {
    CAPS_URI "VCOd",
    Descriptor<VCOd>::_instantiate,
    Descriptor<VCOd>::_connect_port,
    Descriptor<VCOd>::_activate,
    Descriptor<VCOd>::_run,
    Descriptor<VCOd>::_deactivate,
    Descriptor<VCOd>::_cleanup,
    Descriptor<VCOd>::_extension_data
    },
    {
    CAPS_URI "CEO",
    Descriptor<CEO>::_instantiate,
    Descriptor<CEO>::_connect_port,
    Descriptor<CEO>::_activate,
    Descriptor<CEO>::_run,
    Descriptor<CEO>::_deactivate,
    Descriptor<CEO>::_cleanup,
    Descriptor<CEO>::_extension_data
    },
    {
    CAPS_URI "Sin",
    Descriptor<Sin>::_instantiate,
    Descriptor<Sin>::_connect_port,
    Descriptor<Sin>::_activate,
    Descriptor<Sin>::_run,
    Descriptor<Sin>::_deactivate,
    Descriptor<Sin>::_cleanup,
    Descriptor<Sin>::_extension_data
    },
    {
    CAPS_URI "White",
    Descriptor<White>::_instantiate,
    Descriptor<White>::_connect_port,
    Descriptor<White>::_activate,
    Descriptor<White>::_run,
    Descriptor<White>::_deactivate,
    Descriptor<White>::_cleanup,
    Descriptor<White>::_extension_data
    },
    {
    CAPS_URI "Lorenz",
    Descriptor<Lorenz>::_instantiate,
    Descriptor<Lorenz>::_connect_port,
    Descriptor<Lorenz>::_activate,
    Descriptor<Lorenz>::_run,
    Descriptor<Lorenz>::_deactivate,
    Descriptor<Lorenz>::_cleanup,
    Descriptor<Lorenz>::_extension_data
    },
    {
    CAPS_URI "Roessler",
    Descriptor<Roessler>::_instantiate,
    Descriptor<Roessler>::_connect_port,
    Descriptor<Roessler>::_activate,
    Descriptor<Roessler>::_run,
    Descriptor<Roessler>::_deactivate,
    Descriptor<Roessler>::_cleanup,
    Descriptor<Roessler>::_extension_data
    },
    {
    CAPS_URI "JVRev",
    Descriptor<JVRev>::_instantiate,
    Descriptor<JVRev>::_connect_port,
    Descriptor<JVRev>::_activate,
    Descriptor<JVRev>::_run,
    Descriptor<JVRev>::_deactivate,
    Descriptor<JVRev>::_cleanup,
    Descriptor<JVRev>::_extension_data
    },
    {
    CAPS_URI "Plate",
    Descriptor<Plate>::_instantiate,
    Descriptor<Plate>::_connect_port,
    Descriptor<Plate>::_activate,
    Descriptor<Plate>::_run,
    Descriptor<Plate>::_deactivate,
    Descriptor<Plate>::_cleanup,
    Descriptor<Plate>::_extension_data
    },
    {
    CAPS_URI "Plate2x2",
    Descriptor<Plate2x2>::_instantiate,
    Descriptor<Plate2x2>::_connect_port,
    Descriptor<Plate2x2>::_activate,
    Descriptor<Plate2x2>::_run,
    Descriptor<Plate2x2>::_deactivate,
    Descriptor<Plate2x2>::_cleanup,
    Descriptor<Plate2x2>::_extension_data
    },
    {
    CAPS_URI "Click",
    Descriptor<Click>::_instantiate,
    Descriptor<Click>::_connect_port,
    Descriptor<Click>::_activate,
    Descriptor<Click>::_run,
    Descriptor<Click>::_deactivate,
    Descriptor<Click>::_cleanup,
    Descriptor<Click>::_extension_data
    },
    {
    CAPS_URI "Dirac",
    Descriptor<Dirac>::_instantiate,
    Descriptor<Dirac>::_connect_port,
    Descriptor<Dirac>::_activate,
    Descriptor<Dirac>::_run,
    Descriptor<Dirac>::_deactivate,
    Descriptor<Dirac>::_cleanup,
    Descriptor<Dirac>::_extension_data
    },
    {
    CAPS_URI "HRTF",
    Descriptor<HRTF>::_instantiate,
    Descriptor<HRTF>::_connect_port,
    Descriptor<HRTF>::_activate,
    Descriptor<HRTF>::_run,
    Descriptor<HRTF>::_deactivate,
    Descriptor<HRTF>::_cleanup,
    Descriptor<HRTF>::_extension_data
    }
};

static const LV2_Descriptor EQ_Descriptor = {
    CAPS_URI "Eq",
    Descriptor<Eq>::_instantiate,
    Descriptor<Eq>::_connect_port,
    Descriptor<Eq>::_activate,
    Descriptor<Eq>::_run,
    Descriptor<Eq>::_deactivate,
    Descriptor<Eq>::_cleanup,
    Descriptor<Eq>::_extension_data
};


LV2_SYMBOL_EXPORT
const LV2_Descriptor* lv2_descriptor(uint32_t index)
{
    if (index < N) return &Descriptors[index];
    else return NULL;
}
