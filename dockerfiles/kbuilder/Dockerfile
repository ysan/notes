FROM ubuntu:22.04

ENV DEBIAN_FRONTEND noninteractive

RUN grep '^deb ' /etc/apt/sources.list | sed 's/^deb/deb-src/g' > /etc/apt/sources.list.d/deb-src.list

RUN apt-get update && apt-get install -y \
  curl \
  vim \
  tmux \
  sudo

RUN apt-get install -y git ccache fakeroot libncurses5-dev
RUN apt-get build-dep -y linux

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
# --- docker build ---
# docker build -t kbuilder --build-arg UID=$(id -u) --build-arg GID=$(id -g) .

# --- run daemon mode ---
# docker run -it -d --name kbuilder_con --mount type=bind,src=/xxx,dst=/yyy kbuilder

# docker exec -it kbuilder_con /bin/bash
# docker exec -it -u builder -w /home/builder kbuilder_con /bin/bash

