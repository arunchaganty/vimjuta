" Anjuta Helper Functions <anjuta-gvim.vim>
"
" Script Info and Documentation  {{{
"=============================================================================
"
" Name Of File: anjuta-gvim.vim
"  Description: Anjuta-GVim integration helper script
"   Maintainer: Arun Chaganty <arunchaganty@gmail.com>
"          URL: 
"  Last Change: 
"      Version: 0.0.1
"
"        Usage: Normally, this file should never be sourced automatically, it
"               should be handled by Anjuta.
"
"=============================================================================
" }}}

"=============================================================================
" Private Functions {{{


function! AnjutaPos (token)
    if (a:token == "0")
        return 0
    endif
    if (a:token == ".")
        return line2byte (".") + byteidx (getline(line('.')), col('.')) -1
    endif
    if (a:token == "$")
        return line2byte ("$") + byteidx (getline(line('$')), col([line('$'),'$'])-1)
    endif
endfunction

function! AnjutaGetBufAll ()
    
endfunction

function! AnjutaGetPos ()
    
endfunction

function! AnjutaInsert (str, pos)
    
endfunction

function! AnjutaAppend (str, pos)
    
endfunction

function! AnjutaErase (start, end)
    
endfunction

" }}}
"=============================================================================

