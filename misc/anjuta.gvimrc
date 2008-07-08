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
    au BufNewFile   *   if &buftype == ""|call AnjutaSignalBufNewFile(expand('<abuf>'))|endif 
    au BufRead      *   if &buftype == ""|call AnjutaSignalBufRead(expand('<abuf>'), expand('<afile>'))|endif 
    au BufWrite     *   if &buftype == ""|call AnjutaSignalBufWrite(expand('<abuf>'), expand('<afile>'))|endif

    au BufAdd       *   if &buftype == ""|call AnjutaSignalBufAdd(expand('<abuf>'), expand('<afile>'))|endif
    au BufDelete    *   if &buftype == ""|call AnjutaSignalBufDelete(expand('<abuf>'))|endif
    au BufFilePre   *   if &buftype == ""|call AnjutaSignalBufFilePost(expand('<abuf>'), expand('<afile>'))|endif

    au BufEnter     *   if &buftype == ""|call AnjutaSignalBufEnter(expand('<abuf>'), expand('<afile>'))|endif
    au BufLeave     *   if &buftype == ""|call AnjutaSignalBufLeave(expand('<abuf>'))|endif

    au VimLeave     *   if &buftype == ""|call AnjutaSignalVimLeave()|endif

    au MenuPopup    *   if &buftype == ""|call AnjutaSignalMenuPopup(expand('<abuf>'))|endif 
augroup END

