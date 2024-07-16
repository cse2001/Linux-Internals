KDIR = /lib/modules/`uname -r`/build
MODULE_NAMES=lkm1 lkm2 lkm3 lkm4

# kbuild part of makefile
obj-m += $(addsuffix .o,$(MODULE_NAMES))

.PHONY: kbuild clean

kbuild:
	make -C $(KDIR) M=`pwd`

clean:
	make -C $(KDIR) M=`pwd` clean