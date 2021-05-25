set listchars=tab:^_
set ts=4
colorscheme koehler
set fileencodings=utf-8,iso-2022-jp,sjis,euc-jp
set noswapfile
set noautoindent

if has('syntax')
    syntax enable
endif
set nocompatible

set hlsearch
set incsearch
set clipboard=unnamedplus

set cursorline
set t_Co=256
hi CursorLine   term=reverse cterm=none ctermbg=242

set cindent
set shiftwidth=0

"inoremap { {}<Left>
"inoremap ( ()<ESC>i
"inoremap [ []<ESC>i
inoremap {} {}<ESC>i
inoremap () ()<ESC>i
inoremap [] []<ESC>i

inoremap {<Enter> {}<Left><CR><ESC><S-o>
inoremap (<Enter> ()<Left><CR><ESC><S-o>
inoremap [<Enter> []<Left><CR><ESC><S-o>

"inoremap ' ''<ESC>i
"inoremap " ""<ESC>i
"inoremap < <><ESC>i
inoremap '' ''<ESC>i
inoremap "" ""<ESC>i
inoremap <> <><ESC>i

noremap <S-h> ^
noremap <S-j> }
noremap <S-k> {
noremap <S-l> $

call plug#begin('~/.vim/plugged')
Plug 'neoclide/coc.nvim', {'branch': 'release'}
Plug 'prabirshrestha/async.vim'
Plug 'prabirshrestha/vim-lsp'
Plug 'prabirshrestha/asyncomplete.vim'
Plug 'prabirshrestha/asyncomplete-lsp.vim'
call plug#end()

if filereadable(expand('~/.config/nvim/init.vim.local'))
	source ~/.config/nvim/init.vim.local
endif
