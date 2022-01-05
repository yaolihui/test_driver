ifeq ($(KERNELRELEASE),)

KDIR := /lib/modules/$(shell uname -r)/build

all:
	rm -f *.ko
	make -C $(KDIR) M=$(PWD) modules
	rm -f .*.cmd *.o *.mod.o  *.mod.c *.symvers *.mod *.order
else

sysfs_create_file-objs := sysfs_create_file.o char_dev.o

obj-m := sysfs_create_file.o device_create_file.o device_add_group.o device_add_groups.o

endif
