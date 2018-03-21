CC			=	/bin/gcc
CC_FLAGS	=	-Wall -g
INCDIR		=	-I./ -I../../mycommon
LIBDIR		=	-L../../mycommon/
LIBS		=	-lmycommon

TARGET		=	echoserv
SRCS		=	echoserv_main.c\
				echoserv_socket.c\
				echoserv_signal.c\
				echoserv_accept.c\
				echoserv_child_proc.c\
				echoserv_child_info.c\
				echoserv_shm.c
OBJS		=	$(SRCS:%.c=%.o)


.PHONY: all
all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) -o $@ $^ $(CC_FLAGS) $(LIBS) $(INCDIR) $(LIBDIR)

%.o: %.c
	$(CC) -c $< $(CC_FLAGS) $(INCDIR)

.PHONY: clean
clean:
	rm -f $(TARGET) $(OBJS)
