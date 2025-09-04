FROM ubuntu:22.04

ENV DEBIAN_FRONTEND noninteractive

RUN apt-get update && apt-get install -y \
  psmisc \
  bc \
  wget \
  curl \
  vim \
  tmux \
  subversion \
  git \
  locales \
  sudo \
  unzip \
  g++ \
  cmake \
  bear \
  python3 \
  python3-pip \
  python3-venv \
  nodejs \
  npm

RUN apt-get install -y netcat-openbsd telnet iputils-ping net-tools iproute2 dnsutils traceroute

RUN echo root:root | chpasswd

# install vim8.2
RUN apt-get install -y software-properties-common && add-apt-repository ppa:jonathonf/vim && \
  apt-get update && apt-get install -y vim

# install neovim
#RUN apt-get install -y software-properties-common && add-apt-repository ppa:neovim-ppa/stable && \
#  apt-get update && apt-get install -y neovim
ARG NVIM_URL=https://github.com/neovim/neovim/releases/download/v0.8.3/nvim-linux64.tar.gz
RUN cd /root && curl -LOks ${NVIM_URL} && \
  tar xvf nvim-linux64.tar.gz && \
  cp -pr nvim-linux64/* /usr/local/

# install clangd-19
ARG CLANGD_URL=https://github.com/clangd/clangd/releases/download/19.1.2/clangd-linux-19.1.2.zip
RUN cd /root && curl -LOks ${CLANGD_URL} && \
  unzip clangd-linux-19.1.2.zip && \
  cp -p clangd_19.1.2/bin/clangd /usr/local/bin && \
  cp -pr clangd_19.1.2/lib/clang /usr/local/lib

# install shellcheck
ARG SHELLCHECK_STABLE_URL=https://github.com/koalaman/shellcheck/releases/download/stable/shellcheck-stable.linux.x86_64.tar.xz
RUN cd /root && curl -LOks ${SHELLCHECK_STABLE_URL} && \
  tar xvf shellcheck-stable.linux.x86_64.tar.xz && \
  cp -p shellcheck-stable/shellcheck /usr/local/bin

# install nodejs
RUN npm -g install n
RUN apt-get remove -y nodejs npm
RUN n 20.17.0

# optimize locale ja_JP.utf8
RUN localedef -f UTF-8 -i ja_JP ja_JP
ENV LC_CTYPE ja_JP.utf8

# add user
ARG UID=1000
ARG GID=1000
ARG USER=develop
ARG GROUP=develop
ARG PASS=develop
RUN groupadd -g $GID ${GROUP} && \
  useradd -m -s /bin/bash -u ${UID} -g ${GID} -G sudo ${USER} && \
  echo ${USER}:${PASS} | chpasswd

# user sudo no passwd
RUN echo "${USER} ALL=NOPASSWD: ALL" > /etc/sudoers.d/container_users

# switch user
USER ${USER}
WORKDIR /home/${USER}

ENV PATH $PATH:/home/${USER}/.local/bin
ENV GIT_SSL_NO_VERIFY 1

# download vimrc
ARG VIMRC_URL=https://raw.githubusercontent.com/ysan/notes/master/vim_etc/.vimrc
ARG VIMRC_URL2=https://raw.githubusercontent.com/ysan/notes/master/vim_etc/.vimrc.lsp
RUN cd /home/${USER} && \
  curl -LOks ${VIMRC_URL} && \
  curl -LOks ${VIMRC_URL2}

ARG VIMRC_URL=https://raw.githubusercontent.com/ysan/notes/master/vim_etc/init.vim
ARG VIMRC_URL2=https://raw.githubusercontent.com/ysan/notes/master/vim_etc/init.vim.lsp
RUN mkdir -p /home/${USER}/.config/nvim/ && \
  cd /home/${USER}/.config/nvim/ && \
  curl -LOks ${VIMRC_URL} && \
  curl -LOks ${VIMRC_URL2}

RUN sed -i "s/coc.nvim', {'branch': 'release'}/coc.nvim', {'tag': 'v0.0.82'}/g" /home/${USER}/.config/nvim/init.vim

# download vimplug
RUN curl -fLo ~/.vim/autoload/plug.vim --create-dirs \
               https://raw.githubusercontent.com/junegunn/vim-plug/master/plug.vim -k
RUN sh -c 'curl -fLo "${XDG_DATA_HOME:-$HOME/.local/share}"/nvim/site/autoload/plug.vim --create-dirs \
		       https://raw.githubusercontent.com/junegunn/vim-plug/master/plug.vim -k'

# setup vim plugins
RUN vim +PlugInstall +qa
RUN nvim +PlugInstall +qa
#RUN VIMLSP=yes nvim +'CocInstall -sync coc-json coc-jedi coc-diagnostic coc-clangd coc-prettier coc-eslint coc-tsserver coc-html coc-css coc-explorer' +qa
RUN VIMLSP=yes nvim +'CocInstall -sync coc-json@1.9.2 coc-jedi@0.36.1 coc-diagnostic@0.24.1 coc-clangd@0.31.0 coc-prettier@9.3.2 coc-eslint@1.7.0 coc-tsserver@2.2.0 coc-html@1.8.0 coc-css@2.1.0 coc-explorer@0.27.3' +qa

# download coc-settings.json
ARG COC_SETTINGS_URL=https://raw.githubusercontent.com/ysan/notes/master/vim_etc/coc-settings.json
RUN mkdir -p /home/${USER}/.config/nvim/ && \
	cd /home/${USER}/.config/nvim/ && \
	curl -LOks ${COC_SETTINGS_URL}


CMD /bin/bash


# -----------------------------------------------------------------------------
# --- docker build ---
# docker build -t develop .
# docker build -t develop --build-arg UID=$(id -u) --build-arg GID=$(id -g) .

# --- run daemon mode ---
# docker run -it -d --name develop_con develop
# docker run -it -d --name develop_con --mount type=bind,src=/xxx,dst=/yyy develop

# docker exec -it develop_con /bin/bash
# docker exec -it -u develop -w /home/develop develop_con /bin/bash

