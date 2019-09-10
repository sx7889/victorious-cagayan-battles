CFLAGS=-O3 -fomit-frame-pointer -fPIC

ifeq ($(OS),Windows_NT)
CC=x86_64-w64-mingw32-gcc
AR=x86_64-w64-mingw32-ar
ifeq ($(PROCESSOR_ARCHITEW6432),AMD64)
	CFLAGS += -m64
else
ifeq ($(PROCESSOR_ARCHITECTURE),x86)
	CFLAGS += -m32
else
	CFLAGS += -m64
endif
endif
endif

override CFLAGS := $(CFLAGS) -Iinclude $(shell ./check_cflags.sh $(CC) $(CFLAGS))

all: libenet.a

OBJS= \
	callbacks.o \
	compress.o \
	host.o \
	list.o \
	packet.o \
	peer.o \
	protocol.o \
	unix.o \
	win32.o

libenet.a: $(OBJS)
	$(AR) rcs $@ $(OBJS)

default: libenet.a

clean:
	-$(RM) libenet.a $(OBJS)
	 
