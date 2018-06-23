#
#   Defines
#
KERN_VER			=	$(shell uname -r)
KERN_TREE_DIR		=	/lib/modules/$(KERN_VER)/build
DEVICE_FILE_NAME	=	proc_test

#
#   Objects
#
obj-m				=	$(DEVICE_FILE_NAME).o
KERN_MODULE			=	$(DEVICE_FILE_NAME).ko


#
#   Conditions
#
.PHONY: all
all:
	make -C $(KERN_TREE_DIR) M=$(shell pwd) modules

.PHONY: install
install: all
	-sudo rmmod $(KERN_MODULE)
	-sudo insmod $(KERN_MODULE)

.PHONY: clean
clean:
	-sudo rmmod $(KERN_MODULE)
	make -C $(KERN_TREE_DIR) M=$(shell pwd) clean
