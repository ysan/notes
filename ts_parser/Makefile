#
#   Defines
#
BASEDIR		:=	./

CFLAGS		:=	-std=c++11

INCLUDES	:= \
	-I./ \
	-I$(BASEDIR)/psisi/ \
	-I$(BASEDIR)/psisi/descriptor/ \
	-I/usr/include/PCSC/ \

LIBS		:= \
	-lpthread \
	-lpcsclite \
	-L$(BASEDIR)/psisi -lpsisi \
	-L$(BASEDIR)/aribstr -laribstr \

SUBDIRS		:= \
	aribstr \
	psisi \

#
#   Target object
#
TARGET_NAME	:=	ts_parser

#
#   Target type
#     (EXEC/SHARED/STATIC/OBJECT)
#
TARGET_TYPE	:=	EXEC

#
#   Compile sources
#
SRCS		:=

SRCS_CPP	:= \
	main.cpp \
	TsParser.cpp \
	Utils.cpp \
	TsCommon.cpp \


#
#   Configurations
#
include $(BASEDIR)/Config.mak

