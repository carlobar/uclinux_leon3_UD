/*
 * This program is used to generate definitions needed by
 * assembly language modules.
 *
 * We use the technique used in the OSF Mach kernel code:
 * generate asm statements containing #defines,
 * compile this file to assembler, and then extract the
 * #defines from the assembly-language output.
 *
 * On sparc, thread_info data is static and TI_XXX offsets are computed by hand.
 */

#include <linux/config.h>
#include <linux/sched.h>
// #include <linux/mm.h>

#define DEFINE(sym, val) \
	asm volatile("\n->" #sym " %0 " #val : : "i" (val))

#define BLANK() asm volatile("\n->" : : )

#undef offsetof
#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)

int foo(void)
{
  //DEFINE(AOFF_task_thread, offsetof(struct task_struct, thread));
	BLANK();

	DEFINE(TASK_STATE, offsetof(struct task_struct, state));
	DEFINE(TASK_PRIORITY, offsetof(struct task_struct, priority));
	DEFINE(TASK_SIGNAL, offsetof(struct task_struct,  signal));
	DEFINE(TASK_BLOCKED, offsetof(struct task_struct,  blocked));
	DEFINE(TASK_FLAGS, offsetof(struct task_struct,  flags));
	DEFINE(TASK_SAVED_KSTACK, offsetof(struct task_struct,  saved_kernel_stack));
	DEFINE(TASK_KSTACK_PG, offsetof(struct task_struct,  kernel_stack_page));
	
/* Thread stuff. */

BLANK();

	DEFINE(THREAD_UMASK, offsetof(struct task_struct,  tss) + 
	       offsetof(struct thread_struct,uwinmask));

	DEFINE(THREAD_SADDR, offsetof(struct task_struct,  tss) + 
	       offsetof(struct thread_struct,sig_address));

	DEFINE(THREAD_SDESC, offsetof(struct task_struct,  tss) + 
	       offsetof(struct thread_struct,sig_desc));

	DEFINE(THREAD_KSP, offsetof(struct task_struct,  tss) + 
	       offsetof(struct thread_struct,ksp));

	DEFINE(THREAD_KPC, offsetof(struct task_struct,  tss) + 
	       offsetof(struct thread_struct,kpc));

	DEFINE(THREAD_KPSR, offsetof(struct task_struct,  tss) + 
	       offsetof(struct thread_struct,kpsr));

	DEFINE(THREAD_KWIM, offsetof(struct task_struct,  tss) + 
	       offsetof(struct thread_struct,kwim));

	DEFINE(THREAD_FORK_KPSR, offsetof(struct task_struct,  tss) + 
	       offsetof(struct thread_struct,fork_kpsr));

	DEFINE(THREAD_FORK_KWIM, offsetof(struct task_struct,  tss) + 
	       offsetof(struct thread_struct,fork_kwim));

	DEFINE(THREAD_REG_WINDOW, offsetof(struct task_struct,  tss) + 
	       offsetof(struct thread_struct,reg_window));

	DEFINE(THREAD_STACK_PTRS, offsetof(struct task_struct,  tss) + 
	       offsetof(struct thread_struct,rwbuf_stkptrs));

	DEFINE(THREAD_W_SAVED, offsetof(struct task_struct,  tss) + 
	       offsetof(struct thread_struct,w_saved));

	DEFINE(THREAD_FLOAT_REGS, offsetof(struct task_struct,  tss) + 
	       offsetof(struct thread_struct,float_regs));

	DEFINE(THREAD_FSR, offsetof(struct task_struct,  tss) + 
	       offsetof(struct thread_struct,fsr));

	DEFINE(THREAD_SIGSTK, offsetof(struct task_struct,  tss) + 
	       offsetof(struct thread_struct,sstk_info));



	return 0;
}
