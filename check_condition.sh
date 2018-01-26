#!/bin/sh


HTTP_TEST_SITE="www.yahoo.co.jp"
#DNS_TEST_SITE="www.intersony.sony.co.jp"
DNS_TEST_SITE=${HTTP_TEST_SITE}
SYS_ROUTE="/proc/net/route"
ENV_HTTP_PROXY=${http_proxy}
NOTE="/etc/profile.d/note.txt"
MEM_INFO="/proc/meminfo"

NIC_LIST=()
NIC_NUM=0

IS_SUCCESS=true
IS_NIC_RUNNING=true
INET=""


function _echo_sep ()
{
	echo "  ---------------------------------------------------------------"
}

function _calc_networkaddr ()
{
	local IP="$1"
	local PREFIX="$2"

	local OCTET=("" "" "" "")
	local N=0
	local CHK=0
	local LOW_BIT=0

	OCTET[0]=`echo "${IP}" | sed 's/\./ /g' | awk '{print $1}'`
	OCTET[1]=`echo "${IP}" | sed 's/\./ /g' | awk '{print $2}'`
	OCTET[2]=`echo "${IP}" | sed 's/\./ /g' | awk '{print $3}'`
	OCTET[3]=`echo "${IP}" | sed 's/\./ /g' | awk '{print $4}'`

	while [ ${N} -lt 4 ];
	do
#		echo "${OCTET[${N}]}"

		CHK=`expr \( ${N} + 1 \) \* 8`
		if [ ${PREFIX} -lt ${CHK} ]; then
			LOW_BIT=`expr ${CHK} - ${PREFIX}`
			if [ ${LOW_BIT} -gt 8 ]; then
				LOW_BIT=8
			fi

#			echo "low_bit ${LOW_BIT}"
			OCTET[${N}]=$(($((${OCTET[${N}]} >> ${LOW_BIT})) << ${LOW_BIT}))
		fi

		N=`expr ${N} + 1`
	done

	echo "${OCTET[0]}.${OCTET[1]}.${OCTET[2]}.${OCTET[3]}"
}

function _get_gateway ()
{
	local GATEWAY=""
	local OCTET_HEX=("" "" "" "")
	local OCTET_DEC=("0" "0" "0" "0")

	if [ -f "${SYS_ROUTE}" -a -r "${SYS_ROUTE}" ]; then
		while read LINE
		do
			GATEWAY=`echo "${LINE}" | egrep -v "Iface" | awk '{print $3}'`
			if [ "x${GATEWAY}" != "x00000000" -a "x${GATEWAY}" != "x" ]; then
				break
			fi
		done <<- END
		`cat ${SYS_ROUTE}`
		END

#		echo ${GATEWAY}
		OCTET_HEX[3]=`echo ${GATEWAY} | cut -b 1-2`
		OCTET_HEX[2]=`echo ${GATEWAY} | cut -b 3-4`
		OCTET_HEX[1]=`echo ${GATEWAY} | cut -b 5-6`
		OCTET_HEX[0]=`echo ${GATEWAY} | cut -b 7-8`

		OCTET_DEC[0]=`echo "obase=10;ibase=16;${OCTET_HEX[0]}" | bc`
		OCTET_DEC[1]=`echo "obase=10;ibase=16;${OCTET_HEX[1]}" | bc`
		OCTET_DEC[2]=`echo "obase=10;ibase=16;${OCTET_HEX[2]}" | bc`
		OCTET_DEC[3]=`echo "obase=10;ibase=16;${OCTET_HEX[3]}" | bc`

	fi

	echo "${OCTET_DEC[0]}.${OCTET_DEC[1]}.${OCTET_DEC[2]}.${OCTET_DEC[3]}"
}

function _check_network ()
{
#	local NIC=""
#	local IP=""
#	local PREFIX=""
#	local NETWORK_ADDR=""
	local GATEWAY=""
	local RET=""

#	for NIC in ${NIC_LIST[@]}
#	do
#		echo "xxx ${NIC}"
#		IP=`ip -4 addr 2>&1 | grep inet | grep " ${NIC}" | awk '{print $2}' | sed 's/\// /g' | awk '{print $1}'`
#		PREFIX=`ip -4 addr 2>&1 | grep inet | grep " ${NIC}" | awk '{print $2}' | sed 's/\// /g' | awk '{print $2}'`
#		if [ -n "${IP}" -a -n "${PREFIX}" ]; then
#			NETWORK_ADDR=`_calc_networkaddr ${IP} ${PREFIX}`
#			echo "${NETWORK_ADDR}"
#		fi
#	done

	GATEWAY=`_get_gateway`
	if [ ${GATEWAY} != "0.0.0.0" ]; then
		ping -c 1 -w 1 ${GATEWAY} >/dev/null 2>&1
		RET=$?
		if [ ${RET} -eq 0 ]; then
			return 0
		fi
	fi

	return 1
}

function _check_dns ()
{
	local NS=""
	local LINE=""
	local RET=""

	while read LINE
	do
#		echo "${LINE}"
		echo "${LINE}" | grep -i "Server:" >/dev/null 2>&1
		RET=$?
		if [ ${RET} -eq 0 ]; then
			NS=`echo "${LINE}" | awk '{print $2}'`
		fi

		echo "${LINE}" | grep -i "server can't find" >/dev/null 2>&1
		RET=$?
		if [ ${RET} -eq 0 ]; then
			IS_SUCCESS=false
		fi

		echo "${LINE}" | grep -i "connection timed out" >/dev/null 2>&1
		RET=$?
		if [ ${RET} -eq 0 ]; then
			IS_SUCCESS=false
		fi
	done <<- END
	`nslookup -type=A -timeout=1 -retry=0 ${DNS_TEST_SITE}`
	END

	echo -n "  dns check -> "
	if ${IS_SUCCESS}; then
		echo "success.  (nameserver:${NS})"
	else
		if [ -z "${NS}" ]; then
			echo "error."
		else
			echo "error.  (nameserver:${NS})"
		fi
	fi
}

function _check_proxy ()
{
	local PROXY_HOST=""
	local RET=""

	if [ ! -z "${http_proxy+x}" ]; then
		PROXY_HOST=`echo ${http_proxy} | sed 's/[:\/]/ /g' | awk '{print $2}'`
		echo -n "  proxy [${PROXY_HOST}] is "

		ping -c 1 -w 1 ${PROXY_HOST} >/dev/null 2>&1
		RET=$?
		if [ ${RET} -eq 0 ]; then
			echo "alive."

			echo -n "  proxy request/response... "
			wget --spider -4 --timeout 1 --tries=1 ${HTTP_TEST_SITE} >/dev/null 2>&1
			RET=$?
			if [ ${RET} -eq 0 ]; then
				echo "OK."
			else
				echo "NG."
			fi

		else
			echo "down."
		fi
	fi
}

function _check_memory ()
{
	if [ -f "${MEM_INFO}" -a -r "${MEM_INFO}" ]; then
		echo -n "  "
		cat ${MEM_INFO} | grep MemTotal
	fi
}


#--------------------------------#
# ----     main routine     ---- #
#--------------------------------#

echo ""
echo "  `hostname` check condition."
_echo_sep
echo "  system up since `uptime -s`"

_echo_sep
while read LINE
do
	echo "  ${LINE}"
done << END
`lscpu`
END

_echo_sep
_check_memory

_echo_sep
while read LINE
do
	echo "  ${LINE}"
done << END
`vmstat`
END

_echo_sep
while read LINE
do
	echo "  ${LINE}"
done << END
`df -hT`
END

_echo_sep
while read LINE
do
	echo "  ${LINE}"
done << END
`finger`
END


_echo_sep
NIC=""
FLAGS=""
while read LINE
do
	echo "${LINE}" | grep -E "Kernel|Iface" >/dev/null 2>&1
	RET=$?
	if [ ${RET} -eq 1 ]; then
		NIC=`echo "${LINE}" | awk '{print $1}'`
		FLAGS=`echo "${LINE}" | awk '{print $NF}'`
		case "${FLAGS}" in
		*L* )
			# loopback interface
			;;
		*R* )
			# running
#			NIC_LIST[${NIC_NUM}]=${NIC}
			NIC_LIST+=(${NIC})
			NIC_NUM=`expr ${NIC_NUM} + 1`
			;;
		esac
	fi
done << END
`netstat -i`
END
if [ ${NIC_NUM} -gt 0 ]; then
	for NIC in ${NIC_LIST[@]}
	do
		INET=`ip addr show dev ${NIC} | grep "inet " |  awk '{printf $2}'`
		echo "  interface [${NIC}] [${INET}] running."
	done
	IS_NIC_RUNNING=true
else
	echo "  nic is not running."
	IS_NIC_RUNNING=false
fi

if ${IS_NIC_RUNNING}; then
	_check_network
	RET=$?
	if [ ${RET} -eq 0 ]; then
		echo "  network is normal."
		_check_dns
		_check_proxy
	else
		echo "  network is abnormal."
	fi

fi


if [ -f "${NOTE}" -a -r "${NOTE}" ]; then
	_echo_sep
	echo "  note..."
	while read LINE
	do
		echo -n "    * "
		echo "${LINE}"
	done <<- END
	`cat ${NOTE}`
	END
fi

_echo_sep
echo ""

