
KDIR = /lib/modules/$(shell uname -r)/build
MODULE_NAME := ioctl_process

# kbuild part of makefile
obj-m += $(MODULE_NAME).o

TARGETS = control_station soldier

SOURCES = $(TARGETS:=.c)

all: $(TARGETS) $(MODULE_NAME).ko

$(TARGETS): $(SOURCES) $(MODULE_NAME).h
	$(CC) $(CFLAGS) -o $@ $@.c

$(MODULE_NAME).ko: $(MODULE_NAME).c $(MODULE_NAME).h
	make -C $(KDIR) M=$(PWD) modules	


clean:
	rm -f $(TARGETS)
	make -C $(KDIR) M=$(PWD) clean
	-make unload


load:
	@- sudo rmmod $(MODULE_NAME)
	@sudo dmesg -C
	sudo insmod $(MODULE_NAME).ko

unload:
	sudo rmmod $(MODULE_NAME)

see:
	sudo dmesg