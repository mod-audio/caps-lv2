
PREFIX = /usr
DESTDIR =

all: build

build:
	$(MAKE) -C plugins/mod-caps-AmpVTS.lv2
	$(MAKE) -C plugins/mod-caps-AutoFilter.lv2
	$(MAKE) -C plugins/mod-caps-CabinetIII.lv2
	$(MAKE) -C plugins/mod-caps-CabinetIV.lv2
	$(MAKE) -C plugins/mod-caps-CEO.lv2
	$(MAKE) -C plugins/mod-caps-ChorusI.lv2
	$(MAKE) -C plugins/mod-caps-Click.lv2
	$(MAKE) -C plugins/mod-caps-Compress.lv2
	$(MAKE) -C plugins/mod-caps-CompressX2.lv2
	$(MAKE) -C plugins/mod-caps-Eq10.lv2
	$(MAKE) -C plugins/mod-caps-Eq10X2.lv2
	$(MAKE) -C plugins/mod-caps-Eq4p.lv2
	$(MAKE) -C plugins/mod-caps-EqFA4p.lv2
	$(MAKE) -C plugins/mod-caps-Fractal.lv2
	$(MAKE) -C plugins/mod-caps-Narrower.lv2
	$(MAKE) -C plugins/mod-caps-Noisegate.lv2
	$(MAKE) -C plugins/mod-caps-PhaserII.lv2
	$(MAKE) -C plugins/mod-caps-Plate.lv2
	$(MAKE) -C plugins/mod-caps-PlateX2.lv2
	$(MAKE) -C plugins/mod-caps-Saturate.lv2
	$(MAKE) -C plugins/mod-caps-Scape.lv2
	$(MAKE) -C plugins/mod-caps-Sin.lv2
	$(MAKE) -C plugins/mod-caps-Spice.lv2
	$(MAKE) -C plugins/mod-caps-SpiceX2.lv2
	$(MAKE) -C plugins/mod-caps-ToneStack.lv2
	$(MAKE) -C plugins/mod-caps-White.lv2
	$(MAKE) -C plugins/mod-caps-Wider.lv2

install: all
	$(MAKE) install -C plugins/mod-caps-AmpVTS.lv2
	$(MAKE) install -C plugins/mod-caps-AutoFilter.lv2
	$(MAKE) install -C plugins/mod-caps-CabinetIII.lv2
	$(MAKE) install -C plugins/mod-caps-CabinetIV.lv2
	$(MAKE) install -C plugins/mod-caps-CEO.lv2
	$(MAKE) install -C plugins/mod-caps-ChorusI.lv2
	$(MAKE) install -C plugins/mod-caps-Click.lv2
	$(MAKE) install -C plugins/mod-caps-Compress.lv2
	$(MAKE) install -C plugins/mod-caps-CompressX2.lv2
	$(MAKE) install -C plugins/mod-caps-Eq10.lv2
	$(MAKE) install -C plugins/mod-caps-Eq10X2.lv2
	$(MAKE) install -C plugins/mod-caps-Eq4p.lv2
	$(MAKE) install -C plugins/mod-caps-EqFA4p.lv2
	$(MAKE) install -C plugins/mod-caps-Fractal.lv2
	$(MAKE) install -C plugins/mod-caps-Narrower.lv2
	$(MAKE) install -C plugins/mod-caps-Noisegate.lv2
	$(MAKE) install -C plugins/mod-caps-PhaserII.lv2
	$(MAKE) install -C plugins/mod-caps-Plate.lv2
	$(MAKE) install -C plugins/mod-caps-PlateX2.lv2
	$(MAKE) install -C plugins/mod-caps-Saturate.lv2
	$(MAKE) install -C plugins/mod-caps-Scape.lv2
	$(MAKE) install -C plugins/mod-caps-Sin.lv2
	$(MAKE) install -C plugins/mod-caps-Spice.lv2
	$(MAKE) install -C plugins/mod-caps-SpiceX2.lv2
	$(MAKE) install -C plugins/mod-caps-ToneStack.lv2
	$(MAKE) install -C plugins/mod-caps-White.lv2
	$(MAKE) install -C plugins/mod-caps-Wider.lv2

uninstall:
	-rm -rf $(DESTDIR)$(PREFIX)/lib/lv2/mod-caps-*.lv2/

clean:
	rm -f *.o *.s
	rm -f plugins/*.lv2/*.so
	rm -f plugins/*.lv2/.depend
