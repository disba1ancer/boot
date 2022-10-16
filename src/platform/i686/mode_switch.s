.intel_syntax noprefix

.text
.code32

.global I686_RealModeCall
I686_RealModeCall:

        I686_RealModeState.eip = 0
        I686_RealModeState.eflags = 4
        I686_RealModeState.eax = 8
        I686_RealModeState.ecx = 12
        I686_RealModeState.edx = 16
        I686_RealModeState.ebx = 20
        I686_RealModeState.esp = 24
        I686_RealModeState.ebp = 28
        I686_RealModeState.esi = 32
        I686_RealModeState.edi = 36
        I686_RealModeState.es = 40
        I686_RealModeState.cs = 44
        I686_RealModeState.ss = 48
        I686_RealModeState.ds = 52
        I686_RealModeState.fs = 56
        I686_RealModeState.gs = 60

        regframe_ptr_offset = 28
        sub     esp, 24
        mov     20[esp], ebx
        mov     16[esp], ebp
        mov     12[esp], esi
        mov     8[esp], edi

        mov     eax, regframe_ptr_offset[esp]
        mov     ecx, eax
        and     eax, 0xF0000
        and     ecx, 0xFFFF
        shl     eax, 12
        or      eax, ecx
        mov     regframe_ptr_offset[esp], eax

        mov     dword ptr 4[esp], offset 0f
        mov     dword ptr [esp], offset realmod_int_entry
        jmp     I686_EnterRealMode

0:      mov     ebx, 12[esp]
        mov     ebp, 8[esp]
        mov     esi, 4[esp]
        mov     edi, 0[esp]
        add     esp, 16
        ret

.section .text16, "ax"
.code16

realmod_int_entry:
        regframe_ptr_offset = 24
        mov     bp, sp
        les     di, regframe_ptr_offset[bp]
        pushf
        push    cs
        push    offset 0f
        pushf
        push    es:I686_RealModeState.cs[di]
        push    es:I686_RealModeState.eip[di]
        and     word ptr 6[bp], ~0x30
        mov     ax, es:I686_RealModeState.ds[di]
        mov     ds, ax
        mov     ax, es:I686_RealModeState.fs[di]
        mov     fs, ax
        mov     ax, es:I686_RealModeState.gs[di]
        mov     gs, ax
        mov     eax, es:I686_RealModeState.eax[di]
        mov     ecx, es:I686_RealModeState.ecx[di]
        mov     edx, es:I686_RealModeState.edx[di]
        mov     ebx, es:I686_RealModeState.ebx[di]
        mov     ebp, es:I686_RealModeState.ebp[di]
        mov     esi, es:I686_RealModeState.esi[di]
        les     edi, es:I686_RealModeState.edi[di]
        iret
0:      push    edi
        push    es
        mov     di, sp
        les     di, regframe_ptr_offset + 6[di]
        pop     word ptr es:I686_RealModeState.es[di]
        pop     dword ptr es:I686_RealModeState.edi[di]
        mov     es:I686_RealModeState.esi[di], esi
        mov     es:I686_RealModeState.ebp[di], ebp
        mov     es:I686_RealModeState.ebx[di], ebx
        mov     es:I686_RealModeState.edx[di], edx
        mov     es:I686_RealModeState.ecx[di], ecx
        mov     es:I686_RealModeState.eax[di], eax
        mov     ax, gs
        mov     es:I686_RealModeState.gs[di], ax
        mov     ax, fs
        mov     es:I686_RealModeState.fs[di], ax
        mov     dx, fs
        mov     es:I686_RealModeState.ds[di], ax
        pushfd
        pop     dword ptr es:I686_RealModeState.eflags[di]
        mov     ax, cs
        mov     ds, ax
        mov     es, ax

        jmp     I686_EnterProtMode

.text
.code32

.global I686_EnterRealMode
I686_EnterRealMode:
        mov     eax, 0x18
        mov     ds, ax
        mov     es, ax
        mov     ss, ax
        nop
        jmp     far ptr 0x8:realmod_entry

.section .text16, "ax"
.code16

realmod_entry:
        mov     eax, cr0
        and     eax, ~0x80000001
        mov     cr0, eax
        jmp     far ptr 0x1000:0f

0:      xor     ax, ax
        mov     ss, ax
        nop
        mov     ax, cs
        mov     ds, ax
        mov     es, ax

        in      al, 0x70
        and     al, 0x7f
        out     0x70, al
        in      al, 0x71

        sti
        ret     2

.global I686_EnterProtMode
I686_EnterProtMode:
        cli

        in      al, 0x70
        or      al, 0x80
        out     0x70, al
        in      al, 0x71

        lgdt    cs:[gdtr]
        mov     eax, cr0
        or      eax, 0x1
        mov     cr0, eax

data32  jmp     far ptr 0x10:protmod_entry

gdtr:   .2byte  0x3f
        .4byte  i686_gdt

.text
.code32

protmod_entry:
        mov     eax, 0x20
        mov     ds, ax
        mov     es, ax
        mov     ss, ax
        movzx   esp, sp
        ret

.text
.global boot_VirtualEnter
boot_VirtualEnter:
        push    ebp
        mov     ebp, esp
        add     esp, 0x1F
        and     esp, 0xFFFFFFF0
        mov     0[esp], ebx
        mov     4[esp], esi
        mov     8[esp], edi

        mov     ecx, 0xC0000080
        rdmsr
        or      eax, 0x100
        wrmsr
        mov     eax, cr4
        mov     ecx, boot_x86_64_pml4
        mov     edx, cr0
        or      eax, 0x20
        or      edx, 0x80000000
        mov     cr4, eax
        mov     cr3, ecx
        mov     cr0, edx
        call    far ptr 0x28:0f
.code64
0:      #mov     eax, 0x30
        #mov     ss, eax
        mov     dword ptr [esp], offset 0f
        jmp     8[ebp]
.code32
0:      mov     ebx, 0[esp]
        mov     esi, 4[esp]
        mov     edi, 8[esp]
        mov     esp, ebp
        pop     ebp
        ret
