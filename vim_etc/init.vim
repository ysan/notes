set list
set listchars=tab:^\ ,trail:-
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
"set clipboard=unnamedplus
set clipboard=unnamed
set backspace=indent,eol,start

set cursorline
set t_Co=256
hi CursorLine   term=reverse cterm=none ctermbg=242

set cindent
set shiftwidth=0

set statusline=%F%m%h%w%<\ \ (%{&fenc!=''?&fenc:&enc},%{&ff},%Y)\ %=0x%02B\ %l/%L,%02v
set laststatus=2

"inoremap { {}<Left>
imap ( ()<Left>
imap [ []<Left>
"imap < <><Left>
"inoremap {} {}<Left>
"inoremap () ()<Left>
"inoremap [] []<Left>
inoremap <> <><Left>
"inoremap <expr> ) strpart(getline('.'), col('.')-1, 1) == ")" ? "\<Right>" : ")"
"inoremap <expr> ] strpart(getline('.'), col('.')-1, 1) == "]" ? "\<Right>" : "]"
"inoremap <expr> > strpart(getline('.'), col('.')-1, 1) == ">" ? "\<Right>" : ">"

inoremap {<Enter> {}<Left><CR><ESC><S-o>

"imap ' ''<Left>
"imap " ""<Left>
inoremap '' ''<Left>
inoremap "" ""<Left>

set display=uhex

set splitbelow
set splitright

augroup _set_cursorline_only_active_window
  autocmd!
  autocmd VimEnter,BufWinEnter,WinEnter * setlocal cursorline
  autocmd WinLeave * setlocal nocursorline
augroup END

function! ZenkakuSpace()
  highlight ZenkakuSpace cterm=underline ctermfg=lightblue guibg=#666666
endfunction

if has('syntax')
  augroup _set_ZenkakuSpace
    autocmd!
    autocmd ColorScheme       * call ZenkakuSpace()
    autocmd VimEnter,WinEnter * match ZenkakuSpace /　/
  augroup END
  call ZenkakuSpace()
endif

augroup _set_filetype_indent
  autocmd!
  autocmd VimEnter,BufWinEnter,WinEnter *.py   set expandtab ts=4 shiftwidth=4
  autocmd VimEnter,BufWinEnter,WinEnter *.js   set expandtab ts=2 shiftwidth=2
  autocmd VimEnter,BufWinEnter,WinEnter *.html set expandtab ts=2 shiftwidth=2
  autocmd VimEnter,BufWinEnter,WinEnter *.css  set expandtab ts=2 shiftwidth=2
augroup END

if has('nvim')
  set pumblend=30
  set termguicolors
endif

"----------------------------------------------
if has('nvim')
  silent! call plug#begin('~/.vim/plugged')
  Plug 'neoclide/coc.nvim', {'branch': 'release'}
  Plug 'morhetz/gruvbox'
  Plug 'sainnhe/edge'
  Plug 'junegunn/seoul256.vim'
  call plug#end()
else
  silent! call plug#begin()
  Plug 'prabirshrestha/async.vim'
  Plug 'prabirshrestha/vim-lsp'
  Plug 'prabirshrestha/asyncomplete.vim'
  Plug 'prabirshrestha/asyncomplete-lsp.vim'
  Plug 'morhetz/gruvbox'
  Plug 'sainnhe/edge'
  Plug 'junegunn/seoul256.vim'
  call plug#end()
endif

if filereadable(expand("$HOME/.vim/plugged/gruvbox/colors/gruvbox.vim"))
  silent! colorscheme gruvbox
  set background=light
endif

if has('nvim') && filereadable(expand("$HOME/.vim/plugged/edge/colors/edge.vim"))
  silent! colorscheme edge
  set background=dark
endif

if filereadable(expand("$HOME/.vim/plugged/seoul256.vim/colors/seoul256.vim"))
"  let g:seoul256_background = 234 "for dark
  let g:seoul256_background = 254
  silent! colorscheme seoul256
  set background=light
endif

if has('nvim')
  if filereadable(expand('~/.config/nvim/init.vim.lsp')) && $VIMLSP == "yes"
    source ~/.config/nvim/init.vim.lsp
  endif
else
  if filereadable(expand('~/.vimrc.lsp')) && $VIMLSP == "yes"
    source ~/.vimrc.lsp
  endif
endif
