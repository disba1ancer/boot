.intel_syntax noprefix
.text
.code32

.global i686_bios_video_WriteString
i686_bios_video_WriteString:
        push    ebp
        mov     ebp, esp
        lea     esp, -12[esp]
        mov     8[esp], ebx
        mov     dword ptr 4[esp], offset 0f
        mov     dword ptr 0[esp], offset write_string_realmode
        jmp     I686_EnterRealMode
0:      pop     ebx
        pop     ebp
        ret

.section .text16, "ax"
.code16

write_string_realmode:
        mode = 8
        pageNum = 12
        color = 16
        stringSize = 20
        row = 24
        column = 28
        strptr = 32

        mov     ax, strptr + 2[bp]
        shl     ax, 12
        mov     es, ax
        mov     al, mode[bp]
        mov     ah, 0x13
        mov     bh, pageNum[bp]
        mov     bl, color[bp]
        mov     cx, stringSize[bp]
        mov     dh, row[bp]
        mov     dl, column[bp]
        mov     bp, strptr[bp]
        int     0x10

        jmp     I686_EnterProtMode

.text
.code32

.global i686_bios_video_GetVideoMode
i686_bios_video_GetVideoMode:
        lea     esp, -12[esp]
        mov     8[esp], ebx
        mov     dword ptr 4[esp], offset 0f
        mov     dword ptr 0[esp], offset get_video_mode_realmode
        jmp     I686_EnterRealMode
0:      mov     eax, 8[esp]
        mov     [eax], cx
        mov     2[eax], bh
        pop     ebx
        ret     4

.section .text16, "ax"
.code16

get_video_mode_realmode:
        mov     ax, 0xF00
        int     0x10
        mov     cx, ax
        jmp     I686_EnterProtMode

.text
.code32

.global i686_bios_video_GetCursorPosShape
i686_bios_video_GetCursorPosShape:
        lea     esp, -12[esp]
        mov     8[esp], ebx
        mov     dword ptr 4[esp], offset 0f
        mov     dword ptr 0[esp], offset get_cursor_realmode
        jmp     I686_EnterRealMode
0:      mov     eax, 8[esp]
        mov     [eax], cx
        mov     2[eax], dx
        pop     ebx
        ret     4

.section .text16, "ax"
.code16

get_cursor_realmode:
        mov     ax, 0x300
        int     0x10
        jmp     I686_EnterProtMode

.text
.code32

.global i686_bios_video_SetCursorShape
i686_bios_video_SetCursorShape:
        mov     ch, 4[esp]
        mov     cl, 8[esp]
        lea     esp, -8[esp]
        mov     dword ptr 4[esp], offset 0f
        mov     dword ptr 0[esp], offset set_cursor_realmode
        jmp     I686_EnterRealMode
0:      ret

.section .text16, "ax"
.code16

set_cursor_realmode:
        mov     ax, 0x100
        int     0x10
        jmp     I686_EnterProtMode

.text
.code32

.global i686_bios_video_SetCursorPos
i686_bios_video_SetCursorPos:
        mov     dh, 8[esp]
        mov     dl, 12[esp]
        lea     esp, -12[esp]
        mov     dword ptr 8[esp], ebx
        mov     bh, 16[esp]
        mov     dword ptr 4[esp], offset 0f
        mov     dword ptr 0[esp], offset set_cursorpos_realmode
        jmp     I686_EnterRealMode
0:      pop     ebx
        ret

.section .text16, "ax"
.code16

set_cursorpos_realmode:
        mov     ax, 0x200
        int     0x10
        jmp     I686_EnterProtMode

.text
.code32

.global i686_vbe_GetInformation
i686_vbe_GetInformation:
        buf = 8

        push    ebp
        mov     ebp, esp
        mov     eax, buf[ebp]
        shl     eax, 12
        shr     ax, 12
        mov     buf[ebp], eax
        lea     esp, -12[esp]
        mov     8[esp], edi
        mov     dword ptr 4[esp], offset 0f
        mov     dword ptr 0[esp], offset vbe_get_info_rm
        jmp     I686_EnterRealMode
0:      movzx   eax, dx
        pop     edi
        pop     ebp
        ret

.section .text16, "ax"
.code16

vbe_get_info_rm:
        les     di, buf[bp]
        mov     ax, 0x4F00
        int     0x10
        mov     dx, ax
        jmp     I686_EnterProtMode

.text
.code32

.global i686_vbe_GetModeInfo
i686_vbe_GetModeInfo:
        mode = 8
        buf = 12

        push    ebp
        mov     ebp, esp
        mov     eax, buf[ebp]
        mov     ecx, mode[ebp]
        shl     eax, 12
        shr     ax, 12
        mov     buf[ebp], eax
        lea     esp, -12[esp]
        mov     8[esp], edi
        mov     dword ptr 4[esp], offset 0f
        mov     dword ptr 0[esp], offset vbe_get_modeinfo_rm
        jmp     I686_EnterRealMode
0:      movzx   eax, dx
        pop     edi
        pop     ebp
        ret

.section .text16, "ax"
.code16

vbe_get_modeinfo_rm:
        les     di, buf[bp]
        mov     ax, 0x4F01
        int     0x10
        mov     dx, ax
        jmp     I686_EnterProtMode

.global i686_vbe_rom
i686_vbe_rom = 0xC0000
