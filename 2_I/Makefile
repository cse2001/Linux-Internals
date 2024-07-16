KDIR = /lib/modules/$(shell uname -r)/build
MODULE_NAME := ioctl_mem
USER_APP := test

# kbuild part of makefile
obj-m += $(MODULE_NAME).o

all: $(USER_APP) $(MODULE_NAME).ko

$(USER_APP): $(USER_APP).c $(MODULE_NAME).h
	gcc $^ -o $@

$(MODULE_NAME).ko: $(MODULE_NAME).c $(MODULE_NAME).h
	make -C $(KDIR) M=$(PWD) modules

clean:
	rm -rf $(USER_APP) *.o .t*
	rm -rf .tmp*
	make -C $(KDIR) M=$(PWD) clean

load:
	- sudo rmmod $(MODULE_NAME)
	sudo dmesg -C
	sudo insmod $(MODULE_NAME).ko

unload:
	sudo rmmod $(MODULE_NAME)

see:
	sudo dmesg