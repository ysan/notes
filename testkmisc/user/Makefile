CC		:=	g++
CFLAGS	:=	-Wall -static
INCDIR	:=	-I./ -I../driver
LIBDIR	:=
LIBS	:=	-lpthread

TARGET	:=	rw
SRCS	:=	rw.cpp
OBJS	:=	$(SRCS:%.cpp=%.o)


.PHONY: all
all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) -o $@ $^ $(CFLAGS) $(INCDIR) $(LIBDIR) $(LIBS)

%.o: %.cpp
	$(CC) -c $< $(CFLAGS) $(INCDIR)

.PHONY: clean
clean:
	rm -f $(TARGET) $(OBJS)
