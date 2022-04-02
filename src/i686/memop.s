.intel_syntax noprefix
.text

.global memset
.type memset, @function
memset:
        dest    = 4
        val     = 8
        count   = 12
        cur     = edi
        size    = ecx

        movzx   eax, byte ptr val[esp]
        mov     edx, edi
        imul    eax, 0x1010101

        mov     size, count[esp]
        mov     cur, dest[esp]

        test    size, size
        jz      2f

        test    cur, 1
        jz      0f
        lea     size, [size - 1]
        stos    [cur], al

0:      cmp     size, 2
        jb      1f

        test    cur, 2
        jz      0f
        lea     size, [size - 2]
        stos    [cur], ax

0:      mov     count[esp], size
        shr     size, 2
        rep stos [cur], eax
        mov     size, count[esp]

1:      test    size, 2
        jz      0f
        stos    [cur], ax

0:      test    size, 1
        jz      2f
        stos    [cur], al

2:      mov     edi, edx
        mov     eax, dest[esp]
        ret

.global memcpy
.type memcpy, @function
memcpy:
        dest    = 12
        src     = 16
        size    = 20
        push    esi
        mov     ecx, size - 4[esp]
        push    edi
        test    ecx, ecx
        jz      8f

        mov     esi, src[esp]
        mov     eax, esi
        mov     edi, dest[esp]
        sub     eax, edi
.lmvdir:mov     edx, esi

        cmp     ecx, 16
        jc      .Lmemcpy1

        and     eax, 3
        jmp     .Lmemcpy_tbl[eax * 4]

.Lmemcpy_End3:
        movs    [edi], byte ptr [esi]
.Lmemcpy_End2:
        movs    [edi], byte ptr [esi]
.Lmemcpy_End1:
        movs    [edi], byte ptr [esi]
        jmp     8f

.Lmemcpy1:
        rep movs [edi], byte ptr [esi]

8:      mov     eax, dest[esp]
        pop     edi
        pop     esi
        ret

.Lmemcpy2:
        test    edi, 1
        jz      0f
        movs    [edi], byte ptr [esi]
        lea     ecx, [ecx - 1]

0:      mov     eax, ecx
        shr     ecx, 1
        rep movs [edi], word ptr [esi]

        and     eax, 1
        jmp     .Lmemcpy_EndTbl[eax * 4]

.Lmemcpy4:
        and     edx, 3
        jmp     .Lmemcpy4_StartTbl[edx * 4]

.Lmemcpy4_Start3:
        movs    [edi], byte ptr [esi]
.Lmemcpy4_Start2:
        movs    [edi], byte ptr [esi]
.Lmemcpy4_Start1:
        movs    [edi], byte ptr [esi]
        xor     edx, 3
        lea     ecx, [ecx - 1]
        sub     ecx, edx

0:      mov     eax, ecx
        shr     ecx, 2
        rep movs [edi], dword ptr [esi]

        and     eax, 3
        jmp     .Lmemcpy_EndTbl[eax * 4]

.section .rodata
.align 4
.Lmemcpy_tbl:
        .4byte  .Lmemcpy4
        .4byte  .Lmemcpy1
        .4byte  .Lmemcpy2
        .4byte  .Lmemcpy1

.Lmemcpy4_StartTbl:
        .4byte  0b
        .4byte  .Lmemcpy4_Start3
        .4byte  .Lmemcpy4_Start2
        .4byte  .Lmemcpy4_Start1

.Lmemcpy_EndTbl:
        .4byte  8b
        .4byte  .Lmemcpy_End1
        .4byte  .Lmemcpy_End2
        .4byte  .Lmemcpy_End3
.text

.global memmove
.type memmove, @function
memmove:
        dest    = 12
        src     = 16
        size    = 20
        push    esi
        mov     ecx, size - 4[esp]
        push    edi
        test    ecx, ecx
        jz      8f

        mov     esi, src[esp]
        mov     eax, esi
        mov     edi, dest[esp]
        sub     eax, edi
        jz      8f
        jnc     .lmvdir
        std
        lea     esi, [esi + ecx]
        add     edi, ecx
        mov     edx, esi

        cmp     ecx, 16
        jc      .Lmemmove1

        and     eax, 3
        jmp     .Lmemmove_Tbl[eax * 4]

.Lmemmove_End3:
        movs    [edi], byte ptr [esi]
.Lmemmove_End2:
        movs    [edi], byte ptr [esi]
.Lmemmove_End1:
        movs    [edi], byte ptr [esi]
        jmp     8f

.Lmemmove1:
        sub     edi, 1
        lea     esi, [esi - 1]
        rep movs [edi], byte ptr [esi]

8:      cld
        mov     eax, dest[esp]
        pop     edi
        pop     esi
        ret

.Lmemmove2:
        dec     edi
        lea     esi, [esi - 1]
        test    edx, 1
        jz      0f

        lea     ecx, [ecx - 1]
        movs    [edi], byte ptr [esi]

0:      lea     esi, [esi - 1]
        dec     edi
        mov     eax, ecx
        shr     ecx, 1
        rep movs [edi], word ptr [esi]
        inc     edi
        lea     esi, [esi + 1]

        and     eax, 1
        jmp     .Lmemmove_EndTbl[eax * 4]

.Lmemmove4:
        sub     edi, 1
        lea     esi, [esi - 1]

        and     edx, 3
        jmp     .Lmemmove4_StartTbl[edx * 4]

.Lmemmove4_Start3:
        movs    [edi], byte ptr [esi]
.Lmemmove4_Start2:
        movs    [edi], byte ptr [esi]
.Lmemmove4_Start1:
        movs    [edi], byte ptr [esi]
        sub     ecx, edx

.Lmemmove4_Start0:
        lea     esi, [esi - 3]
        sub     edi, 3
        mov     eax, ecx
        shr     ecx, 2
        rep movs [edi], dword ptr [esi]
        add     edi, 3
        lea     esi, [esi + 3]

        and     eax, 3
        jmp     .Lmemmove_EndTbl[eax * 4]

.section .rodata
.align 4
.Lmemmove_Tbl:
        .4byte  .Lmemmove4
        .4byte  .Lmemmove1
        .4byte  .Lmemmove2
        .4byte  .Lmemmove1

.Lmemmove_EndTbl:
        .4byte  8b
        .4byte  .Lmemmove_End1
        .4byte  .Lmemmove_End2
        .4byte  .Lmemmove_End3

.Lmemmove4_StartTbl:
        .4byte  .Lmemmove4_Start0
        .4byte  .Lmemmove4_Start1
        .4byte  .Lmemmove4_Start2
        .4byte  .Lmemmove4_Start3
.text

.globl strcmp
.type strcmp, @function
strcmp:
#        push    ebp
#        mov     ebp, esp
        push    ebx
        mov     edx, DWORD PTR [esp+8]
        mov     ecx, DWORD PTR [esp+12]
        movzx   eax, BYTE PTR [edx]
        movzx   ebx, BYTE PTR [ecx]
        test    al, al
        jne     1f
        jmp     2f
        .align 16
0:
        movzx   eax, BYTE PTR [edx+1]
        add     edx, 1
        lea     ecx, [ecx+1]
        test    al, al
        movzx   ebx, BYTE PTR [ecx]
        je      2f
1:
        cmp     bl, al
        je      0b
2:
        sub     eax, ebx
#        mov     ebx, DWORD PTR [ebp-4]
#        leave
        pop     ebx
        ret

.globl  strlen
.type strlen, @function
strlen:
        mov     edx, DWORD PTR [esp+4]
        cmp     BYTE PTR [edx], 0
        je      1f
        mov     eax, edx
        .align 16
0:
        add     eax, 1
        cmp     BYTE PTR [eax], 0
        jne     0b
        sub     eax, edx
        ret
        .align 16
1:
        xor     eax, eax
        ret

#memcmp memcpy memmove
