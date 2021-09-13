.intel_syntax noprefix

.section .init
.global _init
.type _init, @function
_init:
        push    ebp
        mov     ebp, esp
        lea     esp, -8[esp]

.section .fini
.global _fini
.type _fini, @function
_fini:
        push    ebp
        mov     ebp, esp
        lea     esp, -8[esp]
