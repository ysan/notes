#
#   Defines
#
KERN_VER			=	$(shell uname -r)
KERN_TREE_DIR		=	/lib/modules/$(KERN_VER)/build
DEVICE_FILE_NAME	=	readwrite
MAJOR_NUM			=	1001

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
	-sudo rm $(DEVICE_FILE_NAME)
	-sudo insmod $(KERN_MODULE)
	-sudo mknod $(DEVICE_FILE_NAME) c $(MAJOR_NUM) 0
	-sudo chmod 766 $(DEVICE_FILE_NAME)

.PHONY: clean
clean:
	-sudo rmmod $(KERN_MODULE)
	-sudo rm $(DEVICE_FILE_NAME)
	make -C $(KERN_TREE_DIR) M=$(shell pwd) clean
