#
# Automatically generated make config: don't edit
#
CONFIG_UCLINUX=y

#
# Code maturity level options
#
CONFIG_EXPERIMENTAL=y

#
# Loadable module support
#
# CONFIG_MODULES is not set

#
# Platform dependant setup
#
CONFIG_LEON_2=y
# CONFIG_LEON_3 is not set

#
# Platform
#
CONFIG_LEON4TSIM=y
# CONFIG_RAMKERNEL is not set
CONFIG_ROMKERNEL=y

#
# General setup
#
# CONFIG_PCI is not set
# CONFIG_USE_ARCHMEMCPY is not set
CONFIG_NET=y
# CONFIG_SYSVIPC is not set
CONFIG_REDUCED_MEMORY=y
CONFIG_BINFMT_FLAT=y
# CONFIG_BINFMT_ZFLAT is not set
# CONFIG_CONSOLE is not set
# CONFIG_FRAMEBUFFER is not set

#
# Floppy, IDE, and other block devices
#
CONFIG_BLK_DEV_BLKMEM=y
# CONFIG_BLK_DEV_IDE is not set

#
# Additional Block/FLASH Devices
#
# CONFIG_BLK_DEV_LOOP is not set
# CONFIG_BLK_DEV_MD is not set
# CONFIG_BLK_DEV_RAM is not set
# CONFIG_DEV_FLASH is not set
# CONFIG_BLK_DEV_NFA is not set

#
# Networking options
#
# CONFIG_FIREWALL is not set
# CONFIG_NET_ALIAS is not set
CONFIG_INET=y
# CONFIG_IP_FORWARD is not set
# CONFIG_IP_MULTICAST is not set
# CONFIG_SYN_COOKIES is not set
# CONFIG_IP_ACCT is not set
# CONFIG_IP_ROUTER is not set
# CONFIG_NET_IPIP is not set

#
# (it is safe to leave these untouched)
#
# CONFIG_INET_PCTCP is not set
# CONFIG_INET_RARP is not set
# CONFIG_NO_PATH_MTU_DISCOVERY is not set
# CONFIG_IP_NOSR is not set
# CONFIG_SKB_LARGE is not set

#
#  
#
# CONFIG_IPX is not set
# CONFIG_ATALK is not set
# CONFIG_AX25 is not set
# CONFIG_BRIDGE is not set
# CONFIG_NETLINK is not set
# CONFIG_IPSEC is not set

#
# Network device support
#
CONFIG_NETDEVICES=y
# CONFIG_DUMMY is not set
# CONFIG_SLIP is not set
# CONFIG_PPP is not set
# CONFIG_EQUALIZER is not set
# CONFIG_UCCS8900 is not set
# CONFIG_SMC9194 is not set
# CONFIG_SMC91111 is not set
# CONFIG_NE2000 is not set
# CONFIG_FEC is not set
# CONFIG_OPEN_ETH is not set
# CONFIG_GRLIB_GAISLER_GRETH is not set

#
# Filesystems
#
# CONFIG_QUOTA is not set
# CONFIG_MINIX_FS is not set
# CONFIG_EXT_FS is not set
# CONFIG_EXT2_FS is not set
# CONFIG_XIA_FS is not set
# CONFIG_NLS is not set
CONFIG_PROC_FS=y
CONFIG_NFS_FS=y
# CONFIG_ROOT_NFS is not set
# CONFIG_SMB_FS is not set
# CONFIG_HPFS_FS is not set
# CONFIG_SYSV_FS is not set
# CONFIG_AUTOFS_FS is not set
# CONFIG_AFFS_FS is not set
CONFIG_ROMFS_FS=y
# CONFIG_JFFS_FS is not set
# CONFIG_UFS_FS is not set

#
# Character devices
#
CONFIG_LEON_SERIAL=y

#
# Kernel hacking
#
# CONFIG_FULLDEBUG is not set
# CONFIG_ALLOC2 is not set
# CONFIG_PROFILE is not set

#
# Controller Area Network Cards/Chips
#
# CONFIG_CAN4LINUX is not set
