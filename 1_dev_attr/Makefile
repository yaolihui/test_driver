ifeq ($(KERNELRELEASE),)

KDIR := /lib/modules/$(shell uname -r)/build
KDIR := ~/linux-4.19.211

all:
	rm -f *.ko
	make -C $(KDIR) M=$(PWD) modules
	rm -f .*.cmd *.o *.mod.o  *.mod.c *.symvers *.mod *.order
	ls *.ko

install:
	sudo insmod sysfs_create.ko
	sudo insmod device_create.ko
	sudo insmod add_group.ko
	sudo insmod add_groups.ko

uninstall:
	sudo rmmod sysfs_create device_create add_group add_groups
	
else

sysfs_create-objs := 	sysfs_create_file.o 	char_dev.o
device_create-objs := 	device_create_file.o 	char_dev.o
add_group-objs := 		device_add_group.o		char_dev.o
add_groups-objs := 		device_add_groups.o		char_dev.o 

obj-m := sysfs_create.o device_create.o add_group.o add_groups.o

endif
