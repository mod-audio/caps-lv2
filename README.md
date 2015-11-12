caps-lv2 0.9.24
===============

LV2 port for the [CAPS Audio Plugin Suite](http://quitte.de/dsp/caps.html)

LV2 Port
========

The major changes to the original LADSPA caps were:

- support to LV2_Descriptor in the Descriptor Class (Descriptor.h)
- small changes to main Makefile
- build each plugin separately

So it's really easy to keep in track with new upstream releases

Compile and Install
===================

    make

    sudo make install

The default LV2 path installation is /usr/local/lv2/

CAPS Plugins Documentation
==========================

For all questions, please be referred to the documentation at

    http://quitte.de/dsp/caps.html

If that doesn't help, you can contact

    tim@quitte.de
