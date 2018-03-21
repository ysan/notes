CC			=	/bin/gcc
CC_FLAGS	=	-Wall
LIBS		=	-lpthread -lmycommon
LIBDIR		=	-L../../mycommon/
INCDIR		=	-I./ -I../../mycommon/

TARGET		=	hserv
SRCS		=	hserv.c\
				request.c\
				response.c\
				cgi.c
OBJS		=	$(SRCS:%.c=%.o)


.PHONY: all
all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) -o $@ $^ $(CC_FLAGS) $(INCDIR) $(LIBDIR) $(LIBS) 

%.o: %.c
	$(CC) -c $< $(CC_FLAGS) $(INCDIR)

.PHONY: clean
clean:
	rm -f $(TARGET) $(OBJS)
