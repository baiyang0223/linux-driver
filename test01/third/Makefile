#Makefile:
#make ARCH=arm CROSS_COMPILE=arm-none-linux-gnueabi-
# If KERNELRELEASE is defined, we've been invoked from the
# kernel build system and can use its language.
ifneq ($(KERNELRELEASE),)
	#这部分第二次执行
	obj-m += hello.o
	# Otherwise we were called directly from the command
	# line; invoke the kernel build system.
else
	#这部分第一次执行
	KERNELDIR ?= /lib/modules/$(shell uname -r)/build
	PWD := $(shell pwd)
default:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules_install INSTALL_MOD_PATH=${PWD}
endif
 
clean:
	rm -rf *.ko .*.ko.cmd  *.mod.c  *.mod.o .*.mod.o.cmd *.o .*.o.cmd modules.order  Module.symvers .tmp_versions lib
