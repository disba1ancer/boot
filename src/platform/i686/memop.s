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
        stacksize = 12
        left    = 0 + stacksize + 4
        right   = 4 + stacksize + 4
        sub     esp, stacksize
        mov     [esp], ebx
        mov     [esp + 4], esi
        mov     [esp + 8], edi
        mov     esi, left[esp]
        mov     edi, right[esp]
        test    esi, 3
        jz      1f

0:      movzx   eax, byte ptr [esi]
        movzx   edx, byte ptr [edi]
        add     esi, 1
        add     edi, 1
        sub     eax, edx
        jnz     6f
        test    edx, edx
        jz      6f
        test    esi, 3
        jnz     0b

1:      mov     eax, esi
        sub     eax, edi
        and     eax, 3
        jmp     .Lstrcmp_Tbl[eax * 4]

0:      add     esi, 1
        add     edi, 1
.Lstrcmp1:
        movzx   eax, byte ptr[esi]
        movzx   edx, byte ptr[edi]
        sub     eax, edx
        jnz     6f
        test    edx, edx
        jnz     0b
        jmp     6f

.Lstrcmp2:
0:      movzx   eax, word ptr[esi]
        movzx   edx, word ptr[edi]
        add     esi, 2
        add     edi, 2
        mov     ecx, edx
        mov     ebx, edx
        and     ebx, 0xFFFF7F7F
        or      ecx, 0xFFFF7F7F
        add     ebx, 0xFFFF7F7F
        or      ebx, ecx
        sub     eax, edx
        not     ebx
        jnz     7f
        test    ebx, ebx
        jz      0b
        jmp     5f

.Lstrcmp4:
0:      mov     eax, [esi]
        mov     edx, [edi]
        add     esi, 4
        add     edi, 4
        mov     ecx, edx
        mov     ebx, edx
        and     ebx, 0x7F7F7F7F
        or      ecx, 0x7F7F7F7F
        add     ebx, 0x7F7F7F7F
        or      ebx, ecx
        sub     eax, edx
        not     ebx
        jnz     7f
        test    ebx, ebx
        jz      0b

5:      xor     eax, eax
6:      mov     ebx, [esp]
        mov     esi, [esp + 4]
        mov     edi, [esp + 8]
        add     esp, stacksize
        ret

7:      bsf     ecx, eax
        bsf     ebx, ebx
        jz      0f
        cmp     ecx, ebx
        cmova   ecx, ebx
0:      add     eax, edx
        and     ecx, 0x18
        shr     eax, cl
        shr     edx, cl
        movzx   eax, al
        movzx   ecx, dl
        sub     eax, ecx
        jmp     6b

.section .rodata
.align 4
.Lstrcmp_Tbl:
        .4byte  .Lstrcmp4
        .4byte  .Lstrcmp1
        .4byte  .Lstrcmp2
        .4byte  .Lstrcmp1
.text

.globl  strlen
.type strlen, @function
strlen:
        inStr   = 4
        mov     eax, inStr[esp]
        mov     ecx, eax
        cmp     byte ptr [eax], 0
        jz      1f
0:      test    eax, 3
        jz      0f
        add     eax, 1
        cmp     byte ptr [eax], 0
        jne     0b
        sub     eax, ecx
        ret

1:      xor     eax, eax
        ret

1:      lea     eax, [eax + 4]
0:      mov     ecx, [eax]
        mov     edx, ecx
        and     ecx, 0x7F7F7F7F
        or      edx, 0x7F7F7F7F
        add     ecx, 0x7F7F7F7F
        or      ecx, edx
        not     ecx
        bsf     edx, ecx
        jz      1b
        shr     edx, 3
        add     eax, edx
        sub     eax, inStr[esp]
        ret

.globl  memcmp
.type memcmp, @function
memcmp:
        left    = 12
        right   = 16
        size    = 20
        push    ebx
        push    esi
        mov     esi, left[esp]
        mov     ecx, right[esp]
        mov     ebx, size[esp]
        mov     eax, ebx
        add     ebx, esi
        cmp     eax, 4
        jc      .Lmemcmp1
        test    esi, 3
        jz      1f

0:      movzx   eax, byte ptr [esi]
        movzx   edx, byte ptr [ecx]
        add     esi, 1
        add     ecx, 1
        sub     eax, edx
        jne     8f
        test    esi, 3
        jnz     0b

1:      mov     eax, esi
        sub     eax, ecx
        and     eax, 3
        jmp     .Lmemcmp_Tbl[eax * 4]

.Lmemcmp1:
0:      movzx   eax, byte ptr[esi]
        movzx   edx, byte ptr[ecx]
        add     esi, 1
        add     ecx, 1
        sub     eax, edx
        jne     8f
        cmp     esi, ebx
        jne     0b
        jmp     8f

.Lmemcmp2:
0:      movzx   eax, word ptr[esi]
        movzx   edx, word ptr[ecx]
        sub     eax, edx
        jne     7f
        add     esi, 2
        add     ecx, 2
        cmp     esi, ebx
        jb      0b
        jmp     8f

.Lmemcmp4:
0:      mov     eax, [esi]
        mov     edx, [ecx]
        sub     eax, edx
        jne     7f
        add     esi, 4
        add     ecx, 4
        cmp     esi, ebx
        jb      0b
        jmp     8f

7:      bsf     ecx, eax
        add     eax, edx
        and     ecx, 0x18
        shr     edx, cl
        shr     eax, cl
        shr     ecx, 3
        movzx   eax, al
        add     esi, ecx
        movzx   edx, dl
        cmp     esi, ebx
        jae     9f
        sub     eax, edx

8:      pop     esi
        pop     ebx
        ret
9:      xor     eax, eax
        jmp     8b

.section .rodata
.align 4
.Lmemcmp_Tbl:
        .4byte  .Lmemcmp4
        .4byte  .Lmemcmp1
        .4byte  .Lmemcmp2
        .4byte  .Lmemcmp1
.text

#memcmp memcpy memmove
