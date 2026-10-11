# 使い方: make MCU=attiny1604 PART=t1604   (MCU, PARTは都度指定する)
#
# 型番は用途ごとに別々に定義されているので、チップを変えるときは両方変える。
#   MCU  : avr-gcc用 (-mmcu)。 "attiny" + 型番 (例: attiny404, attiny1604)
#          gccのdevice-specs (/usr/lib/gcc/avr/*/device-specs/specs-<MCU>) が読まれ、
#          __AVR_ATtiny1604__ のようなマクロが自動定義される。
#          avr/io.h がそれを見て iotn1604.h (レジスタ定義) を読むので、
#          ソース側に型番を書く必要はない。 対応一覧: avr-size --mlist-devices
#   PART : avrdude用。 "t" + 型番 (例: t404, t1604)。 /etc/avrdude.conf のpart ID。
#          書き込み時のメモリ構成やUPDI設定に使われる。 対応一覧: avrdude -p ?
#   F_CPU: _delay_ms()等が使う周波数。 実際のクロック設定に合わせる。
#          (tinyAVR 0/1のデフォルトは20MHz内蔵OSC/6分周 = 3.33MHz)
ifneq ($(MAKECMDGOALS),clean)
ifndef MCU
$(error MCU is not set. 例: make MCU=attiny1604 PART=t1604)
endif
ifndef PART
$(error PART is not set. 例: make MCU=attiny1604 PART=t1604)
endif
endif

F_CPU    ?= 3333333UL
PROGRAMMER ?= serialupdi
PORT     ?= /dev/ttyUSB0
TARGET   := blink

CFLAGS := -mmcu=$(MCU) -DF_CPU=$(F_CPU) -Os -Wall -ffunction-sections -fdata-sections
LDFLAGS := -mmcu=$(MCU) -Wl,--gc-sections

all: $(TARGET).hex size

$(TARGET).elf: main.c
	avr-gcc $(CFLAGS) $(LDFLAGS) -o $@ $^

$(TARGET).hex: $(TARGET).elf
	avr-objcopy -O ihex -R .eeprom $< $@

size: $(TARGET).elf
	avr-size --format=avr --mcu=$(MCU) $<

flash: $(TARGET).hex
	avrdude -p $(PART) -c $(PROGRAMMER) -P $(PORT) -U flash:w:$<:i

clean:
	rm -f *.elf *.hex

.PHONY: all size flash clean
