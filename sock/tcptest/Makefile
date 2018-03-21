CC			=	gcc
CFLAGS		=	-Wall
LIBS		=	
INCDIR		=	-I./
LIBDIR		=	

SRCS01		=	tcpserv.c
OBJS01		=	$(SRCS01:%.c=%.o)
TARGET01	=	tcpserv

SRCS02		=	tcpclient.c
OBJS02		=	$(SRCS02:%.c=%.o)
TARGET02	=	tcpclient

.PHONY: all
all		:$(TARGET01) $(TARGET02)

$(TARGET01)	:$(OBJS01)
	$(CC) -o $(TARGET01) $(OBJS01) $(CFLAGS) $(LIBS) $(INCDIR) $(LIBDIR)
$(TARGET02)	:$(OBJS02)
	$(CC) -o $(TARGET02) $(OBJS02) $(CFLAGS) $(LIBS) $(INCDIR) $(LIBDIR)

$(OBJS01)		:$(SRCS01)
	$(CC) -c $(SRCS01) $(CFLAGS) $(INCDIR)
$(OBJS02)		:$(SRCS02)
	$(CC) -c $(SRCS02) $(CFLAGS) $(INCDIR)

.PHONY: clean
clean		:
	rm -f $(TARGET01) *.o
	rm -f $(TARGET02) *.o
