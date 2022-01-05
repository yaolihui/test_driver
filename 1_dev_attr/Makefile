ifeq ($(KERNELRELEASE),)

KDIR := /lib/modules/$(shell uname -r)/build
KDIR := ~/linux-4.19.211

all:
	rm -f *.ko
	make -C $(KDIR) M=$(PWD) modules
	rm -f .*.cmd *.o *.mod.o  *.mod.c *.symvers *.mod *.order
	ls -l *.ko
else

sysfs_create-objs := sysfs_create_file.o char_dev.o

obj-m := sysfs_create.o device_create_file.o device_add_group.o device_add_groups.o

endif
