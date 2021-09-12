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
        mov     ecx, 0x1010101
        mul     ecx
        mov     edx, edi

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
        dest    = 4
        src     = 8
        size    = 12
        mov     ecx, size[esp]
        mov     eax, esi
        test    ecx, ecx
        mov     edx, edi
        jz      9f

        mov     esi, src[esp]
        mov     edi, dest[esp]
        sub     esi, edi

        test    esi, 3
        jz      .Lmemcpy4

        test    esi, 1
        jz      .Lmemcpy2

.Lmemcpy1:
        mov     esi, src[esp]

        rep movs [edi], byte ptr [esi]
        jmp     8f

.Lmemcpy2:
        mov     esi, src[esp]

        test    edi, 1
        jz      0f
        movs    [edi], byte ptr [esi]
        lea     ecx, [ecx - 1]

0:      mov     size[esp], ecx
        shr     ecx, 1
        rep movs [edi], word ptr [esi]
        mov     ecx, size[esp]

        test    ecx, 1
        jz      8f
        movs    [edi], byte ptr [esi]
        jmp     8f

.Lmemcpy4:
        mov     esi, src[esp]

        test    edi, 1
        jz      0f
        lea     ecx, [ecx - 1]
        movs    [edi], byte ptr [esi]

0:      cmp     ecx, 2
        jb      1f

        test    edi, 2
        jz      0f
        lea     ecx, [ecx - 2]
        movs    [edi], word ptr [esi]

0:      mov     size[esp], ecx
        shr     ecx, 2
        rep movs [edi], dword ptr [esi]
        mov     ecx, size[esp]

        test    ecx, 2
        jz      1f
        movs    [edi], word ptr [esi]
1:      test    ecx, 1
        jz      8f
        movs    [edi], byte ptr [esi]

8:      mov     edi, edx
        mov     esi, eax
9:      mov     eax, dest[esp]
        ret


.globl  strcmp
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
        mov     edx, DWORD PTR [ebp+4]
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
