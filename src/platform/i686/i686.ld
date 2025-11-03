ENTRY(_start)

__binary_load_address = 0x10000;

SECTIONS {
    .text16 0 : AT(__binary_load_address) {
        *(.startup)
        *(.text16)
        . = ALIGN(16);
    }
    __binary32 = __binary_load_address + SIZEOF(.text16);
    .text __binary32 : AT(__binary32) {
        *(.init*)
        *(.fini*)
        *(.text*)
    }
    .rodata : {
        *(.rodata*)
    }
    .eh_frame_hdr : {
        *(.eh_frame_hdr*)
    }
    .eh_frame : {
        *(.eh_frame)
    }
    .gcc_except_table : {
        *(.gcc_except_table*)
    }
    .ctors : ALIGN(16) {
        *(.ctors)
    }
    .dtors : ALIGN(16) {
        *(.dtors)
    }
    .data : {
        *(.data*)
    }
    .bss : {
        __bss_start = .;
        *(COMMON)
        *(.bss*)
        __bss_end = ALIGN(16);
    }
    __bss_size = __bss_end - __bss_start;
}
