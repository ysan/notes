#
#   Defines
#
BASEDIR		:=	../../

CFLAGS		:=	-DDEBUG -O2 -Wall -DSTD_B25
ifeq ($(shell getconf LONG_BIT),32)
CFLAGS		+=	-D_FILE_OFFSET_BITS=64
endif

INCLUDES	:= \
	-I./ \
	-I../ \
	-I../it9175 \

LIBS		:= \
	-L../it9175 -lit9175 \
	-lpthread \
	-larib25 \

APPEND_OBJS :=

SUBDIRS		:=

#
#   Target object
#
TARGET_NAME	:=	recfsusb2i

#
#   Target type
#     (EXEC/SHARED/STATIC/OBJECT)
#
TARGET_TYPE	:=	EXEC
NOT_INSTALL :=	YES

#
#   Compile sources
#
#SRCS		:=	main.c

SRCS_CPP	:=	main.cpp


#
#   Configurations
#
include $(BASEDIR)/Config.mak

