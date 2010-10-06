#include <features.h>

#ifdef __linux_2_0_x


.global	_start

.text

	nop				/* filler */
	nop

_start:
        save  %sp, -104, %sp

	ld	[%i6 + 0x40], %o0	/* get argc */
	ld	[%i6 + 0x44], %o1	/* get argv */
	ld	[%i6 + 0x48], %o2	/* get env */
 
#if defined __UCLIBC_CTOR_DTOR__
        set _init,%o3
        set _fini,%o4
	call	__uClibc_start_main
#else
	call	__uClibc_main
#endif                
	 nop
         nop 
	
	/*call _exit
	nop*/
	 ret 
	restore 
	

/****************************************************************************/

#else

	.align 4
	.global _start,__uClibc_main
	
_start:

   save  %sp, -104, %sp
   ld  [ %fp + 0x40 ], %o0
   add  %fp, 0x44, %o1
   sll  %o0, 2, %o2
   add  %o2, %o1, %o2
   add  %o2, 4, %o2
   call  __uClibc_main
   nop 
   nop 
   ret 
   restore 

#endif
