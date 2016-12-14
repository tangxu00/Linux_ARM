ifeq ($(KERNELRELEASE),)

KERNELDIR ?=/home/dooon/ARM/linux-2.6.38

PWD :=$(shell pwd)

modules:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules

modules_install:
	$(MAKE) -C $(KERNELDIR)=$(PWD) module_install

clean:
	rm -rf *.o *~ core .depend .*.cmd *.ko *.mod.c .temp_versions

.PHONY: modules modules_install clean

else
	obj-m :=demo.o
endif
