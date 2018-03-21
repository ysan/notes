CC			=	gcc
CC_FLAGS	=	-Wall
LIBS		=	-lmycommon
INC_DIR		=	-I./ -I../../mycommon
LIB_DIR		=	-L../../mycommon/

SRCS		=	client.c
OBJS		=	$(SRCS:%.c=%.o)
TARGET		=	client


.PHONY: all
all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) -o $@ $^ $(CC_FLAGS) $(LIBS) $(INC_DIR) $(LIB_DIR)

%.o: %.c
	$(CC) -c $< $(CC_FLAGS) $(INC_DIR)

.PHONY: clean
clean:
	rm -f $(TARGET) $(OBJS)
