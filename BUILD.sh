#!/bin/bash

set -u 

PROJECTNAME=advland

TOOLCHAIN=arm-none-eabi
DASM=dasm

CC="$TOOLCHAIN"-gcc
OBJCOPY="$TOOLCHAIN"-objcopy
SIZE="$TOOLCHAIN"-size

OPTIMISATION="-Os"
CFLAGS_NOWARN="-I arm/includes -g3 -gdwarf-4 -gstrict-dwarf -mcpu=arm7tdmi -march=armv4t -mthumb -ffunction-sections -Wl,--build-id=none"
CFLAGS_NOWARN="$CFLAGS_NOWARN $OPTIMISATION"
CFLAGS="$CFLAGS_NOWARN -Wall -Wno-missing-prototypes"

ELF="arm/main.elf"
BIN="arm/main.bin"
OBJS="arm/custom/custom.o arm/main.o arm/advland/advland.o"
MAP="arm/main.map"
SHARED_DEFINES="arm/shared_defines.h"

function clean {
	rm $ELF 2> /dev/null
	rm $BIN 2> /dev/null
	rm $OBJS 2> /dev/null
	rm $MAP 2> /dev/null
	rm $SHARED_DEFINES 2> /dev/null
	rm 6507/*.lst 2> /dev/null
	rm "$PROJECTNAME.bin" "$PROJECTNAME.sym" 2> /dev/null
}

function compileArm {
	$CC -c -o arm/custom/custom.o arm/custom/custom.S 
	if [ $? -ne 0 ]
	then
		echo "compilation of custom.S failed"
		exit 10
	fi

	$CC $CFLAGS -c arm/main.c -o arm/main.o 
	if [ $? -ne 0 ]
	then
		echo "compilation of arm/main.c failed"
		exit 10
	fi

	$CC $CFLAGS_NOWARN -c arm/advland/advland.c -o arm/advland/advland.o 
	if [ $? -ne 0 ]
	then
		echo "compilation of arm/advland/advland.c failed"
		exit 10
	fi

	$CC $CFLAGS -o "$ELF" $OBJS -T arm/custom/custom.boot.lds -nostartfiles -Wl,-Map=arm/main.map,--gc-sections 
	if [ $? -ne 0 ]
	then
		echo "building of main elf file failed"
		exit 10
	fi

	$OBJCOPY -O binary -S "$ELF" "$BIN"
	if [ $? -ne 0 ]
	then
		echo "creation of arm binary file failed"
		exit 10
	fi
}

function assemble6507 {
	$DASM 6507/cart.asm -I6507/includes -f3 -v0 -s"$PROJECTNAME.sym" -l6507/cart.lst -o"$PROJECTNAME.bin"
}

# remove any generated files from previous compilation
clean

# touch main ARM binary to make sure it exists. first 6507 assembly will fail otherwise
touch arm/main.bin

# assemble 6507 to make sure symbols file exists
assemble6507

# extract exported symbols from symbols file
awk '/^_/ { printf "#define %-25s 0x%s\n", $1, $2 }' "$PROJECTNAME.sym" > arm/shared_defines.h

# real compilation/assembly
compileArm
assemble6507

# summary of compilation
$SIZE $OBJS $ELF
ls -l "$PROJECTNAME.bin"

