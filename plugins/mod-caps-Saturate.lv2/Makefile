VERSION = 0.9.24

PREFIX = /usr
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

CXXFLAGS += $(OPTS)
LDFLAGS += -shared -Wl,--no-undefined

SOURCES = ../../Saturate.cc ../../dsp/polynomials.cc
OBJECTS = $(SOURCES:.cc=.o)

PLUG   = Saturate
BUNDLE = mod-caps-$(PLUG).lv2
ifndef LV2_PATH
DEST = $(PREFIX)/lib/lv2/$(BUNDLE)
else
DEST = $(LV2_PATH)/$(BUNDLE)
endif

# targets following -------------------------------------------------------------

all: .depend $(PLUG).so

$(PLUG).so: $(OBJECTS)
	$(CXX) $(OBJECTS) $(LDFLAGS) -o $@

.cc.s:
	$(CXX) $^ $(CXXFLAGS) -S

.cc.o: .depend
	$(CXX) $^ $(CXXFLAGS) -o $@ -c

install: all
	install -d $(DESTDIR)$(LV2DEST)
	install -m 644 manifest.ttl $(PLUG).so $(PLUG).ttl $(DESTDIR)$(LV2DEST)
	cp -r modgui $(DESTDIR)$(LV2DEST)

uninstall:
	-rm -rf $(LV2DEST)

clean:
	rm -f $(OBJECTS) $(PLUG).so *.s .depend

.depend: $(SOURCES)
	$(CXX) -MM $(CXXFLAGS) $(SOURCES) > .depend

-include .depend
