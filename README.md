caps-lv2 0.9.20
===============

LV2 port for the [CAPS Audio Plugin Suite](http://quitte.de/dsp/caps.html)

LV2 Port
========

The major changes to the original LADSPA caps were:

- support to LV2_Descriptor in the Descriptor Class (Descriptor.h)
- added LV2 descriptors in interface.cc
- small changes in Makefile

So it's really easy to keep in track with new upstream releases

Compile and Install
===================

    make

    sudo make install

The default LV2 path installation is /usr/local/lv2/caps.lv2/

LV2 and LADSPA are hosted in same shared object, so when you install you will have both versions

Alternatively you can install the plugins in single format, in other words, one shared object per plugin.
To do this run:

    ./make_single_plugins

    sudo ./make_single_plugins install

This will install all plugins, if you want install specific plugins, you can do in this way:

    sudo ./make_single_plugins install AmpVTS ToneStack


CAPS Plugins Documentation
==========================

For all questions, please be referred to the documentation at

    http://quitte.de/dsp/caps.html

If that doesn't help, you can contact

    tim@quitte.de
