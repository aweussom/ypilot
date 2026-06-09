###############################################################################
#
# Makefile for RetroRocket
#
# Modified for ARM7 part
###############################################################################

AS = $(DEVKITARM)/bin/arm-eabi-gcc
CC = $(DEVKITARM)/bin/arm-eabi-gcc
CPP = $(DEVKITARM)/bin/arm-eabi-g++
LD = $(DEVKITARM)/bin/arm-eabi-g++
OBJCOPY = $(DEVKITARM)/bin/arm-eabi-objcopy
NDSTOOL = $(DEVKITARM)/bin/ndstool
DSBUILD = $(DEVKITARM)/bin/dsbuild

CFLAGS = -Wformat=2 -Winline -Wall -O2 -mcpu=arm7tdmi -mtune=arm7tdmi \
	-fomit-frame-pointer -ffast-math -mthumb-interwork \
	-I$(DEVKITPRO)/libnds/include \
	-I$(DEVKITPRO)/libnds/include/nds \
	-I$(DEVKITPRO)/libnds/include/nds/arm7 \
	-I$(DEVKITPRO)/liblobby/include/ \
	-I$(DEVKITPRO)/mikmod/include/ \
	-I$(DEVKITPRO)/PAlib/include \
	-I$(DEVKITPRO)/PAlib/include/nds \
	-I$(DEVKITPRO)/PAlib/lib/arm7 \
	-I$(DEVKITPRO)/PAlib/PAlib/lib/arm7/build \
	-I./helix \
	-I. \
	-DARM7 

ASFLAGS = -mthumb-interwork

LDFLAGS = -mthumb-interwork -mno-fpu \
	-L$(DEVKITPRO)/PAlib/lib \
	-L$(DEVKITPRO)/libnds/lib \
	-L$(DEVKITPRO)/liblobby/lib \
	-L$(DEVKITPRO)/mikmod/lib \
	-specs=ds_arm7.specs 

LIBS = -lnds7 -llobby7d -lmikmod7 

OBJECTS = as_lib7.o main.o PA.o Sound7.o helix/mp3dec.o helix/mp3tabs.o \
	helix/real/bitstream.o helix/real/buffers.o helix/real/dct32.o \
	helix/real/dequant.o helix/real/dqchan.o helix/real/huffman.o \
	helix/real/hufftabs.o helix/real/imdct.o helix/real/scalfact.o \
	helix/real/stproc.o helix/real/subband.o helix/real/trigtabs.o \
	helix/real/asmpoly_gcc.o

retrorocket.arm7 : retrorocket7.elf
	$(OBJCOPY) -O binary $< $@

retrorocket7.elf : $(OBJECTS)
	$(LD) $(LDFLAGS) $+ $(LIBS) -o $@

%.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o : %.s
	$(AS) $(ASLAGS) -c $< -o $@

###############################################################################

.PHONY : clean
clean :
	rm -rf *.o *.elf *.arm7 helix/*.o helix/real/*.o

###############################################################################
