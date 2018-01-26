#!/bin/bash


BACKUP_FROM=/mnt/Landisk/
BACKUP_TO=/media/pi/EC-PHU3/Landisk_backup


HOME=/home/pi
WORK_DIR=${HOME}/backup_in_rsync

LOG_PREFIX=${WORK_DIR}/log/backup_in_rsync
WORK_LOG_PATH=${LOG_PREFIX}.log

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


	_spindown_usbhdd
	_spindown_usbhdd
	_spindown_usbhdd
	_spindown_usbhdd
	_spindown_usbhdd

	_finalize_log $1
}

function _spindown_usbhdd () {
	# TODO
	sleep 2

	echo "" >> ${WORK_LOG_PATH}
	sudo hdparm -y /dev/sda >> ${WORK_LOG_PATH} 2>&1 
	if [ $? -eq 0 ]; then
		echo "spindown OK" >> ${WORK_LOG_PATH}
		return 0
	else
		echo "spindown NG" >> ${WORK_LOG_PATH}
		return 1
	fi
}

function _finalize_log () {
	# log rename
	local DATE_STR=`date +%Y%m%d%H%M%S`
	local COMP_LOG_PATH=${LOG_PREFIX}_${DATE_STR}_$1.log
	mv ${WORK_LOG_PATH} ${COMP_LOG_PATH}
	gzip ${COMP_LOG_PATH}
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
		sudo mount -t cifs -o sec=ntlm -o username=admin,password= //192.168.50.8/disk /mnt/Landisk >> ${WORK_LOG_PATH} 2>&1
		if [ $? -eq 0 ]; then
			echo "mount OK" >> ${WORK_LOG_PATH}
			return 0
		else
			echo "mount NG" >> ${WORK_LOG_PATH}
			return 1
		fi
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


if [ ! -e ${BACKUP_FROM} -o ! -d ${BACKUP_FROM} ]; then
	echo "BACKUP_FROM:[${BACKUP_FROM}] is not found." >> ${WORK_LOG_PATH}
	_end_proc error
	exit 1
fi

if [ ! -e ${BACKUP_TO} -o ! -d ${BACKUP_TO} ]; then
	echo "BACKUP_TO:[${BACKUP_TO}] is not found." >> ${WORK_LOG_PATH}
	_end_proc error
	exit 1
fi



#rsync -ar --delete ${BACKUP_FROM} ${BACKUP_TO} >> ${WORK_LOG_PATH} 2>&1
#rsync -ar --delete ${BACKUP_FROM} ${BACKUP_TO} --exclude '/dust/' >> ${WORK_LOG_PATH} 2>&1
rsync -arv --delete ${BACKUP_FROM} ${BACKUP_TO} >> ${WORK_LOG_PATH} 2>&1
#rsync -arvvv --delete ${BACKUP_FROM} ${BACKUP_TO} >> ${WORK_LOG_PATH} 2>&1
if [ $? -eq 0 ]; then
	_end_proc success
	exit 0
else
	_end_proc error
	exit 1
fi

