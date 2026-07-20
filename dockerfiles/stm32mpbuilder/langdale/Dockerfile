FROM ubuntu:22.04

ENV DEBIAN_FRONTEND noninteractive

RUN grep '^deb ' /etc/apt/sources.list | sed 's/^deb/deb-src/g' > /etc/apt/sources.list.d/deb-src.list

RUN apt-get update && apt-get install -y software-properties-common
RUN apt-get update && apt-get install -y \
  curl \
  vim \
  tmux \
  bear \
  git \
  sudo \
  build-essential \
  gcc-arm-linux-gnueabihf

# TF-A / OP-TEE / U-Boot build dependency packages
# https://docs.u-boot.org/en/latest/build/gcc.html#dependencies
# (ignore lz4 python3-asteval python3-sphinxcontrib.apidoc)
RUN apt-get update && apt-get install -y \
  bc bison build-essential coccinelle \
  device-tree-compiler dfu-util efitools flex gdisk graphviz imagemagick \
  liblz4-tool libgnutls28-dev libguestfs-tools libncurses-dev \
  libpython3-dev libsdl2-dev libssl-dev lzma lzma-alone openssl \
  pkg-config python3 python3-coverage python3-filelock \
  python3-pkg-resources python3-pycryptodome python3-pyelftools \
  python3-pytest python3-pytest-xdist \
  python3-sphinx-rtd-theme python3-subunit python3-testtools \
  python3-virtualenv swig uuid-dev

# kernel build dependency packages
RUN apt-get update && apt-get install -y \
  libc6-dev libncurses5-dev make u-boot-tools

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

# user sudo no passwd
RUN echo "builder ALL=NOPASSWD: ALL" > /etc/sudoers.d/container_users

# switch user
USER ${USER}
WORKDIR /home/${USER}

ENV GIT_SSL_NO_VERIFY 1

CMD /bin/bash


# =============================================================================
# STM32MP157D-DK1 ブートチェーン (TF-A -> OP-TEE -> U-Boot -> Linux -> busybox rootfs)
# 全てmeta-st-stm32mpの"langdale"ブランチのバージョンに揃えます。
#
#   component   PV (release)            branch (devupstream)
#   ----------  ----------------------  ----------------------
#   TF-A        v2.6-stm32mp-r1         v2.6-stm32mp
#   OP-TEE      3.16.0-stm32mp-r1       3.16.0-stm32mp
#   U-Boot      v2021.10-stm32mp-r1     v2021.10-stm32mp
#   Linux       5.15.24-stm32mp1-r1     v5.15-stm32mp
#
# ST公式のREADME.HOW_TO.txt (各recipeの##ARCHIVER_ST_BRANCH##) と同じく、branch名を直接cloneします。
# =============================================================================


# -----------------------------------------------------------------------------
# --- host側でこのDockerfileと同じ場所に全リポジトリをcloneします ---
# git clone -b v2.6-stm32mp --depth=1 https://github.com/STMicroelectronics/arm-trusted-firmware.git TF-A-v2.6-stm32mp-r1

# git clone -b 3.16.0-stm32mp --depth=1 https://github.com/STMicroelectronics/optee_os.git optee-3.16.0-stm32mp-r1

# git clone -b v2021.10-stm32mp --depth=1 https://github.com/STMicroelectronics/u-boot.git u-boot-v2021.10-stm32mp-r1

# git clone -b v5.15-stm32mp --depth=1 https://github.com/STMicroelectronics/linux.git linux-v5.15-stm32mp-r1

# git clone -b langdale --depth=1 https://github.com/STMicroelectronics/meta-st-stm32mp meta-st-stm32mp-langdale

# git clone -b 1_36_1 --depth=1 https://github.com/mirror/busybox busybox-1_36_1

# --- docker build ---
# docker build -t stm32mpbuilder-langdale --build-arg UID=$(id -u) --build-arg GID=$(id -g) .

# --- daemonモードで起動 ---
# docker run -it -d --name stm32mpbuilder-langdale-dev \
#   --mount type=bind,src=$(pwd)/TF-A-v2.6-stm32mp-r1,dst=/home/builder/TF-A \
#   --mount type=bind,src=$(pwd)/optee-3.16.0-stm32mp-r1,dst=/home/builder/optee \
#   --mount type=bind,src=$(pwd)/u-boot-v2021.10-stm32mp-r1,dst=/home/builder/u-boot \
#   --mount type=bind,src=$(pwd)/linux-v5.15-stm32mp-r1,dst=/home/builder/linux \
#   --mount type=bind,src=$(pwd)/meta-st-stm32mp-langdale,dst=/home/builder/meta-st-stm32mp \
#   --mount type=bind,src=$(pwd)/busybox-1_36_1,dst=/home/builder/busybox \
#   stm32mpbuilder-langdale

# docker exec -it stm32mpbuilder-langdale-dev /bin/bash


# --- build u-boot ---
# (https://wiki.st.com/stm32mpu/wiki/STM32MP15_U-Boot)
# cd u-boot
# export CROSS_COMPILE=arm-linux-gnueabihf-
# make mrproper
# make stm32mp15_defconfig

# early uart(早期デバッグ用シリアル出力)
# ./scripts/config -e CONFIG_DEBUG_UART
# ./scripts/config -e CONFIG_DEBUG_UART_STM32
# ./scripts/config --set-val CONFIG_DEBUG_UART_BASE 0x40010000
# ./scripts/config -e CONFIG_DEBUG_UART_ANNOUNCE
#
# 古いU-Bootにはscripts/config(kernelにはある編集用スクリプト)が同梱されておらず上記は動きません。
# -> scripts/kconfig/merge_config.sh で代用できます。
# cat <<EOF > early_uart.config
# CONFIG_DEBUG_UART=y
# CONFIG_DEBUG_UART_STM32=y
# CONFIG_DEBUG_UART_BASE=0x40010000
# CONFIG_DEBUG_UART_ANNOUNCE=y
# EOF
# ./scripts/kconfig/merge_config.sh -m -r .config early_uart.config

# debugビルド時の対策: "Failed to persist EFI variables" エラー回避
# make menuconfig
#  Library routines ->
#    [ ] Support running UEFI applications
#
# ... または flashlayout に EFI system partition (ESP) を追加します

# time make -j $(nproc) DEVICE_TREE=stm32mp157d-dk1 all
# debugビルド
# time make -j $(nproc) DEVICE_TREE=stm32mp157d-dk1 KCFLAGS=-DDEBUG all


# --- build optee ---
# cd optee
# export CROSS_COMPILE=arm-linux-gnueabihf-
# make clean
# rm -rf out
# time make -j $(nproc) PLATFORM=stm32mp1 CFG_EMBED_DTB_SOURCE_FILE=stm32mp157d-dk1.dts CFG_ARM32_core=y CFG_WITH_LPAE=n CFG_CORE_PAGE_TAG_AND_IV=n
# debugビルド
# time make -j $(nproc) PLATFORM=stm32mp1 CFG_EMBED_DTB_SOURCE_FILE=stm32mp157d-dk1.dts CFG_ARM32_core=y CFG_WITH_LPAE=n CFG_CORE_PAGE_TAG_AND_IV=n CFG_TEE_CORE_DEBUG=y


# --- build TF-A ---
# (https://wiki.st.com/stm32mpu/wiki/TF-A_BL2_overview)
# (https://wiki.st.com/stm32mpu/wiki/How_to_configure_TF-A_BL2)
# (https://wiki.st.com/stm32mpu/wiki/How_to_configure_TF-A_FIP)
# (https://wiki.st.com/stm32mpu/wiki/Boot_chain_overview#STM32MP15_boot_chain)
# (https://trustedfirmware-a.readthedocs.io/en/lts-v2.6/plat/st/stm32mp1.html#build-instructions)
# cd TF-A
# export CROSS_COMPILE=arm-linux-gnueabihf-
# make distclean

# ## sp_min ##  --> 非対応だった
# (https://wiki.st.com/stm32mpu/wiki/TF-A_overview#BL32)
# TF-A作成
# make ARM_ARCH_MAJOR=7 ARCH=aarch32 STM32MP_SDMMC=1 PLAT=stm32mp1 AARCH32_SP=sp_min DTB_FILE_NAME=stm32mp157d-dk1.dtb all
# debugビルド
# make ARM_ARCH_MAJOR=7 ARCH=aarch32 STM32MP_SDMMC=1 PLAT=stm32mp1 AARCH32_SP=sp_min DTB_FILE_NAME=stm32mp157d-dk1.dtb DEBUG=1 LOG_LEVEL=40 STM32MP_EARLY_CONSOLE=1 all
# fip作成
# make ARM_ARCH_MAJOR=7 ARCH=aarch32 STM32MP_SDMMC=1 PLAT=stm32mp1 AARCH32_SP=sp_min DTB_FILE_NAME=stm32mp157d-dk1.dtb BL33=../u-boot/u-boot-nodtb.bin BL33_CFG=../u-boot/u-boot.dtb fip

# ## BL32 optee版 ##
# TF-A作成
# make ARM_ARCH_MAJOR=7 ARCH=aarch32 STM32MP_SDMMC=1 PLAT=stm32mp1 AARCH32_SP=optee DTB_FILE_NAME=stm32mp157d-dk1.dtb all
# debugビルド
# make ARM_ARCH_MAJOR=7 ARCH=aarch32 STM32MP_SDMMC=1 PLAT=stm32mp1 AARCH32_SP=optee DTB_FILE_NAME=stm32mp157d-dk1.dtb DEBUG=1 LOG_LEVEL=40 STM32MP_EARLY_CONSOLE=1 all

# fip作成
# make ARM_ARCH_MAJOR=7 ARCH=aarch32 STM32MP_SDMMC=1 PLAT=stm32mp1 AARCH32_SP=optee DTB_FILE_NAME=stm32mp157d-dk1.dtb BL33=../u-boot/u-boot-nodtb.bin BL33_CFG=../u-boot/u-boot.dtb BL32=../optee/out/arm-plat-stm32mp1/core/tee-header_v2.bin BL32_EXTRA1=../optee/out/arm-plat-stm32mp1/core/tee-pager_v2.bin BL32_EXTRA2=../optee/out/arm-plat-stm32mp1/core/tee-pageable_v2.bin fip

# fip確認
# ./tools/fiptool/fiptool info build/stm32mp1/release/fip.bin
# bl32のdtb確認
# fdtget -t s build/stm32mp1/release/bl32.bin / compatible


# --- buid kernel ---
# cd linux
# export CROSS_COMPILE=arm-linux-gnueabihf-
# make mrproper

# ベースdefconfig
# make ARCH=arm multi_v7_defconfig

# fragment-01/02: カーネルソース側(STのpatchで追加されている)
# scripts/kconfig/merge_config.sh -m -r .config arch/arm/configs/fragment-01-multiv7_cleanup.config
# scripts/kconfig/merge_config.sh -m -r .config arch/arm/configs/fragment-02-multiv7_addons.config

# fragment-03〜06: meta-st-stm32mp側
# META=../meta-st-stm32mp/recipes-kernel/linux/linux-stm32mp/5.15
# scripts/kconfig/merge_config.sh -m -r .config ${META}/fragment-03-systemd.config
# scripts/kconfig/merge_config.sh -m -r .config ${META}/fragment-04-modules.config
# scripts/kconfig/merge_config.sh -m -r .config ${META}/fragment-05-signature.config
# scripts/kconfig/merge_config.sh -m -r .config ${META}/fragment-06-smp.config  ## stm32mp157の場合dualコアなのでこのパッチは不要

# 依存関係の解決
# yes '' | make ARCH=arm oldconfig

# time make -j $(nproc) ARCH=arm uImage dtbs LOADADDR=0xC2000040


# --- build busybox ---
# cd busybox
# export CROSS_COMPILE=arm-linux-gnueabihf-
# make mrproper
# make defconfig
# make menuconfig
#  Settings -> [*] Build static binary (no shared libs)
# make
# make install


# --- host側でbootfs.ext4イメージを作成 ---
# Yoctoが自動的にやる処理の一部(bootfsパーティションのサイズ等)を真似て
# 手作業(dd/mkfs.ext4/cp)で再現しています。64MはYocto公式のbootfsパーティションサイズに合わせた値です。
# dd if=/dev/zero of=bootfs.ext4 bs=1M count=64
# mkfs.ext4 -L bootfs bootfs.ext4
# mkdir -p /tmp/bootfs_mnt
# sudo mount -o loop bootfs.ext4 /tmp/bootfs_mnt

# sudo cp -p ./linux-v5.15-stm32mp-r1/arch/arm/boot/uImage /tmp/bootfs_mnt/
# sudo cp -p ./linux-v5.15-stm32mp-r1/arch/arm/boot/dts/st*.dtb /tmp/bootfs_mnt/

# extlinux.conf の作成
# sudo mkdir -p /tmp/bootfs_mnt/extlinux
# cat <<EOF | sudo tee /tmp/bootfs_mnt/extlinux/extlinux.conf
# TIMEOUT 20
# LABEL linux
#     KERNEL /uImage
#     FDTDIR ../
#     APPEND root=/dev/mmcblk0p4 rootwait rw console=ttySTM0,115200
# EOF

# sudo umount /tmp/bootfs_mnt


# --- host側でrootfs.ext4イメージを作成 ---
# こちらはYocto側のrootfsサイズ(783M前後)には合わせていません。
# busyboxのみの最小構成なので200Mあれば十分という任意のサイズです。
# dd if=/dev/zero of=rootfs.ext4 bs=1M count=200
# mkfs.ext4 rootfs.ext4
# mkdir -p /tmp/rootfs_mnt
# sudo mount -o loop rootfs.ext4 /tmp/rootfs_mnt

# sudo mkdir -p /tmp/rootfs_mnt/{bin,sbin,etc,proc,sys,dev,tmp,root,usr/bin,usr/sbin,run}
# sudo cp -pr ./busybox-1_36_1/_install/* /tmp/rootfs_mnt
# sudo mknod /tmp/rootfs_mnt/dev/console c 5 1
# sudo mknod /tmp/rootfs_mnt/dev/null c 1 3

# sudo mkdir -p /tmp/rootfs_mnt/etc/init.d
# cat <<EOF | sudo tee /tmp/rootfs_mnt/etc/init.d/rcS
# #!/bin/sh
# mount -t proc none /proc
# mount -t sysfs none /sys
# mount -t tmpfs none /run
# mount -t debugfs none /sys/kernel/debug
# /sbin/mdev -s
# EOF
# sudo chmod 755 /tmp/rootfs_mnt/etc/init.d/rcS

# sudo umount /tmp/rootfs_mnt


# --- host側でSDカードに書き込む ---
# (https://wiki.st.com/stm32mpu/wiki/STM32CubeProgrammer_flashlayout)
# (https://wiki.st.com/stm32mpu/wiki/How_to_populate_the_SD_card_with_dd_command)
# cd meta-st-stm32mp
# git checkout langdale
# cp -p ./files/flashlayouts/FlashLayout_sdcard_stm32mp157f-ev1_sample.tsv test.tsv
# test.tsvを以下のように修正  TFAとfipはdebugビルド時はパスが変わるので注意
# #Opt    Id      Name    Type        IP    Offset      Binary
# P       0x04    fsbl1   Binary      mmc0  0x00004400  ./TF-A-v2.6-stm32mp-r1/build/stm32mp1/release/tf-a-stm32mp157d-dk1.stm32
# P       0x05    fsbl2   Binary      mmc0  0x00044400  ./TF-A-v2.6-stm32mp-r1/build/stm32mp1/release/tf-a-stm32mp157d-dk1.stm32
# P       0x08    fip-a   FIP         mmc0  0x00104400  ./TF-A-v2.6-stm32mp-r1/build/stm32mp1/release/fip.bin
# PED     0x09    fip-b   FIP         mmc0  0x00504400  none
# P       0x10    bootfs  System      mmc0  0x00984400  ./bootfs.ext4
# P       0x12    rootfs  FileSystem  mmc0  0x05984400  ./rootfs.ext4

# flashlayout.shを実行
# ./meta-st-stm32mp-langdale/recipes-devtools/sdcard-raw-tools/sdcard-raw-tools/create_sdcard_from_flashlayout.sh test.tsv
# flashlayout.shがddコマンドをサジェストしてくれる
# sudo dd ...

# fip-a パーティションだけ書き換える例
# sudo dd if=fip.bin of=/dev/mmcblk0p3 bs=1M conv=fdatasync status=progress


# --- host側でcompile_commands.jsonを作成 ---
# uboot用 本家ubootに置いてあるものを使います
# curl -LOk https://raw.githubusercontent.com/u-boot/u-boot/refs/heads/master/scripts/gen_compile_commands.py
# cd u-boot
# chmod +x ../gen_compile_commands.py
# ../gen_compile_commands.py

# linux用
# cd linux
# ./scripts/clang-tools/gen_compile_commands.py
# or
# ./scripts/gen_compile_commands.py
# sed -i "s|/home/builder/linux|$(realpath .)|g" compile_commands.json


# --- host側でtftpdを起動 ---
# inetd経由のtftpdはipv6で待ち受けてしまうため、以下のコマンドでipv4で起動します
# sudo in.tftpd -L -4 -s $(pwd)
