#!/bin/bash

if [ ! "$(id -u)" -eq 0  ]; then
	echo "Please run as root..."
	exit 1
fi

WORK_DIR="./rootfs_debootstrap"
TARGET_IMAGE="./rootfs_debootstrap.img"

if [ -z "${PREBUILTS_DIR}" ]; then
	echo "Please set PREBUILTS_DIR."
	echo "  ex. export PREBUILTS_DIR=\"path/to\" && $0"
	echo "  ex. PREBUILTS_DIR=\"path/to\" $0"
	exit 1
fi
if [ ! -d "${PREBUILTS_DIR}" ]; then
	echo "Not found prebuilts directory... [${PREBUILTS_DIR}]"
	exit 1
fi

echo "PREBUILTS_DIR: ${PREBUILTS_DIR}"
echo "WORK_DIR: ${WORK_DIR}"
echo "TARGET_IMAGE: ${TARGET_IMAGE}"

mkdir -p ${WORK_DIR}
if [ -d "${WORK_DIR}" ]; then
	echo "rm ${WORK_DIR}"
	rm -rf ${WORK_DIR}
	mkdir -p ${WORK_DIR}
fi

if [ -e "${TARGET_IMAGE}" ]; then
	echo "rm ${TARGET_IMAGE}"
	rm -f ${TARGET_IMAGE}
fi

set -e

cp -pr "${PREBUILTS_DIR}"/* ${WORK_DIR}
mount --bind /dev ${WORK_DIR}/dev
mount --bind /proc ${WORK_DIR}/proc
mount --bind /sys ${WORK_DIR}/sys
mount --bind /dev/pts ${WORK_DIR}/dev/pts

chroot ${WORK_DIR} /bin/bash <<'CHROOT_EOF'
. /etc/os-release && echo "VERSION_CODENAME:${VERSION_CODENAME}"

cat <<EOF > /etc/apt/sources.list
deb http://archive.ubuntu.com/ubuntu/ ${VERSION_CODENAME} main restricted universe multiverse
deb http://archive.ubuntu.com/ubuntu/ ${VERSION_CODENAME}-updates main restricted universe multiverse
deb http://archive.ubuntu.com/ubuntu/ ${VERSION_CODENAME}-security main restricted universe multiverse
EOF

apt-get update

apt-get install -y \
	locales \
	iproute2 \
	iputils-ping \
	net-tools \
	dnsutils \
	traceroute \
	vim \
	less \
	curl \
	wget \
	sudo \
	tmux \
	netcat-openbsd \
	telnet \
	pciutils \
	usbutils \
	openssh-server \
	openssh-client \
	git \
	xterm \
	strace \
	ca-certificates

locale-gen en_US.UTF-8
update-locale LANG=en_US.UTF-8

ln -sf /usr/share/zoneinfo/Asia/Tokyo /etc/localtime
echo "Asia/Tokyo" > /etc/timezone

echo "debootstrap" > /etc/hostname

cat > /etc/hosts <<EOF
127.0.0.1   localhost
127.0.0.1   debootstrap
EOF

# echo "nameserver 8.8.8.8" > /etc/resolv.conf
cat > /etc/systemd/resolved.conf << 'EOF'
[Resolve]
DNS=8.8.8.8 8.8.4.4 172.25.1.199 172.16.3.199
#                   ^^^^^ inernal DNS ^^^^^
EOF

echo "root:root" | chpasswd

mkdir -p /etc/systemd/system/serial-getty@ttyS0.service.d
cat > /etc/systemd/system/serial-getty@ttyS0.service.d/autologin.conf << 'EOF'
[Service]
ExecStart=
ExecStart=-/sbin/agetty --autologin root --noclear %I $TERM
EOF

apt-get clean
rm -rf /var/lib/apt/lists/*
rm -rf /tmp/*

CHROOT_EOF

echo "chroot settings -- done"

umount ${WORK_DIR}/dev/pts
umount ${WORK_DIR}/dev
umount ${WORK_DIR}/proc
umount ${WORK_DIR}/sys

cd ${WORK_DIR}
find . | cpio -o -H newc | gzip > ../${TARGET_IMAGE}
echo "archive rootfs -- done"

echo -e "\ncompleted\n"
