ifeq ($(KERNELRELEASE),)

KDIR := /lib/modules/$(shell uname -r)/build
#KDIR := /usr/src/$(shell uname -r)

all:
	rm -f *.ko
	make -C $(KDIR) M=$(PWD) modules
	rm -f .*.cmd *.o *.mod.o  *.mod.c *.symvers *.mod *.order
	ls *.ko

install: uninstall
	sudo insmod sysfs_create.ko  && sudo chmod 666 /dev/1_device
	sudo insmod device_create.ko && sudo chmod 666 /dev/2_device
	sudo insmod add_group.ko	 && sudo chmod 666 /dev/3_device
	sudo insmod add_groups.ko	 && sudo chmod 666 /dev/4_device
	sudo insmod kobject_create_and_add.ko	 && sudo chmod -R 777 /sys/test_bootinfo
	lsmod

uninstall:
	-sudo rmmod sysfs_create device_create add_group add_groups kobject_create_and_add
	
else

#kobject_create_and_add-objs := 		kobject_create_and_add.o
sysfs_create-objs := 	sysfs_create_file.o 	char_dev.o
device_create-objs := 	device_create_file.o 	char_dev.o
add_group-objs := 		device_add_group.o		char_dev.o
add_groups-objs := 		device_add_groups.o		char_dev.o 

obj-m := sysfs_create.o device_create.o add_group.o add_groups.o kobject_create_and_add.o

endif
