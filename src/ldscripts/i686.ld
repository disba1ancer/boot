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
    .text : ALIGN(16) {
        *(.init*)
        *(.fini*)
        *(.text*)
    } > main
    .rodata : {
        *(.rodata*)
    } > main
    .eh_frame_hdr : {
        *(.eh_frame_hdr*)
    } > main
    .eh_frame : {
        *(.eh_frame)
    } > main
    .gcc_except_table : {
        *(.gcc_except_table*)
    } > main
    .ctors : ALIGN(16) {
        *(.ctors)
    } > main
    .dtors : ALIGN(16) {
        *(.dtors)
    } > main
    .data : ALIGN(16) {
        *(.data*)
    } > main
    .bss : ALIGN(16) {
        __bss_start = .;
        *(COMMON)
        *(.bss*)
        __bss_end = ALIGN(16);
    } > main
    __bss_size = __bss_end - __bss_start;
}
