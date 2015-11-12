VERSION = 0.9.24

PREFIX = /usr/local
DESTDIR =

CXX ?= g++
STRIP ?= strip

ifdef DEBUG
OPTS = -g -DDEBUG
else
OPTS = -O2 -Wall -fPIC -DPIC
# OPTS = -O3 -ffast-math -funroll-loops -Wall -fPIC -DPIC
LDFLAGS += -Wl,--strip-all
endif

CXXFLAGS += $(OPTS) -I../..
LDFLAGS += -shared -Wl,--no-undefined

SOURCES = ../../Reverb.cc ../../dsp/polynomials.cc interface.cc
OBJECTS = $(SOURCES:.cc=.o)

PLUG   = PlateX2
BUNDLE = mod-caps-$(PLUG).lv2
ifndef LV2_DEST
LV2DEST = $(PREFIX)/lib/lv2/$(BUNDLE)
else
LV2DEST = $(LV2_DEST)/$(BUNDLE)
endif

# targets following -------------------------------------------------------------

all: .depend $(PLUG).so

$(PLUG).so: $(OBJECTS)
	$(CXX) $(OBJECTS) $(LDFLAGS) -o $@

.cc.s:
	$(CXX) $< $(CXXFLAGS) -S

.cc.o: .depend
	$(CXX) $< $(CXXFLAGS) -o $@ -c

install: all
	install -d $(DESTDIR)$(LV2DEST)
	install -m 644 *.ttl *.so $(DESTDIR)$(LV2DEST)
	cp -r modgui $(DESTDIR)$(LV2DEST)

uninstall:
	-rm -rf $(LV2DEST)

clean:
	rm -f $(OBJECTS) $(PLUG).so *.s .depend

.depend: $(SOURCES)
	$(CXX) -MM $(CXXFLAGS) $(SOURCES) > .depend

-include .depend
