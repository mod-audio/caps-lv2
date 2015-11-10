#!/usr/bin/env python

import os, sys, glob

# LADSPA_UID, EFFECT_NAME, FILES
effects_info = [
('2602',    'Noisegate',        ['Noisegate.cc']),
('1772',    'Compress',         ['Compress.cc']),
('2598',    'CompressX2',       ['Compress.cc']),
('2589',    'ToneStack',        ['ToneStack.cc']),
('2592',    'AmpVTS',           ['Amp.cc','ToneStack.cc']),
('2601',    'CabinetIII',       ['CabIII.cc']),
('2606',    'CabinetIV',        ['CabIV.cc']),
('1779',    'Plate',            ['Reverb.cc']),
('1795',    'PlateX2',          ['Reverb.cc']),
('1771',    'Saturate',         ['Saturate.cc']),
('2603',    'Spice',            ['Saturate.cc']),
('2607',    'SpiceX2',          ['Saturate.cc']),
('1767',    'ChorusI',          ['Chorus.cc']),
('2586',    'PhaserII',         ['Phaser.cc']),
('2593',    'AutoFilter',       ['AutoFilter.cc']),
('2588',    'Scape',            ['Scape.cc']),
('1773',    'Eq10',             ['Eq.cc']),
('2594',    'Eq10X2',           ['Eq.cc']),
('2608',    'Eq4p',             ['Eq.cc']),
('2609',    'EqFA4p',           ['Eq.cc']),
('1788',    'Wider',            ['Pan.cc']),
('2595',    'Narrower',         ['Pan.cc']),
('1781',    'Sin',              ['Sin.cc']),
('1785',    'White',            ['White.cc']),
('1774',    'Fractal',          ['Fractals.cc']),
('1769',    'Click',            ['Click.cc']),
('1770',    'CEO',              ['Click.cc'])
]

interface_code = """
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
#include "Noisegate.h"

#include "Descriptor.h"

LV2_SYMBOL_EXPORT
const LV2_Descriptor *
lv2_descriptor(uint32_t i)
{
    static const Descriptor<__EFFECT__> lv2_descriptor(CAPS_URI "__EFFECT__");
    return i == 0 ? &lv2_descriptor : 0;
}
"""

manifest = """\
@prefix dct:  <http://purl.org/dc/terms/> .
@prefix lv2:  <http://lv2plug.in/ns/lv2core#> .
@prefix rdfs: <http://www.w3.org/2000/01/rdf-schema#> .

<http://moddevices.com/plugins/caps/__EFFECT__>
    a lv2:Plugin ;
    dct:replaces <urn:ladspa:__LADSPA_UID__> ;
    lv2:binary <__EFFECT__.so> ;
    rdfs:seeAlso <__EFFECT__.ttl> ,
                 <modgui.ttl> .
"""

if __name__ == "__main__":
    with open("Makefile.single", 'r') as fh:
        makefile_single = fh.read()

    def get_source(fx_name):
        for fx in effects_info:
            if fx_name == fx[1]:
                return ' '.join(fx[2])

    for ladspa_uid, effect_name, sources in effects_info:
        sources.append("dsp/polynomials.cc")
        sources  = "../../%s" % (" ../../".join(sources))
        sources += " interface.cc"
        bundlepath = "plugins/mod-caps-%s.lv2" % effect_name

        # Create dir
        if not os.path.exists(bundlepath):
            os.mkdir(bundlepath)

        # Create interface.cc
        with open(os.path.join(bundlepath, "interface.cc"), 'w') as fh:
            fh.write(interface_code.replace("__EFFECT__", effect_name))

        # Create manifest.ttl
        with open(os.path.join(bundlepath, "manifest.ttl"), 'w') as fh:
            fh.write(manifest.replace("__LADSPA_UID__", ladspa_uid).replace("__EFFECT__", effect_name))

        # Create Makefile
        with open(os.path.join(bundlepath, "Makefile"), 'w') as fh:
            fh.write(makefile_single.replace("__EFFECT__", effect_name).replace("__SOURCES__", sources))
