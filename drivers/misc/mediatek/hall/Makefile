include $(srctree)/drivers/misc/mediatek/Makefile.custom


obj-$(CONFIG_MTK_TINNO_HALL)	+= hall_device.o

WIKO_UNIFY_DEF := $(WIKO_TYPE)
ifeq ($(strip $(WIKO_UNIFY_DEF)),unify)
ccflags-y += -DCONFIG_WIKO_UNIFY
else
$(warning "No flag to WIKO_UNIFY_DEF")
endif
