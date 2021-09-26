.intel_syntax noprefix
.section .startup, "ax"
.code16
.global _start
_start:
        xor     ax, ax
        mov     ss, ax
        xor     sp, sp
        gpt_info_size = 0x38
        disk_num_size = 0x4
        gpt_info = -gpt_info_size
        disk_num = -(gpt_info_size + disk_num_size)
        sub     sp, gpt_info_size + disk_num_size
        push    es
        push    di
        cld
        movzx   edx, dl
        mov     es, ax
        mov     di, gpt_info
        mov     ss:disk_num, edx
        mov     cx, gpt_info_size / 4
        rep movsd
        mov     ax, cs
        mov     ds, ax
        mov     es, ax
        int     0x12
        mov     dx, ax

.a20en: inb     al, 0x92
        orb     al, 2
        outb    0x92, al

data32  push    offset start32
        jmp     I686_EnterProtMode

.section .text16, "ax"
.code16

_end:   hlt
        jmp     _end

.text
.code32
start32:
        movzx   eax, dx
        xor     ebp, ebp
        shl     eax, 10
        lea     esp, [esp - 16]
        mov     heap_end, eax
        mov     dword ptr [esp], offset __bss_start
        mov     dword ptr 4[esp], 0x0
        mov     dword ptr 8[esp], offset __bss_size
        call    memset
        lea     esp, [esp + 12]
        jmp     c_start

.global _Exit
.type _Exit, @function
_Exit:
.global abort
.type abort, @function
abort:
        push    offset _end
        jmp     I686_EnterRealMode
#        mov     esp, 0xFFFC
#        mov     [esp], offset _end
#        jmp     I686_EnterRealMode

.data
.global heap_start
heap_start:
        .4byte  __bss_end
.global heap_end
heap_end:
        .4byte  __bss_end
