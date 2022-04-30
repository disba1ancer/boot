.intel_syntax noprefix

.text
.code32
.global i686_bios_disk_GetDriveParameters
i686_bios_disk_GetDriveParameters:
        diskNum = 8
        bufPtr = 12
        push    ebp
        mov     ebp, esp
        lea     esp, [esp - 12]
        mov     eax, bufPtr[ebp]
        shl     eax, 12
        mov     edx, diskNum[ebp]
        shr     ax, 12
        mov     dword ptr 8[esp], esi
        mov     bufPtr[ebp], eax
        mov     dword ptr 4[esp], offset 0f
        mov     dword ptr 0[esp], offset GetDriveParametersRM
        jmp     I686_EnterRealMode
0:      pop     esi
        mov     eax, edx
        pop     ebp
        ret

.section .text16, "ax"
.code16
GetDriveParametersRM:
        lds     si, bufPtr[bp]
        mov     ax, 0x4800
        int     0x13
        movzx   edx, ah
        jmp     I686_EnterProtMode

.text
.code32
.global i686_bios_disk_Read
i686_bios_disk_Read:
        diskNum = 8
        bufPtr = 12
        push    ebp
        mov     ebp, esp
        lea     esp, [esp - 12]
        mov     eax, bufPtr[ebp]
        shl     eax, 12
        mov     edx, diskNum[ebp]
        shr     ax, 12
        mov     dword ptr 8[esp], esi
        mov     bufPtr[ebp], eax
        mov     dword ptr 4[esp], offset 0f
        mov     dword ptr 0[esp], offset ReadRM
        jmp     I686_EnterRealMode
0:      pop     esi
        mov     eax, edx
        pop     ebp
        ret

.section .text16, "ax"
.code16
ReadRM:
        lds     si, bufPtr[bp]
        mov     ax, 0x4200
        int     0x13
        movzx   edx, ah
        jmp     I686_EnterProtMode
