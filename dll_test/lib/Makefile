#
#   Defines
#
TARGET		=	stub.so
INC_DIR		=	./
BIN_DIR		=	../bin

CC		=	/usr/bin/cc

MYCFLAGS	=
GDBFLAGS	=
CFLAGS		=	$(MYCFLAGS) $(GDBFLAGS) -I$(INC_DIR) -Wall -fPIC
LFLAGS		=	-shared



.SUFFIXES:  .c .o .h .so
.c.o:
	$(CC) $(CFLAGS) -c $*.c



#
#   Objects
#
SRCS		=	stub.c 

OBJS		=	$(SRCS:.c=.o)


#
#   Conditions
#
all		:	$(TARGET)

$(TARGET)	:	$(OBJS)
	$(CC) -o $(TARGET) $(OBJS) $(LFLAGS)

install		:	$(TARGET)
	cp -p $(TARGET) $(BIN_DIR)/.

clean		:
	rm -f $(OBJS) $(TARGET) core

#
#   Compiles
#
$(OBJS)		:	$(SRCS)
$(SRCS) 	:
