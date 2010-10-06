#include <linux/config.h>
#include <linux/ptrace.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/bios32.h>
#include <linux/pci.h>
#include <linux/string.h>

#include <asm/leon3.h>

//#define DEBUG_CONFIG

/* Structure containing address to devices found on the Amba Plug&Play bus */
extern amba_confarea_type amba_conf;
 
int amba_get_free_apbslv_devices (int vendor, int device, amba_apb_device *dev,int nr) {
  unsigned int i,conf,iobar,j = 0;
#ifdef DEBUG_CONFIG 
    printk("Amba: search for apdslv devices\n");
#endif
  for (i = 0; i < amba_conf.apbslv.devnr && j < nr; i++) {
    conf = amba_get_confword(amba_conf.apbslv, i, 0);
#ifdef DEBUG_CONFIG
    printk("Amba: check(%x:%x)==(%x:%x)\n",vendor,device,amba_vendor(conf),amba_device(conf));
#endif
    if ((amba_vendor(conf) == vendor) && (amba_device(conf) == device)) {
      if (!(amba_conf.apbslv.allocbits[i / 32] & (1 << (i & (32-1))))) {
#ifdef DEBUG_CONFIG
        printk("Amba: alloc device idx %i (%x:%x)\n",j,vendor,device);
#endif
        amba_conf.apbslv.allocbits[i / 32] |= (1 << (i & (32-1)));
        dev[j].irq = amba_irq(conf);
        iobar = amba_apb_get_membar(amba_conf.apbslv, i);
        dev[j].start = amba_iobar_start(amba_conf.apbmst, iobar);
        j++;
      }
    }
  }
  return j;
}


//collect ahb slaves
int amba_get_free_ahbslv_devices(int vendor, int device, amba_ahb_device * dev,
				 int nr)
{
	unsigned int addr, i, conf, iobar, j = 0, k;
#ifdef DEBUG_CONFIG
	printk("Ahbslv: search for ahdslv devices\n");
#endif
	for (i = 0; i < amba_conf.ahbslv.devnr && j < nr; i++) {
		conf = amba_get_confword(amba_conf.ahbslv, i, 0);
#ifdef DEBUG_CONFIG
		printk("Ahbslv: check(%x:%x)==(%x:%x)\n", vendor, device,
		       amba_vendor(conf), amba_device(conf));
#endif
		if ((amba_vendor(conf) == vendor)
		    && (amba_device(conf) == device)) {
			if (!
			    (amba_conf.ahbslv.
			     allocbits[i / 32] & (1 << (i & (32 - 1))))) {
#ifdef DEBUG_CONFIG
				printk("Ahbslv: alloc device idx %i (%x:%x)\n",
				       j, vendor, device);
#endif
				amba_conf.ahbslv.allocbits[i / 32] |=
				    (1 << (i & (32 - 1)));
				dev[j].irq = amba_irq(conf);
				for (k = 0; k < 4; k++) {
					iobar =
					    amba_ahb_get_membar(amba_conf.
								ahbslv, i, k);
					addr = amba_membar_start(iobar);
					if (amba_membar_type(iobar) ==
					    AMBA_TYPE_AHBIO) {
						addr =
						    AMBA_TYPE_AHBIO_ADDR(addr);
					}
					dev[j].start[k] = addr;
#ifdef DEBUG_CONFIG
					printk(" +%i: 0x%x \n", k,
					       dev[j].start[k]);
#endif
				}
				j++;
			}
		}
	}
	return j;
}
