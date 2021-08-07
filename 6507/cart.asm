; BSD 2-Clause License

; Copyright (c) 2021, Stephen Illingworth
; All rights reserved.

; Redistribution and use in source and binary forms, with or without
; modification, are permitted provided that the following conditions are met:

; 1. Redistributions of source code must retain the above copyright notice, this
;    list of conditions and the following disclaimer.

; 2. Redistributions in binary form must reproduce the above copyright notice,
;    this list of conditions and the following disclaimer in the documentation
;    and/or other materials provided with the distribution.

; THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
; AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
; IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
; DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
; FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
; DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
; SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
; CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
; OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
; OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

    processor 6502 
    include vcs.h
    include macro.h
    include cdfj.h

; ----------------------------------
; VCS RAM

	SEG.U VCS_RAM
	ORG $80
SCANLINE	ds 1
FRAMENUM	ds 1
KEYPAD		ds 1
_THREE_COUNT_STATE ds 1


; ----------------------------------
; SCREEN TIMINGS

; atari safe timings: values assume that TIM64T timer is started immediately
; before the first WSYNC that leads into the screen region.

VBLANKTIMER = 48 ; starts main screen at scanline 40
OVERSCANTIMER = 36 

; the scanlines in the visible part of the screen are counted manually. ideally
; this would be 192 but the keypad reading code is CPU expensive and it will
; overrun the overscan timer. 188 is the highest value that is a multiple of 4 and
; that keeps the kernel within NTSC spec (see comment below)
VISIBLE_SCANLINES = 188

; check that VISIBLE_SCANLINES value is a multiple of four. issue warning if is not
#if VISIBLE_SCANLINES % 4 != 0
	echo "VISIBLE_SCANLINES shoule be a multiple of 4"
#endif

; there must be a 400us delay before reading the INPTx registers after
; writing to SWCHA. for a CPU running at 1.19Mhz a cycle takes 0.8403us.
; 400us therefore will require 476.02 cycles
;
; DEY takes two cycles and a successful BNE takes three cycles. LDY also
; takes two cycles and the final (unsuccessful) BNE takes two cycles; so
; using a value of 472, we need to loop "waitKeypad" 95 times.
KEYPAD_DELAY = 95

; there is a balance between VISIBLE_SCANLINES, OVERSCANTIMER and the delay
; between writing to SWCHA and reading INPTx registers when scanning the
; keypad.
;
; for a "delay" of $5f (the number of loop iterations) there can be no more
; than 188 VISIBLE_SCANLINES giving us a value of 36 for the OVERSCANTIMER.
;
; for a "delay" of $78 which some example code uses, the number of
; VISIBLE_SCANLINES is 182 and an OVERSCAN_TIMER of 31.
;
; if my calculated delay value of $5f is wrong (ie doesn't work on real
; hardware) then the alternative values can be used.



; ----------------------------------
; ARM - BINARY INCLUSION

	SEG ARM
	ORG $0000
	INCBIN arm/driver/cdfdriver.bin

	ORG $0800
	INCBIN arm/main.bin


; ----------------------------------
; ARM - SHARED DEFINITIONS

; program functions (there is no overscan function. keypad reading is very
; expensive). these values are passed to the ARM program as the first
; parameter. see _CALFN_PARAMETERS values below
_FN_INIT = 0 
_FN_GAME_VB = 1

; number of scanlines in visible portion of screen. the ARM program will use
; this to clear memory and to figure out how many rows of glyphs are possible
_SCANLINES_IN_DATASTREAM = VISIBLE_SCANLINES

; the number of data streams required for the text renderer
; 
; note that the ARM program may pack multiple glyphs into a single datastream
; but we don't need to know about this in in the 6507 program
_NUM_DATASTREAMS = 15

; the first datastream to use for column retrieval. the ARM program uses
; consecutive datastreams up to _NUM_DATASTREAMS. in this 6507 program we
; reference DS0DATA, DS1DATA, DS2DATA etc.
_DATASTREAM_BASE_REG = DS0DATA


; ----------------------------------
; THREE COUNT MACROS

	MAC THREE_COUNT_SETUP_X
		; require 8bit memory address labelled _THREE_COUNT_STATE
		LDX #$2									; 2
		STX _THREE_COUNT_STATE	; 3
		; 5 cycles
	ENDM

	MAC THREE_COUNT_UPDATE_X
		; require 8bit memory address labelled _THREE_COUNT_STATE
		LDX _THREE_COUNT_STATE	; 3
		DEX											; 2
		BPL .store_cycle_count	; 2/3
		LDX #$2									; 2
.store_cycle_count
		STX _THREE_COUNT_STATE	; 3
		; 12/13 cycles
	ENDM

	MAC THREE_COUNT_CMP_X
		; require 8bit memory address labelled _THREE_COUNT_STATE
		; result - branch on BEQ, BMI and BPL - check for equality before positivity (equality implies positivity)
		LDX _THREE_COUNT_STATE	; 3
		DEX											; 2
		; 5 cycles
	ENDM


; ----------------------------------
; SETUP OF CARTRIDGE

	; start of cartridge
	SEG
	ORG $7000
	RORG $f000

init
	CLEAN_START
	THREE_COUNT_SETUP_X
	
	; left keypad on
    lda    #$f0
    sta    SWACNT        ; output for 4 bits for left port

	; Fast Fetch mode must be turned on so we can read the datastreams
	ldx #FASTON
	stx SETMODE
	
	; set origin for DSWRITE
	ldx #<_CALFN_PARAMETERS
	stx DSPTR
	ldx #>_CALFN_PARAMETERS
	stx DSPTR
	ldx #_FN_INIT
	stx DSWRITE 
	ldx #$ff 
	stx CALLFN

	ldx #$f0
	stx COLUBK
	ldx #$fd
	stx COLUP0
	stx COLUP1
	ldx #03
	stx NUSIZ0
	ldx #01
	stx NUSIZ1

	ldx #$2 ; prep for VSYNC on
	stx VBLANK ; turn on VBLANK


; ----------------------------------
; VSYNC/VBLANK - CALLFN & SPRITE POSITIONING

vsync ; x should be set to #$2
	sta WSYNC
	stx VSYNC ; VSYNC on
	ldy #VBLANKTIMER ; prep for TIM64T
	sta WSYNC
	stx VSYNC
	sta WSYNC
	stx VSYNC
	ldx #VISIBLE_SCANLINES
	stx SCANLINE ; reset some houskeeping RAM locations
	sty TIM64T ; load VBLANKTIMER
	sta WSYNC
	stx VSYNC ; VSYNC off

vblank
	ldx #<_CALFN_PARAMETERS
	stx DSPTR
	ldx #>_CALFN_PARAMETERS
	stx DSPTR
	ldx #_FN_GAME_VB
	stx DSWRITE
	ldx KEYPAD
	stx DSWRITE
	ldx #$ff
	stx CALLFN

	THREE_COUNT_UPDATE_X
	THREE_COUNT_CMP_X
	beq setPositionsB
	bpl setPositionsC

setPositionsA
	sta WSYNC
	sta RESP0
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	sta RESP1
	ldx #$00
	stx HMP0
	ldx #$e0
	stx HMP1
	jmp vblankWaitStart

setPositionsB
	sta WSYNC
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop 
	sta RESP0
	sta WSYNC
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop 
	sta RESP1
	ldx #$f0
	stx HMP0
	ldx #$d0
	stx HMP1
	jmp vblankWaitStart

setPositionsC
	sta WSYNC
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop 0
	sta RESP0
	sta WSYNC
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop 0
	sta RESP1
	ldx #$e0
	stx HMP0
	ldx #$c0
	stx HMP1
	jmp vblankWaitStart

vblankWaitStart
	; small horizontal adjustment
	sta WSYNC
	sta HMOVE

	ldx #$0 ; prep for VBLANK off
vblankWait
	bit TIMINT ; using VBLANKTIMER
	bpl vblankWait
	sta WSYNC
	stx VBLANK ; VBLANK off

	; first line starts with a WSYNC so decrease scanline count immediately
	dec SCANLINE

	THREE_COUNT_CMP_X
	beq screenB
	bpl screenC


; ----------------------------------
; MAIN DISPLAY RENDERING

screenA
	sta WSYNC
	lda #_DATASTREAM_BASE_REG+0
	sta GRP0
	lda #_DATASTREAM_BASE_REG+1
	sta GRP1
	lda #_DATASTREAM_BASE_REG+3
	tax
	lda #_DATASTREAM_BASE_REG+4
	tay
	nop 0
	lda #_DATASTREAM_BASE_REG+2
	sta GRP0
	stx GRP1
	sty GRP0

	; scanline check for end of visible screen
	dec SCANLINE
	beq overscan
	jmp screenA

screenB
	sta WSYNC
	lda #_DATASTREAM_BASE_REG+5
	sta GRP0
	lda #_DATASTREAM_BASE_REG+6
	sta GRP1
	lda #_DATASTREAM_BASE_REG+8
	tax
	lda #_DATASTREAM_BASE_REG+9
	tay
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	lda #_DATASTREAM_BASE_REG+7
	sta GRP0
	stx GRP1
	sty GRP0

	; scanline check for end of visible screen
	dec SCANLINE
	beq overscan
	jmp screenB

screenC
	sta WSYNC
	lda #_DATASTREAM_BASE_REG+10
	sta GRP0
	lda #_DATASTREAM_BASE_REG+11
	sta GRP1
	lda #_DATASTREAM_BASE_REG+13
	tax
	lda #_DATASTREAM_BASE_REG+14
	tay
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	lda #_DATASTREAM_BASE_REG+12
	sta GRP0
	stx GRP1
	sty GRP0

	; scanline check for end of visible screen
	dec SCANLINE
	beq overscan
	jmp screenC


; ----------------------------------
; OVERSCAN - KEYPAD DETECTION

overscan
	ldx #$2 ; prep for VBLANK on
	sta WSYNC
	stx VBLANK
	ldy #OVERSCANTIMER ; prep for TIM64T
	sty TIM64T
	sta WSYNC

	; read keypad
    lda    #$ff
    ldx    #$0c
    clc
newKeypadRow
    ror
    sta    SWCHA

    ldy    #KEYPAD_DELAY
waitKeypad
    dey
    bne    waitKeypad

    bit    INPT4
    bpl    endKeypad
    dex
    bit    INPT1
    bpl    endKeypad
    dex
    bit    INPT0
    bpl    endKeypad
    dex
    bne    newKeypadRow
endKeypad
	stx    KEYPAD
	
	ldx #$2 ; prep for VSYNC on
overscanWait
	bit TIMINT ; using OVERSCANTIMER
	bpl overscanWait
	jmp vsync


; ----------------------------------
; MACHINE INITIALISATION

    ORG $7ffa
    RORG $fffa
	.word init ; nmi
	.word init ; reset
	.word init ; irq


; ----------------------------------
; ARM - PARAMETERS & DATASTREAMS

	SEG.U SHARED_RAM
	ORG $0000

_CALFN_PARAMETERS
_RUN_FUNC ds 1
_INPUT_KEY ds 1 ; not used if _RUN_FUNC == _FN_INIT

; the number of bytes required for all data stream collectively
_DATASTREAMS_SIZE = _NUM_DATASTREAMS * _SCANLINES_IN_DATASTREAM

; the first data stream begins here. all datastreams are adjacent in memory
	align 4
_DATASTREAMS_ORIGIN ds _DATASTREAMS_SIZE

; the extent of memory used collectively by all data streams
_DATASTREAMS_MEMTOP = _DATASTREAMS_ORIGIN + _DATASTREAMS_SIZE
