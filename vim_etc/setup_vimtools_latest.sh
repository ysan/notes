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
sudo n lts


# install vim
#sudo apt install -y software-properties-common
#sudo add-apt-repository -y ppa:jonathonf/vim
#sudo apt update
#sudo apt install -y vim

# install neovim
NVIM_URL="https://github.com/neovim/neovim/releases/latest/download/nvim-linux-x86_64.tar.gz"
cd "${SETUP_DIR}"
curl -LOk ${NVIM_URL}
tar xvf nvim-linux-x86_64.tar.gz
cp -pr nvim-linux-x86_64/* "${LOCAL_DIR}"
cd ../

# install clangd (resolve latest version from the release redirect)
CLANGD_VER=$(curl -fsSLk -o /dev/null -w '%{url_effective}' https://github.com/clangd/clangd/releases/latest)
CLANGD_VER="${CLANGD_VER##*/}"
CLANGD_URL="https://github.com/clangd/clangd/releases/download/${CLANGD_VER}/clangd-linux-${CLANGD_VER}.zip"
cd "${SETUP_DIR}"
curl -LOk ${CLANGD_URL}
unzip -o clangd-linux-${CLANGD_VER}.zip
cp -p clangd_${CLANGD_VER}/bin/clangd "${LOCAL_DIR}/bin"
cp -pr clangd_${CLANGD_VER}/lib/clang "${LOCAL_DIR}/lib"
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

# download vimplug
curl -fLo ~/.vim/autoload/plug.vim --create-dirs https://raw.githubusercontent.com/junegunn/vim-plug/master/plug.vim -k
sh -c 'curl -fLo "${XDG_DATA_HOME:-$HOME/.local/share}"/nvim/site/autoload/plug.vim --create-dirs \
						https://raw.githubusercontent.com/junegunn/vim-plug/master/plug.vim -k'

# setup vim plugins
vim +PlugInstall +qa
${LOCAL_DIR}/bin/nvim +PlugInstall +qa
VIMLSP=yes ${LOCAL_DIR}/bin/nvim +'CocInstall -sync coc-json' +qa
VIMLSP=yes ${LOCAL_DIR}/bin/nvim +'CocInstall -sync coc-jedi' +qa
VIMLSP=yes ${LOCAL_DIR}/bin/nvim +'CocInstall -sync coc-diagnostic' +qa
VIMLSP=yes ${LOCAL_DIR}/bin/nvim +'CocInstall -sync coc-clangd' +qa
VIMLSP=yes ${LOCAL_DIR}/bin/nvim +'CocInstall -sync coc-prettier' +qa
VIMLSP=yes ${LOCAL_DIR}/bin/nvim +'CocInstall -sync coc-eslint' +qa
VIMLSP=yes ${LOCAL_DIR}/bin/nvim +'CocInstall -sync coc-tsserver' +qa
VIMLSP=yes ${LOCAL_DIR}/bin/nvim +'CocInstall -sync coc-html' +qa
VIMLSP=yes ${LOCAL_DIR}/bin/nvim +'CocInstall -sync coc-css' +qa
VIMLSP=yes ${LOCAL_DIR}/bin/nvim +'CocInstall -sync coc-explorer' +qa
VIMLSP=yes ${LOCAL_DIR}/bin/nvim +'CocInstall -sync coc-git' +qa

# download coc-settings.json
COC_SETTINGS_URL="https://raw.githubusercontent.com/ysan/notes/master/vim_etc/coc-settings.json"
mkdir -p "${NVIMCONF_DIR}"
cd "${NVIMCONF_DIR}"
curl -LOk ${COC_SETTINGS_URL}

