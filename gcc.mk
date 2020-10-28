##
# @file makefile
# @author cy023
# @date 2020.07.25
# @brief asa m128 專案使用
# 參考 c4mlib makefile

MCU     := atmega128
F_CPU   := 11059200

CROSS   := avr
CC      := $(CROSS)-gcc
OBJDUMP := $(CROSS)-objdump
OBJCOPY := $(CROSS)-objcopy

## Include Path
IPATH  = ./lib

COMMON = -mmcu=$(MCU)

## Compile options common for all C compilation units.
CFLAGS = $(COMMON)
CFLAGS += -DF_CPU=$(F_CPU)UL
CFLAGS += -Wall -gdwarf-2 -std=gnu99 -Os
CFLAGS += -funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums 
CFLAGS += -Wtype-limits
CFLAGS += -no-canonical-prefixes
CFLAGS += -I $(IPATH)

## Assembly specific flags
ASMFLAGS = $(COMMON)
ASMFLAGS += $(CFLAGS)
ASMFLAGS += -x assembler-with-cpp -Wa,-gdwarf2

## Linker flags
LDFLAGS  = $(COMMON)
LDFLAGS += -Wl,--gc-sections
LDFLAGS += -Wl,-u,vfprintf -Wl,-u,vfscanf
LDFLAGS += -Wl,-Map=$*.map

## Archiver flags
# ARFLAGS = rcs

## Intel Hex file production flags
HEX_FLASH_FLAGS   = -R .eeprom -R .fuse -R .lock -R .signature
HEX_EEPROM_FLAGS  = -j .eeprom
HEX_EEPROM_FLAGS += --set-section-flags=.eeprom="alloc,load"
HEX_EEPROM_FLAGS += --change-section-lma .eeprom=0 --no-change-warnings

## Libraries
LIBDIRS = uart.o
LIBS = -lm -lprintf_flt -lscanf_flt 

## Objects that must be built in order to link
# LIBSRC  ?=
# LIBOBJS ?=

VPATH = src/ test/ lib/

LINKOBJS = $(LIBDIRS) $(LIBS)

%.elf: %.o $(LIBOBJS)
	 $(CC) $(LDFLAGS) $< $(LINKOBJS)  -o $@

%.hex: %.elf
	$(OBJCOPY) -O ihex $(HEX_FLASH_FLAGS) $< $@

%.eep: %.elf
	$(OBJCOPY) $(HEX_EEPROM_FLAGS) -O ihex $< $@ || exit 0

%.lss: %.elf
	$(OBJDUMP) -h -S $< > $@

clean:
	-rm -rf *.hex *.lss *.elf *.eep *.map *.o
