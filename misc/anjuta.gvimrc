" GVimRC for Anjuta
"

" Load the default gvimrc
source ~/.gvimrc 
set guioptions-=i
set guioptions-=m
set guioptions-=T

" Do NOT change the following. Failing to heed this 
" advice can and will screw up the Anjuta Vim plugin

pyfile ~/Projects/anjuta-gvim/src/vim-dbus.py
source ~/Projects/anjuta-gvim/misc/anjuta-gvim.vim

" Setup up autocommands to fire DBus signals
augroup Anjuta
    au!
    au BufNewFile   *   if &buftype == ""|call AnjutaSignalBufNewFile()|endif 
    au BufRead      *   if &buftype == ""|call AnjutaSignalBufRead()|endif 
    au BufWrite     *   if &buftype == ""|call AnjutaSignalBufWrite()|endif

    au BufAdd       *   if &buftype == ""|call AnjutaSignalBufAdd()|endif
    au BufDelete    *   if &buftype == ""|call AnjutaSignalBufDelete()|endif
    au BufFilePre   *   if &buftype == ""|call AnjutaSignalBufFilePost()|endif

    au BufEnter     *   if &buftype == ""|call AnjutaSignalBufEnter()|endif
    au BufLeave     *   if &buftype == ""|call AnjutaSignalBufLeave()|endif

    au VimLeave     *   if &buftype == ""|call AnjutaSignalVimLeave()|endif

    au MenuPopup    *   if &buftype == ""|call AnjutaSignalMenuPopup()|endif 
augroup END

