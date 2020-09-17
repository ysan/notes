#!/bin/bash


MOUNT_SRC=//192.168.50.7/disk
MOUNT_POINT=/mnt/Landisk/
DEST_DIR=${MOUNT_POINT}/WinXP_backup/y/My\ Documents/My\ Videos/xxore/er/m2ts/

HOME=/home/pi
WORK_DIR=${HOME}/backuper

LOG_PREFIX=${WORK_DIR}/backuper
WORK_LOG_PATH=${LOG_PREFIX}.log.$$

START_DATE=
END_DATE=


#TARGET_DIR=/home/pi
TARGET_DIR=/etc/auto_rec_mini/data
TARGET_FILE_EXT=m2ts


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


### !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	if [ $1 = "success" ] ; then
		rm -f ${COMP_LOG_PATH}.gz >/dev/null 2>&1
	fi
### !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
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
		sudo mount -t cifs -o sec=ntlm,vers=1.0 -o username=admin,password= ${MOUNT_SRC} ${MOUNT_POINT} >> ${WORK_LOG_PATH} 2>&1
		if [ $? -eq 0 ]; then
			echo "mount OK" >> ${WORK_LOG_PATH}
			return 0
		else
			echo "mount NG" >> ${WORK_LOG_PATH}
			return 1
		fi
	fi
}

function _check_duplicate () {
#	if [ $$ != `pgrep -fo $1`  ]; then
	if [ $$ != `pgrep -fo "bash $1"`  ]; then
		# duplicate
		return 1
	else
		return 0
	fi
}


###---    main flow    ---###

_check_duplicate $0
if [ ! $? -eq 0 ]; then
	logger "`basename $0`: already running. exit..."
	exit 1
fi

_start_proc
if [ ! $? -eq 0 ]; then
	_end_proc error
	exit 1
fi


# test
#sleep 120
#_end_proc success
#exit 0


ERR_FLAG=0

find ${TARGET_DIR} -type f -name "*.${TARGET_FILE_EXT}" > ${WORK_DIR}/list.$$  2>/dev/null
RET=$?
if [ ${RET} -eq 0 ]; then
	while read LINE
	do
		echo "line:[${LINE}]"  >> ${WORK_LOG_PATH} 2>&1
		if [ ${#LINE} -gt 0 ] ; then
			sudo cp -p "${LINE}" "${DEST_DIR}"  >> ${WORK_LOG_PATH} 2>&1
			RET=$?
			if [ ${RET} -eq 0 ]; then
				sudo rm -f "${LINE}" >/dev/null 2>&1
			else
				ERR_FLAG=1
			fi
		fi
	done <<- END
	`cat ${WORK_DIR}/list.$$`
	END
fi

rm -f ${WORK_DIR}/list.$$ >/dev/null 2>&1


if [ ${ERR_FLAG} -eq 0 ]; then
	_end_proc success
else
	_end_proc error
fi
