#
#   Defines
#
BASEDIR		:=	./

INCLUDES	:= \
	-I./msgpack-c/include \
	-I./msgpack-c/include/msgpack \

LIBS		:= -L$(BASEDIR)/msgpack-c -lmsgpack

LDFLAGS		:=

APPEND_OBJS	:=

SUBDIRS		:= msgpack-c

USERDEFS	:= -std=c++11


#
#   Target object
#
TARGET_NAME	:=	testmsgpack

#
#   Target type
#     (EXEC/SHARED/STATIC/OBJECT)
#
TARGET_TYPE	:=	EXEC

#
#   Compile sources
#
SRCS		:=

SRCS_CPP	:= test.cpp


#
#   Configurations
#
include $(BASEDIR)/Config.make

