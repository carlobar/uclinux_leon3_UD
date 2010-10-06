#include <linux/sched.h>
#include <linux/timer.h>
#include <linux/interrupt.h>
#include <linux/tty.h>
#include <linux/tty_flip.h>
#include <linux/config.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/errno.h>
#include <linux/kd.h>
#include <linux/malloc.h>
#include <linux/major.h>
#include <linux/mm.h>
#include <linux/ioport.h>

#include <asm/io.h>
#include <asm/system.h>
#include <asm/segment.h>
#include <asm/bitops.h>

#include <kbd_kern.h>
#include <vt_kern.h>
#include <consolemap.h>
#include <selection.h>
#include <console_struct.h>

#define LEON_VGAMEM_SIZE (80*51)

#if !(defined(CONFIG_GRLIB_GAISLER_VGAMEM_BASE) && defined(LEON_VGAMEM_SIZE))
#error defined CONFIG_GRLIB_GAISLER_VGAMEM_BASE && LEON_VGAMEM_SIZE
#endif

void
set_palette (void)
{
	int i, j ;

	if (video_type != VIDEO_TYPE_VGAC || console_blanked ||
	    vt_cons[fg_console]->vc_mode == KD_GRAPHICS)
		return ;

	for (i=j=0; i<16; i++) {
/* 		outb_p (color_table[i], dac_reg) ; */
/* 		outb_p (vc_cons[fg_console].d->vc_palette[j++]>>2, dac_val) ; */
/* 		outb_p (vc_cons[fg_console].d->vc_palette[j++]>>2, dac_val) ; */
/* 		outb_p (vc_cons[fg_console].d->vc_palette[j++]>>2, dac_val) ; */
	}
}

void
__set_origin(unsigned short offset)
{
	unsigned long flags;

	clear_selection();

	save_flags(flags); cli();
	__origin = offset;
/* 	outb_p(12, video_port_reg); */
/* 	outb_p(offset >> 8, video_port_val); */
/* 	outb_p(13, video_port_reg); */
/* 	outb_p(offset, video_port_val); */
	restore_flags(flags);
}

/*
 * Put the cursor just beyond the end of the display adaptor memory.
 */
void
hide_cursor(void)
{
  /* This is inefficient, we could just put the cursor at 0xffff,
     but perhaps the delays due to the inefficiency are useful for
     some hardware... */
/* 	outb_p(14, video_port_reg); */
/* 	outb_p(0xff&((video_mem_term-video_mem_base)>>9), video_port_val); */
/* 	outb_p(15, video_port_reg); */
/* 	outb_p(0xff&((video_mem_term-video_mem_base)>>1), video_port_val); */
}

void
set_cursor(int currcons)
{
	unsigned long flags;

	if (currcons != fg_console || console_blanked || vcmode == KD_GRAPHICS)
		return;
	if (__real_origin != __origin)
		__set_origin(__real_origin);
	save_flags(flags); cli();
	if (deccm) {
/* 		outb_p(14, video_port_reg); */
/* 		outb_p(0xff&((pos-video_mem_base)>>9), video_port_val); */
/* 		outb_p(15, video_port_reg); */
/* 		outb_p(0xff&((pos-video_mem_base)>>1), video_port_val); */
	} else
		hide_cursor();
	restore_flags(flags);
}

void
get_scrmem(int currcons)
{
 	memcpyw((unsigned short *)vc_scrbuf[currcons], 
 		(unsigned short *)origin, video_screen_size); 
	origin = video_mem_start = (unsigned long)vc_scrbuf[currcons];
	scr_end = video_mem_end = video_mem_start + video_screen_size;
	pos = origin + y*video_size_row + (x<<1);
}

void
set_scrmem(int currcons, long offset)
{
  int i;
  if (video_mem_term - video_mem_base < offset + video_screen_size)
    offset = 0;	/* strange ... */
  memcpyw((unsigned short *)(video_mem_base + offset), 
          (unsigned short *) origin, video_screen_size); 
  video_mem_start = video_mem_base;
  video_mem_end = video_mem_term;
  origin = video_mem_base + offset;
  scr_end = origin + video_screen_size;
  pos = origin + y*video_size_row + (x<<1);
  has_wrapped = 0;
}

int
set_get_font(char * arg, int set, int ch512)
{
	return -EINVAL;
}

/*
 * Adjust the screen to fit a font of a certain height
 *
 * Returns < 0 for error, 0 if nothing changed, and the number
 * of lines on the adjusted console if changed.
 */
int
con_adjust_height(unsigned long fontheight)
{
	int rows, maxscan;
	unsigned char ovr, vde, fsr, curs, cure;

        return -EINVAL;
}

int
set_get_cmap(unsigned char * arg, int set) {
	return -EINVAL;
}

volatile LEON3_VGA_Regs_Map *leon_apbvga = 0;

con_type_init(unsigned long kmem_start, const char **display_desc)
{
  int  i,j; 
  amba_apb_device dev[1];
  video_type = VIDEO_TYPE_LEON;
  *display_desc = "*Leon";
  video_mem_base = kmem_start;
  video_mem_term = kmem_start + ((LEON_VGAMEM_SIZE) * sizeof (short));

  printk("Attaching textvga drivers [0x%x-0x%x]: ",video_mem_base,video_mem_term);
  j = amba_get_free_apbslv_devices (VENDOR_GAISLER, GAISLER_VGA, dev, 1);
  if (j >= 1) {
    leon_apbvga = (LEON3_VGA_Regs_Map *)dev[0].start;
    printk("0x%x\n",leon_apbvga);
  } else {
    printk("##! error GAISLER_VGA(0x%x) not found\n",GAISLER_VGA);
  }
  return video_mem_term;
}





void vesa_blank(void)
{
}	

void vesa_unblank(void)
{
}

void set_vesa_blanking(const unsigned long arg)
{
}

void vesa_powerdown(void)
{
}
