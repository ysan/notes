FROM ubuntu:22.04

ENV DEBIAN_FRONTEND noninteractive

RUN grep '^deb ' /etc/apt/sources.list | sed 's/^deb/deb-src/g' > /etc/apt/sources.list.d/deb-src.list

RUN apt-get update && apt-get install -y \
  curl \
  vim \
  tmux \
  git \
  sudo

RUN apt-get install -y bc bison flex libssl-dev make libc6-dev libncurses5-dev
RUN apt-get install -y build-essential gcc-arm-linux-gnueabihf

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
# KERNEL_VERSION="rpi-5.15.y"
# git clone -b ${KERNEL_VERSION} --depth=1 https://github.com/raspberrypi/linux linux-${KERNEL_VERSION}

# BUSYBOX_VERSION="1_36_1"
# git clone -b ${BUSYBOX_VERSION} --depth=1 https://github.com/mirror/busybox busybox-${BUSYBOX_VERSION}

# --- docker build ---
# IMAGE_NAME="kbuilder-${KERNEL_VERSION}"
# docker build -t ${IMAGE_NAME} --build-arg UID=$(id -u) --build-arg GID=$(id -g) .

# --- run daemon mode ---
# CONTAINER_NAME="${IMAGE_NAME}-dev"
# docker run -it -d --name ${CONTAINER_NAME} \
#   --mount type=bind,src=$(realpath linux-${KERNEL_VERSION}),dst=/home/builder/linux \
#   --mount type=bind,src=$(realpath busybox-${BUSYBOX_VERSION}),dst=/home/builder/busybox \
#   ${IMAGE_NAME}

# docker exec -it ${CONTAINER_NAME} /bin/bash
# docker exec -it -u builder -w /home/builder ${CONTAINER_NAME} /bin/bash

# --- build kernel ---
# cd linux
# export ARCH=arm
# export CROSS_COMPILE=arm-linux-gnueabihf-
# make mrproper
# make bcm2709_defconfig
# time make -j $(nproc)

# for unicam code reading
# time make -j $(nproc) zImage
# time make -j $(nproc) drivers/media/

# mkdir modules_output
# make modules_install INSTALL_MOD_PATH=./modules_output

# --- build busybox ---
# cd busybox
# export CROSS_COMPILE=arm-linux-gnueabihf-
# make defconfig
# make menuconfig
#  Settings -> [*] Build static binary (no shared libs)
# make
# make install

# --- create compile_commands.json at host ---
# cd linux-${KERNEL_VERSION}
# ./scripts/clang-tools/gen_compile_commands.py
# or
# ./scripts/gen_compile_commands.py
# sed -i "s|/home/builder/linux|$(realpath .)|g" compile_commands.json
