set listchars=tab:^_
set ts=4
colorscheme koehler
set fileencodings=utf-8,iso-2022-jp,sjis,euc-jp,cp932
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

inoremap { {}<Left>
inoremap ( ()<Left>
inoremap [ []<Left>
inoremap {} {}
inoremap () ()
inoremap [] []

inoremap {<Enter> {}<Left><CR><ESC><S-o>
inoremap (<Enter> ()<Left><CR><ESC><S-o>
inoremap [<Enter> []<Left><CR><ESC><S-o>

"inoremap ' ''<Left>
"inoremap " ""<Left>
"inoremap < <><Left>
inoremap '' ''<Left>
inoremap "" ""<Left>
inoremap <> <><Left>


call plug#begin()
Plug 'prabirshrestha/async.vim'
Plug 'prabirshrestha/vim-lsp'
Plug 'prabirshrestha/asyncomplete.vim'
Plug 'prabirshrestha/asyncomplete-lsp.vim'
call plug#end()

if filereadable(expand('~/.vimrc.lsp')) && $VIMLSP == "on"
	source ~/.vimrc.lsp
endif
