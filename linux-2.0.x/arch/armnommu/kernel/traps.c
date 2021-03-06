/*
 *  linux/arch/arm/kernel/traps.c
 *
 *  Copyright (C) 1995, 1996 Russell King
 *  Fragments that appear the same as linux/arch/i386/kernel/traps.c (C) Linus Torvalds
 */

/*
 * 'traps.c' handles hardware exceptions after we have saved some state in
 * 'linux/arch/arm/lib/traps.S'.  Mostly a debugging aid, but will probably
 * kill the offending process.
 */
#include <linux/config.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/signal.h>
#include <linux/sched.h>
#include <linux/mm.h>

extern void fpe_save(struct fp_soft_struct *);
extern void fpe_restore(struct fp_soft_struct *);
extern void die_if_kernel(char *str, struct pt_regs *regs, int err, int ret);
extern void c_backtrace (unsigned long fp, int pmode);
extern int ptrace_cancel_bpt (struct task_struct *);

char *processor_modes[]=
{ "USER_26", "FIQ_26" , "IRQ_26" , "SVC_26" , "UK4_26" , "UK5_26" , "UK6_26" , "UK7_26" ,
  "UK8_26" , "UK9_26" , "UK10_26", "UK11_26", "UK12_26", "UK13_26", "UK14_26", "UK15_26",
  "USER_32", "FIQ_32" , "IRQ_32" , "SVC_32" , "UK4_32" , "UK5_32" , "UK6_32" , "ABT_32" ,
  "UK8_32" , "UK9_32" , "UK10_32", "UND_32" , "UK12_32", "UK13_32", "UK14_32", "SYS_32"
};

static char *handler[]= { "prefetch abort", "data abort", "address exception", "interrupt" };

static inline void console_verbose(void)
{
	extern int console_loglevel;
	console_loglevel = 15;
}

int kstack_depth_to_print = 200;

static int verify_stack_pointer (unsigned long stackptr, int size)
{
#if defined(CONFIG_CPU_ARM2) || defined(CONFIG_CPU_ARM3)
	if (stackptr < 0x02048000 || stackptr + size > 0x03000000)
        	return -EFAULT;
#else
	if (stackptr < 0xc0000000 || stackptr + size > high_memory)
		return -EFAULT;
#endif
		return 0;
}

static void dump_stack (unsigned long *start, unsigned long *end, int offset, int max)
{
	unsigned long *p;
	int i;

	for (p = start + offset, i = 0; i < max && p < end; i++, p++) {
		if (i && (i & 7) == 0)
			printk ("\n       ");
		printk ("%08lx ", *p);
	}
	printk ("\n");
}

/*
 * These constants are for searching for possible module text
 * segments.  VMALLOC_OFFSET comes from mm/vmalloc.c; MODULE_RANGE is
 * a guess of how much space is likely to be vmalloced.
 */
#define VMALLOC_OFFSET (8*1024*1024)
#define MODULE_RANGE (8*1024*1024)

static void dump_instr (unsigned long pc)
{
	unsigned long module_start, module_end;
	int pmin = -2, pmax = 3, ok = 0;
	extern char start_kernel, _etext;

	module_start = ((high_memory + VMALLOC_OFFSET) & ~(VMALLOC_OFFSET-1));
	module_end   = module_start + MODULE_RANGE;

	if ((pc >= (unsigned long) &start_kernel) &&
	    (pc <= (unsigned long) &_etext)) {
		if (pc + pmin < (unsigned long) &start_kernel)
			pmin = ((unsigned long) &start_kernel) - pc;
		if (pc + pmax > (unsigned long) &_etext)
			pmax = ((unsigned long) &_etext) - pc;
		ok = 1;
	} else if (pc >= module_start && pc <= module_end) {
		if (pc + pmin < module_start)
			pmin = module_start - pc;
		if (pc + pmax > module_end)
			pmax = module_end - pc;
		ok = 1;
	}
	printk ("Code: ");
	if (ok) {
		int i;
		for (i = pmin; i < pmax; i++)
			printk("%08lx ", ((unsigned long *)pc)[i]);
		printk ("\n");
	} else
		printk ("pc not in code space\n");
}	

/*
 * This function is protected against kernel-mode re-entrancy.  If it
 * is re-entered it will hang the system since we can't guarantee in
 * this case that any of the functions that it calls are safe any more.
 * Even the panic function could be a problem, but we'll give it a go.
 */
void die_if_kernel(char *str, struct pt_regs *regs, int err, int ret)
{
	static int died = 0;
	unsigned long cstack, sstack, frameptr;
	
	if (user_mode(regs))
    		return;

	switch (died) {
	case 2:
		while (1);
	case 1:
		died ++;
		panic ("die_if_kernel re-entered.  Major kernel corruption.  Please reboot me!");
		break;
	case 0:
		died ++;
		break;
	}

	console_verbose ();
	printk ("Internal error: %s: %x\n", str, err);
	printk ("CPU: %d", smp_processor_id());
	show_regs (regs);
	printk ("Process %s (pid: %d, stackpage=%08lx)\nStack: ",
		current->comm, current->pid, current->kernel_stack_page);

	cstack = (unsigned long)(regs + 1);
	sstack = (unsigned long)current->kernel_stack_page;

	if (*(unsigned long *)sstack != STACK_MAGIC)
		printk ("*** corrupted stack page\n       ");

	if (verify_stack_pointer (cstack, 4))
		printk ("%08lx invalid kernel stack pointer\n", cstack);
	else if(cstack > sstack + 4096)
		printk("(sp overflow)\n");
	else if(cstack < sstack)
		printk("(sp underflow)\n");
	else
		dump_stack ((unsigned long *)sstack, (unsigned long *)sstack + 1024,
			cstack - sstack, kstack_depth_to_print);

	frameptr = regs->ARM_fp;
	if (frameptr) {
		if (verify_stack_pointer (frameptr, 4))
			printk ("Backtrace: invalid frame pointer\n");
		else {
			printk("Backtrace: \n");
			c_backtrace (frameptr, processor_mode(regs));
		}
	}

	dump_instr (instruction_pointer(regs));
	died = 0;
	if (ret != -1)
		do_exit (ret);
	else {
		cli ();
		while (1);
	}
}

void bad_user_access_alignment (const void *ptr)
{
	void *pc;
	__asm__("mov %0, lr\n": "=r" (pc));
	printk ("bad_user_access_alignment called: ptr = %p, pc = %p\n", ptr, pc);
	current->tss.error_code = 0;
	current->tss.trap_no = 11;
	force_sig (SIGBUS, current);
/*	die_if_kernel("Oops - bad user access alignment", regs, mode, SIGBUS);*/
}

asmlinkage void do_undefinstr (int address, struct pt_regs *regs, int mode)
{
	current->tss.error_code = 0;
	current->tss.trap_no = 6;
	force_sig (SIGILL, current);
	die_if_kernel("Oops - undefined instruction", regs, mode, SIGILL);
}

asmlinkage void do_excpt (int address, struct pt_regs *regs, int mode)
{
	current->tss.error_code = 0;
	current->tss.trap_no = 11;
	force_sig (SIGBUS, current);
	die_if_kernel("Oops - address exception", regs, mode, SIGBUS);
}

asmlinkage void do_unexp_fiq (struct pt_regs *regs)
{
#ifndef CONFIG_IGNORE_FIQ
	printk ("Hmm.  Unexpected FIQ received, but trying to continue\n");
	printk ("You may have a hardware problem...\n");
#endif
}

asmlinkage void bad_mode(struct pt_regs *regs, int reason, int proc_mode)
{
	printk ("Bad mode in %s handler detected: mode %s\n",
		handler[reason],
		processor_modes[proc_mode]);
	die_if_kernel ("Oops", regs, 0, -1);
}

/*
 * 'math_state_restore()' saves the current math information in the
 * old math state array, and gets the new ones from the current task.
 *
 * We no longer save/restore the math state on every context switch
 * any more.  We only do this now if it actually gets used.
 */
asmlinkage void math_state_restore (void)
{
	if (last_task_used_math == current)
		return;
	if (last_task_used_math)
		/*
		 * Save current fp state into last_task_used_math->tss.fpe_save
		 */
		fpe_save (&last_task_used_math->tss.fpstate.soft);
	last_task_used_math = current;
	if (current->used_math) {
	    	/*
    		 * Restore current fp state from current->tss.fpe_save
	    	 */
    		fpe_restore (&current->tss.fpstate.soft);
	} else {
    		/*
	    	 * initialise fp state
    		 */
	    	fpe_restore (&init_task.tss.fpstate.soft);
	    	current->used_math = 1;
	}
}

asmlinkage void arm_syscall (int no, struct pt_regs *regs)
{
	switch (no) {
	case 0: /* branch through 0 */
		printk ("[%d] %s: branch through zero\n", current->pid, current->comm);
		force_sig (SIGILL, current);
		if (user_mode(regs)) {
			show_regs (regs);
			c_backtrace (regs->ARM_fp, processor_mode(regs));
		}
		die_if_kernel ("Oops", regs, 0, SIGILL);
		break;

	case 1: /* SWI_BREAK_POINT */
		regs->ARM_pc -= 4; /* Decrement PC by one instruction */
		ptrace_cancel_bpt (current);
		force_sig (SIGTRAP, current);
		break;

	default:
		printk ("[%d] %s: arm syscall %d\n", current->pid, current->comm, no);
		force_sig (SIGILL, current);
		if (user_mode(regs)) {
			show_regs (regs);
			c_backtrace (regs->ARM_fp, processor_mode(regs));
		}
		die_if_kernel ("Oops", regs, no, SIGILL);
		break;
	}
}

asmlinkage void deferred(int n, struct pt_regs *regs)
{
	printk ("[%d] %s: old system call %X\n", current->pid, current->comm, n);
	show_regs (regs);
	force_sig (SIGILL, current);
}

asmlinkage void arm_malalignedptr(const char *str, void *pc, volatile void *ptr)
{
	printk ("Mal-aligned pointer in %s: %p (PC=%p)\n", str, ptr, pc);
}

asmlinkage void arm_invalidptr (const char *function, int size)
{
	printk ("Invalid pointer size in %s (PC=%p) size %d\n",
		function, __builtin_return_address(0), size);
}
