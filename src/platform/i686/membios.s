.intel_syntax noprefix
.text

.global i686_bios_mem_GetMap
i686_bios_mem_GetMap:
        push    ebp
        mov     ebp, esp
        lea     esp, [esp - 8]
        context_ptr = 8
        memchunk_ptr = 12
        mov     -4[ebp], edi
        mov     -8[ebp], ebx
        mov     eax, memchunk_ptr[ebp]
        mov     ecx, 24
        mov     dword ptr 20[eax], 1
        shr     eax, 4
        mov     edx, 0x534D4150
        and     eax, 0xF000
        mov     ebx, context_ptr[ebp]
        mov     memchunk_ptr + 2[ebp], ax
        mov     ebx, [ebx]
        push    offset 0f
        push    offset memorymap_rm
        jmp     I686_EnterRealMode
0:      mov     eax, context_ptr[ebp]
        mov     [eax], ebx
        movsx   eax, dx
        mov     edi, -4[ebp]
        mov     ebx, -8[ebp]
        add     eax, 1
        leave
        ret

.section .text16, "ax"
.code16

memorymap_rm:
        les     di, memchunk_ptr[bp]
        mov     eax, 0xE820
        int     0x15
        sbb     dx, dx
        jmp     I686_EnterProtMode
