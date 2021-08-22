.intel_syntax noprefix
.section .init
        pop     ebp
        ret

.section .fini
        pop     ebp
        ret

.section .custom, "aw"
        .8byte  0x5555555555555555
