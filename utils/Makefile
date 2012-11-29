
# source files directory
SRC_DIR = .

# extension source file
EXT = cc

# compiler and linker
CC = g++
LD = g++

# flags
CFLAGS = -O2 -Wall -fPIC -DPIC -funroll-loops $(SSE_CFLAGS) $(SSE3_CFLAGS) -I. -Idsp -I../amp

ifneq (cat /proc/cpuinfo | grep sse >/dev/null,)
SSE_CFLAGS = -msse -mfpmath=sse
endif

ifneq (cat /proc/cpuinfo | grep ssse3 >/dev/null,)
SSE3_CFLAGS = -msse3
endif

# remove command
RM = rm -f

# sources and objects
SRC = $(wildcard $(SRC_DIR)/*.$(EXT))
OBJ = $(SRC:.$(EXT)=.o)

## rules
all: $(OBJ)

%.o: %.$(EXT)
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	$(RM) $(SRC_DIR)/*.o
