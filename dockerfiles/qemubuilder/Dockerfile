FROM ubuntu:18.04
#FROM ubuntu:22.04

ENV DEBIAN_FRONTEND noninteractive

RUN grep '# deb-src' /etc/apt/sources.list | sed 's/# deb-src/deb-src/g' > /etc/apt/sources.list.d/deb-src.list

RUN apt-get update && apt-get install -y \
  curl \
  vim \
  tmux \
  bear \
  sudo

RUN apt-get build-dep -y qemu
#RUN apt-get build-dep -y qemu-user-static
RUN apt-get install -y git ninja-build

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
# git clone -b v6.2.0 --depth=1 https://github.com/qemu/qemu

# --- docker build ---
# docker build -t qemubuilder --build-arg UID=$(id -u) --build-arg GID=$(id -g) .

# --- run daemon mode ---
# docker run -it -d --name qemubuilder_con --mount type=bind,src=/path/to/qemu,dst=/home/builder/qemu qemubuilder

# docker exec -it qemubuilder_con /bin/bash
# docker exec -it -u builder -w /home/builder qemubuilder_con /bin/bash

# --- build ---
# mkdir -p qemu/build
# cd qemu/build
# ../configure --target-list=x86_64-softmmu
# or
# ../configure --target-list=x86_64-softmmu --static   <-- static build error
# or
# ../configure --target-list=x86_64-linux-user --static
# time make -j $(nproc)
# --- change path description compile_commands.json for host ---
# sed -i -e 's/\/home\/builder\/qemu/\/home\/user\/path\/to\/qemu/g' compile_commands.json
# mv compile_commands.json ../
