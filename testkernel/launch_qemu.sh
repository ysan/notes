#!/bin/bash

if [ ! "$(id -u)" -eq 0  ]; then
	echo "Please run as root..."
	exit 1
fi

QEMU_DIR="${HOME}/prog/github_repos/qemu/"
QEMU_CMD="${QEMU_DIR}/build/qemu-system-x86_64 -L ${QEMU_DIR}/pc-bios"
#QEMU_CMD="qemu-system-x86_64"
KERNEL_IMG="${HOME}/prog/github_repos/linux/arch/x86/boot/bzImage"
#KERNEL_IMG="/boot/vmlinuz-5.0.0-23-generic"
ROOTFS="./rootfs.img"

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


#${QEMU_CMD} -kernel "${KERNEL_IMG}" -initrd "${ROOTFS}" -append "console=ttyS0 root=/dev/ram rdinit=/sbin/init nokaslr" -nographic -device edu
#${QEMU_CMD} -kernel "${KERNEL_IMG}" -initrd "${ROOTFS}" -append "console=ttyS0 root=/dev/ram rdinit=/sbin/init nokaslr" -nographic -device edu -gdb tcp::12345 -S

#${QEMU_CMD} -kernel "${KERNEL_IMG}" -initrd "${ROOTFS}" -append "console=ttyS0 root=/dev/ram rdinit=/sbin/init nokaslr dyndbg=\"file irqdomain.c +p; file irqdesc.c +p\"" -nographic -device edu

#${QEMU_CMD} -kernel "${KERNEL_IMG}" -initrd "${ROOTFS}" -append "console=ttyS0 root=/dev/ram rdinit=/sbin/init nokaslr tp_printk trace_buf_size=1M ftrace=function_graph ftrace_graph_filter=pci_assign_irq" -nographic -device edu
#${QEMU_CMD} -kernel "${KERNEL_IMG}" -initrd "${ROOTFS}" -append "console=ttyS0 root=/dev/ram rdinit=/sbin/init nokaslr tp_printk trace_buf_size=1M ftrace=function_graph ftrace_graph_filter=pin_2_irq" -nographic -device edu

#${QEMU_CMD} -kernel "${KERNEL_IMG}" -initrd "${ROOTFS}" -append "console=ttyS0 root=/dev/ram rdinit=/sbin/init nokaslr apic=verbose debug" -nographic -device edu -hda disk.img -smp 2


create_br_tap
${QEMU_CMD} -kernel "${KERNEL_IMG}" -initrd "${ROOTFS}" -append "console=ttyS0 root=/dev/ram rdinit=/sbin/init nokaslr apic=verbose debug" -nographic -device edu -hda disk.img -smp 4 -m 256 -net nic -net tap,ifname=${TAP},script=no,downscript=no
clean_br_tap
