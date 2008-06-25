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
    au BufNewFile   *   py  daemon.BufNewFile (vim.eval("expand('%:p')")  
    au BufRead      *   py  daemon.BufRead (vim.eval("bufnr('%')"), vim.eval("expand('%:p')"))  
    au BufWrite     *   py  daemon.BufWrite (vim.eval("bufnr('%')"), vim.eval("expand('%:p')"))

    au BufAdd       *   py  daemon.BufAdd (vim.eval("bufnr('%')"), vim.eval("expand('%:p')"))
    au BufDelete    *   py  daemon.BufDelete (vim.eval("bufnr('%')"))
    au BufFilePre   *   py  daemon.BufFilePre (vim.eval("bufnr('%')"), vim.eval("expand('%:p')"))

    au BufEnter     *   py  daemon.BufEnter (vim.eval("bufnr('%')"))
    au BufLeave     *   py  daemon.BufLeave (vim.eval("bufnr('%')"))

    au VimLeave     *   py  daemon.VimLeave ()

    au MenuPopup    *   py  daemon.MenuPopup (vim.eval("bufnr('%')"))
augroup END

