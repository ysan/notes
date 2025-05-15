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
# --- clone TF-A ---
# git clone -b lts-v2.10.16 --depth=1 https://github.com/ARM-software/arm-trusted-firmware TF-A-lts-v2.10.16
# git clone -b v2.10-stm32mp-r1.2 --depth=1 https://github.com/STMicroelectronics/arm-trusted-firmware.git TF-A-v2.10-stm32mp-r1.2

# --- clone u-boot ---
# git clone -b v2025.04 --depth=1 https://github.com/u-boot/u-boot u-boot-v2025.04
# git clone -b v2023.10-stm32mp-r1.2 --depth=1 https://github.com/STMicroelectronics/u-boot u-boot-v2023.10-stm32mp-r1.2

# --- docker build ---
# docker build -t ubuilder --build-arg UID=$(id -u) --build-arg GID=$(id -g) .

# --- run daemon mode ---
# docker run -it -d --name ubuilder_con \
#   --mount type=bind,src=/path/to/TF-A-xxx,dst=/home/builder/TF-A \
#   --mount type=bind,src=/path/to/u-boot-xxx,dst=/home/builder/u-boot \
#   ubuilder

# docker exec -it ubuilder_con /bin/bash
# docker exec -it -u builder -w /home/builder ubuilder_con /bin/bash

# --- build uboot ---
# (https://wiki.st.com/stm32mpu/wiki/STM32MP15_U-Boot)
# cd u-boot
# export CROSS_COMPILE=arm-linux-gnueabihf-
# make stm32mp15_trusted_defconfig
# time make -j $(nproc) DEVICE_TREE=stm32mp157d-dk1 all

# --- build TF-A ---
# (https://wiki.st.com/stm32mpu/wiki/TF-A_BL2_overview)
# (https://wiki.st.com/stm32mpu/wiki/How_to_configure_TF-A_BL2)
# (https://wiki.st.com/stm32mpu/wiki/How_to_configure_TF-A_FIP)
# (https://trustedfirmware-a.readthedocs.io/en/lts-v2.10/plat/st/stm32mp1.html#build-instructions)
# cd TF-A
# export CROSS_COMPILE=arm-linux-gnueabihf-
# rm -rf build
# make ARM_ARCH_MAJOR=7 ARCH=aarch32 STM32MP_SDMMC=1 PLAT=stm32mp1 AARCH32_SP=sp_min DTB_FILE_NAME=stm32mp157d-dk1.dtb BL33=../u-boot/u-boot-nodtb.bin BL33_CFG=../u-boot/u-boot.dtb all fip

# --- write SD image ---


# --- create compile_commands.json at host ---
# cd u-boot
# ./scripts/gen_compile_commands.py

