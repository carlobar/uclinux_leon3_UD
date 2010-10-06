#include <linux/config.h>
#include <linux/ptrace.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/bios32.h>
#include <linux/pci.h>
#include <linux/string.h>

#include <asm/leon3.h>

#define DEBUG_CONFIG

/* Structure containing address to devices found on the Amba Plug&Play bus */
amba_confarea_type amba_conf;

/* Pointers to Interrupt Controller configuration registers */
volatile LEON3_IrqCtrl_Regs_Map *LEON3_IrqCtrl_Regs = 0;
volatile LEON3_GpTimer_Regs_Map *LEON3_GpTimer_Regs = 0;
unsigned long LEON3_GpTimer_Irq = 0;

static void vendor_dev_string(unsigned long conf, char *vendorbuf,char *devbuf) {
  int vendor = amba_vendor(conf); int dev = amba_device(conf);
  char *devstr; char *vendorstr; 
  sprintf(vendorbuf, "Unknown vendor %2x",vendor);
  sprintf(devbuf, "Unknown device %2x",dev);
  vendorstr = vendor_id2str(vendor);
  if (vendorstr) {
    sprintf(vendorbuf, "%s",vendorstr);
  } 
  devstr = device_id2str(vendor,dev);
  if (devstr) {
    sprintf(devbuf, "%s",devstr);
  }
}

void amba_prinf_config()  
{
  char devbuf[128]; char vendorbuf[128]; unsigned int conf;
  int i = 0; int j = 0; char *devstr; char *vendorstr; int vendor; int dev;
  unsigned int addr; unsigned int m;
  printk("             Vendors         Slaves\n");
  printk("Ahb masters:\n");
  i = 0;
  while (i < amba_conf.ahbmst.devnr) 
  {
    conf = amba_get_confword(amba_conf.ahbmst, i, 0);
    vendor_dev_string(conf,vendorbuf,devbuf);
    printk("%2i(%2x:%3x|%2i): %16s %16s \n", i, amba_vendor(conf), amba_device(conf), amba_irq(conf), vendorbuf, devbuf);
    for (j = 0;j < 4;j++) {
      m = amba_ahb_get_membar(amba_conf.ahbmst,i,j);
      if (m) {
        addr = amba_membar_start(m);
        printk(" +%i: 0x%x \n", j, addr);
      }
    }
    i++;
  }
  printk("Ahb slaves:\n");
  i = 0;
  while (i < amba_conf.ahbslv.devnr) 
  {
    conf = amba_get_confword(amba_conf.ahbslv, i, 0);
    vendor_dev_string(conf,vendorbuf,devbuf);
    printk("%2i(%2x:%3x|%2i): %16s %16s \n", i, amba_vendor(conf), amba_device(conf), amba_irq(conf), vendorbuf, devbuf);
    for (j = 0;j < 4;j++) {
      m = amba_ahb_get_membar(amba_conf.ahbslv,i,j);
      if (m) {
        addr = amba_membar_start(m);
        printk(" +%i: 0x%x \n", j, addr);
      }
    }
    i++;
  }
  printk("Apb slaves:\n");
  i = 0;
  while (i < amba_conf.apbslv.devnr) 
  {
    
    conf = amba_get_confword(amba_conf.apbslv, i, 0);
    vendor_dev_string(conf,vendorbuf,devbuf);
    printk("%2i(%2x:%3x|%2i): %16s %16s \n", i, amba_vendor(conf), amba_device(conf), amba_irq(conf), vendorbuf, devbuf);
    
    m = amba_apb_get_membar(amba_conf.apbslv, i);
    addr = amba_iobar_start(amba_conf.apbmst, m);
    printk(" +%2i: 0x%x 0x%x \n", 0, addr, m);
    
    i++;
    
  }
  
}

#define amba_insert_device(tab, address) \
{ \
  if (*(address)) \
  { \
    (tab)->addr[(tab)->devnr] = (address); \
    (tab)->devnr ++; \
  } \
} while(0)

/*
 *  Used to scan system bus. Probes for AHB masters, AHB slaves and 
 *  APB slaves. Addresses to configuration areas of the AHB masters,
 *  AHB slaves, APB slaves and APB master are storeds in 
 *  amba_ahb_masters, amba_ahb_slaves and amba.
 */

void amba_init() 
{
  unsigned int *cfg_area;  /* address to configuration area */
  unsigned int mbar, iobar, conf;
  int i, j;
  
#ifdef DEBUG_CONFIG
  printk("Reading AMBA Plug&Play configuration area\n");
#endif

  memset(&amba_conf,0,sizeof(amba_conf));
  //amba_conf.ahbmst.devnr = 0; amba_conf.ahbslv.devnr = 0; amba_conf.apbslv.devnr = 0;
  
  cfg_area = (unsigned int *) (LEON3_IO_AREA | LEON3_CONF_AREA);

  for (i = 0; i < LEON3_AHB_MASTERS; i++) 
  {
    amba_insert_device(&amba_conf.ahbmst, cfg_area);
    cfg_area += LEON3_AHB_CONF_WORDS;
  }

  cfg_area = (unsigned int *) (LEON3_IO_AREA | LEON3_CONF_AREA | LEON3_AHB_SLAVE_CONF_AREA);
  for (i = 0; i < LEON3_AHB_SLAVES; i++) 
  {
    amba_insert_device(&amba_conf.ahbslv, cfg_area);
    cfg_area += LEON3_AHB_CONF_WORDS;
  }  

  for (i = 0; i < amba_conf.ahbslv.devnr; i ++) 
  {
    conf = amba_get_confword(amba_conf.ahbslv, i, 0);
    mbar = amba_ahb_get_membar(amba_conf.ahbslv, i, 0);
    if ((amba_vendor(conf) == VENDOR_GAISLER) && (amba_device(conf) == GAISLER_APBMST))
    {
      amba_conf.apbmst = amba_membar_start(mbar);
      cfg_area = (unsigned int *) (amba_conf.apbmst | LEON3_CONF_AREA);
      
      printk("Found apbmst, cfg: 0x%x\n",cfg_area);
      
      for (j = amba_conf.apbslv.devnr; j < LEON3_APB_SLAVES; j++)
      {
	amba_insert_device(&amba_conf.apbslv, cfg_area);
	cfg_area += LEON3_APB_CONF_WORDS;
      }
    }
  }    
 
  /* Find LEON3 Interrupt controler */
  LEON3_IrqCtrl_Regs = (volatile LEON3_IrqCtrl_Regs_Map *) amba_find_apbslv_addr(VENDOR_GAISLER, GAISLER_IRQMP, 0);
  LEON3_GpTimer_Regs = (volatile LEON3_GpTimer_Regs_Map *) amba_find_apbslv_addr(VENDOR_GAISLER, GAISLER_GPTIMER, &LEON3_GpTimer_Irq);
  
}

unsigned long amba_find_apbslv_addr(unsigned long vendor, unsigned long device, unsigned long *irq) {
  unsigned int i,conf,iobar;
  for (i = 0; i < amba_conf.apbslv.devnr; i++) {
    conf = amba_get_confword(amba_conf.apbslv, i, 0);
    if ((amba_vendor(conf) == vendor) && (amba_device(conf) == device))
    {
      if (irq) {
        *irq = amba_irq(conf);
      }
      iobar = amba_apb_get_membar(amba_conf.apbslv, i);
      return amba_iobar_start(amba_conf.apbmst, iobar);
    }
  }
  return 0;
}
