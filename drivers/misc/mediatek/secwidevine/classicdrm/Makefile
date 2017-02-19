
ifneq ($(MTK_ROOT_BUILD),)
$(info "-- MTK_ROOT_BUILD secwidevine")
include $(MTK_ROOT_BUILD)/Makefile
endif

ifeq ($(CONFIG_TRUSTONIC_TEE_SUPPORT),y)
$(info "-- Build widevine driver")
$(info "MTK_PATH_PLATFORM=$(MTK_PATH_PLATFORM)")

ccflags-y += -I$(srctree)/drivers/misc/mediatek/mach/$(MTK_PLATFORM)/include/trustzone/utils/tlutils
include $(srctree)/drivers/misc/mediatek/gud/Makefile.include
obj-y += secwidevine.o

endif
