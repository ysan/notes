#!/bin/sh

../configure\
	--prefix=/home/yoshi/prog/12step/tools\
	--target=h8300-elf\
	--disable-nls

### make前に...
### 以下通りパスを通しておく
### PATH=/home/yoshi/prog/12step/tools/bin:$PATH
