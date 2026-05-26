/* simple.s - The "Nuclear Option" in Assembly */
.syntax unified
.cpu cortex-m4
.thumb

/* Make symbols visible */
.global vector_table
.global Reset_Handler

/* --- 1. VECTOR TABLE (At 0x0) --- */
.section .isr_vector, "a"
vector_table:
    .word 0x20004000        /* Initial Stack Pointer */
    .word Reset_Handler     /* Reset Vector (Linker handles the +1 Thumb bit automatically here) */

/* --- 2. THE CODE --- */
.section .text
.thumb_func                 /* Tells Linker this is Thumb code (Sets the +1 bit) */
Reset_Handler:
    /* A. Write CAFEBABE to GPREGRET (0x4000051C) */
    ldr r0, =0x4000051C     /* Load Register Address */
    ldr r1, =0xCAFEBABE     /* Load Value */
    str r1, [r0]            /* Store Value into Address */

    /* B. Turn on Backlight (P0.22) */
    /* DIRSET register (0x50000518) */
    ldr r0, =0x50000518
    ldr r1, =0x00400000     /* Bit 22 set (1 << 22) */
    str r1, [r0]            /* Write to DIRSET */

    /* OUTSET register (0x50000508) */
    ldr r0, =0x50000508
    str r1, [r0]            /* Write to OUTSET */

    /* C. Freeze */
loop:
    b loop                  /* Infinite Jump */