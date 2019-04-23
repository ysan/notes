#!/bin/bash

which git >/dev/null 2>&1
RET=$?
if [ ! ${RET} -eq 0 ]; then
	echo "git is not installed."
	exit 1
fi

if [ ! -d "./cereal" ]; then
	git clone https://github.com/USCiLab/cereal
	if [ ! ${RET} -eq 0 ]; then
		echo "git clone failure. [git clone https://github.com/USCiLab/cereal]"
		exit 1
	fi
fi

echo "setup successful."
