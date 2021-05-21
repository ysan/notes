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

set cindent
set shiftwidth=0

inoremap {} {}<ESC>i
"inoremap { {}<Left>
inoremap {<Enter> {}<Left><CR><ESC><S-o>
inoremap () ()<ESC>i
"inoremap ( ()<ESC>i
inoremap (<Enter> ()<Left><CR><ESC><S-o>
inoremap [] []<ESC>i
"inoremap [ []<ESC>i
inoremap [<Enter> []<Left><CR><ESC><S-o>

inoremap '' ''<ESC>i
"inoremap ' ''<ESC>i
inoremap "" ""<ESC>i
"inoremap " ""<ESC>i
inoremap <> <><ESC>i
"inoremap < <><ESC>i

noremap <S-h> ^
noremap <S-j> }
noremap <S-k> {
noremap <S-l> $

call plug#begin()
Plug 'prabirshrestha/async.vim'
Plug 'prabirshrestha/vim-lsp'
Plug 'prabirshrestha/asyncomplete.vim'
Plug 'prabirshrestha/asyncomplete-lsp.vim'
call plug#end()

if filereadable(expand('~/.vimrc.local'))
	source ~/.vimrc.local
endif
