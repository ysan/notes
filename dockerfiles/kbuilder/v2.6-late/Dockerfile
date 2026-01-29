FROM --platform=linux/386 debian:squeeze

RUN echo "deb http://archive.debian.org/debian/ squeeze main contrib non-free" > /etc/apt/sources.list
RUN echo 'Acquire::Check-Valid-Until "false";' > /etc/apt/apt.conf.d/90ignore-release-date

RUN apt-get update && \
    apt-get install -y --force-yes --allow-unauthenticated \
    libc6=2.11.3-4 \
    libc6-dev=2.11.3-4 \
    libc-bin=2.11.3-4 \
    build-essential \
    gcc \
    g++ \
    make \
    libncurses5-dev \
    wget \
    bzip2 \
    vim \
    bc

RUN echo root:root | chpasswd

# add user
ARG UID=1000
ARG GID=1000
ARG USER=builder
ARG GROUP=builder
ARG PASS=builder
RUN groupadd -g $GID ${GROUP} && \
  useradd    -s /bin/bash -u ${UID} -g ${GID} -G sudo ${USER} && \
  echo ${USER}:${PASS} | chpasswd

# switch user
USER ${USER}
WORKDIR /home/${USER}

ENV GIT_SSL_NO_VERIFY 1

CMD /bin/bash


# -----------------------------------------------------------------------------
# git clone -b v2.6.32 --depth=1 https://github.com/torvalds/linux linux-2.6.32

# KERNEL_VERSION=v2.6.32.71
# git clone -b ${KERNEL_VERSION} --depth=1 https://git.kernel.org/pub/scm/linux/kernel/git/stable/linux.git linux-${KERNEL_VERSION}

# BUSYBOX_VERSION=1_20_2
# git clone -b ${BUSYBOX_VERSION} --depth=1 https://github.com/mirror/busybox busybox-${BUSYBOX_VERSION}

# --- docker build ---
# IMAGE_NAME=kbuilder-v2.6-late
# docker build -t ${IMAGE_NAME} --build-arg UID=$(id -u) --build-arg GID=$(id -g) .

# --- run daemon mode ---
# CONTAINER_NAME=${IMAGE_NAME}-dev
# docker run -it -d --name ${CONTAINER_NAME} \
#   --mount type=bind,src=$(realpath linux-${KERNEL_VERSION}),dst=/home/builder/linux \
#   --mount type=bind,src=$(realpath busybox-${BUSYBOX_VERSION}),dst=/home/builder/busybox \
#   ${IMAGE_NAME}

# docker exec -it ${CONTAINER_NAME} /bin/bash
# docker exec -it -u builder -w /home/builder ${CONTAINER_NAME} /bin/bash

# --- build kernel ---
# cd linux
# make defconfig
# time make -j $(nproc)
# make V=1 > build_log.txt # or create buildlog

# --- if need build busybox ---
# cd busybox
# make defconfig
# make menuconfig
#  Settings -> [*] Build static binary (no shared libs)
# make
# make install

# --- create compile_commands.json at host ---
# pip3 install compiledb
# compiledb -p build_log.txt
# sed -i "s|/home/builder/linux|$(realpath .)|g" compile_commands.json
