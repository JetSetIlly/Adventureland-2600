; Copyright (c) 2021, Stephen Illingworth
; 
; Redistribution and use in source and binary forms, with or without
; modification, are permitted provided that the following conditions are met:
; 
; 1. Redistributions of source code must retain the above copyright notice, this
;    list of conditions and the following disclaimer.
; 2. Redistributions in binary form must reproduce the above copyright notice,
;    this list of conditions and the following disclaimer in the documentation
;    and/or other materials provided with the distribution.
; 
; THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
; ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
; WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
; DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
; ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
; (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
; ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
; (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
; SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

    processor 6502 
    include vcs.h
    include macro.h
    include cdfj.h

; ----------------------------------
; VCS RAM

	SEG.U VCS_RAM
	ORG $80
SCANLINE ds 1
FRAMENUM ds 1

; ----------------------------------
; ARM 

	SEG ARM
	ORG $0000
	INCBIN arm/driver/cdfdriver.bin

	ORG $0800
	INCBIN arm/main.bin

; program functions
_FN_INIT = 0 
_FN_GAME_VB = 1
_FN_GAME_OS = 2

; data stream aliases
_DS_COL0 = DS0DATA
_DS_COL1 = DS1DATA
_DS_COL2 = DS2DATA
_DS_COL3 = DS3DATA
_DS_COL4 = DS4DATA
_DS_COL5 = DS5DATA
_DS_COL6 = DS6DATA
_DS_COL7 = DS7DATA
_DS_COL8 = DS8DATA
_DS_COL9 = DS9DATA

; ----------------------------------
; SETUP

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
	ldx #<_DS_TO_ARM
	stx DSPTR
	ldx #>_DS_TO_ARM
	stx DSPTR
	ldx #_FN_INIT
	stx DSWRITE 
	ldx #$FF 
	stx CALLFN

	ldx #$00
	stx COLUBK
	ldx #$0f
	stx COLUP0
	stx COLUP1
	ldx #03
	stx NUSIZ0
	ldx #01
	stx NUSIZ1

	ldx #$2 ; prep for VSYNC on
	stx VBLANK ; turn on VBLANK


; ----------------------------------
; MAIN

; atari safe timings: values assume that TIM64T timer is started immediately
; before the first WSYNC that leads into the screen region.

VBLANKTIMER = 48 ; starts main screen at scanline 40
OVERSCANTIMER = 31 ; starts vblank at scanline 259 (first WSYNC of VBLANK takes it to 259)

; the scanlines in the visible part of the screen are counted manually
VISIBLESCANLINES = 192 

vsync ; x should be set to #$2
	sta WSYNC
	stx VSYNC ; VSYNC on
	ldy #VBLANKTIMER ; prep for TIM64T
	sta WSYNC
	stx VSYNC
	sta WSYNC
	stx VSYNC
	ldx #VISIBLESCANLINES
	stx SCANLINE ; reset some houskeeping RAM locations
	sty TIM64T ; load VBLANKTIMER
	sta WSYNC
	stx VSYNC ; VSYNC off

vblank
	ldx #<_DS_TO_ARM
	stx DSPTR
	ldx #>_DS_TO_ARM
	stx DSPTR
	ldx #_FN_GAME_VB
	stx DSWRITE
	ldx #$ff
	stx CALLFN

	inc FRAMENUM
	lda FRAMENUM
	and $01
	beq setPositionsEvenFrame

setPositionsOddFrame
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

setPositionsEvenFrame
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

	lda FRAMENUM
	and $01
	beq screenEvenFrame

screenOddFrame
	sta WSYNC
	lda #_DS_COL0
	sta GRP0
	lda #_DS_COL1
	sta GRP1
	lda #_DS_COL3
	tax
	lda #_DS_COL4
	tay
	nop 0
	lda #_DS_COL2
	sta GRP0
	stx GRP1
	sty GRP0

	; scanline check for end of visible screen
	dec SCANLINE
	beq endScreen
	jmp screenOddFrame

screenEvenFrame
	sta WSYNC
	lda #_DS_COL5
	sta GRP0
	lda #_DS_COL6
	sta GRP1
	lda #_DS_COL8
	tax
	lda #_DS_COL9
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
	lda #_DS_COL7
	sta GRP0
	stx GRP1
	sty GRP0

	; scanline check for end of visible screen
	dec SCANLINE
	beq endScreen
	jmp screenEvenFrame

endScreen
	ldx #$2 ; prep for VBLANK on
	sta WSYNC
	stx VBLANK
	ldy #OVERSCANTIMER ; prep for TIM64T
	sty TIM64T
	sta WSYNC

overscan
	ldx INPT4
	ldx #<_DS_TO_ARM
	stx DSPTR
	ldx #>_DS_TO_ARM
	stx DSPTR
	ldx #_FN_GAME_OS
	stx DSWRITE ; _RUN_FUNC
	ldx INPT4
	stx DSWRITE ; _FIREBUTTON
	ldx #$FF
	stx CALLFN

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
; SHARED RAM
	SEG.U SHARED_RAM
	ORG $0000

_DS_TO_ARM
_RUN_FUNC ds 1
_FIREBUTTON ds 1

	align 4
_COL0_DATA ds 192
_COL1_DATA ds 192
_COL2_DATA ds 192
_COL3_DATA ds 192
_COL4_DATA ds 192
_COL5_DATA ds 192
_COL6_DATA ds 192
_COL7_DATA ds 192
_COL8_DATA ds 192
_COL9_DATA ds 192
