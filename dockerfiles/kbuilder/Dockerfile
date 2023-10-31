FROM ubuntu:22.04

ENV DEBIAN_FRONTEND noninteractive

RUN grep '^deb ' /etc/apt/sources.list | sed 's/^deb/deb-src/g' > /etc/apt/sources.list.d/deb-src.list

RUN apt-get update && apt-get install -y \
  curl \
  vim \
  tmux \
  sudo

RUN apt-get install -y git ccache fakeroot libncurses5-dev
RUN apt-get build-dep -y linux

RUN echo root:root | chpasswd

# add user
ARG UID=1000
ARG GID=1000
ARG USER=builder
ARG GROUP=builder
ARG PASS=builder
RUN groupadd -g $GID ${GROUP} && \
  useradd -m -s /bin/bash -u ${UID} -g ${GID} -G sudo ${USER} && \
  echo ${USER}:${PASS} | chpasswd

# switch user
USER ${USER}
WORKDIR /home/${USER}

ENV GIT_SSL_NO_VERIFY 1

CMD /bin/bash


# -----------------------------------------------------------------------------
# git clone -b v5.15 --depth=1 https://github.com/torvalds/linux
# git clone -b 1_36_0 --depth=1 https://github.com/mirror/busybox

# --- docker build ---
# docker build -t kbuilder --build-arg UID=$(id -u) --build-arg GID=$(id -g) .

# --- run daemon mode ---
# docker run -it -d --name kbuilder_con \
#   --mount type=bind,src=/path/to/linux,dst=/home/builder/linux \
#   --mount type=bind,src=/path/to/busybox,dst=/home/builder/busybox \
#   kbuilder

# docker exec -it kbuilder_con /bin/bash
# docker exec -it -u builder -w /home/builder kbuilder_con /bin/bash

# --- build kernel ---
# cd linux
# make defconfig
# make menuconfig
#  Kernel hacking -> [*] Kernel debugging
#  Kernel hacking -> Compile-time checks and compiler options -> [*] Compile the kernel with debug info
#  Kernel hacking -> printk and dmesg options -> [*] Enable dynamic printk() support
#  Kernel hacking -> Tracers -> [*] Kernel Function Tracer
#                               [*] Kernel Function Graph Tracer
#  Processor type and features -> [ ] EFI runtime service support
#                              -> [ ] Build a relocatable kernel
# time make -j $(nproc)

# --- create compile_commands.json at host ---
# cd linux
# ./scripts/clang-tools/gen_compile_commands.py
# or
# ./scripts/gen_compile_commands.py

# --- if need build busybox ---
# cd busybox
# make defconfig
# make menuconfig
#  Settings -> [*] Build static binary (no shared libs)
# make
# make install

# --- create rootfs (busybox simple) at host ---
# mkdir -p rootfs/{proc,bin,sbin,dev,sys}
# cd rootfs/bin
# curl -LOks https://busybox.net/downloads/binaries/1.35.0-x86_64-linux-musl/busybox
# chmod 755 ./busybox
# ln -s busybox sh
# ln -s busybox mount
# cd ../sbin
# cat <<EOF > init
# > #!/bin/sh
# > mount -t proc none /proc
# > mount -t sysfs none /sys
# > exec /bin/sh
# > EOF
# chmod 755 init
# cd ../dev
# sudo mknod console c 5 1
# sudo mknod null c 1 3
# cd ..
# find ./ | cpio -o -H newc | gzip > ../rootfs.img

# --- launch qemu at host ---
# qemu-system-x86_64 -kernel /path/to/bzImage -initrd /path/to/rootfs.img -append "console=ttyS0 rdinit=/sbin/init" -nographic
# finish -> ctrl+a x

# --- create rootfs (busybox full) at host ---
# mkdir -p rootfs/{proc,dev,sys,run,etc/init.d}
# cp -pr /path/to/busybox/_install/* rootfs/
# cd rootfs/dev
# sudo mknod console c 5 1
# sudo mknod null c 1 3
# sudo mknod -m 666 tty c 5 0
# sudo mknod ram b 1 0
# cd ../etc/init.d
# cat <<EOF > rcS
# > #!/bin/sh
# > mount -v --bind /dev /dev
# > mount -v --bind /dev/pts /dev/pts
# > mount -t proc none /proc
# > mount -t sysfs none /sys
# > mount -t tmpfs none /run
# > mount -t debugfs none /sys/kernel/debug
# > /sbin/mdev -s
# > EOF
# chmod 755 rcS
# cd ../..
# find ./ | cpio -o -H newc | gzip > ../rootfs.img

# --- launch qemu + gdb at host ---
# qemu-system-x86_64 -kernel /path/to/bzImage -initrd /path/to/rootfs.img -append "console=ttyS0 rdinit=/sbin/init root=/dev/ram" -nographic -gdb tcp::12345 -S
# gdb
# target remote localhost:12345
# symbol-file /path/to/linux/vmlinux
# directory /path/to/linux
# b start_kernel
# layout split
# c
