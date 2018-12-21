#
#   Defines
#
BASEDIR		:=	../

CFLAGS		:=	-std=c++11

INCLUDES	:= \
	-I./ \
	-I$(BASEDIR) \
	-I$(BASEDIR)/section_parser/ \
	-I$(BASEDIR)/psisi/ \
	-I$(BASEDIR)/descriptor/ \

LIBS		:= \
	-L$(BASEDIR)/section_parser -lsection_parser \
	-L$(BASEDIR)/aribstr -laribstr \
	-L$(BASEDIR)/descriptor -ldescriptor \
	-L$(BASEDIR)/psisi -lpsisi \


APPEND_OBJS := \

SUBDIRS		:= \

#
#   Target object
#
TARGET_NAME	:=	dsmcc

#
#   Target type
#     (EXEC/SHARED/STATIC/OBJECT)
#
TARGET_TYPE	:=	SHARED

#
#   Compile sources
#
SRCS		:=

SRCS_CPP	:= \
	Dsmcc.cpp \


#
#   Configurations
#
include $(BASEDIR)/Config.mak

