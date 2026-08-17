ifeq ($(KERNELRELEASE),)

KDIR := /lib/modules/$(shell uname -r)/build
#KDIR := /usr/src/$(shell uname -r)

all:
	rm -f *.ko
	make -C $(KDIR) M=$(PWD) modules
	rm -rf .*.cmd *.o *.mod.o  *.mod.c *.symvers *.mod *.order .*.o.d .tmp_versions
	ls -l *.ko

install: uninstall
	sudo insmod platform_device_set_res.ko
	sudo insmod platform_driver_get_res.ko
	sudo insmod platform_device_add.ko
	sudo insmod platform_driver_add.ko
	sudo insmod platform_add_devices.ko
	sudo insmod platform_add_driver.ko
	sudo insmod platform_add_groups.ko
	lsmod
	ls -l /sys/devices/platform

uninstall:
	-sudo rmmod platform_device_set_res platform_driver_get_res \
				platform_device_add platform_driver_add \
				platform_add_devices platform_add_driver \
				platform_add_groups
	lsmod
	ls -l /sys/devices/platform
else

ccflags-y := -std=gnu99 -Wno-declaration-after-statement

obj-m := platform_device_set_res.o platform_driver_get_res.o \
		platform_device_add.o platform_driver_add.o	\
		platform_add_devices.o platform_add_driver.o \
		platform_add_groups.o

endif
