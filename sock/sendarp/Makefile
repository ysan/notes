#
#   Defines
#
CC			=	/bin/gcc
CC_FLAGS	=	-Wall -O0 -MD
INCDIR		=	-I./ -I../../mycommon/
LIBDIR		=	-L../../mycommon/
LIBS		=	-lmycommon
OBJDIR		=	./objs/
MKDIR		=	/bin/mkdir
RM			=	/bin/rm

#
#   Objects
#
TARGET		=	sendarp
SRCS		=	sendarp.c\
				pcap_socket.c
OBJS		=	$(SRCS:%.c=$(OBJDIR)/%.o)
DEPENDS		=	$(OBJS:%.o=%.d)


#
#   Conditions
#
$(TARGET): $(OBJS)
	$(CC) -o $@ $^ $(CC_FLAGS) $(INCDIR) $(LIBDIR) $(LIBS)

$(OBJDIR)/%.o: %.c
	$(MKDIR) -p -m 775 $(OBJDIR)
	$(CC) -c $< -o $@ $(CC_FLAGS) $(INCDIR)


.PHONY: all
all: $(TARGET)

.PHONY: clean
clean:
	$(RM) -f $(TARGET) $(OBJS) $(DEPENDS)

-include $(DEPENDS)
