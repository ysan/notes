FROM ubuntu:22.04

ENV DEBIAN_FRONTEND noninteractive

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

# uboot build dependency packages
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


# -----------------------------------------------------------------------------
# --- clone TF-A at host ---
# git clone https://github.com/ARM-software/arm-trusted-firmware
# git clone -b v2.10-stm32mp-r1.2 --depth=1 https://github.com/STMicroelectronics/arm-trusted-firmware.git TF-A-v2.10-stm32mp-r1.2

# --- clone OP-TEE at host ---
# git clone https://github.com/OP-TEE/optee_os.git
# git clone -b 4.0.0-stm32mp --depth=1 https://github.com/STMicroelectronics/optee_os.git optee-4.0.0-stm32mp

# --- clone u-boot at host ---
# git clone https://github.com/u-boot/u-boot
# git clone -b v2023.10-stm32mp-r1.2 --depth=1 https://github.com/STMicroelectronics/u-boot u-boot-v2023.10-stm32mp-r1.2

# --- docker build ---
# docker build -t ubuilder --build-arg UID=$(id -u) --build-arg GID=$(id -g) .

# --- run daemon mode ---
# docker run -it -d --name ubuilder_con \
#   --mount type=bind,src=/path/to/TF-A-xxx,dst=/home/builder/TF-A \
#   --mount type=bind,src=/path/to/optee-xxx,dst=/home/builder/optee \
#   --mount type=bind,src=/path/to/u-boot-xxx,dst=/home/builder/u-boot \
#   ubuilder

# docker exec -it ubuilder_con /bin/bash
# docker exec -it -u builder -w /home/builder ubuilder_con /bin/bash


# --- build uboot ---
# (https://wiki.st.com/stm32mpu/wiki/STM32MP15_U-Boot)
# cd u-boot
# export CROSS_COMPILE=arm-linux-gnueabihf-
# make mrproper
# make stm32mp15_defconfig

# early uart
# ./scripts/config -e CONFIG_DEBUG_UART
# ./scripts/config -e CONFIG_DEBUG_UART_STM32
# ./scripts/config --set-val CONFIG_DEBUG_UART_BASE 0x40010000
# ./scripts/config -e CONFIG_DEBUG_UART_ANNOUNCE

# optee not support
# make menuconfig
#  Device Drivers ->
     [ ] Trusted Execution Environment support

# time make -j $(nproc) DEVICE_TREE=stm32mp157d-dk1 KCFLAGS=-DDEBUG all


# --- build optee ---
# cd optee
# export CROSS_COMPILE=arm-linux-gnueabihf-
# time make -j $(nproc) PLATFORM=stm32mp1 CFG_EMBED_DTB_SOURCE_FILE=stm32mp157d-dk1.dts CFG_ARM32_core=y CFG_WITH_LPAE=n

# --- build TF-A ---
# (https://wiki.st.com/stm32mpu/wiki/TF-A_BL2_overview)
# (https://wiki.st.com/stm32mpu/wiki/How_to_configure_TF-A_BL2)
# (https://wiki.st.com/stm32mpu/wiki/How_to_configure_TF-A_FIP)
# (https://wiki.st.com/stm32mpu/wiki/Boot_chain_overview#STM32MP15_boot_chain)
# (https://trustedfirmware-a.readthedocs.io/en/lts-v2.10/plat/st/stm32mp1.html#build-instructions)
# cd TF-A
# export CROSS_COMPILE=arm-linux-gnueabihf-
# make distclean

# ## sp_min ##  --> not supported
# create TF-A
# make ARM_ARCH_MAJOR=7 ARCH=aarch32 STM32MP_SDMMC=1 PLAT=stm32mp1 AARCH32_SP=sp_min DTB_FILE_NAME=stm32mp157d-dk1.dtb DEBUG=1 LOG_LEVEL=40 all
# create fip
# make ARM_ARCH_MAJOR=7 ARCH=aarch32 STM32MP_SDMMC=1 PLAT=stm32mp1 AARCH32_SP=sp_min DTB_FILE_NAME=stm32mp157d-dk1.dtb BL33=../u-boot/u-boot-nodtb.bin BL33_CFG=../u-boot/u-boot.dtb DEBUG=1 fip

# ## optee ##
# create TF-A
# make ARM_ARCH_MAJOR=7 ARCH=aarch32 STM32MP_SDMMC=1 PLAT=stm32mp1 AARCH32_SP=optee DTB_FILE_NAME=stm32mp157d-dk1.dtb DEBUG=1 LOG_LEVEL=40 all
# create fip
# make ARM_ARCH_MAJOR=7 ARCH=aarch32 STM32MP_SDMMC=1 PLAT=stm32mp1 AARCH32_SP=optee DTB_FILE_NAME=stm32mp157d-dk1.dtb BL33=../u-boot/u-boot-nodtb.bin BL33_CFG=../u-boot/u-boot.dtb BL32=../optee/out/arm-plat-stm32mp1/core/tee-header_v2.bin BL32_EXTRA1=../optee/out/arm-plat-stm32mp1/core/tee-pager_v2.bin BL32_EXTRA2=../optee/out/arm-plat-stm32mp1/core/tee-pageable_v2.bin DEBUG=1 LOG_LEVEL=40 fip

# check fip
# ./tools/fiptool/fiptool info build/stm32mp1/debug/fip.bin
# check bl32 dtb
# fdtget -t s build/stm32mp1/debug/bl32.bin / compatible


# --- write SD image at host ---
# (https://wiki.st.com/stm32mpu/wiki/STM32CubeProgrammer_flashlayout)
# (https://wiki.st.com/stm32mpu/wiki/How_to_populate_the_SD_card_with_dd_command)
# git clone https://github.com/STMicroelectronics/meta-st-openstlinux
# cd meta-st-stm32mp
# git checkout scarthgap
# sample.tsv
#Opt    Id      Name    Type    IP      Offset  Binary
P       0x04    fsbl1   Binary  mmc0    0x00004400      tf-a-stm32mp157d-dk1.stm32
P       0x05    fsbl2   Binary  mmc0    0x00044400      tf-a-stm32mp157d-dk1.stm32
P       0x08    fip-a   FIP     mmc0    0x00104400      fip.bin
PED     0x09    fip-b   FIP     mmc0    0x00504400      none
# ./recipes-devtools/sdcard-raw-tools/sdcard-raw-tools/create_sdcard_from_flashlayout.sh sample.tsv
# sudo dd ...


# --- create compile_commands.json at host ---
# curl -LOk https://raw.githubusercontent.com/u-boot/u-boot/refs/heads/master/scripts/gen_compile_commands.py
# cd u-boot
# /path/to/gen_compile_commands.py

