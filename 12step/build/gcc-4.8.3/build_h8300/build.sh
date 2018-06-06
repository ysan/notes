#!/bin/sh

../configure\
	--prefix=/home/yoshi/prog/12step/tools\
	--target=h8300-elf\
	--disable-nls\
	--disable-werror\
	--disable-shared\
	--enable-languages=c\
	--disable-threads\
	--with-newlib\
	--with-headers=../../newlib-1.19.0/newlib/libc/include\
	--disable-libssp\
	--with-gmp=/usr/local\
	--with-mpfr=/usr/local\
	--with-mpc=/usr/local\
	--with-ppl=/usr/local\
	--with-cloog=/usr/local

## --disable-multilib このオプションは入れない
## lib関係の h8300h h8300s のディレクトリ以下が生成されなくなる

### make前に...

### 以下の通りパスを通しておく
### PATH=/home/yoshi/prog/12step/tools/bin:$PATH

### 解凍ディレクトリに以下のリンクを作成しておく
### ln -s ../newlib-1.19.0/newlib .
### ln -s ../newlib-1.19.0/libgloss .
