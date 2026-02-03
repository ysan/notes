#!/bin/bash

if [ ! "$(id -u)" -eq 0  ]; then
	echo "Please run as root..."
	exit 1
fi

if [ -z "${KERNEL_IMG}" -o -z "${ROOTFS_IMG}" ]; then
	echo "Please set KERNEL_IMG and ROOTFS_IMG."
	echo "  ex. export KERNEL_IMG=\"path/to\"; export ROOTFS_IMG=\"path/to\"; $0"
	echo "  ex. KERNEL_IMG=\"path/to\" ROOTFS_IMG=\"path/to\" $0"
	exit 1
fi
if [ ! -f "${KERNEL_IMG}" ]; then
	echo "Not found kernel image... [${KERNEL_IMG}]"
	exit 1
fi
if [ ! -f "${ROOTFS_IMG}" ]; then
	echo "Not found rootfs image... [${ROOTFS_IMG}]"
	exit 1
fi

BRIDGE="br-qemu"
BRIDGE_CIDR="10.10.0.2/24"
TAP="tap-qemu"

create_br_tap () {
	clean_br_tap

	echo "${FUNCNAME[0]}"

	ip link add ${BRIDGE} type bridge
	ip link set ${BRIDGE} up
	ip addr add ${BRIDGE_CIDR} dev ${BRIDGE}

	ip tuntap add ${TAP} mode tap
	ip link set ${TAP} up
	ip link set dev ${TAP} master ${BRIDGE}
}

clean_br_tap () {
	echo "${FUNCNAME[0]}"

	ip link list ${TAP} 2>&1 > /dev/null
	if [ $? -eq 0 ]; then
		ip link set dev ${TAP} nomaster
		ip link set ${TAP} down
		ip tuntap del ${TAP} mode tap
	fi

	ip link list ${BRIDGE} 2>&1 > /dev/null
	if [ $? -eq 0 ]; then
		ip addr del ${BRIDGE_CIDR} dev ${BRIDGE}
		ip link del ${BRIDGE}
	fi
}

create_br_tap

qemu-system-x86_64 -kernel "${KERNEL_IMG}" -initrd "${ROOTFS_IMG}" -append "console=ttyS0 root=/dev/ram rdinit=/sbin/init nokaslr" -nographic -smp 2 -m 256 -net nic -net tap,ifname=${TAP},script=no,downscript=no

clean_br_tap
