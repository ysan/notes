#!/bin/bash

set -e
set -o pipefail

LOCAL_DIR="${HOME}/.local/"
SETUP_DIR="${HOME}/setup_vimtools/"
NVIMCONF_DIR="${HOME}/.config/nvim/"

export DEBIAN_FRONTEND=noninteractive
export GIT_SSL_NO_VERIFY=1

mkdir -p "${LOCAL_DIR}"
mkdir -p "${SETUP_DIR}"

sudo apt update
sudo apt install -y \
  curl \
  vim \
  unzip \
  g++ \
  cmake \
  bear \
  python3 \
  python3-pip \
  python3-venv \
  nodejs \
  npm

# install nodejs
sudo npm -g install n
sudo apt remove -y nodejs npm
sudo n 24.14.0


# install vim
#sudo apt install -y software-properties-common
#sudo add-apt-repository -y ppa:jonathonf/vim
#sudo apt update
#sudo apt install -y vim

# install neovim
NVIM_URL="https://github.com/neovim/neovim/releases/download/v0.11.6/nvim-linux-x86_64.tar.gz"
cd "${SETUP_DIR}"
curl -LOk ${NVIM_URL}
tar xvf nvim-linux-x86_64.tar.gz
cp -pr nvim-linux-x86_64/* "${LOCAL_DIR}"
cd ../

# install clangd
CLANGD_URL="https://github.com/clangd/clangd/releases/download/21.1.8/clangd-linux-21.1.8.zip"
cd "${SETUP_DIR}"
curl -LOk ${CLANGD_URL}
unzip -o clangd-linux-21.1.8.zip
cp -p clangd_21.1.8/bin/clangd "${LOCAL_DIR}/bin"
cp -pr clangd_21.1.8/lib/clang "${LOCAL_DIR}/lib"
cd ..

# install shellcheck
SHELLCHECK_STABLE_URL="https://github.com/koalaman/shellcheck/releases/download/stable/shellcheck-stable.linux.x86_64.tar.xz"
cd "${SETUP_DIR}"
curl -LOk ${SHELLCHECK_STABLE_URL}
tar xvf shellcheck-stable.linux.x86_64.tar.xz
cp -p shellcheck-stable/shellcheck "${LOCAL_DIR}/bin"
cd ..

# download vimrc
VIMRC_URL="https://raw.githubusercontent.com/ysan/notes/master/vim_etc/.vimrc"
VIMRC_URL2="https://raw.githubusercontent.com/ysan/notes/master/vim_etc/.vimrc.lsp"
cd "${HOME}"
curl -LOk ${VIMRC_URL}
curl -LOk ${VIMRC_URL2}

VIMRC_URL="https://raw.githubusercontent.com/ysan/notes/master/vim_etc/init.vim"
VIMRC_URL2="https://raw.githubusercontent.com/ysan/notes/master/vim_etc/init.vim.lsp"
mkdir -p "${NVIMCONF_DIR}"
cd "${NVIMCONF_DIR}"
curl -LOk ${VIMRC_URL}
curl -LOk ${VIMRC_URL2}

sed -i "s/coc.nvim', {'branch': 'release'}/coc.nvim', {'tag': 'v0.0.82'}/g" "${NVIMCONF_DIR}/init.vim"

# download vimplug
curl -fLo ~/.vim/autoload/plug.vim --create-dirs https://raw.githubusercontent.com/junegunn/vim-plug/master/plug.vim -k
sh -c 'curl -fLo "${XDG_DATA_HOME:-$HOME/.local/share}"/nvim/site/autoload/plug.vim --create-dirs \
						https://raw.githubusercontent.com/junegunn/vim-plug/master/plug.vim -k'

# setup vim plugins
vim +PlugInstall +qa
${LOCAL_DIR}/bin/nvim +PlugInstall +qa
VIMLSP=yes ${LOCAL_DIR}/bin/nvim +'CocInstall -sync coc-json@1.9.3' +qa
VIMLSP=yes ${LOCAL_DIR}/bin/nvim +'CocInstall -sync coc-jedi@0.36.1' +qa
VIMLSP=yes ${LOCAL_DIR}/bin/nvim +'CocInstall -sync coc-diagnostic@0.24.1' +qa
VIMLSP=yes ${LOCAL_DIR}/bin/nvim +'CocInstall -sync coc-clangd@0.32.0' +qa
VIMLSP=yes ${LOCAL_DIR}/bin/nvim +'CocInstall -sync coc-prettier@11.0.1' +qa
VIMLSP=yes ${LOCAL_DIR}/bin/nvim +'CocInstall -sync coc-eslint@3.0.15' +qa
VIMLSP=yes ${LOCAL_DIR}/bin/nvim +'CocInstall -sync coc-tsserver@2.3.1' +qa
VIMLSP=yes ${LOCAL_DIR}/bin/nvim +'CocInstall -sync coc-html@1.8.0' +qa
VIMLSP=yes ${LOCAL_DIR}/bin/nvim +'CocInstall -sync coc-css@2.1.0' +qa
VIMLSP=yes ${LOCAL_DIR}/bin/nvim +'CocInstall -sync coc-explorer@0.27.3' +qa

# download coc-settings.json
COC_SETTINGS_URL="https://raw.githubusercontent.com/ysan/notes/master/vim_etc/coc-settings.json"
mkdir -p "${NVIMCONF_DIR}"
cd "${NVIMCONF_DIR}"
curl -LOk ${COC_SETTINGS_URL}

