/******************************************************************************
File: custom.S
Description: Harmony Custom C Code Booter
(C) Copyright 2016 - Chris Walton, Fred Quimby, Darrell Spice Jr
******************************************************************************/

/* Start in ARM mode */
.arm

/* Put code in boot section */
.section  .boot,"ax",%progbits

/* Entry Point */
.global   Custom
.func     Custom
Custom:

  /* Switch to THUMB mode */
  add     r0, pc, #1
  bx      r0
.thumb

  /* Fetch value from CU header */
  ldr     r0, =0x10000024
  ldr     r2, [r0]
  
  /* Skip if data already copied */
  cmp     r2, #0
  bmi     SkipCopy
  add     r2, #1
  str     r2,[r0]
  cmp     r2, #1
  bne     SkipCopy

  /* Clear BSS section */
  mov     r0, #0
  ldr     r1, =_ebss
  ldr     r2, =_sbss
  sub     r1, r1, r2
ClearBssLoop:
  sub     r1, #4
  bmi     EndClearBss
  str     r0, [r2, +r1]
  bne     ClearBssLoop
EndClearBss:

  /* Copy C data section to RAM */
  ldr     r0, =_etext
  ldr     r1, =_edata
  ldr     r2, =_sdata
  sub     r1, r1, r2
CopyLoop:
  sub     r1, #4
  bmi     SkipCopy
  ldr     r3, [r0, +r1]
  str     r3, [r2, +r1]
  bne     CopyLoop
SkipCopy:

  /* Push return addess on stack */
  push    {lr}
  
  /* Put ReturnToBUS address in LR */
  ldr     r0, =ReturnToBUS+1
  mov     lr, r0

  /* Jump to main in C code */
  ldr     r0, =main
  bx      r0

ReturnToBUS:
  /* Return to BUS (switch to ARM mode) */
  pop     {r0}
  bx      r0

