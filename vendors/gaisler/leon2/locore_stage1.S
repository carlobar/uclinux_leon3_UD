#include <linux/config.h>
#include <asm/leon.h> 
#include <asm/cprefix.h>
#include <asm/ptrace.h>
#include <asm/asm_offsets.h>
#include "../common/bootloader.h"
		  
/* Entry for traps which jump to a programmer-specified trap handler.  */
#define TRAPR(H)  wr %g0, 0xfe0, %psr; mov %g0, %tbr; ba (H); mov %g0, %wim;
#define TRAP(H)  mov %psr, %l0; sethi %hi(H), %l4; jmp %l4+%lo(H); nop;

/* Unexcpected trap will halt the processor by forcing it to error state */
#undef BAD_TRAP
#define BAD_TRAP ta 0; nop; nop; nop;

/* Software trap. Treat as BAD_TRAP for the time being... */
#define SOFT_TRAP TRAP(_hwerr)

#define PSR_INIT   0x00C0 //0x1FC0	/* Disable traps, set s and ps */
#define WIM_INIT   2
#define EF_INIT    0x1000 /*enable floating point*/

WINDOWSIZE = (16 * 4)
ARGPUSHSIZE = (6 * 4)
ARGPUSH = (WINDOWSIZE + 4)
MINFRAME = (WINDOWSIZE + ARGPUSHSIZE + 4)
#define STACK_ALIGN	8
#define SA(X)	(((X)+(STACK_ALIGN-1)) & ~(STACK_ALIGN-1))
	
  .seg	"text"
  .global _trap_table, start, _memcfg1, _memcfg2, _stack, _ftreg, bdinit1, bdinit2, main, trapbase
  .global C_LABEL(bootup_user_stack), C_LABEL(bootup_kernel_stack),C_LABEL(init_task)
  .global C_LABEL(bootloader_supplied_stack)
  .global start_kernel
  .global bootaddress

sysini:
        save    %sp,-104,%sp     ! must begin with a save insn. bzlocore.S checks for it
	
        set	C_LABEL(bootloader_supplied_stack), %g2
        st      %sp,[%g2]
        	
	set     0x40000000,%g2
	st      %g0,[%g2]
	
	set	BOOTLOADER_stack, %fp
	andn	%fp, 0x0f, %fp
	sub	%fp, 64, %sp
	
	! check weather bootloader params where correct
	! (in case the bootloader.h params allocated too much memory)
	add %g0,1,%g2
	st  %g2,[%fp]
	ld  [%fp],%g4
	cmp %g2,%g4
	bne 9001f
	st  %g0,[%fp]
	ld  [%fp],%g4
	cmp %g0,%g4
	beq 9002f
	nop
9001:	
	set msg1err,%i0
	set (msg1err_end-msg1err)-1,%i1
	call _print_locore
	nop 
	ba 9001b
	nop
	
9002:	
	set _bss_start,%g2
	set _bss_end,%g4
2:	stb   %g0,[%g2]
	inc  1,%g2
	cmp  %g2,%g4
	bl   2b
	nop

	set	_fptest_trap_table, %g3
	wr	%g3, 0x0, %tbr
	nop; nop; nop 
	set	WIM_INIT, %g3
	mov	%g3, %wim
	set	0x1000, %g3
	mov	%psr, %g2
	wr	%g2, %g3, %psr
	nop; nop; nop

	mov     %g0,%g4
	set	0x80000000, %g1			! LEON register base
	ld	[%g1+0x90], %fsr		! Check if FPU is present
	nop
	
	mov     EF_INIT,%g4
	
_fpu_dis:		
	/* Here we go, start using Linux's trap table... */
	set	trapbase, %g3
	wr	%g3, 0x0, %tbr
	nop
	nop
	nop

	set	WIM_INIT, %g2
	mov	%g2, %wim
	nop; nop; nop

	set     PSR_INIT, %g1
	!or      %g1,%g4,%g1
        wr      %g1, 0x20, %psr		! enable traps
        nop; nop; nop;

	!set	BOOTLOADER_stack, %fp
	!andn	%fp, 0x0f, %fp
	!sub	%fp, 64, %sp


	/* I want a kernel stack NOW! */
	set	C_LABEL(bootup_user_stack), %g1
	add	%g1, (0x2000 - REGWIN_SZ), %sp
	mov	0, %fp			/* And for good luck */
	 
	set	C_LABEL(init_task), %g4
		
	set	C_LABEL(bootup_kernel_stack), %g3
	st	%g3, [%g4 + TASK_SAVED_KSTACK]
	st	%g0, [%g4 + THREAD_UMASK]



	
330:

!	call	bdinit2
	nop

	set    0x40000000,%o1
	ld     [%o1] , %o2
	
	set    0xf0000000,%o3
	and    %o2,%o3,%o2	
	set    0x90000000,%o3	       ! check weather this is somewhere like a save %sp instr
	cmp    %o2,%o3
	bne    3f
	nop

	! dsumon loaded image, alread there
	!call	%o1			! jump to __main, addr should be in 0x40000000
        !nop
!
!	ta 	0			! Halt if _main would return ...
!	nop

	! flash resident image, decompress
3:	call   start_kernel ! jump to <linux>/init/main.c:start_kernel()
	!call	0x4000 !__main		
	nop

	ba 330b
	nop
	
	
     .global _print_locore
_print_locore:
!211: sethi  %hi(0x80000074), %g1  
!     ld  [ %g1+%lo(0x80000074) ] , %g1
!     btst  4, %g1
!     be   211b
!     nop 
!     ldsb  [ %o0 ], %g3
!     sethi  %hi(0x80000070), %g1  
!     st  %g3, [ %g1+%lo(0x80000070) ] 
!     add  %o1, -1, %o1
!     cmp  %o1, -1
!     bne  211b
!     inc  %o0
     nop 
     retl 
     nop
 
	 .section        ".rodata"
msg1:
	.asciz "Boot:\n"
msg1_end:
msg1err:
	.asciz "Memparams wrong, change config\n"
msg1err_end:

	.align 0x1000
_fptest_trap_table:
  BAD_TRAP			! 00 reset trap 
  BAD_TRAP;      		! 01 instruction_access_exception
  BAD_TRAP;      		! 02 illegal_instruction
  BAD_TRAP;      		! 03 priveleged_instruction
  TRAP(_fpu_dis);		! 04 fp_disabled
  BAD_TRAP	! 05 window_overflow
  BAD_TRAP	! 06 window_underflow
