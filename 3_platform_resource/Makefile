ifeq ($(KERNELRELEASE),)

KDIR := /lib/modules/$(shell uname -r)/build
KDIR := /usr/src/$(shell uname -r)

all:
	rm -f *.ko
	make -C $(KDIR) M=$(PWD) modules
	rm -rf .*.cmd *.o *.mod.o  *.mod.c *.symvers *.mod *.order .*.o.d .tmp_versions
	ls *.ko

install: uninstall
	sudo insmod platform_device_set_res.ko
	sudo insmod platform_driver_get_res.ko
	lsmod

uninstall:
	-sudo rmmod platform_device_set_res platform_driver_get_res
	lsmod
else

ccflags-y := -std=gnu99 -Wno-declaration-after-statement
obj-m := platform_device_set_res.o platform_driver_get_res.o

endif
