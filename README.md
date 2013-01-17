caps-lv2 0.9.7
==============

LV2 port for the CAPS Audio Plugin Suite

port
====

The major changes to the original LADSPA caps were:

- new Descriptor.h 
- new interface.cc 
- #include <lv2.h> in basics.h

So it's really easy to keep in track with new upstream releases

compile and install
===================

make

make install

(currently make install will create and install to $HOME/.lv2/caps.lv2/)

caps
====

For all questions about CAPS, please be referred to the HTML documentation of this software package in the file 'caps.html', also available from:
 
http://quitte.de/dsp/caps.html
