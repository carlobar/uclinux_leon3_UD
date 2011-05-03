#
#	Makefile -- Build instructions for user mibench level apps
#

.EXPORT_ALL_VARIABLES:

dir_y =
dir_n =
dir_ = 


# automotive
dir_$(CONFIG_USER_MIBENCH_AUTO_BASICMATH)        += automotive/basicmath
dir_$(CONFIG_USER_MIBENCH_AUTO_BITCOUNT)         += automotive/bitcount
dir_$(CONFIG_USER_MIBENCH_AUTO_QSORT)            += automotive/qsort
dir_$(CONFIG_USER_MIBENCH_AUTO_SUSAN)            += automotive/susan
dir_$(CONFIG_USER_MIBENCH_RECURSIVE_FIBO)            += recursive/fibo

# telecomunications
dir_$(CONFIG_USER_MIBENCH_TELE_FFT)            += telecomm/FFT


all:
	for i in $(dir_y) dummy ; do \
		[ $$i = dummy ] || make -C $$i || exit $? ; \
	done

romfs:
	for i in $(dir_y) dummy ; do \
		[ $$i = dummy ] || make -C $$i romfs || exit $? ; \
	done

clean:
	-for i in $(dir_y) $(dir_n) $(dir_) ; do \
		[ ! -d $$i ] || make -C $$i clean; \
	done

