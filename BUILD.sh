#!/bin/bash

set -u 

PROJECTNAME=advland

TOOLCHAIN=arm-none-eabi
DASM=dasm

CC="$TOOLCHAIN"-gcc
OBJCOPY="$TOOLCHAIN"-objcopy
SIZE="$TOOLCHAIN"-size

CFLAGS_NOWARN="-I arm/includes -mcpu=arm7tdmi -march=armv4t -mthumb -ffunction-sections -O2 -Wl,--build-id=none"
CFLAGS="$CFLAGS_NOWARN -Wall"

function clean {
	rm arm/main.o arm/main.elf arm/main.bin arm/custom/custom.o 2> /dev/null
	rm arm/shared_defines.h 2> /dev/null
	rm 6507/cart.lst 2> /dev/null
	rm "$PROJECTNAME.bin" "$PROJECTNAME.sym" 2> /dev/null
}

function compileArm {
	$CC -c -o arm/custom/custom.o arm/custom/custom.S 
	if [ $? -ne 0 ]
	then
		echo "compilation of custom.S failed"
		exit 10
	fi

	# create assembly output
	$CC $CFLAGS -c -g -Wa,-a,-ad arm/main.c -o arm/main.o > arm/main.lst
	if [ $? -ne 0 ]
	then
		echo "generation of arm/main.c to .lst file"
		exit 10
	fi

	$CC $CFLAGS -c -g -Wa,-a,-ad arm/advland/advland.c -o arm/advland/advland.o > arm/advland/advland.lst 2> /dev/null
	if [ $? -ne 0 ]
	then
		echo "generation of arm/advland/advland.c to .lst file"
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

	$CC $CFLAGS -o arm/main.elf arm/custom/custom.o arm/main.o arm/advland/advland.o -T arm/custom/custom.boot.lds -nostartfiles -Wl,-Map=arm/main.map,--gc-sections 
	if [ $? -ne 0 ]
	then
		echo "building of main elf file failed"
		exit 10
	fi

	$OBJCOPY -O binary -S arm/main.elf arm/main.bin 
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
$SIZE arm/custom/custom.o arm/main.o arm/main.elf
ls -l "$PROJECTNAME.bin"

