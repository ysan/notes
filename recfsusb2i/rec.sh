#!/bin/bash


MOUNT_SRC=//192.168.50.7/disk
MOUNT_POINT=/mnt/Landisk/
DEST_DIR=${MOUNT_POINT}/WinXP_backup/y/My\ Documents/My\ Videos/xxore/er/m2ts/

HOME=/home/pi
WORK_DIR=${HOME}/recfsusb2i

LOG_PREFIX=${WORK_DIR}/log/rec
WORK_LOG_PATH=${LOG_PREFIX}.log.$$

WORK_TS_PATH=${WORK_DIR}/tmp.m2ts.$$

START_DATE=
END_DATE=


function _start_proc () {
	START_DATE=`date +%s`
	echo "`basename $0`: start   --- `date +%Y.%m.%d\ %H:%M:%S` ---" >> ${WORK_LOG_PATH}
	echo "" >> ${WORK_LOG_PATH}

	_check_mount_landisk
	if [ $? -eq 0 ]; then
		return 0
	else
		return 1
	fi
}

function _end_proc () {
	echo "" >> ${WORK_LOG_PATH}
	echo "`basename $0`: end [$1]   --- `date +%Y.%m.%d\ %H:%M:%S` ---"  >> ${WORK_LOG_PATH}

	END_DATE=`date +%s`
	local TT=`expr ${END_DATE} - ${START_DATE}`
	local H=`expr ${TT} / 3600`
	TT=`expr ${TT} % 3600`
	local M=`expr ${TT} / 60`
	local S=`expr ${TT} % 60`
	echo "elapsed time ${H}:${M}:${S}" >> ${WORK_LOG_PATH}

	_finalize_log $1
}

function _finalize_log () {
	# log rename
	local DATE_STR=`date +%Y-%m-%d-%H%M%S`
	local COMP_LOG_PATH=${LOG_PREFIX}_${DATE_STR}_$1.log
	mv ${WORK_LOG_PATH} ${COMP_LOG_PATH} >/dev/null 2>&1
	gzip ${COMP_LOG_PATH} >/dev/null 2>&1
}

function _check_mount_landisk () {
	mount | grep Landisk >/dev/null 2>&1
	if [ $? -eq 0 ]; then
		# already mounted
		echo "already mounted Landisk." >> ${WORK_LOG_PATH}
		echo "" >> ${WORK_LOG_PATH}
		return 0
	else
		# not mounted
		echo "not mounted Landisk. --> do mount..." >> ${WORK_LOG_PATH}
		sudo mount -t cifs -o sec=ntlm -o username=admin,password= ${MOUNT_SRC} ${MOUNT_POINT} >> ${WORK_LOG_PATH} 2>&1
		if [ $? -eq 0 ]; then
			echo "mount OK" >> ${WORK_LOG_PATH}
			return 0
		else
			echo "mount NG" >> ${WORK_LOG_PATH}
			return 1
		fi
	fi
}

function _is_num () {
	expr $1 + 1 > /dev/null 2>&1
	if [ $? -lt 2 ]; then
		return 0
	else
		return 1
	fi
}


###---    main flow    ---###

touch ${WORK_LOG_PATH}
if [ ! $? -eq 0 ]; then
	logger "`basename $0`: WORK_LOG_PATH:[${WORK_LOG_PATH}] is invalid."
	exit 1
fi


_start_proc
if [ ! $? -eq 0 ]; then
	_end_proc error
	exit 1
fi


# check arguments
if [ $# -lt 2 -o $# -gt 3 ]; then
	echo "Usage: $0 CHANNEL RECMIN [TAG]" >> ${WORK_LOG_PATH}
	exit 1
fi

_is_num $1
if [ $? -ne 0 ]; then
	echo "check argument: CHANNEL is not num..." >> ${WORK_LOG_PATH}
	exit 1
fi

_is_num $2
if [ $? -ne 0 ]; then
	echo "check argument: RECMIN is not num..." >> ${WORK_LOG_PATH}
	exit 1
fi

CHANNEL=$1
RECSEC=`expr $2 \* 60` > /dev/null 2>&1
TAG=rec
if [ $# -eq 3 ]; then
	TAG=$3
fi


if [ ! -e ${MOUNT_POINT} -o ! -d ${MOUNT_POINT} ]; then
	echo "MOUNT_POINT:[${MOUNT_POINT}] is not found." >> ${WORK_LOG_PATH}
	_end_proc error
	exit 1
fi


RETRY_CNT=0
RET=0
while [ ${RETRY_CNT} -lt 5 ]
do
	# rec start
	sudo ${WORK_DIR}/recfsusb2i ${CHANNEL} ${RECSEC} ${WORK_TS_PATH} >> ${WORK_LOG_PATH} 2>&1
	RET=$?
	if [ ! ${RET} -eq 0 ]; then
		rm -f ${WORK_TS_PATH} >/dev/null 2>&1
	else
		break
	fi

	sleep 1
	RETRY_CNT=`expr ${RETRY_CNT} + 1`
done

if [ ${RET} -eq 0 ]; then
	if [ -e ${WORK_TS_PATH} ]; then
		DATE_STR=`date +%Y-%m-%d-%H%M%S`
		COMP_TS=${WORK_DIR}/${TAG}_${DATE_STR}.m2ts
		mv ${WORK_TS_PATH} ${COMP_TS} >/dev/null 2>&1

		sudo cp -p ${COMP_TS} "${DEST_DIR}" >/dev/null 2>&1
		rm -f ${COMP_TS} >/dev/null 2>&1
	fi
	_end_proc success

else
	if [ -e ${WORK_TS_PATH} ]; then
		rm -f ${WORK_TS_PATH} >/dev/null 2>&1
	fi
	_end_proc error
fi
