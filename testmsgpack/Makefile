#
#   Defines
#
BASEDIR		:=	./

INCLUDES	:= \
	-I./msgpack-3.3.0/include \
	-I./msgpack-3.3.0/include/msgpack \

LIBS		:= -L$(BASEDIR)/msgpack-3.3.0 -lmsgpack

LDFLAGS		:=

APPEND_OBJS	:=

SUBDIRS		:= msgpack-3.3.0

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

