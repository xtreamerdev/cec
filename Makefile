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

default:
	$(MAKE) CONFIG_CEC=m CONFIG_CEC_CHARDEV=y CONFIG_MARS_CEC=y -C $(KERNELDIR) SUBDIRS=$(PWD) modules

debug:
	$(MAKE) CONFIG_CEC=m CONFIG_CEC_CHARDEV=y CONFIG_MARS_CEC=y CONFIG_CEC_DEBUG=y -C $(KERNELDIR) SUBDIRS=$(PWD) modules

endif

DIRS = . core adapter
clean:
	$(foreach dir,$(DIRS),cd $(PWD)/$(dir); rm -rf *.o *.ko *~ .depend *.mod.c .*.cmd .tmp_versions .*.o.d;)

depend .depend dep:
	$(CC) $(CFLAGS) -M *.c > .depend


ifeq (.depend,$(wildcard .depend))
include .depend
endif

