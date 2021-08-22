ENTRY(_start)

__binary_load_address = 0x10000;

MEMORY {
    main : o = __binary_load_address, l = 64K
    startupseg : o = 0, l = 64K
}

SECTIONS {
    .text16 : {
        *(.startup)
        *(.text16)
    } > startupseg AT > main
    .text : {
        *(.text)
    } > main
    .rodata : {
        __ctors_start = .;
        *(.ctors)
        __ctors_end = .;
        LONG(__ctors_end)
        __dtors_start = .;
        *(.dtors)
        __dtors_end = .;
        LONG(__dtors_end)
        *(.rodata)
    } > main
    .data : {
        *(.data)
    } > main
    .custom : {
        *(.custom)
    } > main
    .bss : ALIGN(16) {
        __bss_start = .;
        *(COMMON)
        *(.bss)
        __bss_end = .;
    } > main
    __bss_size = __bss_end - __bss_start;
}
