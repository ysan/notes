PREFIX		:=	/home/yoshi/prog/12step/tools
ARCH		:=	h8300-elf
GCC_VER		:=	4.8.3

AR			:=	$(PREFIX)/bin/$(ARCH)-ar
AS			:=	$(PREFIX)/bin/$(ARCH)-as
CC			:=	$(PREFIX)/bin/$(ARCH)-gcc
LD			:=	$(PREFIX)/bin/$(ARCH)-ld
NM			:=	$(PREFIX)/bin/$(ARCH)-nm
OBJCOPY		:=	$(PREFIX)/bin/$(ARCH)-objcopy
OBJDUMP		:=	$(PREFIX)/bin/$(ARCH)-objdump
RANLIB		:=	$(PREFIX)/bin/$(ARCH)-ranlib
STRIP		:=	$(PREFIX)/bin/$(ARCH)-strip

H8WRITE		:=	$(PREFIX)/kz_h8write

# シリアルポート デバイスファイル
#	FreeBSD-4.x		/dev/cuaaX
#	FreeBSD-6.x		/dev/cuadX
#	FreeBSD(USB)	/dev/cuaUx
#	Linux 			/dev/ttySx
#	Linux(USB)		/dev/ttyUSBx
#	Windows			comX
SERIAL_DEV	:=	/dev/ttyUSB0

MKDIR		:=	/bin/mkdir
RM			:=	/bin/rm
RMDIR		:=	/bin/rmdir

TARGET		:=	bootloader

SRCS		:=	vector.c
SRCS		+=	main.c
SRCS		+=	initialize.c
SRCS		+=	intc.c
SRCS		+=	sci.c
SRCS		+=	timer.c
SRCS		+=	bsc.c
SRCS		+=	xmodem.c
SRCS		+=	elf_loader.c
SRCS		+=	led.c
SRCS		+=	command.c
SRCS		+=	util.c

ASMS		:=	startup.S
ASMS		+=	intr.S

OBJDIR		:=	./objs

OBJS		:=	$(SRCS:%.c=$(OBJDIR)/%.o)
OBJS		+=	$(ASMS:%.S=$(OBJDIR)/%.o)

DEPENDS		:=	$(OBJS:%.o=%.d)


CFLAGS		:=	-Wall
CFLAGS		+=	-mh				# H8/300H用のコードを生成 (H8/300S用は-msとする)
#CFLAGS		+=	-mn				# ノーマルモード (使用する場合はlibc等のリンク先も変更する必要ある)
#CFLAGS		+=	-mint32			# int32bitモード (使用する場合はlibc等のリンク先も変更する必要ある)
CFLAGS		+=	-nostdinc		# 標準システムディレクトリを検索しない
CFLAGS		+=	-nostdlib		# 標準システムライブラリとスタートアップファイルを使用しない
CFLAGS		+=	-fno-builtin	# ビルトイン関数の最適化しない
CFLAGS		+=	-static			# shared libraryでもstaticにリンクする
CFLAGS		+=	-Os
#CFLAGS		+=	-g
CFLAGS		+=	-MD

INCDIR		:=	-I.
INCDIR		+=	-I$(PREFIX)/$(ARCH)/include
INCDIR		+=	-I$(PREFIX)/lib/gcc/$(ARCH)/$(GCC_VER)/include

LDSCR		:=	ld.scr
LFLAGS		:=	-T$(LDSCR)		# リンカスクリプトの指定
#LFLAGS		+=	-L$(PREFIX)/$(ARCH)/lib/h8300h -lm
LFLAGS		+=	-L$(PREFIX)/$(ARCH)/lib/h8300h -lg
LFLAGS		+=	-L$(PREFIX)/$(ARCH)/lib/h8300h -lc
LFLAGS		+=	-L$(PREFIX)/lib/gcc/$(ARCH)/$(GCC_VER)/h8300h -lgcc



.SUFFIXES: .o .c .S


$(TARGET).mot: $(TARGET)
	$(OBJCOPY) -O srec $(TARGET) $@

$(TARGET): $(OBJS)
	$(CC) -o $@ $^ $(CFLAGS) $(INCDIR) $(LFLAGS) $(LIBS)
	cp -p $(TARGET) $(TARGET).elf
	$(STRIP) $(TARGET)

###.c.o:
$(OBJDIR)/%.o: %.c
	$(MKDIR) -p -m 775 $(OBJDIR)
	$(CC) -c $< $(CFLAGS) $(INCDIR) -o $@

###.S.o:
$(OBJDIR)/%.o: %.S
	$(MKDIR) -p -m 775 $(OBJDIR)
	$(CC) -c $< -o $@ $(CFLAGS) $(INCDIR)


.PHONY: all
all: $(TARGET)

.PHONY: image
image: $(TARGET).mot

.PHONY: install
install: $(TARGET).mot
	sudo $(H8WRITE) -3069 -f20 $(TARGET).mot $(SERIAL_DEV)

.PHONY: clean
clean:
	$(RM) -f $(OBJS) $(TARGET) $(TARGET).elf $(TARGET).mot $(DEPENDS)
#	$(RMDIR) $(OBJDIR)

-include $(DEPENDS)

