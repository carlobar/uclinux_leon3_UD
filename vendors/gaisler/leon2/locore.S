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
  !.section        ".start", "ax"
  .global _trap_table, start, _memcfg1, _memcfg2, _stack, _ftreg, bdinit1, bdinit2, main, trapbase
  .global C_LABEL(bootup_user_stack), C_LABEL(bootup_kernel_stack),C_LABEL(init_task)
  .global bootaddress

start:
_start:	
_trap_table:
  TRAPR(_hardreset);		! 00 reset trap 
  BAD_TRAP;      		! 01 instruction_access_exception
  BAD_TRAP;      		! 02 illegal_instruction
  BAD_TRAP;      		! 03 priveleged_instruction
  TRAP(_fpu_dis);		! 04 fp_disabled
  TRAP(_locore_window_overflow);	! 05 window_overflow
  TRAP(_locore_window_underflow);	! 06 window_underflow


/* Number of register windows */
#define NWINDOWS 	8

  	.global _locore_window_overflow

_locore_window_overflow:

  	mov  %wim, %l3       		! Calculate next WIM
  	mov  %g1, %l7           
  	srl  %l3, 1, %g1        
  	sll  %l3, NWINDOWS-1 , %l4
  	or   %l4, %g1, %g1

  	save                              ! Get into window to be saved.
  	mov  %g1, %wim
  	nop; nop; nop
        st   %l0, [%sp + 0];
        st   %l1, [%sp + 4];
        st   %l2, [%sp + 8];
        st   %l3, [%sp + 12];
        st   %l4, [%sp + 16]; 
        st   %l5, [%sp + 20];
        st   %l6, [%sp + 24]; 
        st   %l7, [%sp + 28]; 
        st   %i0, [%sp + 32]; 
        st   %i1, [%sp + 36]; 
        st   %i2, [%sp + 40]; 
        st   %i3, [%sp + 44]; 
        st   %i4, [%sp + 48]; 
        st   %i5, [%sp + 52]; 
        st   %i6, [%sp + 56]; 
        st   %i7, [%sp + 60];
  	restore				! Go back to trap window.
  	mov  %l7, %g1
  	jmp  %l1			! Re-execute save.
  	rett %l2

  /* Window underflow trap handler.  */

  	.global  _locore_window_underflow

_locore_window_underflow:

	mov  %wim, %l3			! Calculate next WIM
	sll  %l3, 1, %l4
  	srl  %l3, NWINDOWS-1, %l5
  	or   %l5, %l4, %l5
  	mov  %l5, %wim
  	nop; nop; nop
  	restore				! Two restores to get into the
  	restore				! window to restore
        ld   [%sp + 0], %l0; 		! Restore window from the stack
        ld   [%sp + 4], %l1; 
        ld   [%sp + 8], %l2; 
        ld   [%sp + 12], %l3; 
        ld   [%sp + 16], %l4; 
        ld   [%sp + 20], %l5; 
        ld   [%sp + 24], %l6; 
        ld   [%sp + 28], %l7; 
        ld   [%sp + 32], %i0; 
        ld   [%sp + 36], %i1; 
        ld   [%sp + 40], %i2; 
        ld   [%sp + 44], %i3; 
        ld   [%sp + 48], %i4; 
        ld   [%sp + 52], %i5; 
        ld   [%sp + 56], %i6; 
        ld   [%sp + 60], %i7;
  	save				! Get back to the trap window.
  	save
  	jmp  %l1			! Re-execute restore.
  	rett  %l2

	

_hardreset:
	
/* First, we wash the global registers to avoid possible EDAC errors.
 */

	flush

		
	clr	%g1
	clr	%g2
	clr	%g3
	clr	%g4
	clr	%g5
	clr	%g6
	clr	%g7

	set	0x80000000, %g1		! LEON register base
	set	0x1000f, %g2

      st    %g2, [%g1 + 0x14]	! clear some LEON registers
      st    %g0, [%g1 + 0x08]     
      st    %g0, [%g1 + 0x90]     
      st    %g0, [%g1 + 0x94]    
      st    %g0, [%g1 + 0x98]   
      st    %g0, [%g1 + 0x9C]  
      st    %g0, [%g1 + 0xA0] 
      st    %g0, [%g1 + 0xA4]
      st    %g0, [%g1 + 0xA8]
      st    %g0, [%g1 + 0x78]
      st    %g0, [%g1 + 0x88]

	set	0xaa00, %g2		! io port initialization
	st    %g2, [%g1 + 0xA0] 

!	call	reginit
	nop

	set	BOOTLOADER_memcfg1, %g2		! Load memory config register 1
	ld	[%g1], %g3		! 
	and	%g3, 0x300, %g3
	or	%g2, %g3, %g2
  	st	%g2, [%g1 + 0x00]
	set	BOOTLOADER_memcfg2, %g2		! Load memory config register 2
#if !( defined(TSIM) || !defined(BZIMAGE))
	st	%g2, [%g1 + 0x04]		! only for prom version, else done by "dumon -i"	
#endif 
	set	BOOTLOADER_ftreg, %g2		! Init FT register
	st	%g2, [%g1 + 0x08]
  	ld	[%g1 + 0x08], %g2
	srl	%g2, 30, %g2
	andcc	%g2, 3, %g6
	bne,a	1f
	mov	%g0, %asr16		! clear err_reg
1:
	set	(BOOTLOADER_FREQ_KHZ+500)/1000, %g2		! Load timer scaler register
	sub	%g2, 1, %g2
  	st	%g2, [%g1 + 0x64]
  	st	%g2, [%g1 + 0x60] 
	set	BOOTLOADER_uart, %g2		! Load UART scaler register
	st	%g2, [%g1 + 0x7c]
  	st	%g2, [%g1 + 0x8c]

#if defined(CONFIG_UARTLOOPBACK) || (BOOTLOADER_loopback==1)
	set	0x83|(BOOTLOADER_flow<<6), %g2			! Enable uarts, (flow ctrl), loopback
#else
	set	0x03|(BOOTLOADER_flow<<6), %g2			! Enable uarts, (flow ctrl)
#endif	  
  	st	%g2, [%g1 + 0x78]
  	st	%g2, [%g1 + 0x88]
	set	0xaa00, %g2	
  	st	%g2, [%g1 + 0xa4]

	set msg1,%o0
	set (msg1_end-msg1)-1,%o1
	call _print_locore
	nop

	set	WIM_INIT, %g3
	mov	%g3, %wim
	set	0x1000, %g3
	mov	%psr, %g2
	wr	%g2, %g3, %psr
	nop; nop; nop
	mov     EF_INIT,%g5
	ld	[%g1+0x90], %fsr		! Check if FPU is present

	ba	fixfpu
	nop

! FPU disabled trap address

_fpu_dis:
	mov  %g0,%g5
	mov	%g2, %psr
	nop; nop; nop
	ba    sysini
	nop
	

! Wash FPU register file

fixfpu:

	subcc	%g6, 0, %g0
	be	sysini		! skip reg init if no FT

	ld	[%g0], %f0
	ld	[%g0], %f1
	ld	[%g0], %f2
	ld	[%g0], %f3
	ld	[%g0], %f4
	ld	[%g0], %f5
	ld	[%g0], %f6
	ld	[%g0], %f7
	ld	[%g0], %f8
	ld	[%g0], %f9
	ld	[%g0], %f10
	ld	[%g0], %f11
	ld	[%g0], %f12
	ld	[%g0], %f13
	ld	[%g0], %f14
	ld	[%g0], %f15
	ld	[%g0], %f16
	ld	[%g0], %f17
	ld	[%g0], %f18
	ld	[%g0], %f19
	ld	[%g0], %f20
	ld	[%g0], %f21
	ld	[%g0], %f22
	ld	[%g0], %f23
	ld	[%g0], %f24
	ld	[%g0], %f25
	ld	[%g0], %f26
	ld	[%g0], %f27
	ld	[%g0], %f28
	ld	[%g0], %f29
	ld	[%g0], %f30
	ld	[%g0], %f31

! System init

sysini:
	
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
	!or      %g1,%g5,%g1
        wr      %g1, 0x20, %psr		! enable traps
        nop; nop; nop;

	set	BOOTLOADER_stack, %fp
	andn	%fp, 0x0f, %fp
	sub	%fp, 64, %sp

        set	C_LABEL(bootloader_supplied_stack), %g1 /* tell the kernel the memory size */
        st      %sp,[%g1]

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
3:	call   main    !jump to <linux>/arch/sparcnommu/platform/LEON-2.x/start.c:main()
                       !main() will copy .data section 
	!call	0x4000 !__main		
	nop

        set     BOOTLOADER_stack,%o0
        set	C_LABEL(bootloader_supplied_stack), %g1 /* tell the kernel the memory size */
        st      %o0,[%g1]
        
        call start_kernel !and call start_kernel()
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
	