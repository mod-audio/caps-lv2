PREFIX = /usr/local

CC = g++

OPTS = -O6 -ffast-math -funroll-loops -Wall -fPIC -DPIC
#OPTS = -g -DDEBUG

_LDFLAGS = -shared
STRIP = strip

-include defines.make

CFLAGS = $(OPTS) $(_CFLAGS)
LDFLAGS = $(_LDFLAGS) $(CFLAGS)

LV2FLAGS = -DAMPVTS_ -DAUTOFILTER_ -DCABINETII_ -DCABINETIII_ -DCABINETIV_ -DCEO_ -DCHORUSI_ -DCHORUSII_ -DCLICK_ -DCOMPRESS2X2_ -DCOMPRESS_ -DDIRAC_ -DEQ2X2_ -DEQ_ -DJVREV_ -DLORENZ_ -DNARROWER_ -DNOISEGATE_ -DPAN_ -DPHASERII_ -DPLATE2X2_ -DPLATE_ -DROESSLER_ -DSATURATE_ -DSCAPE_ -DSIN_ -DSTEREOCHORUSII2X2_ -DSTEREOCHORUSII_ -DSTEREOPHASERII2X2_ -DSTEREOPHASERII_ -DTONESTACK_ -DWHITE_
LV2RULES = AMPVTS_ AUTOFILTER_ CABINETII_ CABINETIII_ CABINETIV_ CEO_ CHORUSI_ CHORUSII_ CLICK_ COMPRESS2X2_ COMPRESS_ DIRAC_ EQ2X2_ EQ_ JVREV_ LORENZ_ NARROWER_ NOISEGATE_ PAN_ PHASERII_ PLATE2X2_ PLATE_ ROESSLER_ SATURATE_ SCAPE_ SIN_ STEREOCHORUSII2X2_ STEREOCHORUSII_ STEREOPHASERII2X2_ STEREOPHASERII_ TONESTACK_ WHITE_

VERSION = 0.9.7
PLUG = caps

SOURCES = $(wildcard *.cc) $(wildcard dsp/*.cc)
OBJECTS	= $(SOURCES:.cc=.o)
HEADERS = $(wildcard *.h) $(wildcard dsp/*.h) $(wildcard util/*.h) $(wildcard dsp/tonestack/*.h)

PDF = releases/caps-$(VERSION).pdf

DEST = ~/.lv2/$(PLUG).lv2/

# targets following -------------------------------------------------------------

all: LADSPAFLAG = -DLADSPAFLAG
all: depend $(PLUG).so tags

run: all
	python -i bin/rack.py White AutoFilter cream.Audio.Meter Pan
	#python -i bin/rack.py White AutoFilter Pan
	#@~/cream/gdb-python html/graph.py Compress,spectrum.png

rdf: $(PLUG).rdf
$(PLUG).rdf: all tools/make-rdf.py
	python tools/make-rdf.py > $(PLUG).rdf

$(PLUG).so: $(OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $(OBJECTS)

.cc.s:
	$(CC) $(CFLAGS) -S $<

.cc.o: depend
	$(CC) $(CFLAGS) $(LADSPAFLAG) -o $@ -c $<

tags: $(SOURCES) $(HEADERS)
	@echo making tags
	@-if [ -x /usr/bin/ctags ]; then ctags $(SOURCES) $(HEADERS) ; fi

installLV2all:
	@$(STRIP) $(PLUG) > /dev/null
	install -d $(DEST)
	install -m 644 $(PLUG) $(DEST)
	install -m 644 ttls/* $(DEST)
	
installLV2single:
	./make_install.sh

installLADSPA: all
	@$(STRIP) $(PLUG).so > /dev/null
	install -d $(DEST)
	install -m 644 $(PLUG).so $(DEST)
	install -d $(RDFDEST)
	install -m 644 $(PLUG).rdf $(RDFDEST)

fake-install: all
	-rm $(DEST)/$(PLUG).so
	ln -s `pwd`/$(PLUG).so $(DEST)/$(PLUG).so
	-rm $(RDFDEST)/$(PLUG).rdf
	ln -s `pwd`/$(PLUG).rdf $(RDFDEST)/$(PLUG).rdf

rdf-install:
	install -d $(RDFDEST)
	install -m 644 $(PLUG).rdf $(RDFDEST)

uninstall:
	-rm $(DEST)/$(PLUG).so
	-rm $(DEST)/$(PLUG)-ng.so

clean:
	rm -f $(OBJECTS) *.so *.s depend

version.h:
	@VERSION=$(VERSION) python tools/make-version.h.py

dist: all $(PLUG).rdf version.h
	-rm doc/*.html
	tools/make-dist.py caps $(VERSION) $(CFLAGS)

depend: $(SOURCES) $(HEADERS)
	$(CC) -MM $(CFLAGS) $(DEFINES) $(SOURCES) > depend

-include depend

######## MOD TEAM CHANGES

LV2all: $(OBJECTS)
	$(CC) $(LDFLAGS) $(LV2FLAGS) -o interface.o -c interface.cc
	$(CC) $(LDFLAGS) -o $(PLUG).so $(OBJECTS)
	cp ttls/manifest.all .
	mv manifest.all manifest.ttl
	mv manifest.ttl ttls/

LV2single: $(LV2RULES)
	rm manifest.ttl.*
	cp ttls/manifest.single .
	mv manifest.single manifest.ttl
	mv manifest.ttl ttls/

AMPVTS_: Amp.o ToneStack.o dsp/polynomials.o

	echo "@prefix lv2: <http://lv2plug.in/ns/lv2core#>.\n\
	@prefix rdfs: <http://www.w3.org/2000/01/rdf-schema#>.\n\n\
	<http://portalmod.com/plugins/caps/`echo $(@:_=) | tr A-Z a-z`> a lv2:Plugin; lv2:binary \
	<`echo $(@:_=) | tr A-Z a-z`.so>; rdfs:seeAlso <`echo $(@:_=) | tr A-Z a-z`.ttl>." > manifest.ttl.`echo $(@:_=) | tr A-Z a-z`

	$(CC) $(LDFLAGS) -D$@ -o interface.o -c interface.cc
	$(CC) $(LDFLAGS) -o `echo $(@:_=) | tr A-Z a-z`.so interface.o $?

AUTOFILTER_: AutoFilter.o dsp/polynomials.o Amp.o ToneStack.o

	echo "@prefix lv2: <http://lv2plug.in/ns/lv2core#>.\n\
	@prefix rdfs: <http://www.w3.org/2000/01/rdf-schema#>.\n\n\
	<http://portalmod.com/plugins/caps/`echo $(@:_=) | tr A-Z a-z`> a lv2:Plugin; lv2:binary \
	<`echo $(@:_=) | tr A-Z a-z`.so>; rdfs:seeAlso <`echo $(@:_=) | tr A-Z a-z`.ttl>." > manifest.ttl.`echo $(@:_=) | tr A-Z a-z`

	$(CC) $(LDFLAGS) -D$@ -o interface.o -c interface.cc
	$(CC) $(LDFLAGS) -o `echo $(@:_=) | tr A-Z a-z`.so interface.o $?

CABINETII_: Cabinet.o

	echo "@prefix lv2: <http://lv2plug.in/ns/lv2core#>.\n\
	@prefix rdfs: <http://www.w3.org/2000/01/rdf-schema#>.\n\n\
	<http://portalmod.com/plugins/caps/`echo $(@:_=) | tr A-Z a-z`> a lv2:Plugin; lv2:binary \
	<`echo $(@:_=) | tr A-Z a-z`.so>; rdfs:seeAlso <`echo $(@:_=) | tr A-Z a-z`.ttl>." > manifest.ttl.`echo $(@:_=) | tr A-Z a-z`

	$(CC) $(LDFLAGS) -D$@ -o interface.o -c interface.cc
	$(CC) $(LDFLAGS) -o `echo $(@:_=) | tr A-Z a-z`.so interface.o $?

CABINETIII_: Cabinet.o

	echo "@prefix lv2: <http://lv2plug.in/ns/lv2core#>.\n\
	@prefix rdfs: <http://www.w3.org/2000/01/rdf-schema#>.\n\n\
	<http://portalmod.com/plugins/caps/`echo $(@:_=) | tr A-Z a-z`> a lv2:Plugin; lv2:binary \
	<`echo $(@:_=) | tr A-Z a-z`.so>; rdfs:seeAlso <`echo $(@:_=) | tr A-Z a-z`.ttl>." > manifest.ttl.`echo $(@:_=) | tr A-Z a-z`

	$(CC) $(LDFLAGS) -D$@ -o interface.o -c interface.cc
	$(CC) $(LDFLAGS) -o `echo $(@:_=) | tr A-Z a-z`.so interface.o $?

CABINETIV_: Cabinet.o

	echo "@prefix lv2: <http://lv2plug.in/ns/lv2core#>.\n\
	@prefix rdfs: <http://www.w3.org/2000/01/rdf-schema#>.\n\n\
	<http://portalmod.com/plugins/caps/`echo $(@:_=) | tr A-Z a-z`> a lv2:Plugin; lv2:binary \
	<`echo $(@:_=) | tr A-Z a-z`.so>; rdfs:seeAlso <`echo $(@:_=) | tr A-Z a-z`.ttl>." > manifest.ttl.`echo $(@:_=) | tr A-Z a-z`

	$(CC) $(LDFLAGS) -D$@ -o interface.o -c interface.cc
	$(CC) $(LDFLAGS) -o `echo $(@:_=) | tr A-Z a-z`.so interface.o $?

CEO_: Click.o

	echo "@prefix lv2: <http://lv2plug.in/ns/lv2core#>.\n\
	@prefix rdfs: <http://www.w3.org/2000/01/rdf-schema#>.\n\n\
	<http://portalmod.com/plugins/caps/`echo $(@:_=) | tr A-Z a-z`> a lv2:Plugin; lv2:binary \
	<`echo $(@:_=) | tr A-Z a-z`.so>; rdfs:seeAlso <`echo $(@:_=) | tr A-Z a-z`.ttl>." > manifest.ttl.`echo $(@:_=) | tr A-Z a-z`

	$(CC) $(LDFLAGS) -D$@ -o interface.o -c interface.cc
	$(CC) $(LDFLAGS) -o `echo $(@:_=) | tr A-Z a-z`.so interface.o $?

CHORUSI_: Chorus.o

	echo "@prefix lv2: <http://lv2plug.in/ns/lv2core#>.\n\
	@prefix rdfs: <http://www.w3.org/2000/01/rdf-schema#>.\n\n\
	<http://portalmod.com/plugins/caps/`echo $(@:_=) | tr A-Z a-z`> a lv2:Plugin; lv2:binary \
	<`echo $(@:_=) | tr A-Z a-z`.so>; rdfs:seeAlso <`echo $(@:_=) | tr A-Z a-z`.ttl>." > manifest.ttl.`echo $(@:_=) | tr A-Z a-z`

	$(CC) $(LDFLAGS) -D$@ -o interface.o -c interface.cc
	$(CC) $(LDFLAGS) -o `echo $(@:_=) | tr A-Z a-z`.so interface.o $?

CHORUSII_: Chorus.o

	echo "@prefix lv2: <http://lv2plug.in/ns/lv2core#>.\n\
	@prefix rdfs: <http://www.w3.org/2000/01/rdf-schema#>.\n\n\
	<http://portalmod.com/plugins/caps/`echo $(@:_=) | tr A-Z a-z`> a lv2:Plugin; lv2:binary \
	<`echo $(@:_=) | tr A-Z a-z`.so>; rdfs:seeAlso <`echo $(@:_=) | tr A-Z a-z`.ttl>." > manifest.ttl.`echo $(@:_=) | tr A-Z a-z`

	$(CC) $(LDFLAGS) -D$@ -o interface.o -c interface.cc
	$(CC) $(LDFLAGS) -o `echo $(@:_=) | tr A-Z a-z`.so interface.o $?

CLICK_: Click.o

	echo "@prefix lv2: <http://lv2plug.in/ns/lv2core#>.\n\
	@prefix rdfs: <http://www.w3.org/2000/01/rdf-schema#>.\n\n\
	<http://portalmod.com/plugins/caps/`echo $(@:_=) | tr A-Z a-z`> a lv2:Plugin; lv2:binary \
	<`echo $(@:_=) | tr A-Z a-z`.so>; rdfs:seeAlso <`echo $(@:_=) | tr A-Z a-z`.ttl>." > manifest.ttl.`echo $(@:_=) | tr A-Z a-z`

	$(CC) $(LDFLAGS) -D$@ -o interface.o -c interface.cc
	$(CC) $(LDFLAGS) -o `echo $(@:_=) | tr A-Z a-z`.so interface.o $?

COMPRESS2X2_: Compress.o dsp/polynomials.o

	echo "@prefix lv2: <http://lv2plug.in/ns/lv2core#>.\n\
	@prefix rdfs: <http://www.w3.org/2000/01/rdf-schema#>.\n\n\
	<http://portalmod.com/plugins/caps/`echo $(@:_=) | tr A-Z a-z`> a lv2:Plugin; lv2:binary \
	<`echo $(@:_=) | tr A-Z a-z`.so>; rdfs:seeAlso <`echo $(@:_=) | tr A-Z a-z`.ttl>." > manifest.ttl.`echo $(@:_=) | tr A-Z a-z`

	$(CC) $(LDFLAGS) -D$@ -o interface.o -c interface.cc
	$(CC) $(LDFLAGS) -o `echo $(@:_=) | tr A-Z a-z`.so interface.o $?

COMPRESS_: Compress.o dsp/polynomials.o

	echo "@prefix lv2: <http://lv2plug.in/ns/lv2core#>.\n\
	@prefix rdfs: <http://www.w3.org/2000/01/rdf-schema#>.\n\n\
	<http://portalmod.com/plugins/caps/`echo $(@:_=) | tr A-Z a-z`> a lv2:Plugin; lv2:binary \
	<`echo $(@:_=) | tr A-Z a-z`.so>; rdfs:seeAlso <`echo $(@:_=) | tr A-Z a-z`.ttl>." > manifest.ttl.`echo $(@:_=) | tr A-Z a-z`

	$(CC) $(LDFLAGS) -D$@ -o interface.o -c interface.cc
	$(CC) $(LDFLAGS) -o `echo $(@:_=) | tr A-Z a-z`.so interface.o $?

DIRAC_: Click.o

	echo "@prefix lv2: <http://lv2plug.in/ns/lv2core#>.\n\
	@prefix rdfs: <http://www.w3.org/2000/01/rdf-schema#>.\n\n\
	<http://portalmod.com/plugins/caps/`echo $(@:_=) | tr A-Z a-z`> a lv2:Plugin; lv2:binary \
	<`echo $(@:_=) | tr A-Z a-z`.so>; rdfs:seeAlso <`echo $(@:_=) | tr A-Z a-z`.ttl>." > manifest.ttl.`echo $(@:_=) | tr A-Z a-z`

	$(CC) $(LDFLAGS) -D$@ -o interface.o -c interface.cc
	$(CC) $(LDFLAGS) -o `echo $(@:_=) | tr A-Z a-z`.so interface.o $?

EQ2X2_: Eq.o

	echo "@prefix lv2: <http://lv2plug.in/ns/lv2core#>.\n\
	@prefix rdfs: <http://www.w3.org/2000/01/rdf-schema#>.\n\n\
	<http://portalmod.com/plugins/caps/`echo $(@:_=) | tr A-Z a-z`> a lv2:Plugin; lv2:binary \
	<`echo $(@:_=) | tr A-Z a-z`.so>; rdfs:seeAlso <`echo $(@:_=) | tr A-Z a-z`.ttl>." > manifest.ttl.`echo $(@:_=) | tr A-Z a-z`

	$(CC) $(LDFLAGS) -D$@ -o interface.o -c interface.cc
	$(CC) $(LDFLAGS) -o `echo $(@:_=) | tr A-Z a-z`.so interface.o $?

EQ_: Eq.o

	echo "@prefix lv2: <http://lv2plug.in/ns/lv2core#>.\n\
	@prefix rdfs: <http://www.w3.org/2000/01/rdf-schema#>.\n\n\
	<http://portalmod.com/plugins/caps/`echo $(@:_=) | tr A-Z a-z`> a lv2:Plugin; lv2:binary \
	<`echo $(@:_=) | tr A-Z a-z`.so>; rdfs:seeAlso <`echo $(@:_=) | tr A-Z a-z`.ttl>." > manifest.ttl.`echo $(@:_=) | tr A-Z a-z`

	$(CC) $(LDFLAGS) -D$@ -o interface.o -c interface.cc
	$(CC) $(LDFLAGS) -o `echo $(@:_=) | tr A-Z a-z`.so interface.o $?

JVREV_: Reverb.o

	echo "@prefix lv2: <http://lv2plug.in/ns/lv2core#>.\n\
	@prefix rdfs: <http://www.w3.org/2000/01/rdf-schema#>.\n\n\
	<http://portalmod.com/plugins/caps/`echo $(@:_=) | tr A-Z a-z`> a lv2:Plugin; lv2:binary \
	<`echo $(@:_=) | tr A-Z a-z`.so>; rdfs:seeAlso <`echo $(@:_=) | tr A-Z a-z`.ttl>." > manifest.ttl.`echo $(@:_=) | tr A-Z a-z`

	$(CC) $(LDFLAGS) -D$@ -o interface.o -c interface.cc
	$(CC) $(LDFLAGS) -o `echo $(@:_=) | tr A-Z a-z`.so interface.o $?

LORENZ_: Fractals.o

	echo "@prefix lv2: <http://lv2plug.in/ns/lv2core#>.\n\
	@prefix rdfs: <http://www.w3.org/2000/01/rdf-schema#>.\n\n\
	<http://portalmod.com/plugins/caps/`echo $(@:_=) | tr A-Z a-z`> a lv2:Plugin; lv2:binary \
	<`echo $(@:_=) | tr A-Z a-z`.so>; rdfs:seeAlso <`echo $(@:_=) | tr A-Z a-z`.ttl>." > manifest.ttl.`echo $(@:_=) | tr A-Z a-z`

	$(CC) $(LDFLAGS) -D$@ -o interface.o -c interface.cc
	$(CC) $(LDFLAGS) -o `echo $(@:_=) | tr A-Z a-z`.so interface.o $?

NARROWER_: Pan.o

	echo "@prefix lv2: <http://lv2plug.in/ns/lv2core#>.\n\
	@prefix rdfs: <http://www.w3.org/2000/01/rdf-schema#>.\n\n\
	<http://portalmod.com/plugins/caps/`echo $(@:_=) | tr A-Z a-z`> a lv2:Plugin; lv2:binary \
	<`echo $(@:_=) | tr A-Z a-z`.so>; rdfs:seeAlso <`echo $(@:_=) | tr A-Z a-z`.ttl>." > manifest.ttl.`echo $(@:_=) | tr A-Z a-z`

	$(CC) $(LDFLAGS) -D$@ -o interface.o -c interface.cc
	$(CC) $(LDFLAGS) -o `echo $(@:_=) | tr A-Z a-z`.so interface.o $?

NOISEGATE_: NoiseGate.o

	echo "@prefix lv2: <http://lv2plug.in/ns/lv2core#>.\n\
	@prefix rdfs: <http://www.w3.org/2000/01/rdf-schema#>.\n\n\
	<http://portalmod.com/plugins/caps/`echo $(@:_=) | tr A-Z a-z`> a lv2:Plugin; lv2:binary \
	<`echo $(@:_=) | tr A-Z a-z`.so>; rdfs:seeAlso <`echo $(@:_=) | tr A-Z a-z`.ttl>." > manifest.ttl.`echo $(@:_=) | tr A-Z a-z`

	$(CC) $(LDFLAGS) -D$@ -o interface.o -c interface.cc
	$(CC) $(LDFLAGS) -o `echo $(@:_=) | tr A-Z a-z`.so interface.o $?

PAN_: Pan.o

	echo "@prefix lv2: <http://lv2plug.in/ns/lv2core#>.\n\
	@prefix rdfs: <http://www.w3.org/2000/01/rdf-schema#>.\n\n\
	<http://portalmod.com/plugins/caps/`echo $(@:_=) | tr A-Z a-z`> a lv2:Plugin; lv2:binary \
	<`echo $(@:_=) | tr A-Z a-z`.so>; rdfs:seeAlso <`echo $(@:_=) | tr A-Z a-z`.ttl>." > manifest.ttl.`echo $(@:_=) | tr A-Z a-z`

	$(CC) $(LDFLAGS) -D$@ -o interface.o -c interface.cc
	$(CC) $(LDFLAGS) -o `echo $(@:_=) | tr A-Z a-z`.so interface.o $?

PHASERII_: Phaser.o

	echo "@prefix lv2: <http://lv2plug.in/ns/lv2core#>.\n\
	@prefix rdfs: <http://www.w3.org/2000/01/rdf-schema#>.\n\n\
	<http://portalmod.com/plugins/caps/`echo $(@:_=) | tr A-Z a-z`> a lv2:Plugin; lv2:binary \
	<`echo $(@:_=) | tr A-Z a-z`.so>; rdfs:seeAlso <`echo $(@:_=) | tr A-Z a-z`.ttl>." > manifest.ttl.`echo $(@:_=) | tr A-Z a-z`

	$(CC) $(LDFLAGS) -D$@ -o interface.o -c interface.cc
	$(CC) $(LDFLAGS) -o `echo $(@:_=) | tr A-Z a-z`.so interface.o $?

PLATE2X2_: Reverb.o

	echo "@prefix lv2: <http://lv2plug.in/ns/lv2core#>.\n\
	@prefix rdfs: <http://www.w3.org/2000/01/rdf-schema#>.\n\n\
	<http://portalmod.com/plugins/caps/`echo $(@:_=) | tr A-Z a-z`> a lv2:Plugin; lv2:binary \
	<`echo $(@:_=) | tr A-Z a-z`.so>; rdfs:seeAlso <`echo $(@:_=) | tr A-Z a-z`.ttl>." > manifest.ttl.`echo $(@:_=) | tr A-Z a-z`

	$(CC) $(LDFLAGS) -D$@ -o interface.o -c interface.cc
	$(CC) $(LDFLAGS) -o `echo $(@:_=) | tr A-Z a-z`.so interface.o $?

PLATE_: Reverb.o

	echo "@prefix lv2: <http://lv2plug.in/ns/lv2core#>.\n\
	@prefix rdfs: <http://www.w3.org/2000/01/rdf-schema#>.\n\n\
	<http://portalmod.com/plugins/caps/`echo $(@:_=) | tr A-Z a-z`> a lv2:Plugin; lv2:binary \
	<`echo $(@:_=) | tr A-Z a-z`.so>; rdfs:seeAlso <`echo $(@:_=) | tr A-Z a-z`.ttl>." > manifest.ttl.`echo $(@:_=) | tr A-Z a-z`

	$(CC) $(LDFLAGS) -D$@ -o interface.o -c interface.cc
	$(CC) $(LDFLAGS) -o `echo $(@:_=) | tr A-Z a-z`.so interface.o $?

ROESSLER_: Fractals.o

	echo "@prefix lv2: <http://lv2plug.in/ns/lv2core#>.\n\
	@prefix rdfs: <http://www.w3.org/2000/01/rdf-schema#>.\n\n\
	<http://portalmod.com/plugins/caps/`echo $(@:_=) | tr A-Z a-z`> a lv2:Plugin; lv2:binary \
	<`echo $(@:_=) | tr A-Z a-z`.so>; rdfs:seeAlso <`echo $(@:_=) | tr A-Z a-z`.ttl>." > manifest.ttl.`echo $(@:_=) | tr A-Z a-z`

	$(CC) $(LDFLAGS) -D$@ -o interface.o -c interface.cc
	$(CC) $(LDFLAGS) -o `echo $(@:_=) | tr A-Z a-z`.so interface.o $?

SATURATE_: Saturate.o dsp/polynomials.o

	echo "@prefix lv2: <http://lv2plug.in/ns/lv2core#>.\n\
	@prefix rdfs: <http://www.w3.org/2000/01/rdf-schema#>.\n\n\
	<http://portalmod.com/plugins/caps/`echo $(@:_=) | tr A-Z a-z`> a lv2:Plugin; lv2:binary \
	<`echo $(@:_=) | tr A-Z a-z`.so>; rdfs:seeAlso <`echo $(@:_=) | tr A-Z a-z`.ttl>." > manifest.ttl.`echo $(@:_=) | tr A-Z a-z`

	$(CC) $(LDFLAGS) -D$@ -o interface.o -c interface.cc
	$(CC) $(LDFLAGS) -o `echo $(@:_=) | tr A-Z a-z`.so interface.o $?

SCAPE_: Scape.o

	echo "@prefix lv2: <http://lv2plug.in/ns/lv2core#>.\n\
	@prefix rdfs: <http://www.w3.org/2000/01/rdf-schema#>.\n\n\
	<http://portalmod.com/plugins/caps/`echo $(@:_=) | tr A-Z a-z`> a lv2:Plugin; lv2:binary \
	<`echo $(@:_=) | tr A-Z a-z`.so>; rdfs:seeAlso <`echo $(@:_=) | tr A-Z a-z`.ttl>." > manifest.ttl.`echo $(@:_=) | tr A-Z a-z`

	$(CC) $(LDFLAGS) -D$@ -o interface.o -c interface.cc
	$(CC) $(LDFLAGS) -o `echo $(@:_=) | tr A-Z a-z`.so interface.o $?

SIN_: Sin.o

	echo "@prefix lv2: <http://lv2plug.in/ns/lv2core#>.\n\
	@prefix rdfs: <http://www.w3.org/2000/01/rdf-schema#>.\n\n\
	<http://portalmod.com/plugins/caps/`echo $(@:_=) | tr A-Z a-z`> a lv2:Plugin; lv2:binary \
	<`echo $(@:_=) | tr A-Z a-z`.so>; rdfs:seeAlso <`echo $(@:_=) | tr A-Z a-z`.ttl>." > manifest.ttl.`echo $(@:_=) | tr A-Z a-z`

	$(CC) $(LDFLAGS) -D$@ -o interface.o -c interface.cc
	$(CC) $(LDFLAGS) -o `echo $(@:_=) | tr A-Z a-z`.so interface.o $?

STEREOCHORUSII2X2_: Chorus.o

	echo "@prefix lv2: <http://lv2plug.in/ns/lv2core#>.\n\
	@prefix rdfs: <http://www.w3.org/2000/01/rdf-schema#>.\n\n\
	<http://portalmod.com/plugins/caps/`echo $(@:_=) | tr A-Z a-z`> a lv2:Plugin; lv2:binary \
	<`echo $(@:_=) | tr A-Z a-z`.so>; rdfs:seeAlso <`echo $(@:_=) | tr A-Z a-z`.ttl>." > manifest.ttl.`echo $(@:_=) | tr A-Z a-z`

	$(CC) $(LDFLAGS) -D$@ -o interface.o -c interface.cc
	$(CC) $(LDFLAGS) -o `echo $(@:_=) | tr A-Z a-z`.so interface.o $?

STEREOCHORUSII_: Chorus.o

	echo "@prefix lv2: <http://lv2plug.in/ns/lv2core#>.\n\
	@prefix rdfs: <http://www.w3.org/2000/01/rdf-schema#>.\n\n\
	<http://portalmod.com/plugins/caps/`echo $(@:_=) | tr A-Z a-z`> a lv2:Plugin; lv2:binary \
	<`echo $(@:_=) | tr A-Z a-z`.so>; rdfs:seeAlso <`echo $(@:_=) | tr A-Z a-z`.ttl>." > manifest.ttl.`echo $(@:_=) | tr A-Z a-z`

	$(CC) $(LDFLAGS) -D$@ -o interface.o -c interface.cc
	$(CC) $(LDFLAGS) -o `echo $(@:_=) | tr A-Z a-z`.so interface.o $?

STEREOPHASERII2X2_: Phaser.o

	echo "@prefix lv2: <http://lv2plug.in/ns/lv2core#>.\n\
	@prefix rdfs: <http://www.w3.org/2000/01/rdf-schema#>.\n\n\
	<http://portalmod.com/plugins/caps/`echo $(@:_=) | tr A-Z a-z`> a lv2:Plugin; lv2:binary \
	<`echo $(@:_=) | tr A-Z a-z`.so>; rdfs:seeAlso <`echo $(@:_=) | tr A-Z a-z`.ttl>." > manifest.ttl.`echo $(@:_=) | tr A-Z a-z`

	$(CC) $(LDFLAGS) -D$@ -o interface.o -c interface.cc
	$(CC) $(LDFLAGS) -o `echo $(@:_=) | tr A-Z a-z`.so interface.o $?

STEREOPHASERII_: Phaser.o

	echo "@prefix lv2: <http://lv2plug.in/ns/lv2core#>.\n\
	@prefix rdfs: <http://www.w3.org/2000/01/rdf-schema#>.\n\n\
	<http://portalmod.com/plugins/caps/`echo $(@:_=) | tr A-Z a-z`> a lv2:Plugin; lv2:binary \
	<`echo $(@:_=) | tr A-Z a-z`.so>; rdfs:seeAlso <`echo $(@:_=) | tr A-Z a-z`.ttl>." > manifest.ttl.`echo $(@:_=) | tr A-Z a-z`

	$(CC) $(LDFLAGS) -D$@ -o interface.o -c interface.cc
	$(CC) $(LDFLAGS) -o `echo $(@:_=) | tr A-Z a-z`.so interface.o $?

TONESTACK_: ToneStack.o

	echo "@prefix lv2: <http://lv2plug.in/ns/lv2core#>.\n\
	@prefix rdfs: <http://www.w3.org/2000/01/rdf-schema#>.\n\n\
	<http://portalmod.com/plugins/caps/`echo $(@:_=) | tr A-Z a-z`> a lv2:Plugin; lv2:binary \
	<`echo $(@:_=) | tr A-Z a-z`.so>; rdfs:seeAlso <`echo $(@:_=) | tr A-Z a-z`.ttl>." > manifest.ttl.`echo $(@:_=) | tr A-Z a-z`

	$(CC) $(LDFLAGS) -D$@ -o interface.o -c interface.cc
	$(CC) $(LDFLAGS) -o `echo $(@:_=) | tr A-Z a-z`.so interface.o $?

WHITE_: White.o

	echo "@prefix lv2: <http://lv2plug.in/ns/lv2core#>.\n\
	@prefix rdfs: <http://www.w3.org/2000/01/rdf-schema#>.\n\n\
	<http://portalmod.com/plugins/caps/`echo $(@:_=) | tr A-Z a-z`> a lv2:Plugin; lv2:binary \
	<`echo $(@:_=) | tr A-Z a-z`.so>; rdfs:seeAlso <`echo $(@:_=) | tr A-Z a-z`.ttl>." > manifest.ttl.`echo $(@:_=) | tr A-Z a-z`

	$(CC) $(LDFLAGS) -D$@ -o interface.o -c interface.cc
	$(CC) $(LDFLAGS) -o `echo $(@:_=) | tr A-Z a-z`.so interface.o $?
