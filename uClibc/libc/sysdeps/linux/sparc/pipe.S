/* Konrad Eisele <eiselekd@web.de> */
        
#include <asm/errno.h>
#include <asm/unistd.h>

/* int pipe(int filedesc[2]); */

	.text
	.align	4
	.globl	__pipe
	.type	__pipe,@function

__pipe:
	save	%sp,-96,%sp

	/* sanity check arguments */
	tst	%i0
	be	.Lerror
	nop

	/* Do the system call */
	set	__NR_pipe,%g1
	ta	0x10
	bcs	.Lerror
	 nop
        
	st %o0,[%i0]
	st %o1,[%i0+4]
	ret
	 restore %g0,%g0,%o0

.Lerror:
	call	__errno_location
	 or	%g0,EINVAL,%i0
	st	%i0,[%o0]
	ret
	 restore %g0,-1,%o0

	.size	__pipe, .-__pipe


.weak    pipe    ;        pipe    =   __pipe
