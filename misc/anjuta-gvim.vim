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

function! AnjutaCheckBuf (bufno)
    return bufloaded (a:bufno) && buflisted(a:bufno) && getbufvar(str2nr(a:bufno), '&buftype') == ""
endfunction

function! AnjutaPos (token)
    if (a:token == "0")
        return 1
	elseif (a:token == ".")
        return line2byte (".") + byteidx (getline(line('.')), col('.')) -1
	elseif (a:token == "$")
        return line2byte ("$") + byteidx (getline(line('$')), col([line('$'),'$'])-1)
    endif
endfunction

function! AnjutaPos2Byte (pos)
	return line2byte (a:pos[1]) + a:pos[2] + a:pos[3] - 1
endfunction

function! AnjutaByte2Pos (buffer, byte)
    let curpos = getpos('.')
    let byte = a:byte
    exec "goto ".byte
    let pos = getpos('.')
    call setpos('.', curpos)
    let pos[0] = a:buffer
    return pos
endfunction

function! AnjutaGetChar (buffer, byte)
	let pos = AnjutaByte2Pos (a:buffer, a:byte)
	let line = getbufline(pos[0], pos[1])[0]
	return char2nr(line[pos[2]])
endfunction

"=============================================================================
"
" Autocmd Helpers
"=============================================================================

function! AnjutaSignalBufNewFile(bufno)
    if !AnjutaCheckBuf(a:bufno)
        return
    endif
    py  daemon.BufNewFile (vim.eval("a:bufno"));
endfunction

function! AnjutaSignalBufRead(bufno, file)
    if !AnjutaCheckBuf(a:bufno)
        return
    endif
    if a:file == ""
        py  daemon.BufRead (vim.eval("a:bufno"), "");
    else
        py  daemon.BufRead (vim.eval("a:bufno"), vim.eval("a:file"));
    endif
endfunction

function! AnjutaSignalBufWrite(bufno, file)
    if !AnjutaCheckBuf(a:bufno)
        return
    endif
    if a:file == ""
        py  daemon.BufWrite (vim.eval("a:bufno"), "");
    else
        py  daemon.BufWrite (vim.eval("a:bufno"), vim.eval("a:file"));
    endif
endfunction

function! AnjutaSignalBufAdd(bufno, file)
    if !AnjutaCheckBuf(a:bufno)
        return
    endif
    if a:file == ""
        py  daemon.BufAdd (vim.eval("a:bufno"), "");
    else
        py  daemon.BufAdd (vim.eval("a:bufno"), vim.eval("a:file"));
    endif
endfunction

function! AnjutaSignalBufDelete(bufno)
    " Special check, as buffer is about to be unloaded
    if !(buflisted(a:bufno) && getbufvar(str2nr(a:bufno), '&buftype') == "")
        return
    endif
    py  daemon.BufDelete (vim.eval("a:bufno"));

"	let bufno = bufnr('%')
"	let filename = expand('%:p')

"    if AnjutaCheckBuf(filename)
"        py  daemon.BufEnter (vim.eval("bufno"), vim.eval("filename"));
"    endif
endfunction

function! AnjutaSignalBufFilePost(bufno, file)
    if !AnjutaCheckBuf(a:bufno)
        return
    endif
    if a:file == ""
        py  daemon.BufFilePost (vim.eval("a:bufno"), "");
    else
        py  daemon.BufFilePost (vim.eval("a:bufno"), vim.eval("a:file"));
    endif
endfunction

function! AnjutaSignalBufEnter(bufno, file)
    let check = !buflisted(a:bufno) || getbufvar(str2nr(a:bufno), '&buftype') != ""
    echo "BufEnter " a:bufno a:file buflisted(a:bufno) getbufvar(str2nr(a:bufno), '&buftype') == "" check AnjutaCheckBuf(a:bufno)
    if check
        return
    endif
    if a:file == ""
        return
        py  daemon.BufEnter (vim.eval("a:bufno"), "")
    else
        py  daemon.BufEnter (vim.eval("a:bufno"), vim.eval("a:file"))
    endif
endfunction

function! AnjutaSignalBufLeave(bufno)
    if !AnjutaCheckBuf(a:bufno)
        return
    endif
    py  daemon.BufLeave (vim.eval("a:bufno"));
endfunction

function! AnjutaSignalVimLeave()
    py  daemon.VimLeave ();
endfunction

function! AnjutaSignalMenuPopup(bufno)
    if !AnjutaCheckBuf(a:bufno)
        return
    endif
    py  daemon.MenuPopup (vim.eval("a:bufno"));
endfunction

function! AnjutaSignalMenuPopup(bufno)
    if !AnjutaCheckBuf(a:bufno)
        return
    endif
    py  daemon.MenuPopup (vim.eval("a:bufno"));
endfunction

function! AnjutaSignalFileType(bufno, filetype)
    if !AnjutaCheckBuf(a:bufno)
        return
    endif
    py  daemon.FileType (vim.eval("a:bufno"), vim.eval("a:filetype"));
endfunction

function! AnjutaSignalInsertLeave(bufno)
    if !AnjutaCheckBuf(a:bufno)
        return
    endif
    py  daemon.InsertLeave (vim.eval("a:bufno"));
endfunction

function! AnjutaSignalCursorHold(bufno, word)
    if !AnjutaCheckBuf(a:bufno)
        return
    endif
    py  daemon.CursorHold (vim.eval("a:bufno"), vim.eval("a:word"));
endfunction

function! AnjutaSignalMarkChanged(bufno, handle, state)
    if !AnjutaCheckBuf(a:bufno)
        return
    endif
    py  daemon.MarkChanged (vim.eval("a:bufno"), vim.eval("a:handle"), vim.eval("a:state"));
endfunction

"=============================================================================
"
" Anjuta Helpers
"=============================================================================

function! AnjutaGetBuf (buffer,start, end)
	return AnjutaGetBufPos (a:buffer, AnjutaPos(a:start), AnjutaPos(a:end))
endfunction

function! AnjutaGetBufPos (buffer,start, end)
	let startpos = AnjutaByte2Pos(a:buffer, a:start)
	let endpos = AnjutaByte2Pos(a:buffer, a:end)

    let s = getpos("'s")
    let e = getpos("'e")
    call setpos("'s", startpos)
    call setpos("'e", endpos)

    let a = @z
    normal `s"zy`e
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

    call setpos ("'s", l:startp)
    call setpos ("'e", l:endp)

    normal `sd`e

    try
        call setpos("'s", s)
    catch /^Vim\%((\a\+)\)\=:E20/
    endtry
    try
        call setpos("'e", e)
    catch /^Vim\%((\a\+)\)\=:E20/
    endtry
endfunction


function! AnjutaGoto (token)
    if (a:token == "s")
        call search ('}') 
        normal %
        redraw
    elseif (a:token == "e")
        call search ('{','b') 
        normal %
        redraw
    elseif (a:token == "m")
        normal %
        redraw
    endif
endfunction

" Gets the line containing this byte, and returns
" it's byte extents
" TODO: Support multiple lines.
function! AnjutaGetLine (buffer, byte)
	let pos = AnjutaByte2Pos (a:buffer, a:byte)
	let line = getbufline(pos[0], pos[1])
    if len (line) < 1
        return string (a:byte, a:byte, "")
    else
        return string([line2byte(pos[1]), line2byte(pos[1]+len(line))-1, line[0]])
    endif
endfunction

function! AnjutaRSearch (buffer, query, flags, start, end, case)
	let case_ = 0
	let magic_ = 0
	if (&ignorecase)
		if (a:case)
			let case_ = 1
			set noignorecase
		endif
	else
		if (!a:case)
			let case_ = 1
			set ignorecase
		endif
	endif
	if (&magic)
		let magic_ = 1
		set nomagic
	endif
	call setpos('.', AnjutaByte2Pos(a:buffer, a:end))
	let endpos = [a:buffer] + searchpos(a:query, a:flags, byte2line(a:start)) + [0]
	if (case_)
		if (a:case)
			set ignorecase
		else
			set noignorecase
		endif
	endif
	if (magic_)
		set magic
	endif
	return AnjutaPos2Byte(endpos)
endfunction

function! AnjutaSearch (buffer, query, flags, start, end, case)
	let case_ = 0
	let magic_ = 0
    if (a:start == 0)
        let a:start = 1
    endif
    if (a:end == 0)
        let a:end = 1
    endif
	if (&ignorecase)
		if (a:case)
			let case_ = 1
			set noignorecase
		endif
	else
		if (!a:case)
			let case_ = 1
			set ignorecase
		endif
	endif
	if (&magic)
		let magic_ = 1
		set nomagic
	endif
	call setpos('.', AnjutaByte2Pos(a:buffer, a:start))
	let endpos = [a:buffer] + searchpos(a:query, a:flags, byte2line(a:end)) + [0]
	if (case_)
		if (a:case)
			set ignorecase
		else
			set noignorecase
		endif
	endif
	if (magic_)
		set magic
	endif
    echo AnjutaByte2Pos(a:buffer, a:start) endpos
	return AnjutaPos2Byte(endpos)
endfunction

" TODO: Find a way of using mode() without changing modes
function! AnjutaSelectionGet ()
	let start = AnjutaPos2Byte (getpos("'<"))
	let end = AnjutaPos2Byte (getpos("'>"))
	normal `<"*y`>
    if @* != ""
        let text = @*
        return string ([start] + [end] + [text])
    else
        return "[0,0,'']"
    endif
endfunction

function! AnjutaSelectionReplace (str)
	let start = AnjutaPos2Byte (getpos("'<"))
	let end = AnjutaPos2Byte (getpos("'>"))
	normal `<"*d`>
	exec "normal`<i".a:str
endfunction

function! AnjutaSelectionMakePos (start, end)
    normal l
	call setpos("'v", AnjutaByte2Pos (0, a:start))
	call setpos("'b", AnjutaByte2Pos (0, a:end))
	normal `vv`b
endfunction

function! AnjutaSelectionMake (arg)
    normal 
	if (a:arg == "a")
		normal ggVG
	elseif (a:arg == "b")
        call search ('}') 
        normal v%
	elseif (a:arg == "f")
        normal [[v][
	elseif (a:arg == "t")
		let pos = [0] + searchpos('}', 'n') + [0]
		call setpos ("'v", pos) 
		normal v'v
	endif
endfunction

" Enum Marks {
"   REMOVED=0,
"   LINEMARK,
"   BREAKPOINT_ENABLED,
"   BREAKPOINT_DISABLED,
"   PROGRAM_COUNTER,
"   BOOKMARK
"   }
"

function! AnjutaMarkSet (bufno, handle, type, line)
    if a:type == 1
        let icon = "anjuta-linemark"
    elseif a:type == 2
        let icon = "anjuta-breakpoint-enabled"
    elseif a:type == 3
        let icon = "anjuta-breakpoint-disabled"
    elseif a:type == 4
        let icon = "anjuta-pcmark"
    elseif a:type == 5
        let icon = "anjuta-bookmark"
    else
        return
    endif
    echo "sign place ".a:handle." name=".icon." line=".a:line." buffer=".a:bufno
    exec "sign place ".a:handle." name=".icon." line=".a:line." buffer=".a:bufno
    call AnjutaSignalMarkChanged (a:bufno, a:handle, a:type)
endfunction

function! AnjutaMarkRemove (bufno, handle)
    exec "sign unplace ".a:handle." buffer=".a:bufno
    call AnjutaSignalMarkChanged (a:bufno, a:handle, 0)
endfunction
 
function! AnjutaMarkChange (bufno, handle, type)
    if a:type == 1
        let icon = "anjuta-linemark"
    elseif a:type == 2
        let icon = "anjuta-breakpoint-enabled"
    elseif a:type == 3
        let icon = "anjuta-breakpoint-disabled"
    elseif a:type == 4
        let icon = "anjuta-pcmark"
    elseif a:type == 5
        let icon = "anjuta-bookmark"
    else
        return
    endif
    exec "sign place ".a:handle." name=".icon." buffer=".a:bufno
    call AnjutaSignalMarkChanged (a:bufno, a:handle, a:type)
endfunction

function! AnjutaMarkGetLine (bufno, handle)
    let pos = getpos ('.')
    exec "sign jump ".a:handle." buffer=".a:bufno
    let line = line('.')
    call setpos ('.', pos)
    return line
endfunction

function! AnjutaMarkMove (bufno, handle, type, line)
    if a:type == 1
        let icon = "anjuta-linemark"
    elseif a:type == 2
        let icon = "anjuta-breakpoint-enabled"
    elseif a:type == 3
        let icon = "anjuta-breakpoint-disabled"
    elseif a:type == 4
        let icon = "anjuta-pcmark"
    elseif a:type == 5
        let icon = "anjuta-bookmark"
    else
        return
    endif
    exec "sign unplace ".a:handle." buffer=".a:bufno
    exec "sign place ".a:handle." name=".icon." line=".a:line." buffer=".a:bufno
    call AnjutaSignalMarkChanged (a:bufno, a:handle, a:type)
endfunction

function! AnjutaAssistSuggest (tips)
    let curpos = getpos('.')
    normal b
    let cols = col('.')
    call setpos('.', curpos)
    call complete(cols,a:tips)
    return ""
endfunction


" }}}
