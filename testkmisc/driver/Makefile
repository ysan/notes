obj-m			:= testkmisc.o
KERNELDIR		?= /lib/modules/$(shell uname -r)/build
#KERNELDIR		?= ${HOME}/prog/github_repos/linux
CFLAGS_$(obj-m)	:= -DDEBUG

all:
	$(MAKE) -C $(KERNELDIR) M=$(PWD)

clean:
	rm -rf *.o *~ core .depend .*.cmd *.ko *.mod *.order *.mod.c *.symvers .tmp_versions *.a

.PHONY: clean all
