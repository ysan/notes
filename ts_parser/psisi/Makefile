#
#   Defines
#
BASEDIR		:=	../

CFLAGS		:=	-std=c++11

INCLUDES	:= \
	-I./ \
	-I../ \
	-I./descriptor/ \

LIBS		:= \
	-L./aribstr -laribstr

APPEND_OBJS := \
	./descriptor/descriptor.o \

SUBDIRS		:= \
	aribstr \
	descriptor \

#
#   Target object
#
TARGET_NAME	:=	psisi

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
	SectionParser.cpp \
	ProgramAssociationTable.cpp \
	ProgramMapTable.cpp \
	TimeOffsetTable.cpp \
	EventInformationTable.cpp \


#
#   Configurations
#
include $(BASEDIR)/Config.mak

