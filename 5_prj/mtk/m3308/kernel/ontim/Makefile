#include $(srctree)/drivers/misc/mediatek/Makefile.custom
ccflags-y += -I$(srctree)/drivers/misc/mediatek/include/mt-plat/
ccflags-y += -I$(srctree)/drivers/misc/mediatek/include/mt-plat/$(MTK_PLATFORM)/include/
ccflags-y += -I$(srctree)/drivers/misc/mediatek

ifeq ($(MTK_PROJECT),Romanee)
KBUILD_CFLAGS += -DCAM_ERR_STATUS
$(info KBUILD_CFLAGS DCAM_ERR_STATUS)
endif

$(warning -felix-----)
$(warning -felix-----)
$(warning -felix-----)
$(warning -felix-----)
obj-$(CONFIG_ODM_BOOTINFO) += ontim_debug_core.o
obj-$(CONFIG_ODM_BOOTINFO) += bootinfo.o
