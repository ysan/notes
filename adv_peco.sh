#!/bin/sh

function agvim () {
	if [ $# -ne 1 ]; then
		echo "Usage:"
		echo "   agvim arg  (one argument specified)"
		return
	fi

	ag $1 > /dev/null 2>&1
	if [ $? -eq 1 ]; then
		echo "not found..."
	else
		local WK_P
		WK_P=`ag $1 | peco --query "$LBUFFER" | awk -F : '{print "-c " $2 " " $1}'`
		if [ -n "${WK_P}" ]; then
			vim ${WK_P}
		else
			echo "canceled."
		fi
	fi
}

function fvim () {
	if [ $# -gt 1 ]; then
		echo "Usage:"
		echo "   fvim      (no arguments)"
		echo "   fvim arg  (one argument specified)"
		return
	fi

	local WK
	WK=`find . -name "*$1"`
	if [ -n "${WK}" ]; then
		local WK_P
		WK_P=`find . -type f -name "*$1" | peco`
		if [ -n "${WK_P}" ]; then
			vim ${WK_P}
		else
			echo "canceled."
		fi	
	else
		echo "not found..."
	fi
}

