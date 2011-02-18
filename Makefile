ifneq ($(KERNELRELEASE),)
# call from kernel build system

obj-$(CONFIG_CEC)+= cec-dev.o

cec-dev-objs := core/cec_core.o core/cm_buff.o

ifeq ($(CONFIG_CEC_CHARDEV),y)
cec-dev-objs += core/cec_dev.o
endif

ifeq ($(CONFIG_MARS_CEC),y)
cec-dev-objs += adapter/cec_mars.o
endif

else

KERNELDIR ?= ../linux-2.6.12
PWD       := $(shell pwd)

# the CONFIG_### macros in the source are only defined according to the kernel's config.h
# Especially, they are _NOT_ overridden from the CONFIG_###=y on the commandline
# Therefore, something else but CONFIG_### must be used when buidling standalone
# NB: If you override MODFLAGS, don't forget to have at least -DMODULE defined
MODFLAGS  += -DMODULE -mlong-calls -DWITH_CEC_CHARDEV -DWITH_MARS_CEC

default:
	$(MAKE) MODFLAGS='$(MODFLAGS)' CONFIG_CEC=m CONFIG_CEC_CHARDEV=y CONFIG_MARS_CEC=y -C $(KERNELDIR) M=$(PWD)
#	$(MAKE) CONFIG_CEC=m CONFIG_CEC_CHARDEV=y CONFIG_MARS_CEC=y -C $(KERNELDIR) M=$(PWD)

debug:
	$(MAKE) MODFLAGS='$(MODFLAGS) -DWITH_CEC_DEBUG' CONFIG_CEC=m CONFIG_CEC_CHARDEV=y CONFIG_MARS_CEC=y -C $(KERNELDIR) M=$(PWD)

endif

DIRS = . core adapter
clean:
	$(MAKE) CONFIG_CEC=m CONFIG_CEC_CHARDEV=y CONFIG_MARS_CEC=y -C $(KERNELDIR) M=$(PWD) clean

depend .depend dep:
	$(CC) $(CFLAGS) -M *.c > .depend


ifeq (.depend,$(wildcard .depend))
include .depend
endif

