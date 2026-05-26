ENTRY(Reset_Handler)
MEMORY {
    FLASH (rx) : ORIGIN = 0x00000000, LENGTH = 512K
    RAM (rwx)  : ORIGIN = 0x20000000, LENGTH = 64K
}
SECTIONS {
    .text : {
        KEEP(*(.isr_vector))
        *(.text*)
    } > FLASH
}
