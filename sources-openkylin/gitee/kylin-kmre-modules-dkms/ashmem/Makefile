# SPDX-License-Identifier: GPL-2.0
ccflags-y += -I$(src) -Wno-error=implicit-int -Wno-int-conversion

obj-m := kmre-ashmem.o
kmre-ashmem-objs := ashmem.o

ifneq ($(KERNELRELEASE),)
KERNELDIR ?= /lib/modules/$(KERNELRELEASE)/build
else
KERNELDIR ?= /lib/modules/$(shell uname -r)/build
endif

PWD := $(shell pwd)

ifeq ($(CONFIG_RETHUNK),y)
ccflags-y += -mindirect-branch=thunk-extern -mfunction-return=thunk-extern
endif

.PHONY: all
all: clean modules #install

.PHONY: modules
modules:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules

.PHONY: clean
clean:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) clean

