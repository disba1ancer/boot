.intel_syntax noprefix
.section .startup, "awx"
.code16
.global _start
_start:
        xor     ax, ax
        mov     ss, ax
        xor     sp, sp
        mov     ax, cs
        mov     ds, ax
        mov     es, ax
        mov     bp, sp

.a20en: inb     al, 0x92
        orb     al, 2
        outb    0x92, al

data32  push    offset start32
        jmp     I686_EnterProtMode

.section .text16, "awx"
.code16

_end:   hlt
        jmp     _end

.text
.code32
start32:
        xor     ebp, ebp
        sub     esp, 12
        mov     dword ptr [esp], offset __bss_start
        mov     dword ptr 4[esp], 0x0
        mov     dword ptr 8[esp], offset __bss_size
        call    memset
        add     esp, 12
        call    _ctors
        call    boot_main
        call    _dtors
        push    offset _end
        jmp     I686_EnterRealMode

.global memset
memset:
        dest    = 4
        val     = 8
        count   = 12
        cur     = edx
        end     = ecx

        mov     eax, val[esp]
        mov     ecx, 0x1010101
        mul     ecx

        mov     cur, dest[esp]
        mov     end, count[esp]
        add     end, cur

        cmp     cur, end
        jz      1f

        test    cur, 0x1
        jz      0f
        mov     [cur], al
        add     cur, 1

0:      cmp     cur, end
        jz      1f

        test    end, 0x1
        jz      0f
        sub     end, 1
        mov     [end], al

0:      cmp     cur, end
        jz      1f

        test    cur, 0x2
        jz      0f
        mov     [cur], al
        add     cur, 2

0:      cmp     cur, end
        jz      1f

        test    end, 0x2
        jz      0f
        sub     end, 2
        mov     [end], al

0:      cmp     cur, end
        jz      1f

        mov     [cur], eax
        add     cur, 4
        jmp     0b
1:      ret
