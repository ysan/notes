FROM ubuntu:18.04

ENV DEBIAN_FRONTEND noninteractive

RUN grep '# deb-src' /etc/apt/sources.list | sed 's/# deb-src/deb-src/g' > /etc/apt/sources.list.d/deb-src.list

RUN apt-get update && apt-get install -y software-properties-common
RUN add-apt-repository universe
RUN apt-get update && apt-get install -y \
  autoconf \
  automake \
  cmake \
  curl \
  vim \
  tmux \
  bear \
  git \
  gcc-arm-linux-gnueabihf \
  g++-arm-linux-gnueabihf \
  pkg-config-arm-linux-gnueabihf \
#  libomxil-bellagio-dev \
  sudo

#RUN apt-get build-dep -y ffmpeg

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

# build raspberrypi userland library for mmal
RUN git clone --depth=1 https://github.com/raspberrypi/userland.git
RUN mkdir -p userland/build
RUN cd userland/build && cmake ../ -DCMAKE_TOOLCHAIN_FILE=../makefiles/cmake/toolchains/arm-linux-gnueabihf.cmake
RUN cd userland/build && make && sudo make install

# build libx264
#RUN git clone --depth=1 https://code.videolan.org/videolan/x264.git
#RUN mkdir -p x264/build
#RUN cd x264/build && ../configure --host=arm-linux-gnueabihf --cross-prefix=/usr/bin/arm-linux-gnueabihf- --enable-static -enable-shared --disable-cli
#RUN cd x264/build && make && sudo make install

# build ffmpeg
RUN git clone -b n4.1.11 --depth=1 https://github.com/ffmpeg/ffmpeg.git
RUN mkdir -p ffmpeg/build && mkdir -p out
#RUN cd ffmpeg/build && ../configure -h
RUN cd ffmpeg/build && ../configure --enable-cross-compile --cross-prefix=/usr/bin/arm-linux-gnueabihf- --arch=arm --target-os=linux --prefix=/home/builder/out --enable-omx-rpi --enable-omx --enable-mmal --enable-gpl --enable-neon --extra-cflags="-I/opt/vc/include -I/opt/vc/include/IL -I/usr/local/include" --extra-ldflags="-L/opt/vc/lib" --extra-libs="-ldl -lrt -lvcos -lvcsm -lvchiq_arm"
RUN cd ffmpeg/build && make -j $(nproc) && make install

CMD /bin/bash


# -----------------------------------------------------------------------------
# --- docker build ---
# docker build -t ffmpegbuilder --build-arg UID=$(id -u) --build-arg GID=$(id -g) .

# --- run daemon mode ---
# docker run -it -d --name ffmpegbuilder_con ffmpegbuilder

# docker exec -it ffmpegbuilder_con /bin/bash
# docker exec -it -u builder -w /home/builder ffmpegbuilder_con /bin/bash

# docker cp ffmpegbuilder_con:/home/builder/out/bin/ffmpeg .
