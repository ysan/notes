#!/bin/bash

which git >/dev/null 2>&1
RET=$?
if [ ! ${RET} -eq 0 ]; then
	echo "git is not installed."
	exit 1
fi

if [ ! -d "./msgpack-c" ]; then
	git clone https://github.com/msgpack/msgpack-c
	if [ ! ${RET} -eq 0 ]; then
		echo "git clone failure. [git clone https://github.com/msgpack/msgpack-c]"
		exit 1
	fi
fi

cp -p Makefile.msgpack msgpack-c/Makefile
if [ ${RET} -eq 0 ]; then
	echo "setup successful."
else
	echo "setup failure."
fi
