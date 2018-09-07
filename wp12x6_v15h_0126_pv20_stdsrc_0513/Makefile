#
# Makefile for WP12x6
#
# 'make' builds everything
# 'make clean' deletes everything except source files and Makefile
# You need to set TARGET, MCU and SOURCES for your project.
# TARGET is the name of the executable file to be produced 
# $(TARGET).elf $(TARGET).hex and $(TARGET).txt nad $(TARGET).map are all generated.
# The TXT file is used for BSL loading, the ELF can be used for JTAG use
# 
TARGET     = WP12x6
MCU        = wp100f12
# List all the source files here
# eg if you have a source file foo.c then list it here
SOURCE_PATH = src
SOURCES = ctm_user_protocol.c flash_para.c knl_protocol_wp.c knl_touch.c \
					 drv_scan.c com_service.c wp12x6_v15h_0126_lib.c com_global.c system.c main.c

# Include are located in the Include directory
INCLUDE_PATH = include
# Library source
#LIBS = -Llib -lwp12x6_V5H_0126_stdlib
# Add or subtract whatever MSPGCC flags you want. There are plenty more
#######################################################################################
CFLAGS   = -mmcu=$(MCU) -Os -Wall -Wunused -I$(INCLUDE_PATH)
ASFLAGS  = -mmcu=$(MCU) -x assembler-with-cpp -Wa,-gstabs
LDFLAGS  = -mmcu=$(MCU) -Wl,-Map=$(TARGET).map,-T wp12x6x_lkscript.x
########################################################################################
CC       = wp100-gcc
LD       = wp100-ld
AR       = wp100-ar
AS       = wp100-gcc
GASP     = wp100-gasp
NM       = wp100-nm
OBJCOPY  = wp100-objcopy
OBJDUMP  = wp100-objdump
RANLIB   = wp100-ranlib
STRIP    = wp100-strip
SIZE     = wp100-size
READELF  = wp100-readelf
MAKETXT  = srec_cat
CP       = cp -p
RM       = rm -f
MV       = mv
########################################################################################
# the file which will include dependencies
DEPEND = $(SOURCES:.c=.d)
# all the object files
OBJECTS = $(SOURCES:.c=.o)
# Mark, TODO, need SRecord , http://srecord.sourceforge.net/windows.html
all: clean $(TARGET).elf $(TARGET).hex $(TARGET).lst $(TARGET).bin
#all: $(TARGET).elf $(TARGET).hex $(TARGET).txt
$(TARGET).elf: $(OBJECTS)
	echo "Linking $@"
#	$(CC) $(OBJECTS) $(LDFLAGS) $(LIBS) -o $@
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@
	echo
	echo ">>>> Size of Firmware <<<<"
	$(SIZE) $(TARGET).elf
	echo
%.hex: %.elf
	$(OBJCOPY) -O ihex $< $@

%.lst: %.elf
	$(OBJDUMP) -dSt $^ >$@

%.bin: %.hex
	echo "Generate binary file $@..."
	echo
	./hex2bin -s B000 -e bin $<

%.txt: %.hex
	$(MAKETXT) -O $@ -TITXT $< -I
	unix2dos $(TARGET).txt
#  The above line is required for the DOS based TI BSL tool to be able to read the txt file generated from linux/unix systems.
%.o: $(SOURCE_PATH)/%.c
	echo "Compiling $<"
	$(CC) -c -g $(CFLAGS) -o $@ $<
# rule for making assembler source listing, to see the code
#%.lst: %.c
#	$(CC) -c $(ASFLAGS) -Wa,-anlhd $< > $@
# include the dependencies unless we're going to clean, then forget about them.
ifneq ($(MAKECMDGOALS), clean)
-include $(DEPEND)
endif
# dependencies file
# includes also considered, since some of these are our own
# (otherwise use -MM instead of -M)
%.d: $(SOURCE_PATH)/%.c
	echo "Generating dependencies $@ from $<"
	$(CC) -M $(CFLAGS) $< >$@
.SILENT:
.PHONY:	clean
clean:
	-$(RM) $(OBJECTS)
	-$(RM) $(TARGET).o
	-$(RM) $(TARGET).hex
	-$(RM) $(TARGET).map
	-$(RM) $(TARGET).elf
	-$(RM) $(TARGET).lst
	-$(RM) $(SOURCES:.c=.lst)
	-$(RM) $(DEPEND)

