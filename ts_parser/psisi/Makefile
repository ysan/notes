#
#   Defines
#
BASEDIR		:=	../

CFLAGS		:=	-std=c++11

INCLUDES	:= \
	-I./ \
	-I../ \
	-I./descriptor/ \
	-I/usr/include/PCSC/ \

LIBS		:= \
	-lpcsclite

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
	descriptor/Descriptor.cpp \


#
#   Configurations
#
include $(BASEDIR)/Config.mak

