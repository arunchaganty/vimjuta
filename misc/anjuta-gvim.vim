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

function! AnjutaGetBuf (buffer,start, end)
	if (type(a:start) == 1)
		let l:startp = AnjutaByte2Pos(a:buffer, AnjutaPos(a:start))
	else
		let l:startp = AnjutaByte2Pos(a:buffer, a:start)
	endif
	if (type(a:end) == 1)
		let l:endp = AnjutaByte2Pos(a:buffer, AnjutaPos(a:end))
	else
		let l:endp = AnjutaByte2Pos(a:buffer, a:end)
	endif

	let s = getpos("'s")
	let e = getpos("'e")
    call setpos("'s", l:startp)
	call setpos("'e", l:endp)

    let a = @z
    's,'eyank z
    let b = @z

	try
		call setpos("'s", s)
	catch /^Vim\%((\a\+)\)\=:E20/
	endtry
	try
		call setpos("'e", e)
	catch /^Vim\%((\a\+)\)\=:E20/
	endtry

    return b
endfunction

function! AnjutaByte2Pos (buffer, byte)
    let lineno = byte2line (a:byte)
    return [a:buffer, lineno, a:byte - line2byte(lineno), 0,]
endfunction

function! AnjutaInsert (buffer, str, pos)
	if (type(a:pos) == 1)
		let l:pos = AnjutaByte2Pos(a:buffer, AnjutaPos(a:pos))
	else
		let l:pos = AnjutaByte2Pos(a:buffer, a:pos)
	endif
    let a = @z
    let @z = a:str 
    call setpos('.', l:pos)
    normal "zp
    let @z = a
endfunction

function! AnjutaErase (buffer, start, end)
	if (type(a:start) == 1)
		let l:startp = AnjutaByte2Pos(a:buffer, AnjutaPos(a:start))
	else
		let l:startp = AnjutaByte2Pos(a:buffer, a:start)
	endif
	if (type(a:end) == 1)
		let l:endp = AnjutaByte2Pos(a:buffer, AnjutaPos(a:end))
	else
		let l:endp = AnjutaByte2Pos(a:buffer, a:end)
	endif

    let pos1 = getpos("'s")
    let pos2 = getpos("'e")

    setpos ("'s", l:startp)
	setpos ("'e", l:endp)

    's,'edelete

    setpos ("'s", pos1)
    setpos ("'e", pos2)
endfunction

" }}}
"=============================================================================
"
" Autocmd Helpers
"=============================================================================

function! AnjutaSignalBufNewFile(bufno)
	py  daemon.BufNewFile (vim.eval("a:bufno"));
endfunction

function! AnjutaSignalBufRead(bufno, file)
	if a:file == ""
		py  daemon.BufRead (vim.eval("a:bufno"), "");
	else
		py  daemon.BufRead (vim.eval("a:bufno"), vim.eval("a:file"));
	endif
endfunction

function! AnjutaSignalWrite(bufno, file)
	if a:file == ""
		py  daemon.BufWrite (vim.eval("a:bufno"), "");
	else
		py  daemon.BufWrite (vim.eval("a:bufno"), vim.eval("a:file"));
	endif
endfunction

function! AnjutaSignalBufAdd(bufno, file)
	if a:file == ""
		py  daemon.BufAdd (vim.eval("a:bufno"), "");
	else
		py  daemon.BufAdd (vim.eval("a:bufno"), vim.eval("a:file"));
	endif
endfunction

function! AnjutaSignalBufDelete(bufno)
	py  daemon.BufDelete (vim.eval("a:bufno"));
endfunction

function! AnjutaSignalBufFilePost(bufno, file)
	if a:file == ""
		py  daemon.BufFilePost (vim.eval("a:bufno"), "");
	else
		py  daemon.BufFilePost (vim.eval("a:bufno"), vim.eval("a:file"));
	endif
endfunction

function! AnjutaSignalBufEnter(bufno, file)
	if a:file == ""
		py  daemon.BufEnter (vim.eval("a:bufno"), "");
	else
		py  daemon.BufEnter (vim.eval("a:bufno"), vim.eval("a:file"));
	endif
endfunction

function! AnjutaSignalBufLeave(bufno)
	py  daemon.BufLeave (vim.eval("a:bufno"));
endfunction

function! AnjutaSignalVimLeave()
	py  daemon.VimLeave ();
endfunction

function! AnjutaSignalMenuPopup(bufno)
	py  daemon.MenuPopup (vim.eval("a:bufno"));
endfunction

