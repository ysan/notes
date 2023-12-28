#!/bin/bash

if [ ! "$(id -u)" -eq 0  ]; then
	echo "Please run as root..."
	exit 1
fi

PREBUILT_ROOTFS=${HOME}/prog/github_repos/busybox/_install
TARGET_DIR=./rootfs
TARGET_NAME=rootfs.img

if [ ! -d "${PREBUILT_ROOTFS}" ]; then
	echo "Not found prebuilt rootfs directory... [${PREBUILT_ROOTFS}]"
	exit 1
fi

REAL_PATH=$(realpath ${TARGET_DIR})
if [ "${REAL_PATH}" == "/" ]; then
	echo "Please use a target directory other than /."
	exit 1
fi
if [ -d "${TARGET_DIR}" ]; then
	echo "rm ${TARGET_DIR}"
	rm -rf ${TARGET_DIR}
	mkdir -p ${TARGET_DIR}
fi

REAL_PATH=$(realpath ${TARGET_NAME})
if [ "${REAL_PATH}" == "/" ]; then
	echo "Please use a target name other than /."
	exit 1
fi
if [ -e "${TARGET_NAME}" ]; then
	echo "rm ${TARGET_NAME}"
	rm -f ${TARGET_NAME}
fi

set -e

mkdir -p ${TARGET_DIR}/{proc,dev,sys,run,etc/init.d,mnt}
cp -pr "${PREBUILT_ROOTFS}"/* ${TARGET_DIR}
mknod ${TARGET_DIR}/dev/console c 5 1
mknod ${TARGET_DIR}/dev/null c 1 3
mknod ${TARGET_DIR}/dev/ram b 1 0
mknod ${TARGET_DIR}/dev/zero c 1 5
mknod ${TARGET_DIR}/dev/urandom c 1 9
mknod ${TARGET_DIR}/dev/ttyS0 c 4 64
echo "append directories -- done"

cat <<EOF > ${TARGET_DIR}/etc/init.d/rcS
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
chmod 755 ${TARGET_DIR}/etc/init.d/rcS
echo "create ${TARGET_DIR}/etc/init.d/rcS -- done"

cd ${TARGET_DIR}
find . | cpio -o -H newc | gzip > ../${TARGET_NAME}
echo "archive rootfs -- done"

echo -e "\ncompleted\n"
