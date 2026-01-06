#!/bin/bash

if [ ! "$(id -u)" -eq 0  ]; then
	echo "Please run as root..."
	exit 1
fi

WORK_DIR="./rootfs"
TARGET_IMAGE="./rootfs.img"

if [ -z "${PREBUILTS_DIR}" ]; then
	echo "Please set PREBUILTS_DIR."
	echo "  ex. export PREBUILTS_DIR=\"path/to\" && $0"
	echo "  ex. PREBUILTS_DIR=\"path/to\" $0"
	exit 1
fi
if [ ! -d "${PREBUILTS_DIR}" ]; then
	echo "Not found prebuilt rootfs directory... [${PREBUILTS_DIR}]"
	exit 1
fi

echo "PREBUILTS_DIR: ${PREBUILTS_DIR}"
echo "WORK_DIR: ${WORK_DIR}"
echo "TARGET_IMAGE: ${TARGET_IMAGE}"

REAL_PATH=$(realpath ${WORK_DIR})
if [ "${REAL_PATH}" == "/" ]; then
	echo "Please use a target directory other than /."
	exit 1
fi
if [ -d "${WORK_DIR}" ]; then
	echo "rm ${WORK_DIR}"
	rm -rf ${WORK_DIR}
	mkdir -p ${WORK_DIR}
fi

REAL_PATH=$(realpath ${TARGET_IMAGE})
if [ "${REAL_PATH}" == "/" ]; then
	echo "Please use a target name other than /."
	exit 1
fi
if [ -e "${TARGET_IMAGE}" ]; then
	echo "rm ${TARGET_IMAGE}"
	rm -f ${TARGET_IMAGE}
fi

set -e

mkdir -p ${WORK_DIR}/{proc,dev,sys,run,etc/init.d,mnt}
cp -pr "${PREBUILTS_DIR}"/* ${WORK_DIR}
mknod ${WORK_DIR}/dev/console c 5 1
mknod ${WORK_DIR}/dev/null c 1 3
mknod ${WORK_DIR}/dev/ram b 1 0
mknod ${WORK_DIR}/dev/zero c 1 5
mknod ${WORK_DIR}/dev/urandom c 1 9
mknod ${WORK_DIR}/dev/ttyS0 c 4 64
echo "append directories -- done"

cat <<EOF > ${WORK_DIR}/etc/init.d/rcS
#!/bin/sh
mount -t proc none /proc
mount -t sysfs none /sys
mount -t tmpfs none /run
mount -t debugfs none /sys/kernel/debug
mount -t tmpfs cgroup /sys/fs/cgroup
mkdir /sys/fs/cgroup/cpu
mkdir /sys/fs/cgroup/devices
mount -t cgroup -o cpu cgroup /sys/fs/cgroup/cpu
mount -t cgroup -o devices cgroup /sys/fs/cgroup/devices
/sbin/mdev -s
EOF
chmod 755 ${WORK_DIR}/etc/init.d/rcS
echo "create ${WORK_DIR}/etc/init.d/rcS -- done"

cd ${WORK_DIR}
find . | cpio -o -H newc | gzip > ../${TARGET_IMAGE}
echo "archive rootfs -- done"

echo -e "\ncompleted\n"
