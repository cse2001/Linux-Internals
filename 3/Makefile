
KDIR = /lib/modules/$(shell uname -r)/build
MODULE_NAME_1 :=hello_procfs
MODULE_NAME_2 :=get_pgfaults

# kbuild part of makefile
obj-m += $(MODULE_NAME_1).o $(MODULE_NAME_2).o

all: $(MODULE_NAME_1).ko $(MODULE_NAME_2).ko


$(MODULE_NAME_1).ko: $(MODULE_NAME_1).c
	make -C $(KDIR) M=$(PWD) modules	

$(MODULE_NAME_2).ko: $(MODULE_NAME_2).c
	make -C $(KDIR) M=$(PWD) modules


clean:
	make -C $(KDIR) M=$(PWD) clean
	rm -rf .tmp*


load:
	-make unload
	sudo dmesg -C
	sudo insmod $(MODULE_NAME_1).ko
	sudo insmod $(MODULE_NAME_2).ko

unload:
	sudo rmmod $(MODULE_NAME_1)
	sudo rmmod $(MODULE_NAME_2)

see:
	sudo dmesg