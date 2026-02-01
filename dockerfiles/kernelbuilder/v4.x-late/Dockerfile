FROM ubuntu:20.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && \
    apt-get install -y \
    build-essential \
    libncurses-dev \
    bison \
    flex \
    libssl-dev \
    libelf-dev \
    bc \
    wget \
    xz-utils \
    kmod \
    cpio \
    rsync

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
# git clone -b v4.19 --depth=1 https://github.com/torvalds/linux linux-4.19
# git clone -b v4.19.325 --depth=1 https://git.kernel.org/pub/scm/linux/kernel/git/stable/linux.git linux-4.19.325

# --- docker build ---
# docker build -t kbuilder-v4.x-late --build-arg UID=$(id -u) --build-arg GID=$(id -g) .

# --- run daemon mode ---
# docker run -it -d --name kbuilder-v4.x-late_con \
#   --mount type=bind,src=/path/to/linux,dst=/home/builder/linux \
#   kbuilder-v4.x-late

# docker exec -it kbuilder-v4.x-late_con /bin/bash
# docker exec -it -u builder -w /home/builder kbuilder-v4.x-late_con /bin/bash

# --- build kernel ---
# cd linux
# make defconfig
# time make -j $(nproc)

# --- create build log ---
# make V=1 > build_log.txt
#
# --- create compile_commands.json at host ---
# pip3 install compiledb
# compiledb -p build_log.txt
# sed -i -e 's/\/home\/builder\/linux/\/home\/user\/path\/to\/linux-4.19.325/g' compile_commands.json

