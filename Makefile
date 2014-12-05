VERSION = 0.9.24

PREFIX = /usr
DESTDIR = 

CC = g++

OPTS = -O3 -ffast-math -funroll-loops -Wall -fPIC -DPIC
#OPTS = -g -DDEBUG 

CAPS_URI=\"http://portalmod.com/plugins/caps/\"
OPTS += -DCAPS_URI=${CAPS_URI}

_LDFLAGS = -shared 
STRIP = strip

-include defines.make

CFLAGS += $(OPTS) $(_CFLAGS)
LDFLAGS += $(_LDFLAGS) $(CFLAGS)

PLUG = caps

SOURCES = $(wildcard *.cc) $(wildcard dsp/*.cc)
OBJECTS	= $(SOURCES:.cc=.o) 
HEADERS = $(wildcard *.h) $(wildcard dsp/*.h) $(wildcard util/*.h) $(wildcard dsp/tonestack/*.h)
TTL		= ttl/*.ttl

DEST = $(PREFIX)/lib/ladspa
RDFDEST = $(PREFIX)/share/ladspa/rdf
LV2BUNDLE = $(PLUG).lv2

ifndef LV2_PATH
	LV2DEST = $(PREFIX)/lib/lv2/$(LV2BUNDLE)
else
	LV2DEST = $(LV2_PATH)/$(LV2BUNDLE)
endif

# targets following -------------------------------------------------------------

all: depend $(PLUG).so tags

run: all
	@#python bin/enhance_dp_wsop.py
	@#python -i bin/rack.py White AutoFilter cream.Audio.Meter Pan
	@#python -i bin/rack.py White AutoFilter Pan
	@#python -i bin/rack.py Click Plate
	@#python -i bin/rack.py PhaserII 
	@#~/cream/gdb-python html/graph.py Eq4p,a.f=100,a.gain=30.png
	@#~/cream/gdb-python bin/eqtest.py
	@#~/cream/gdb-python bin/fractalstest.py
	@#python bin/sinsweep.py
	@#python -i ~/reve/bin/noisegate.py
	@#rack.py Noisegate AmpVI Plate
	@#python ~/reve/bin/hum.py
	@python bin/cabtest.py
	@#cd ~/reve && python bin/cabmake.py

rdf: $(PLUG).rdf
$(PLUG).rdf: all tools/make-rdf.py
	python tools/make-rdf.py > $(PLUG).rdf

$(PLUG).so: $(OBJECTS)
	$(CC) $(ARCH) $(LDFLAGS) -o $@ $(OBJECTS)

.cc.s: 
	$(CC) $(ARCH) $(CFLAGS) -S $<

.cc.o: depend 
	$(CC) $(ARCH) $(CFLAGS) -o $@ -c $<

tags: $(SOURCES) $(HEADERS)
	@-if [ -x /usr/bin/ctags ]; then ctags $(SOURCES) $(HEADERS) >/dev/null 2>&1 ; fi

install: all install-lv2
	@$(STRIP) $(PLUG).so > /dev/null
	install -d $(DESTDIR)$(DEST)
	install -m 644 $(PLUG).so $(DESTDIR)$(DEST)
	install -d $(DESTDIR)$(RDFDEST)
	install -m 644 $(PLUG).rdf $(DESTDIR)$(RDFDEST)

install-lv2: all
	@$(STRIP) $(PLUG).so > /dev/null
	install -d $(DESTDIR)$(LV2DEST)
	install -m 644 $(PLUG).so $(DESTDIR)$(LV2DEST)
	install -m 644 $(TTL) $(DESTDIR)$(LV2DEST)
	cp -r ttl/modgui/ $(DESTDIR)$(LV2DEST)

fake-install: all
	-rm $(DESTDIR)$(DEST)/$(PLUG).so
	ln -s `pwd`/$(PLUG).so $(DESTDIR)$(DEST)/$(PLUG).so
	-rm $(DESTDIR)$(RDFDEST)/$(PLUG).rdf
	ln -s `pwd`/$(PLUG).rdf $(DESTDIR)$(RDFDEST)/$(PLUG).rdf

rdf-install:
	install -d $(DESTDIR)$(RDFDEST)
	install -m 644 $(PLUG).rdf $(DESTDIR)$(RDFDEST)

uninstall:
	-rm -rf $(DESTDIR)$(DEST)/$(PLUG).so
	-rm -rf $(DESTDIR)$(RDFDEST)/$(PLUG).rdf
	-rm -rf $(LV2DEST)

clean:
	rm -f $(OBJECTS) $(PLUG).so *.s depend

version.h:
	@VERSION=$(VERSION) python tools/make-version.h.py

dist: all $(PLUG).rdf version.h
	tools/make-dist.py caps $(VERSION) $(CFLAGS)

depend: $(SOURCES) $(HEADERS)
	$(CC) -MM $(CFLAGS) $(DEFINES) $(SOURCES) > depend

-include depend
