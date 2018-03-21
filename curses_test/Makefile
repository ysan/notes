#
#   Defines
#
CC			=	/bin/gcc
CC_FLAGS	=	-Wall -MD
INCDIR		=	-I./ -I../mycommon -I../cursutil
LIBDIR		=	-L../mycommon -L../cursutil
LIBS		=	-lncurses -lmycommon -lcursutil
OBJDIR		=	./objs/
MKDIR		=	/bin/mkdir
RM			=	/bin/rm


#
#   Objects
#
TARGET		=	curses_test
SRCS		=	curses_test.c
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
