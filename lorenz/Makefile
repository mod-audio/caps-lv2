
# source files directory
SRC_DIR = .

# extension source file
EXT = cc

# compiler and linker
CC = g++
LD = g++

# flags
CFLAGS = -O2 -Wall -fPIC -DPIC -funroll-loops $(SSE_CFLAGS) $(SSE3_CFLAGS) -I../utils
LDFLAGS = -shared -fPIC -DPIC -nostartfiles

ifneq (cat /proc/cpuinfo | grep sse >/dev/null,)
SSE_CFLAGS = -msse -mfpmath=sse
endif

ifneq (cat /proc/cpuinfo | grep ssse3 >/dev/null,)
SSE3_CFLAGS = -msse3
endif

# libs
LIBS = -lc -lm

# remove command
RM = rm -f

# package name
PACKAGE = $(shell cd .. && basename `pwd` | tr A-Z a-z)

# plugin name
PLUGIN = $(shell basename `pwd` | tr A-Z a-z)
PLUGIN_SO = $(PLUGIN).so

# installation path
INSTALL_PATH = /usr/local/lib/lv2
INSTALLATION_PATH = $(INSTALL_PATH)/$(PACKAGE)-$(PLUGIN).lv2

# sources and objects
SRC = $(wildcard $(SRC_DIR)/*.$(EXT))
OBJ = $(SRC:.$(EXT)=.o)

## rules
$(PLUGIN): depend $(OBJ)
	$(LD) $(LDFLAGS) $(CFLAGS) $(OBJ) ../utils/*.o -o $(PLUGIN_SO) $(LIBS)

%.o: %.$(EXT)
	$(CC) $(CFLAGS) -c -o $@ $<

depend:
	@cd ../utils ;\
	$(MAKE) ;\
	cd -;


clean:
	$(RM) $(SRC_DIR)/*.o *.so

install:
	mkdir -p $(INSTALLATION_PATH)
	cp $(PLUGIN_SO) $(INSTALLATION_PATH)
	cp $(SRC_DIR)/*.ttl $(INSTALLATION_PATH)
