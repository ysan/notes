#
#   Defines
#
BASEDIR		:=	./

CFLAGS		:=	-std=c++11

INCLUDES	:= \
	-I./ \
	-I$(BASEDIR)/section_parser/ \
	-I$(BASEDIR)/aribstr/ \
	-I$(BASEDIR)/psisi/ \
	-I$(BASEDIR)/psisi/descriptor \
	-I$(BASEDIR)/dsmcc/ \
	-I/usr/include/PCSC/ \

LIBS		:= \
	-lpthread \
	-lpcsclite \
	-L$(BASEDIR)/section_parser -lsection_parser \
	-L$(BASEDIR)/aribstr -laribstr \
	-L$(BASEDIR)/psisi -lpsisi \
	-L$(BASEDIR)/dsmcc -ldsmcc \

SUBDIRS		:= \
	section_parser \
	aribstr \
	psisi \
	dsmcc \

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

