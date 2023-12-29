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

; ----------------------------------
; COLOURS
COLOUR_PLAYFIELD = $c4
COLOUR_TEXTAREA_BACKGROUND = $c2
COLOUR_TEXTAREA_TEXT = $cc
COLOUR_KEYBOARD_BACKGROUND = $c4
COLOUR_KEYBOARD_TEXT = $cb

; ----------------------------------
; SCREEN TIMINGS

; atari safe timings: values assume that TIM64T timer is started immediately
; before the first WSYNC that leads into the screen region.

VBLANKTIMER = 48 ; starts main screen at scanline 40
OVERSCANTIMER = 31 

; the scanlines in the visible part of the screen are counted manually
VISIBLE_SCANLINES = 192

; the on-screen keyboard appears below the text area
KEYBOARD_SCANLINES = 40
TEXTINPUT_SCANLINES = 9
TEXTAREA_SCANLINES = VISIBLE_SCANLINES - KEYBOARD_SCANLINES - TEXTINPUT_SCANLINES

; ----------------------------------
; ARM - BINARY INCLUSION

	SEG ARM
	ORG $0000
	INCBIN arm/driver/cdfdriver.bin

	ORG $0800
	INCBIN arm/main.bin


; ----------------------------------
; ARM - SHARED DEFINITIONS

; program functions. these values are passed to the ARM program as the first
; parameter. see _CALFN_PARAMETERS values below
_FN_INIT = 0 
_FN_GAME = 1

; number of scanlines in visible portion of screen. the ARM program will use
; this to clear memory and to figure out how many rows of glyphs are possible
_SCANLINES_IN_TEXTAREA = TEXTAREA_SCANLINES

; the number of data streams required for the textarea
; 
; note that the ARM program may pack multiple glyphs into a single datastream
; but we don't need to know about this in in the 6507 program
_NUM_TEXTAREA_DATASTREAMS = 12

; the first datastream to use for column retrieval. the ARM program uses
; consecutive datastreams up to _NUM_TEXTAREA_DATASTREAMS. in this 6507 program we
; reference DS0DATA, DS1DATA, DS2DATA etc.
_TEXTAREA_DATASTREAM_BASE_REG = DS0DATA

; as above but for the textinput area
_SCANLINES_IN_TEXTINPUT = TEXTINPUT_SCANLINES
_NUM_TEXTINPUT_DATASTREAMS = 12
_TEXTINPUT_DATASTREAM_BASE_REG = DS19DATA

; as above but for the keybaord area
_SCANLINES_IN_KEYBOARD = KEYBOARD_SCANLINES
_NUM_KEYBOARD_DATASTREAMS = 6
_KEYBOARD_DATASTREAM_BASE_REG = DS13DATA

; ----------------------------------
; SETUP OF CARTRIDGE

	; start of cartridge
	SEG
	ORG $7000
	RORG $f000

init
	CLEAN_START
	
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
	ldx SWCHA
	stx DSWRITE 
	ldx INPT4
	stx DSWRITE 
	ldx #$ff 
	stx CALLFN

	; playfield
	lda #$f0
	sta PF0
	lda #$ff
	sta CTRLPF
	lda #COLOUR_PLAYFIELD
	sta COLUPF

	; player size/spacing
	ldx #06
	stx NUSIZ0
	ldx #06
	stx NUSIZ1

	; backgroud and player colours changed at start of text area and start of
	; keyboard area

	ldx #$2 ; prep for VSYNC on
	stx VBLANK ; turn on VBLANK


; ----------------------------------
; VSYNC/VBLANK - CALLFN & SPRITE POSITIONING FOR TEXT AREA
;     note that SPRITE POSITIONING for the keyboard happens
;     in the scanlines between the two areas

vsync ; x should be set to #$2
	sta WSYNC
	stx VSYNC ; VSYNC on
	ldy #VBLANKTIMER ; prep for TIM64T
	sta WSYNC
	stx VSYNC
	sta WSYNC
	stx VSYNC
	sty TIM64T ; load VBLANKTIMER
	sta WSYNC

	; using current Y value because VBLANKTIMER just so happens
	; to be suitable for turning VSYNC off. this may need to be
	; changed if the value of VBLANKTIMER changes
	sty VSYNC ; VSYNC off

vblank
	ldx #<_CALFN_PARAMETERS
	stx DSPTR
	ldx #>_CALFN_PARAMETERS
	stx DSPTR
	ldx #_FN_GAME
	stx DSWRITE
	ldx SWCHA
	stx DSWRITE
	ldx INPT4
	stx DSWRITE 
	ldx #$ff
	stx CALLFN

	inc FRAMENUM
	lda FRAMENUM
	and #$01
	bne setPositionsB

setPositionsA
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
	nop 0
	sta RESP1
	ldx #$f0
	stx HMP0
	ldx #$d0
	stx HMP1
	jmp vblankWaitStart

vblankWaitStart
	; change background and player color for text area
	ldx #COLOUR_TEXTAREA_BACKGROUND
	stx COLUBK
	ldx #COLOUR_TEXTAREA_TEXT
	stx COLUP0
	stx COLUP1

	; small horizontal adjustment
	sta WSYNC
	sta HMOVE

	ldx #$0 ; prep for VBLANK off
vblankWait
	bit TIMINT ; using VBLANKTIMER
	bpl vblankWait
	sta WSYNC
	stx VBLANK ; VBLANK off

	; scanline count preparation for text area
	ldx #TEXTAREA_SCANLINES
	stx SCANLINE 

	; first line starts with a WSYNC so decrease scanline count immediately
	dec SCANLINE

	lda FRAMENUM
	and #$01
	bne textAreaB

; ----------------------------------
; TEXTAREA 

textAreaA
	sta WSYNC
	lda #_TEXTAREA_DATASTREAM_BASE_REG+0
	sta GRP0
	lda #_TEXTAREA_DATASTREAM_BASE_REG+1
	sta GRP1

	lda SCANLINE 
	sta PF0 
	nop 0
	nop 
	nop 
	nop 
	nop

	lda #_TEXTAREA_DATASTREAM_BASE_REG+5
	tax
	lda #_TEXTAREA_DATASTREAM_BASE_REG+8
	tay
	lda #_TEXTAREA_DATASTREAM_BASE_REG+4
	sta GRP0
	stx GRP1
	sty GRP0
	lda #_TEXTAREA_DATASTREAM_BASE_REG+9
	sta GRP1

	; scanline check for end of visible screen
	dec SCANLINE
	beq textInputAPrep
	jmp textAreaA

textAreaB
	sta WSYNC
	lda #_TEXTAREA_DATASTREAM_BASE_REG+2
	sta GRP0
	lda #_TEXTAREA_DATASTREAM_BASE_REG+3
	sta GRP1

	lda SCANLINE 
	sta PF0 
	nop 0
	nop
	nop
	nop
	nop
	nop 
	nop
	nop

	lda #_TEXTAREA_DATASTREAM_BASE_REG+7
	tax
	lda #_TEXTAREA_DATASTREAM_BASE_REG+10
	tay
	lda #_TEXTAREA_DATASTREAM_BASE_REG+6
	sta GRP0
	stx GRP1
	sty GRP0
	lda #_TEXTAREA_DATASTREAM_BASE_REG+11
	sta GRP1

	; scanline check for end of visible screen
	dec SCANLINE
	beq textInputBPrep
	jmp textAreaB

; ----------------------------------
; TEXTINPUT 

textInputAPrep
	sta WSYNC

	; blank player data 
	ldx #$00
	stx GRP0
	stx GRP1

	ldx #TEXTINPUT_SCANLINES-3
	stx SCANLINE 
textInputA
	sta WSYNC
	lda #_TEXTINPUT_DATASTREAM_BASE_REG+0
	sta GRP0
	lda #_TEXTINPUT_DATASTREAM_BASE_REG+1
	sta GRP1

	nop 
	nop
	nop
	nop 
	nop 
	nop 
	nop 
	nop 
	nop

	lda #_TEXTINPUT_DATASTREAM_BASE_REG+5
	tax
	lda #_TEXTINPUT_DATASTREAM_BASE_REG+8
	tay
	lda #_TEXTINPUT_DATASTREAM_BASE_REG+4
	sta GRP0
	stx GRP1
	sty GRP0
	lda #_TEXTINPUT_DATASTREAM_BASE_REG+9
	sta GRP1

	; scanline check for end of visible screen
	dec SCANLINE
	beq keyboardPrep
	jmp textInputA

textInputBPrep
	sta WSYNC

	; blank player data 
	ldx #$00
	stx GRP0
	stx GRP1

	ldx #TEXTINPUT_SCANLINES-3
	stx SCANLINE 
textInputB
	sta WSYNC
	lda #_TEXTINPUT_DATASTREAM_BASE_REG+2
	sta GRP0
	lda #_TEXTINPUT_DATASTREAM_BASE_REG+3
	sta GRP1

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

	lda #_TEXTINPUT_DATASTREAM_BASE_REG+7
	tax
	lda #_TEXTINPUT_DATASTREAM_BASE_REG+10
	tay
	lda #_TEXTINPUT_DATASTREAM_BASE_REG+6
	sta GRP0
	stx GRP1
	sty GRP0
	lda #_TEXTINPUT_DATASTREAM_BASE_REG+11
	sta GRP1

	; scanline check for end of visible screen
	dec SCANLINE
	beq keyboardPrep
	jmp textInputB

; ----------------------------------
; KEYBOARD

	; preparation for keyboard area
keyboardPrep
	; blank player data 
	ldx #$00
	stx GRP0
	stx GRP1
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
	sta RESP0
	nop 
	sta RESP1
	ldx #$10
	stx HMP0
	ldx #$00
	stx HMP1

	sta WSYNC
	sta HMOVE

	ldx #KEYBOARD_SCANLINES-3 ; minus three because we'll have consumed
	; three scanlines by the time we get to the keyboard display
	stx SCANLINE
	ldx #COLOUR_KEYBOARD_TEXT
	stx COLUP0
	stx COLUP1

	; background is black temporarily to disguise the HMOVE bar
	ldx #$00
	stx COLUBK

	sta WSYNC
	ldx #COLOUR_KEYBOARD_BACKGROUND
	stx COLUBK

keyboard
	sta WSYNC

	lda #_KEYBOARD_DATASTREAM_BASE_REG+0
	sta GRP0
	lda #_KEYBOARD_DATASTREAM_BASE_REG+1
	sta GRP1

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

	lda #_KEYBOARD_DATASTREAM_BASE_REG+3
	tax
	lda #_KEYBOARD_DATASTREAM_BASE_REG+4
	tay
	lda #_KEYBOARD_DATASTREAM_BASE_REG+2
	sta GRP0
	stx GRP1
	sty GRP0
	lda #_KEYBOARD_DATASTREAM_BASE_REG+5
	sta GRP1


	dec SCANLINE
	beq overscan
	jmp keyboard

; ----------------------------------
; OVERSCAN 

overscan
	ldx #$2 ; prep for VBLANK on
	sta WSYNC
	stx VBLANK
	ldy #OVERSCANTIMER ; prep for TIM64T
	sty TIM64T
	sta WSYNC

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
_SWCHA ds 1
_INPT4 ds 1

; the number of bytes required for all data stream collectively
_TEXTAREA_SIZE = _NUM_TEXTAREA_DATASTREAMS * _SCANLINES_IN_TEXTAREA

; the first data stream begins here. all datastreams are adjacent in memory
	align 4
_TEXTAREA_ORIGIN ds _TEXTAREA_SIZE

; the extent of memory used collectively by all data streams
_TEXTAREA_MEMTOP = _TEXTAREA_ORIGIN + _TEXTAREA_SIZE

; as above but for for the textinput area
_TEXTINPUT_SIZE = _NUM_TEXTINPUT_DATASTREAMS * _SCANLINES_IN_TEXTINPUT
	align 4
_TEXTINPUT_ORIGIN ds _TEXTINPUT_SIZE
_TEXTINPUT_MEMTOP = _TEXTINPUT_ORIGIN + _TEXTINPUT_SIZE

; as above but for for the keyboard area
_KEYBOARD_SIZE = _NUM_KEYBOARD_DATASTREAMS * _SCANLINES_IN_KEYBOARD
	align 4
_KEYBOARD_ORIGIN ds _KEYBOARD_SIZE
_KEYBOARD_MEMTOP = _KEYBOARD_ORIGIN + _KEYBOARD_SIZE
