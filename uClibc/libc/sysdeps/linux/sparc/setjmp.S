/* Copyright (C) 1991, 93, 94, 96, 97, 98 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <sys/syscall.h>

#define _ASM 1
#define _SETJMP_H
#include <bits/setjmp.h>
#ifdef __linux__
#define ST_FLUSH_WINDOWS 0x83   //SP_TRAP_FWIN 
#else
#define ST_FLUSH_WINDOWS 3
#endif
	 	
.global   _setjmp;
.align 4;
.type  _setjmp ,@function; 

_setjmp: 
	b	1f
	 set	0, %o1
.size _setjmp,.-_setjmp;






.global   setjmp;
.align 4;
.type  setjmp ,@function; 

setjmp: 
	set	1, %o1
.size setjmp,.-setjmp;




.global   __sigsetjmp;
.align 4;
.type  __sigsetjmp ,@function; 

__sigsetjmp: 
1:
	/* Save our PC, SP and FP.  Save the signal mask if requested with
	   a tail-call for simplicity; it always returns zero.  */
	ta	ST_FLUSH_WINDOWS

	st	%o7, [%o0 + (JB_PC * 4)]
	st	%sp, [%o0 + (JB_SP * 4)]
	st	%fp, [%o0 + (JB_FP * 4)]

	mov	%o7, %g1
	call	__sigjmp_save
	 mov	%g1, %o7
.size __sigsetjmp,.-__sigsetjmp;



.weak   _setjmp    
.weak   setjmp  
