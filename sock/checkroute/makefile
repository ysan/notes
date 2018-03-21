CC			=	/bin/gcc
CC_FLAGS	=	-Wall
INCDIR		=	-I./ -I../../mycommon/
LIBDIR		=	-L../../mycommon/
LIBS		=	-lmycommon

TARGET		=	checkroute
SRCS		=	checkroute.c\
				pcap_checksum.c
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
