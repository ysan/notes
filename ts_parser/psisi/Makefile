#
#   Defines
#
BASEDIR		:=	../

CFLAGS		:=	-std=c++11

INCLUDES	:= \
	-I./ \
	-I$(BASEDIR) \
	-I$(BASEDIR)/section_parser/ \
	-I$(BASEDIR)/descriptor/ \

LIBS		:= \
	-L$(BASEDIR)/section_parser -lsection_parser \
	-L$(BASEDIR)/aribstr -laribstr \
	-L$(BASEDIR)/descriptor -ldescriptor \


APPEND_OBJS := \

SUBDIRS		:= \

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

