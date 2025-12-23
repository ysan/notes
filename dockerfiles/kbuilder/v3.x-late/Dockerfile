FROM debian:jessie

RUN rm -rf /var/lib/apt/lists/*

RUN echo "deb http://archive.debian.org/debian/ jessie main contrib non-free" > /etc/apt/sources.list
RUN echo "deb http://archive.debian.org/debian-security/ jessie/updates main contrib non-free" >> /etc/apt/sources.list

RUN echo 'Acquire::Check-Valid-Until "false";' > /etc/apt/apt.conf.d/99ignore-valid-until
RUN echo 'Acquire::AllowInsecureRepositories "true";' >> /etc/apt/apt.conf.d/99ignore-valid-until
RUN echo 'Acquire::AllowDowngradeToInsecureRepositories "true";' >> /etc/apt/apt.conf.d/99ignore-valid-until

RUN apt-get update && \
    apt-get install -y --force-yes --allow-unauthenticated \
    build-essential \
    libncurses5-dev \
    bc \
    wget \
    curl \
    cpio \
    kmod

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
# git clone -b v3.10 --depth=1 https://github.com/torvalds/linux linux-3.10
# git clone -b v3.10.108 --depth=1 https://git.kernel.org/pub/scm/linux/kernel/git/stable/linux.git linux-3.10.108

# --- docker build ---
# docker build -t kbuilder-v3.x-late --build-arg UID=$(id -u) --build-arg GID=$(id -g) .

# --- run daemon mode ---
# docker run -it -d --name kbuilder-v3.x-late_con \
#   --mount type=bind,src=/path/to/linux,dst=/home/builder/linux \
#   kbuilder-v3.x-late

# docker exec -it kbuilder-v3.x-late_con /bin/bash
# docker exec -it -u builder -w /home/builder kbuilder-v3.x-late_con /bin/bash

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
# sed -i -e 's/\/home\/builder\/linux/\/home\/user\/path\/to\/linux-3.10.108/g' compile_commands.json

