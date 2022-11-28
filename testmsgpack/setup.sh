#!/bin/bash

set -e

rm -rf ./msgpack-3.3.0

if [ ! -d "./msgpack-3.3.0" ]; then
	curl -LOk https://github.com/msgpack/msgpack-c/releases/download/cpp-3.3.0/msgpack-3.3.0.tar.gz
	if [ ! ${RET} -eq 0 ]; then
		echo "msgpack download failure."
		exit 1
	fi
	tar xvf msgpack-3.3.0.tar.gz
fi

cp -p Makefile.msgpack msgpack-3.3.0/Makefile
if [ ${RET} -eq 0 ]; then
	echo "setup successful."
else
	echo "setup failure."
fi
