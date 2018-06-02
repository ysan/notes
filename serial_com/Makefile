CC			=	/bin/gcc
CFLAGS		=	-Wall
INCDIR		=	-I./ -I../mycommon
LIBDIR		=	-L../mycommon
LFLAGS		=	-lmycommon

TARGET		=	serial_com

SRCS		=	serial_com.c
SRCS		+=	xmodem.c

OBJS		=	$(SRCS:%.c=%.o)


$(TARGET): $(OBJS)
	$(CC) -o $@ $^ $(CFLAGS) $(LFLAGS) $(INCDIR) $(LIBDIR)

%.o: %.c
	$(CC) -c $< $(CFLAGS) $(INCDIR)


.PHONY: all
all: $(TARGET)

.PHONY: clean
clean:
	rm -f $(TARGET) $(OBJS)
