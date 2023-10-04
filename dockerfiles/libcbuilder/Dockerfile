FROM ubuntu:22.04

ENV DEBIAN_FRONTEND noninteractive

RUN apt-get update && apt-get install -y \
  curl \
  vim \
  tmux \
  sudo \
  bear \
  git

RUN apt-get install -y bison build-essential gawk gettext openssl python3 texinfo wget

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
# git clone -b glibc-2.37 --depth=1 https://sourceware.org/git/glibc.git

# --- docker build ---
# docker build -t libcbuilder --build-arg UID=$(id -u) --build-arg GID=$(id -g) .

# --- run daemon mode ---
# docker run -it -d --name libcbuilder_con --mount type=bind,src=/path/to/glibc,dst=/home/builder/glibc libcbuilder

# docker exec -it libcbuilder_con /bin/bash
# docker exec -it -u builder -w /home/builder libcbuilder_con /bin/bash

# --- build ---
# mkdir -p glibc/build
# cd glibc/build
# ../configure --prefix="$(pwd)/out" --libdir="$(pwd)/out/lib" --libexecdir="$(pwd)/out/lib" --enable-multi-arch --enable-stack-protector=strong
# make
# --- create compile_commands.json for host ---
# bear -- make
# sed -i -e 's/\/home\/builder\/glibc/\/home\/user\/path\/to\/glibc/g' compile_commands.json
# mv compile_commands.json ../
