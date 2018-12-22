#
#   Defines
#
BASEDIR		:=	../

CFLAGS		:=	-std=c++11

INCLUDES	:= \
	-I./ \
	-I./descriptor \
	-I$(BASEDIR) \
	-I$(BASEDIR)/section_parser/ \

LIBS		:= \
	-L$(BASEDIR)/section_parser -lsection_parser \
	-L$(BASEDIR)/aribstr -laribstr \


APPEND_OBJS := ./descriptor/descriptor.o

SUBDIRS		:= ./descriptor

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
	ProgramAssociationTable.cpp \
	ProgramMapTable.cpp \
	TimeOffsetTable.cpp \
	EventInformationTable.cpp \
	NetworkInformationTable.cpp \
	ServiceDescriptionTable.cpp \
	RunningStatusTable.cpp \
	BroadcasterInformationTable.cpp \


#
#   Configurations
#
include $(BASEDIR)/Config.mak

