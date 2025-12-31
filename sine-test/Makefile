export CC_USE_MATH = 1

TARGET  = sine-test
CLASSES =
SOURCE  = $(TARGET).c $(CLASSES:%=%.c)
OBJECTS = $(TARGET).o $(CLASSES:%=%.o)
HFILES  = $(CLASSES:%=%.h)
OPT     = -O2 -Wall
CFLAGS  = $(OPT) -I.
LDFLAGS = -Lliblanczos -llanczos -Llibcc -lcc -lm
CCC     = gcc

all: $(TARGET)

$(TARGET): $(OBJECTS) libcc liblanczos
	$(CCC) $(OPT) $(OBJECTS) -o $@ $(LDFLAGS)

.PHONY: libcc liblanczos

libcc:
	$(MAKE) -C libcc

liblanczos:
	$(MAKE) -C liblanczos

clean:
	rm -f $(OBJECTS) *~ \#*\# $(TARGET)
	$(MAKE) -C libcc clean
	$(MAKE) -C liblanczos clean
	rm libcc liblanczos

$(OBJECTS): $(HFILES)
