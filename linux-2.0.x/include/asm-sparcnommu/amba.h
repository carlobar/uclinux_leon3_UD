#ifndef _LEON3_AMBA_H__
#define _LEON3_AMBA_H__

/*
 *  AMBA Plag & Play Bus Driver Macros
 *
 *  Macros used for AMBA Plug & Play bus scanning
 *
 *  COPYRIGHT (c) 2004.
 *  Gaisler Research
 *
 *  The license and distribution terms for this file may be
 *  found in the file LICENSE in this distribution or at
 *  http://www.rtems.com/license/LICENSE.
 *
 */


#define LEON3_IO_AREA 0xfff00000
#define LEON3_CONF_AREA 0xff000
#define LEON3_AHB_SLAVE_CONF_AREA (1 << 11)

#define LEON3_AHB_CONF_WORDS 8
#define LEON3_APB_CONF_WORDS 2
#define LEON3_AHB_MASTERS 8
#define LEON3_AHB_SLAVES 8
#define LEON3_APB_SLAVES 16
#define LEON3_APBUARTS 8

/* Vendor codes */ 
#define VENDOR_GAISLER   1
#define VENDOR_PENDER    2
#define VENDOR_ESA       4 
#define VENDOR_OPENCORES 8 

/* Gaisler Research device id's */
#define GAISLER_LEON3    0x003
#define GAISLER_LEON3DSU 0x004
#define GAISLER_ETHAHB   0x005
#define GAISLER_APBMST   0x006
#define GAISLER_AHBUART  0x007
#define GAISLER_SRCTRL   0x008
#define GAISLER_SDCTRL   0x009
#define GAISLER_APBUART  0x00C
#define GAISLER_IRQMP    0x00D
#define GAISLER_AHBRAM   0x00E
#define GAISLER_GPTIMER  0x011
#define GAISLER_PCITRG   0x012
#define GAISLER_PCISBRG  0x013
#define GAISLER_PCIFBRG  0x014
#define GAISLER_PCITRACE 0x015
#define GAISLER_PCIDMA   0x016
#define GAISLER_AHBTRACE 0x017
#define GAISLER_ETHDSU   0x018
#define GAISLER_PIOPORT  0x01A
#define GAISLER_AHBJTAG  0x01c
#define GAISLER_VGA      0x061 //0x01d
#define GAISLER_KBD      0X060
#define GAISLER_ETHMAC   0x01D

#define GAISLER_L2TIME   0xffd /* internal device: leon2 timer */
#define GAISLER_L2C      0xffe /* internal device: leon2compat */
#define GAISLER_PLUGPLAY 0xfff /* internal device: plug & play configarea */


#ifndef __ASSEMBLER__

extern inline char *gaisler_device_str(int id) {
  switch(id) {
  case GAISLER_LEON3:    return "GAISLER_LEON3";
  case GAISLER_LEON3DSU: return "GAISLER_LEON3DSU";
  case GAISLER_ETHAHB:   return "GAISLER_ETHAHB";
  case GAISLER_ETHMAC:   return "GAISLER_ETHMAC";
  case GAISLER_APBMST:   return "GAISLER_APBMST";
  case GAISLER_AHBUART:  return "GAISLER_AHBUART";
  case GAISLER_SRCTRL:   return "GAISLER_SRCTRL";  
  case GAISLER_SDCTRL:   return "GAISLER_SDCTRL"; 
  case GAISLER_APBUART:  return "GAISLER_APBUART"; 
  case GAISLER_IRQMP:    return "GAISLER_IRQMP"; 
  case GAISLER_AHBRAM:   return "GAISLER_AHBRAM"; 
  case GAISLER_GPTIMER:  return "GAISLER_GPTIMER";
  case GAISLER_PCITRG:   return "GAISLER_PCITRG"; 
  case GAISLER_PCISBRG:  return "GAISLER_PCISBRG"; 
  case GAISLER_PCIFBRG:  return "GAISLER_PCIFBRG"; 
  case GAISLER_PCITRACE: return "GAISLER_PCITRACE"; 
  case GAISLER_AHBTRACE: return "GAISLER_AHBTRACE";
  case GAISLER_ETHDSU:   return "GAISLER_ETHDSU";
  case GAISLER_PIOPORT:  return "GAISLER_PIOPORT";
  case GAISLER_AHBJTAG:  return "GAISLER_AHBJTAG";
  case GAISLER_VGA:      return "GAISLER_VGA";
  case GAISLER_KBD:      return "GAISLER_KBD";
    
  case GAISLER_L2TIME:   return "GAISLER_L2TIME";
  case GAISLER_L2C:      return "GAISLER_L2C";
  case GAISLER_PLUGPLAY: return "GAISLER_PLUGPLAY";
    
  default: break;
  }
  return 0;
}

#endif

/* European Space Agency device id's */
#define ESA_LEON2        0x2 
#define ESA_MCTRL        0xF 


#ifndef __ASSEMBLER__

extern inline char *esa_device_str(int id) {
  switch(id) {
  case ESA_LEON2:  return "ESA_LEON2";
  case ESA_MCTRL:  return "ESA_MCTRL";
  default: break;
  }
  return 0;
}

#endif

/* Opencores device id's */
#define OPENCORES_PCIBR  0x4  
#define OPENCORES_ETHMAC 0x5


#ifndef __ASSEMBLER__

extern inline char *opencores_device_str(int id) {
  switch(id) {
  case OPENCORES_PCIBR:  return "OPENCORES_PCIBR";
  case OPENCORES_ETHMAC:  return "OPENCORES_ETHMAC";
  default: break;
  }
  return 0;
}

extern inline char *device_id2str(int vendor, int id) {
  switch(vendor) {
  case VENDOR_GAISLER:    return gaisler_device_str(id);
  case VENDOR_ESA:        return esa_device_str(id);
  case VENDOR_OPENCORES:  return opencores_device_str(id);
  case VENDOR_PENDER:
  default: break;
  }
  return 0;
}

extern inline char *vendor_id2str(int vendor) {
  switch(vendor) {
  case VENDOR_GAISLER:    return "VENDOR_GAISLER";
  case VENDOR_ESA:        return "VENDOR_ESA";
  case VENDOR_OPENCORES:  return "VENDOR_OPENCORES";
  case VENDOR_PENDER:     return "VENDOR_PENDER";
  default: break;
  }
  return 0;
}

#endif



/* Vendor codes */ 



/* 
 *
 * Macros for manipulating Configuration registers  
 *
 */


#define amba_get_confword(tab, index, word) (*((tab).addr[(index)]+(word)))

#define amba_vendor(x) (((x) >> 24) & 0xff)

#define amba_device(x) (((x) >> 12) & 0xfff)

#define amba_ahb_get_membar(tab, index, nr) (*((tab).addr[(index)]+4+(nr)))

#define amba_apb_get_membar(tab, index) (*((tab).addr[(index)]+1))

#define amba_membar_start(mbar) (((mbar) & 0xfff00000) & (((mbar) & 0xfff0) << 16))

#define amba_iobar_start(base, iobar) ((base) | ((((iobar) & 0xfff00000)>>12) & (((iobar) & 0xfff0)<<4)) )

#define amba_irq(conf) ((conf) & 0xf)

#define amba_membar_type(mbar) ((mbar) & 0xf)

#define AMBA_TYPE_APBIO 0x1
#define AMBA_TYPE_MEM   0x2
#define AMBA_TYPE_AHBIO 0x3

#define AMBA_TYPE_AHBIO_ADDR(addr) (LEON3_IO_AREA | ((addr) >> 12))


#endif

