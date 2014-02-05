#!/usr/bin/env python

import os, sys, glob

TMP_DIR = '.tmp'
os.system('mkdir -p ' + TMP_DIR)
os.system('cp -ruf ./dsp ' + TMP_DIR)
os.system('cp -ruf ./waves ' + TMP_DIR)
os.system('cp -ruf ./*.h ' + TMP_DIR)
os.system('cp -ruf ./*.cc ' + TMP_DIR)
os.system('cp -ruf ./Makefile ' + TMP_DIR)

dsp_sources = ' '.join(glob.glob('dsp/*.cc'))

# LADSPA_UID, EFFECT_NAME, FILES
effects_info = [
('2602',    'Noisegate',        ['Noisegate.cc']),
('1772',    'Compress',         ['Compress.cc']),
('2598',    'CompressX2',       ['Compress.cc']),
('2589',    'ToneStack',        ['ToneStack.cc']),
('2592',    'AmpVTS',           ['Amp.cc','ToneStack.cc']),
('2606',    'CabinetIV',        ['Cabinet.cc']),
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

#define N 1

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
    *d++ = new Descriptor<__EFFECT__>(__LADSPA_UID__);
    assert (d - descriptors <= N);

    /* LV2 */
    d = lv2_descriptors;
    memset (d, 0, sizeof (lv2_descriptors));
    *d++ = new Descriptor<__EFFECT__>(CAPS_URI "__EFFECT__");
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
"""

manifest = """
@prefix lv2: <http://lv2plug.in/ns/lv2core#>.
@prefix rdfs: <http://www.w3.org/2000/01/rdf-schema#>.

<http://quitte.de/dsp/caps.html#__EFFECT__> a lv2:Plugin; lv2:binary <__EFFECT__.so>; rdfs:seeAlso <__EFFECT__.ttl>.
"""

manifest_mod = """
@prefix lv2: <http://lv2plug.in/ns/lv2core#>.
@prefix rdfs: <http://www.w3.org/2000/01/rdf-schema#>.

<http://portalmod.com/plugins/caps/__EFFECT__> a lv2:Plugin; lv2:binary <__EFFECT__.so>; rdfs:seeAlso <__EFFECT__.ttl>.
"""

# comment the below line to use the caps URI
manifest = manifest_mod

if __name__ == "__main__":
    os.chdir(TMP_DIR)

    if manifest == manifest_mod:
        f = open('./basics.h', 'r')
        basics = f.read()
        basics = basics.replace('#define CAPS_URI "http://quitte.de/dsp/caps.html#"', '#define CAPS_URI "http://portalmod.com/plugins/caps/"')
        f = open('./basics.h', 'w')
        f.write(basics);
        f.close()

    def get_source(fx_name):
        for fx in effects_info:
            if fx_name == fx[1]:
                return ' '.join(fx[2])

    if len(sys.argv) == 1:
        for fx in effects_info:
            ladspa_uid = fx[0]
            effect_name = fx[1]
            files = ' '.join(fx[2])

            # composes the interface code
            interface = interface_code
            interface = interface.replace('__LADSPA_UID__', ladspa_uid)
            interface = interface.replace('__EFFECT__', effect_name)

            # create the interface file
            f = open('interface.cc', 'w')
            f.writelines(interface)
            f.close()

            # sources and plugin name
            sources = ' SOURCES=\"interface.cc ' + dsp_sources + ' ' + files + '\"'
            plugin_name = ' PLUG=' + effect_name

            # run make
            os.system('make' + sources + plugin_name)
    else:
        fx_names = []
        for effect in effects_info:
            fx_names.append(effect[1])

        sys.argv.pop(0)
        effects_in_args = list(set(fx_names) & set(sys.argv))
        args = list(set(sys.argv) - set(effects_in_args))

        if 'install' in args or 'install-lv2' in args:
            if effects_in_args == []: effects_in_args = fx_names

            for fx in effects_in_args:
                # create a fake rdf file to avoid error on make install
                os.system('touch ' + fx + '.rdf')
                rdf_dest = ' RDFDEST=/tmp/rdf'

                # ttl files
                os.system('rm -rf ./ttl')
                os.system('mkdir ./ttl')
                os.system('cp ../ttl/' + fx + '.ttl ./ttl')

                # modgui
                os.system('mkdir -p ./ttl/modgui/')
                os.system('cp ../ttl/modgui/' + fx + '.* ./ttl/modgui/')

                f = open('./ttl/manifest.ttl', 'w')
                f.write(manifest.replace('__EFFECT__', fx))
                f.close()

                sources = ' SOURCES=\"\" HEADERS=\"\" '
                plugin_name = ' PLUG=' + fx
                bundle_name = ' LV2BUNDLE=caps-' + fx + '.lv2'

                os.system('make ' + ' '.join(args) + sources + plugin_name + bundle_name + rdf_dest)

        elif 'clean' in args:
            os.system('make ' + ' '.join(args) + ' PLUG=*')
