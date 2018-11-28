#
#   Defines
#
BASEDIR		:=	../

CFLAGS		:=	-std=c++11

INCLUDES	:= \
	-I./ \
	-I$(BASEDIR) \
	-I$(BASEDIR)/psisi/ \
	-I$(BASEDIR)/psisi/descriptor/ \

LIBS		:= \
	-L$(BASEDIR)/aribstr -laribstr

APPEND_OBJS := \
	./descriptor/descriptor.o \

SUBDIRS		:= \
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
	NetworkInformationTable.cpp \
	ServiceDescriptionTable.cpp \
	RunningStatusTable.cpp \


#
#   Configurations
#
include $(BASEDIR)/Config.mak

