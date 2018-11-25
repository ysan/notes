#!/bin/bash

BASE_DIR=${HOME}/prog/ts_parser


export LD_LIBRARY_PATH=${BASE_DIR}/psisi:${BASE_DIR}/aribstr

if [ $# -ne 1 ]; then
	echo "specify 1 argument."
	exit 1
fi

if [ ! -e $1 -o ! -f $1 ]; then
	echo "not existed [$1]"
	exit 1
fi

#cat $1 | ${BASE_DIR}/ts_parser
${BASE_DIR}/ts_parser -f $1

