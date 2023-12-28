#!/bin/bash
# usage: create_disk.sh [args...]
#    args: Specify the files and directories to be stored in the disk image.
#          (If a disk image already exists, add it.)

if [ ! "$(id -u)" -eq 0  ]; then
	echo "Please run as root..."
	exit 1
fi

echo "rundir: $(dirname $0)"

RUN_DIR="$(dirname $0)"
DISK_BS=1M
DISK_BS_CNT=10
DISK_IMG=${RUN_DIR}/disk.img
MOUNT_POINT=${RUN_DIR}/disk

REAL_PATH=$(realpath "${MOUNT_POINT}")
mount | grep "${REAL_PATH}"
if [ $? -eq 0 ]; then
	umount "${MOUNT_POINT}"
	echo "umount ${MOUNT_POINT} -- done"
fi

set -e

if [ ! -e "${DISK_IMG}" ]; then
	dd if=/dev/zero of="${DISK_IMG}" bs="${DISK_BS}" count="${DISK_BS_CNT}"
	mkfs.ext4 "${DISK_IMG}"
	chmod 777 "${DISK_IMG}"
fi

mkdir -p "${MOUNT_POINT}"

mount -o loop "${DISK_IMG}" "${MOUNT_POINT}"
echo "mount -o loop ${DISK_IMG} ${MOUNT_POINT} -- done"

for ARG in "${@}"; do
	echo "arg: ${ARG}"
	cp -pr "${ARG}" "${MOUNT_POINT}"
done

ls -ltr "${MOUNT_POINT}"

umount "${MOUNT_POINT}"
echo "umount ${MOUNT_POINT} -- done"

echo -e "\ncompleted\n"
