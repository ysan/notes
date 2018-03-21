#
#   Defines
#
CC			=	/bin/gcc
CC_FLAGS	=	-Wall -shared -MD -fPIC
INCDIR		=	-I./
OBJDIR		=	./objs/
MKDIR		=	/bin/mkdir
RM			=	/bin/rm

LINKBASEDIR	=	$(shell pwd)/
LINKDIR		=	/usr/lib/

#
#   Objects
#
TARGET		=	libmycommon.so
SRCS		=	mycommon.c
OBJS		=	$(SRCS:%.c=$(OBJDIR)/%.o)
DEPENDS		=	$(OBJS:%.o=%.d)


#
#   Conditions
#
$(TARGET): $(OBJS)
	$(CC) -o $@ $^ $(CC_FLAGS) $(INCDIR)

$(OBJDIR)/%.o: %.c
	$(MKDIR) -p -m 775 $(OBJDIR)
	$(CC) -c $< -o $@ $(CC_FLAGS) $(INCDIR)

.PHONY: all
all: $(TARGET)

.PHONY: link
link:
	sudo ln -s $(LINKBASEDIR)$(TARGET) $(LINKDIR)$(TARGET)

.PHONY: unlink
unlink:
	sudo $(RM) -f $(LINKTODIR)$(TARGET)

.PHONY: clean
clean:
	$(RM) -f $(TARGET) $(OBJS) $(DEPENDS)

-include $(DEPENDS)
