ENTRY(Reset_Handler)

MEMORY {
  FLASH (rx) : ORIGIN = 0x00000000, LENGTH = 464K
  RAM (rwx)  : ORIGIN = 0x20000000, LENGTH = 64K
}

SECTIONS {
    .isr_vector : {
        __isr_vector_start = .;
        KEEP(*(.isr_vector))
        __isr_vector_end = .;
    } > FLASH

    .text : {
        *(.text*)
        *(.rodata*)
        . = ALIGN(4);
    } > FLASH

    /* The 'AT' command specifies where the data lives in FLASH */
    .data : {
        _data = .;
        *(.data*)
        . = ALIGN(4);
        _edata = .;
    } > RAM AT > FLASH

    /* Define where the FLASH image of the data ends */
    _etext = LOADADDR(.data) + SIZEOF(.data);

    .bss : {
        _bss = .;
        *(.bss*)
        *(COMMON)
        . = ALIGN(4);
        _ebss = .;
    } > RAM
}
