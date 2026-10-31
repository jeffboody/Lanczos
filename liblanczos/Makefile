export CC_USE_MATH = 1

TARGET  = liblanczos.a
CLASSES = lanczos_resampler lanczos_signal
SOURCE  = $(CLASSES:%=%.c)
OBJECTS = $(SOURCE:.c=.o)
HFILES  = $(CLASSES:%=%.h) lanczos_param.h
OPT     = -O2 -Wall
CFLAGS  = $(OPT) -I.
AR      = ar

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(AR) rcs $@ $(OBJECTS)

clean:
	rm -f $(OBJECTS) *~ \#*\# $(TARGET)

$(OBJECTS): $(HFILES)
