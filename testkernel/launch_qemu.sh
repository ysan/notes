#!/bin/bash

QEMU_DIR="${HOME}/prog/github_repos/qemu/"
QEMU_CMD="${QEMU_DIR}/build/qemu-system-x86_64 -L ${QEMU_DIR}/pc-bios"
KERNEL_IMG="${HOME}/prog/github_repos/linux/arch/x86/boot/bzImage"
ROOTFS="./rootfs.img"

#${QEMU_CMD} -kernel "${KERNEL_IMG}" -initrd "${ROOTFS}" -append "console=ttyS0 root=/dev/ram rdinit=/sbin/init nokaslr" -nographic -device edu
#${QEMU_CMD} -kernel "${KERNEL_IMG}" -initrd "${ROOTFS}" -append "console=ttyS0 root=/dev/ram rdinit=/sbin/init nokaslr" -nographic -device edu -gdb tcp::12345 -S

#${QEMU_CMD} -kernel "${KERNEL_IMG}" -initrd "${ROOTFS}" -append "console=ttyS0 root=/dev/ram rdinit=/sbin/init nokaslr dyndbg=\"file irqdomain.c +p; file irqdesc.c +p\"" -nographic -device edu

#${QEMU_CMD} -kernel "${KERNEL_IMG}" -initrd "${ROOTFS}" -append "console=ttyS0 root=/dev/ram rdinit=/sbin/init nokaslr tp_printk trace_buf_size=1M ftrace=function_graph ftrace_graph_filter=pci_assign_irq" -nographic -device edu
#${QEMU_CMD} -kernel "${KERNEL_IMG}" -initrd "${ROOTFS}" -append "console=ttyS0 root=/dev/ram rdinit=/sbin/init nokaslr tp_printk trace_buf_size=1M ftrace=function_graph ftrace_graph_filter=pin_2_irq" -nographic -device edu

${QEMU_CMD} -kernel "${KERNEL_IMG}" -initrd "${ROOTFS}" -append "console=ttyS0 root=/dev/ram rdinit=/sbin/init nokaslr apic=verbose debug" -nographic -device edu -hda disk.img -smp 2
