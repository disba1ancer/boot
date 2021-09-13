.intel_syntax noprefix

.section .init
        /* gcc will nicely put the contents of crtend.o's .init section here. */
        leave
        ret

.section .fini
        /* gcc will nicely put the contents of crtend.o's .fini section here. */
        leave
        ret
