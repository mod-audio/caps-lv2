#!/usr/bin/env python
# -*- coding: utf-8 -*-

from __future__ import print_function

import os, re
from pyparsing import nestedExpr
from math import exp, log

lv2_categ = ['DelayPlugin', 'ReverbPlugin', 'DistortionPlugin', 'WaveshaperPlugin', 'DynamicsPlugin', 'AmplifierPlugin', 'CompressorPlugin', 'ExpanderPlugin', 'GatePlugin', 'LimiterPlugin', 'FilterPlugin', 'AllpassPlugin', 'BandpassPlugin', 'CombPlugin', 'EQPlugin', 'MultiEQPlugin', 'ParaEQPlugin', 'HighpassPlugin', 'LowpassPlugin', 'GeneratorPlugin', 'ConstantPlugin', 'InstrumentPlugin', 'OscillatorPlugin', 'ModulatorPlugin', 'ChorusPlugin', 'FlangerPlugin', 'PhaserPlugin', 'ReverbPlugin', 'SimulatorPlugin', 'ReverbPlugin', 'SpatialPlugin', 'SpectralPlugin', 'PitchPlugin', 'UtilityPlugin', 'AnalyserPlugin', 'ConverterPlugin', 'FunctionPlugin', 'MixerPlugin']

def parse_port_info(port_info):
    info = ''
    port_info = port_info.replace('|', '')
    port_info = port_info.replace('MIN_GAIN,', '0.000001').replace('M_PI * .5', '1.57079632679489661923').replace('M_PI', '3.14159265358979323846')
    port_info = port_info.replace('TS_N_PRESETS - 1', '5')
    for port in port_info.split('\n'):
        info += port.strip()

    ports = []
    for a in nestedExpr('{','}').parseString(info).asList()[0]:
        if a != ',':
            ports.append(a)
    ports.pop(-1)

    ttl = '    lv2:port\n'
    index = 0
    for p in ports:
        name = p[0]
        input_output = 'InputPort' if 'INPUT' in p[1] else 'OutputPort'
        audio_control = 'AudioPort' if 'AUDIO' in p[2] else 'ControlPort'
        symbol = re.sub(r'[^\w]', '', name).lower()
        if 'BOUNDED' in p[3][0]:
            minimum = p[3][-2].replace(',','')
            maximum = p[3][-1].replace(',','')
            lower = float(minimum)
            upper = float(maximum)

        logarithmic = False
        integer = False
        if len(p[3]) > 1:
            logarithmic = True if 'LOG' in p[3][1] else False
            integer = True if 'INTEGER' in p[3][1] else False

            _def = ''
            if 'DEFAULT' in p[3][1]:
                _def = p[3][1].replace(',','')
            elif 'DEFAULT' in p[3][2]:
                _def = p[3][2].replace(',','')

            default = '????'
            if _def == 'DEFAULT_0':
                default = '0.0'
            if _def == 'DEFAULT_1':
                default = '1.0'
            if _def == 'DEFAULT_100':
                default = '100.0'
            if _def == 'DEFAULT_440':
                default = '440.0'
            if _def == 'DEFAULT_MIN':
                default = minimum
            if _def == 'DEFAULT_LOW':
                default = exp(log(lower) * 0.5 + log(upper) * 0.5) if logarithmic else (lower * 0.5 + upper * 0.5)
                default = str(default)
            if _def == 'DEFAULT_MID':
                default = exp(log(lower) * 0.5 + log(upper) * 0.5) if logarithmic else (lower * 0.5 + upper *0.5)
                default = str(default)
            if _def == 'DEFAULT_HIGH':
                default = exp(log(lower) * 0.25 + log(upper) * 0.75) if logarithmic else (lower * 0.25 + upper * 0.75)
                default = str(default)
            if _def == 'DEFAULT_MAX':
                default = maximum

        port =  '    [\n'
        port += '        a lv2:' + audio_control + ', lv2:' + input_output + ';\n'
        port += '        lv2:index ' + str(index) + ';\n'
        port += '        lv2:symbol \"' + symbol + '\";\n'
        port += '        lv2:name ' + name + ';\n'

        if integer:
            port += '        lv2:portProperty lv2:integer;\n'

        if logarithmic:
            port += '        lv2:portProperty epp:logarithmic;\n'

        if audio_control == 'ControlPort':
            port += '        lv2:default ' + default + ';\n'
            port += '        lv2:minimum ' + minimum + ';\n'
            port += '        lv2:maximum ' + maximum + ';\n'

        port += '    ],\n'

        index += 1
        ttl += port

    ttl = ttl[:-2]
    return ttl


def main():
    categ_rdf = open('caps.rdf', 'r').read()

    ttl = ''
    ttl += '@prefix lv2:  <http://lv2plug.in/ns/lv2core#>.\n'
    ttl += '@prefix doap: <http://usefulinc.com/ns/doap#>.\n'
    ttl += '@prefix epp:  <http://lv2plug.in/ns/ext/port-props/#>.\n'
    ttl += '@prefix foaf: <http://xmlns.com/foaf/0.1/>.\n'
    ttl += '@prefix rdf:  <http://www.w3.org/1999/02/22-rdf-syntax-ns#>.\n'
    ttl += '@prefix rdfs: <http://www.w3.org/2000/01/rdf-schema#>.\n\n'

    manifest = '@prefix lv2: <http://lv2plug.in/ns/lv2core#>.\n'
    manifest += '@prefix rdfs: <http://www.w3.org/2000/01/rdf-schema#>.\n\n'

    os.chdir('src')
    for dirname, dirnames, filenames in os.walk('.'):
        for filename in filenames:
            if filename.find('.cc') >= 0:
                content = open(filename, 'r').read()

                start = 0
                name_s = 0
                uri_s = 0
                for i in range(content.count('::port_info')):
                    start = content.find('::port_info', start)
                    end = start + 1

                    # uri
                    uri_s = content.find('Label =', uri_s) + len('Label =') + 1
                    uri_e = content.find(';', uri_s)
                    uri = content[uri_s:uri_e].replace('"', '')

                    manifest += '<http://quitte.de/dsp/caps/lv2/' + uri + '> a lv2:Plugin; lv2:binary <caps.so>; rdfs:seeAlso <caps.ttl>.\n'

                    # name
                    name_s = content.find('Name =', name_s) + len('Name =')
                    name_e = content.find(';', name_s)
                    name = content[name_s:name_e].replace('CAPS "', '"C* ')

                    if start >= 0:
                        start = content.find('{', start)
                        end = content.find('};', start) + 1
                        port_info = content[start:end]
                        port_info = parse_port_info(port_info)

                        cat_s = categ_rdf.find('\n', categ_rdf.find(uri)) + 1
                        cat_e = categ_rdf.find('\n', cat_s)
                        categ = categ_rdf[cat_s:cat_e].replace(' ', '').replace('\t', '').replace('<ladspa:', '')

                        if categ == 'TimePlugin':
                            categ = 'UtilityPlugin'

                        ttl += '<http://quitte.de/dsp/caps/lv2/' + uri + '> a lv2:Plugin, lv2:' + categ + ';\n'
                        ttl += '    doap:name ' + name + ';\n'
                        ttl += '    doap:developer [\n'
                        ttl += '        foaf:name "Tim Goetze";\n'
                        ttl += '        foaf:homepage <http://quitte.de/dsp/caps.html>;\n'
                        ttl += '        foaf:mbox <mailto:tim@quitte.de>;\n'
                        ttl += '        ];\n'
                        ttl += '    doap:maintainer [\n'
                        ttl += '        foaf:name "Tim Goetze";\n'
                        ttl += '        foaf:homepage <http://quitte.de/dsp/caps.html>;\n'
                        ttl += '        foaf:mbox <mailto:tim@quitte.de>;\n'
                        ttl += '        ];\n'
                        ttl += '    doap:license <http://usefulinc.com/doap/licenses/gpl>;\n'
                        ttl += '    lv2:optionalFeature lv2:hardRtCapable;\n'
                        ttl += port_info + '.\n'

                    start = end

    f = open('manifest.ttl', 'w')
    f.write(manifest)
    f.close()
    f = open('caps.ttl', 'w')
    f.write(ttl)
    f.close()


    os.chdir('..')

if __name__ == "__main__":
    main()
