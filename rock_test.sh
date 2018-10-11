#!/bin/sh

LPATH=`pwd`
PRGM=`basename $0 .sh`


lockmyself()
{
    DIRECTORY=${2}
    DISTNAME=${DIRECTORY}/${1}.pid
    LOCKNAME=${DIRECTORY}/${1}.lck
    
    if [ -d ${DIRECTORY} ] && [ -w ${DIRECTORY} ]; then
	if [ -f ${DISTNAME} ]; then
	    :
	else
	    echo $$  > /dev/null 2>&1 > ${DISTNAME}
	fi
	
	ln -s ${DISTNAME} ${LOCKNAME} > /dev/null 2>&1
	if [ $? -eq 0 ]; then
	    # success to lock
	    echo $$  > /dev/null 2>&1 > ${DISTNAME}
	    if [ $? -ne 0 ]; then
		return 1
	    fi
	    return 0
	else
	    # failed to lock
	    return 1
	fi
    else
	# you cannot execute this command
	return 2	
    fi
}

unlockmyself()
{
#    rm -f ${2}/${1}.lck > /dev/null 2>&1
    rm ${2}/${1}.lck > /dev/null 2>&1
}



################ start main ################

echo "${LPATH}/${PRGM} start."


lockmyself ${PRGM} ${LPATH}
LOCKSTAT=$?
case ${LOCKSTAT} in
    0)
	;;
    *)
	echo error - lockfile existed!
	exit 1
	;;
esac


TMP=""
NUM=0
while [ "${TMP}" != "n" ];
do
    NUM=`expr ${NUM} + 1`
    printf " -- ${PRGM}[PID:$$][${NUM}] $ "
    read TMP
    eval ${TMP} 2>${LPATH}/tmp_$$
    if [ "${TMP}" != "n" ] ; then
	cat ${LPATH}/tmp_$$
    fi
    rm -f ${LPATH}/tmp_$$ 2>/dev/null
done

echo while end.


unlockmyself ${PRGM} ${LPATH}
LOCKSTAT=$?
case ${LOCKSTAT} in
    0)
        ;;
    *)
        echo error - unable to erase lockfile.
        exit 1
        ;;
esac


echo "${LPATH}/${PRGM} successful end."