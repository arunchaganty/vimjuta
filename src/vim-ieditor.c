/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * anjuta-gvim
 * Copyright (C) Arun Tejasvi Chaganty 2008 <arunchaganty@gmail.com>
 * 
 * anjuta-gvim is free software.
 * 
 * You may redistribute it and/or modify it under the terms of the
 * GNU General Public License, as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option)
 * any later version.
 * 
 * anjuta-gvim is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with anjuta-gvim.  If not, write to:
 * 	The Free Software Foundation, Inc.,
 * 	51 Franklin Street, Fifth Floor
 * 	Boston, MA  02110-1301, USA.
 *
 *
 * 	Implementation of the IAnjutaFile Interface
 */


#include <libanjuta/interfaces/ianjuta-file.h>
#include <libanjuta/interfaces/ianjuta-editor.h>
#include <libanjuta/interfaces/ianjuta-editor-multiple.h>
#include <libanjuta/interfaces/ianjuta-editor-folds.h>
#include <libanjuta/interfaces/ianjuta-editor-goto.h>
#include <libanjuta/interfaces/ianjuta-editor-language.h>
#include <libanjuta/interfaces/ianjuta-editor-line-mode.h>
#include <libanjuta/interfaces/ianjuta-editor-master.h>
#include <libanjuta/interfaces/ianjuta-editor-search.h>
#include <libanjuta/interfaces/ianjuta-editor-selection.h>
#include <libanjuta/interfaces/ianjuta-editor-assist.h>
#include <libanjuta/interfaces/ianjuta-markable.h>
#include "vim-widget.h"
#include "vim-editor.h"
#include "vim-widget-priv.h"
#include "vim-editor-priv.h"
#include "vim-dbus.h"
#include "vim-cell.h"
#include "vim-util.h"

#define VIM_EDITOR_LINEMARK 1
#define VIM_EDITOR_BREAKPOINT_ENABLED 2
#define VIM_EDITOR_BREAKPOINT_DISABLED 3
#define VIM_EDITOR_PCMARK 4
#define VIM_EDITOR_BOOKMARK 5

static gint
ieditor_get_tabsize (IAnjutaEditor *ieditor, GError **err)
{
	VimEditor *editor = (VimEditor*) ieditor;
	return vim_dbus_int_query (editor->priv->widget, "&tabstop", err);
}

static void
ieditor_set_tabsize (IAnjutaEditor *ieditor, gint tabsize, GError **err)
{
	VimEditor *editor = (VimEditor*) ieditor;
	gchar* query = NULL;

	g_assert (err == NULL);
	// Create query string
	query = g_strdup_printf (":set tabstop=%d", tabsize);

	vim_dbus_exec_without_reply (editor->priv->widget, query, err);
	
	// TODO: Error Handling...

	g_free (query);
}

static gboolean
ieditor_get_use_spaces (IAnjutaEditor *ieditor, GError **err)
{
	VimEditor *editor = (VimEditor*) ieditor;
	return vim_dbus_int_query (editor->priv->widget, "&expandtab", err);
	return TRUE;
}

static void
ieditor_set_use_spaces (IAnjutaEditor *ieditor, gboolean use_spaces, GError **err)
{
	VimEditor *editor = (VimEditor*) ieditor;
	gchar* query = NULL;

	g_assert (err == NULL);
	// Create query string
	if (use_spaces)
		query = g_strdup_printf (":set expandtab");
	else
		query = g_strdup_printf (":set noexpandtab");

	vim_dbus_exec_without_reply (editor->priv->widget, query, err);
	
	// TODO: Error Handling...

	g_free (query);
}

static void
ieditor_set_auto_indent (IAnjutaEditor *ieditor, gboolean auto_indent, GError **err)
{
	VimEditor *editor = (VimEditor*) ieditor;
	gchar* query = NULL;

	g_assert (err == NULL);
	// Create query string
	if (auto_indent)
		query = g_strdup_printf (":set autoindent");
	else
		query = g_strdup_printf (":set noautoindent");


	vim_dbus_exec_without_reply (editor->priv->widget, query, err);
	
	// TODO: Error Handling...

	g_free (query);
}


/* Scroll to line */
static void 
ieditor_goto_line(IAnjutaEditor *ieditor, gint line, GError **err)
{
	VimEditor *editor = (VimEditor*) ieditor;
	gchar* query = NULL;

	g_assert (err == NULL);
	// Create query string
	query = g_strdup_printf (":%d", line);
	vim_dbus_exec_without_reply (editor->priv->widget, query, err);
	g_free (query);

	vim_widget_grab_focus (editor->priv->widget);
}

/* Scroll to position */
static void 
ieditor_goto_position(IAnjutaEditor *ieditor, IAnjutaIterable* icell,
								  GError **err)
{
	VimEditor *editor = VIM_EDITOR (ieditor);
	gchar* query = NULL;

	g_assert (err == NULL);
	// Create query string
	query = g_strdup_printf (":goto %d", ianjuta_iterable_get_position (icell, err));
	vim_dbus_exec_without_reply (editor->priv->widget, query, err);
	g_free (query);
	
	vim_widget_grab_focus (editor->priv->widget);
}

static gchar* 
ieditor_get_text (IAnjutaEditor* ieditor, 
				IAnjutaIterable* start,
				IAnjutaIterable* end, GError **err)
{
	VimEditor *editor = (VimEditor*) ieditor;
	gchar* query = NULL;
	gchar* reply = NULL;

	g_assert (err == NULL);
	// Create query string
	query = g_strdup_printf ("AnjutaGetBufPos(%d, %d, %d)", 
			editor->priv->bufno,
			ianjuta_iterable_get_position (start,err), 
			ianjuta_iterable_get_position (end,err));

	reply = vim_dbus_query (editor->priv->widget, query, err);
	
	// TODO: Error Handling...

	return reply;	
}

static gchar*
ieditor_get_text_all (IAnjutaEditor* ieditor, GError **err)
{
	VimEditor *editor = (VimEditor*) ieditor;
	gchar* query = NULL;
	gchar* reply = NULL;

	g_assert (err == NULL);
	query = g_strdup_printf ("AnjutaGetBuf(%d,'0', '$')", 
			editor->priv->bufno);
	reply = vim_dbus_query (editor->priv->widget, query, err);
	
	// TODO: Error Handling...

	return reply;	
}

/* Get cursor position */
static IAnjutaIterable*
ieditor_get_position (IAnjutaEditor* ieditor, GError **err)
{
	VimEditor *editor = (VimEditor*) ieditor;
	VimEditorCell *cell;
	gchar* reply = NULL;
	gint position;

	g_assert (err == NULL);
	position = vim_dbus_int_query(editor->priv->widget, "AnjutaPos('.')", err);

	cell = vim_cell_new (editor, position);
	// TODO: Error Handling...

	return IANJUTA_ITERABLE (cell);
}

/* Deprecated. Not supporting */
static gint
ieditor_get_offset (IAnjutaEditor* ieditor, GError **err)
{
	return 0;
}

/* Return line of cursor */
static gint
ieditor_get_lineno(IAnjutaEditor *ieditor, GError **err)
{
	VimEditor *editor = (VimEditor*) ieditor;
	return vim_dbus_int_query (editor->priv->widget, "line ('.')", err);
}

/* Return the length of the text in the buffer */
/* FIXME: Returns the bytes in file. Doesn't match with the number of
 * characters in a multibyte file */
static gint 
ieditor_get_length(IAnjutaEditor *ieditor, GError **err)
{
	VimEditor *editor = (VimEditor*) ieditor;
	return vim_dbus_int_query (editor->priv->widget, "AnjutaPos('$')", err);
}

/* Return word on cursor position */
static gchar*
ieditor_get_current_word(IAnjutaEditor *ieditor, GError **err)
{
	VimEditor *editor = (VimEditor*) ieditor;
	return vim_dbus_query (editor->priv->widget, "expand('<cword>')", err);
}

/* Insert text at position */
static void 
ieditor_insert(IAnjutaEditor *ieditor, IAnjutaIterable* icell,
							   const gchar* text, gint length, GError **err)
{
	VimEditor *editor = (VimEditor*) ieditor;
	gchar* query = NULL;
	gint position = ianjuta_iterable_get_position (icell, err);

	g_assert (err == NULL);
    query = g_strdup_printf ("call AnjutaInsert (%d, \"%s\", %d)", 
			editor->priv->bufno,
			text, 
			position-1); /* the default insert is infact an append */

	vim_dbus_exec_without_reply (editor->priv->widget, query, err);
	g_free (query);
	
	// TODO: Error Handling...
}

/* Append text to buffer */
static void 
ieditor_append(IAnjutaEditor *ieditor, const gchar* text,
							   gint length, GError **err)
{
	VimEditor *editor = (VimEditor*) ieditor;
	gchar* query = NULL;

	g_assert (err == NULL);
	query = g_strdup_printf ("call AnjutaInsert (%d, \"%s\", '$')", 
			editor->priv->bufno, 
			text);

	vim_dbus_exec_without_reply (editor->priv->widget, query, err);
	g_free (query);
	
	// TODO: Error Handling...
}

static void 
ieditor_erase(IAnjutaEditor* ieditor, IAnjutaIterable* istart_cell, 
						  IAnjutaIterable* iend_cell, GError **err)
{
	VimEditor *editor = (VimEditor*) ieditor;
	gchar* reply = NULL;
	gchar* query = NULL;

	g_assert (err == NULL);
	// Create query string
	query = g_strdup_printf ("AnjutaErase(%d, %d, %d)", 
			editor->priv->bufno,
			ianjuta_iterable_get_position (istart_cell, err), 
			ianjuta_iterable_get_position (iend_cell, err));

	reply = vim_dbus_query (editor->priv->widget, query, err);
	
	// TODO: Error Handling...
}

static void 
ieditor_erase_all(IAnjutaEditor *ieditor, GError **err)
{
	VimEditor *editor = (VimEditor*) ieditor;

	g_assert (err == NULL);

	vim_dbus_exec_without_reply (editor->priv->widget, ":\%del", err);
	
	// TODO: Error Handling...
	
}

/* Return column of cursor */
static gint 
ieditor_get_column(IAnjutaEditor *ieditor, GError **err)
{
	VimEditor *editor = (VimEditor*) ieditor;

	g_assert (err == NULL);
	return vim_dbus_int_query (editor->priv->widget, "col ('.')", err);
}

/* Return TRUE if ieditor is in overwrite mode */
/* FIXME: If currently editing, it's nearly impossible to get 
 * write mode, as functions don't work... */
static gboolean 
ieditor_get_overwrite(IAnjutaEditor *ieditor, GError **err)
{
	return FALSE;
}


/* Set the ieditor popup menu */
/* FIXME: There is a function for this :completefunc ... Fetch data from the
 * menu widget first... */
static void 
ieditor_set_popup_menu(IAnjutaEditor *ieditor, 
								   GtkWidget* menu, GError **err)
{
	
}

/* Convert from position to line */
static gint 
ieditor_get_line_from_position(IAnjutaEditor *ieditor, 
										   IAnjutaIterable* icell, GError **err)
{
	VimEditor *editor = (VimEditor*) ieditor;
	gchar* query = NULL;

	g_assert (err == NULL);
	// Create query string
	query = g_strdup_printf ("byte2line(%d)", ianjuta_iterable_get_position (icell, err));
	return vim_dbus_int_query (editor->priv->widget, query, err);
	
	// TODO: Error Handling...
}

static IAnjutaIterable* 
ieditor_get_line_begin_position(IAnjutaEditor *ieditor,
											gint line, GError **err)
{
	VimEditor *editor = (VimEditor*) ieditor;
	VimEditorCell *cell = NULL;
	gint position = 0;
	gchar* query = NULL;

	query = g_strdup_printf ("line2byte(%d)", line);
	position = vim_dbus_int_query (editor->priv->widget, query, err);

	cell = vim_cell_new (editor, position);
	
	return IANJUTA_ITERABLE(cell);
}

static IAnjutaIterable*
ieditor_get_line_end_position(IAnjutaEditor *ieditor,
											gint line, GError **err)
{
	VimEditor *editor = (VimEditor*) ieditor;
	VimEditorCell *cell = NULL;
	gint position = 1;
	gchar* query = NULL;

	query = g_strdup_printf ("line2byte(%d+1)", line);
	position = vim_dbus_int_query (editor->priv->widget, query, err) - 1;

	cell = vim_cell_new (editor, position);
	
	return IANJUTA_ITERABLE(cell);
}

static IAnjutaIterable*
ieditor_get_position_from_offset(IAnjutaEditor* ieditor, gint position, GError** err)
{
	VimEditor *editor = (VimEditor*) ieditor;
	return IANJUTA_ITERABLE (vim_cell_new (editor, position));
}

static IAnjutaIterable*
ieditor_get_start_position (IAnjutaEditor* ieditor, GError** err)
{
	VimEditor *editor = (VimEditor*) ieditor;
	VimEditorCell *cell = vim_cell_new (editor, 1);

	ianjuta_iterable_first (IANJUTA_ITERABLE(cell), err);
	
	return IANJUTA_ITERABLE(cell);
}

static IAnjutaIterable*
ieditor_get_end_position (IAnjutaEditor* ieditor, GError** err)
{
	VimEditor *editor = (VimEditor*) ieditor;
	VimEditorCell *cell = vim_cell_new (editor, 1);

	ianjuta_iterable_last (IANJUTA_ITERABLE(cell), err);
	
	return IANJUTA_ITERABLE(cell);
}

static void
ieditor_goto_start (IAnjutaEditor* ieditor, GError** err)
{
	VimEditor *editor = (VimEditor*) ieditor;

	g_assert (err == NULL);

	vim_dbus_exec_without_reply (editor->priv->widget, ":goto 1", err);
	
	// TODO: Error Handling...
	
}

static void
ieditor_goto_end (IAnjutaEditor* ieditor, GError** err)
{
	VimEditor *editor = (VimEditor*) ieditor;

	g_assert (err == NULL);

	vim_dbus_exec_without_reply (editor->priv->widget, ":%", err);
	
	// TODO: Error Handling...
}

void 
ieditor_iface_init (IAnjutaEditorIface *iface)
{
	iface->get_tabsize = ieditor_get_tabsize;
	iface->set_tabsize = ieditor_set_tabsize;
	iface->get_use_spaces = ieditor_get_use_spaces;
	iface->set_use_spaces = ieditor_set_use_spaces;
	iface->set_auto_indent = ieditor_set_auto_indent;
	iface->goto_line = ieditor_goto_line;
	iface->goto_position = ieditor_goto_position;
	iface->get_text = ieditor_get_text;
	iface->get_text_all = ieditor_get_text_all;
	iface->get_position = ieditor_get_position;
	iface->get_offset = ieditor_get_offset;
	iface->get_lineno = ieditor_get_lineno;
	iface->get_length = ieditor_get_length;
	iface->get_current_word = ieditor_get_current_word;
	iface->insert = ieditor_insert;
	iface->append = ieditor_append;
	iface->erase = ieditor_erase;
	iface->erase_all = ieditor_erase_all;
	iface->get_column = ieditor_get_column;
	iface->get_overwrite = ieditor_get_overwrite;
	iface->set_popup_menu = ieditor_set_popup_menu;
	iface->get_line_from_position = ieditor_get_line_from_position;
	iface->get_line_begin_position = ieditor_get_line_begin_position;
	iface->get_line_end_position = ieditor_get_line_end_position;
	iface->goto_start = ieditor_goto_start;
	iface->goto_end = ieditor_goto_end;
	iface->get_position_from_offset = ieditor_get_position_from_offset;
	iface->get_start_position = ieditor_get_start_position;
	iface->get_end_position = ieditor_get_end_position;
}

/* IAnjutaEditorMultiple */

static IAnjutaEditorMaster*
imultiple_get_master (IAnjutaEditorMultiple *imultiple, GError **err)
{
	VimEditor* editor = VIM_EDITOR (imultiple);
	return IANJUTA_EDITOR_MASTER (editor->priv->widget);
}

void 
imultiple_iface_init (IAnjutaEditorMultipleIface *iface)
{
	iface->get_master = imultiple_get_master;
}

static void
ifolds_close_all (IAnjutaEditorFolds *ifolds, GError **err)
{
	VimEditor *editor = (VimEditor*) ifolds;

	g_assert (err == NULL);
	vim_dbus_exec_without_reply (editor->priv->widget, "normal zM", err);
}
static void
ifolds_open_all (IAnjutaEditorFolds *ifolds, GError **err)
{
	VimEditor *editor = (VimEditor*) ifolds;

	g_assert (err == NULL);
	vim_dbus_exec_without_reply (editor->priv->widget, "normal zR", err);
}
static void
ifolds_toggle_current (IAnjutaEditorFolds *ifolds, GError **err)
{
	VimEditor *editor = (VimEditor*) ifolds;

	g_assert (err == NULL);
	vim_dbus_exec_without_reply (editor->priv->widget, "normal za", err);
}

void 
ifolds_iface_init (IAnjutaEditorFoldsIface *iface)
{
	iface->close_all = ifolds_close_all;
	iface->open_all = ifolds_open_all;
	iface->toggle_current = ifolds_toggle_current;
}


static void
igoto_end_block (IAnjutaEditorGoto *igoto, GError **err)
{
	VimEditor *editor = (VimEditor*) igoto;

	g_assert (err == NULL);
	vim_dbus_exec_without_reply (editor->priv->widget, "call AnjutaGoto('e')", err);
}

static void
igoto_matching_brace (IAnjutaEditorGoto *igoto, GError **err)
{
	VimEditor *editor = (VimEditor*) igoto;

	g_assert (err == NULL);
	vim_dbus_exec_without_reply (editor->priv->widget, "call AnjutaGoto('m')", err);
}

static void
igoto_start_block (IAnjutaEditorGoto *igoto, GError **err)
{
	VimEditor *editor = (VimEditor*) igoto;

	g_assert (err == NULL);
	vim_dbus_exec_without_reply (editor->priv->widget, "call AnjutaGoto('s')", err);
}

void 
igoto_iface_init (IAnjutaEditorGotoIface *iface)
{
	iface->end_block = igoto_end_block;
	iface->matching_brace = igoto_matching_brace;
	iface->start_block = igoto_start_block;
}

static const gchar *
ilanguage_get_language (IAnjutaEditorLanguage *ilanguage, GError **err)
{
	VimEditor *editor = (VimEditor*) ilanguage;

	g_assert (err == NULL);
	return vim_dbus_query (editor->priv->widget, "&ft", err);
}

static const gchar *
ilanguage_get_language_name (IAnjutaEditorLanguage *ilanguage, const gchar* language, GError **err)
{
	return language;
}

static const GList*
ilanguage_get_supported_languages (IAnjutaEditorLanguage *ilanguage, GError **err)
{
	VimEditor *editor = (VimEditor*) ilanguage;
	static GList* list = NULL;

	/* A common list */
	if (!list)
	{
		list = g_list_prepend (list, g_strdup_printf ("c"));
		list = g_list_prepend (list, g_strdup_printf ("cpp"));
		list = g_list_prepend (list, g_strdup_printf ("java"));
		list = g_list_prepend (list, g_strdup_printf ("python"));
		list = g_list_prepend (list, g_strdup_printf ("make"));
		list = g_list_prepend (list, g_strdup_printf ("vala"));
		list = g_list_prepend (list, g_strdup_printf ("html"));
		list = g_list_prepend (list, g_strdup_printf ("sh"));
	}

	return list;
}

static void
ilanguage_set_language (IAnjutaEditorLanguage *ilanguage, const gchar* language, GError **err)
{
	VimEditor *editor = (VimEditor*) ilanguage;
	gchar *cmd = g_strdup_printf ("set ft=%s", language);

	g_assert (err == NULL);
	vim_dbus_exec_without_reply (editor->priv->widget, cmd, err);
	g_free (cmd);
}

void 
ilanguage_iface_init (IAnjutaEditorLanguageIface *iface)
{
	iface->get_language = ilanguage_get_language;
	iface->get_language_name = ilanguage_get_language_name;
	iface->get_supported_languages = ilanguage_get_supported_languages;
	iface->set_language = ilanguage_set_language;
}

static void
ilinemode_convert (IAnjutaEditorLineMode *ilinemode, IAnjutaEditorLineModeType mode, GError **err)
{
	VimEditor *editor = (VimEditor*) ilinemode;
	gchar *cmd;
	g_assert (err == NULL);
	switch (mode)
	{
		case IANJUTA_EDITOR_LINE_MODE_CR:
			cmd = g_strdup_printf ("set fileformat=mac");
			break;
		case IANJUTA_EDITOR_LINE_MODE_CRLF:
			cmd = g_strdup_printf ("set fileformat=dos");
			break;
		case IANJUTA_EDITOR_LINE_MODE_LF:
			cmd = g_strdup_printf ("set fileformat=unix");
			break;
	}
	vim_dbus_exec_without_reply (editor->priv->widget, cmd, err);
	g_free (cmd);
}

static void
ilinemode_fix (IAnjutaEditorLineMode *ilinemode, GError **err)
{
	VimEditor *editor = (VimEditor*) ilinemode;
	gchar *cmd;
	g_assert (err == NULL);
	/* Set the default to unix format. Seems like the best resolution */
	cmd = g_strdup_printf ("set fileformat=unix");
	vim_dbus_exec_without_reply (editor->priv->widget, cmd, err);
	g_free (cmd);
}

static IAnjutaEditorLineModeType
ilinemode_get (IAnjutaEditorLineMode *ilinemode, GError **err)
{
	VimEditor *editor = (VimEditor*) ilinemode;
	gchar *reply = NULL;

	g_assert (err == NULL);
	reply = vim_dbus_query (editor->priv->widget, "&fileformat", err);
	if (strcmp (reply, "mac"))
	{
		g_free (reply);
		return IANJUTA_EDITOR_LINE_MODE_CR;
	}
	else if (strcmp (reply, "dos"))
	{
		g_free (reply);
		return IANJUTA_EDITOR_LINE_MODE_CRLF;
	}	
	else if (strcmp (reply, "unix"))
	{
		g_free (reply);
		return IANJUTA_EDITOR_LINE_MODE_LF;
	}
	else
	{
		g_free (reply);
		g_return_val_if_reached (IANJUTA_EDITOR_LINE_MODE_LF);
	}

}

static void
ilinemode_set (IAnjutaEditorLineMode *ilinemode, IAnjutaEditorLineModeType mode, GError **err)
{
	VimEditor *editor = (VimEditor*) ilinemode;
	gchar *cmd;
	g_assert (err == NULL);
	switch (mode)
	{
		case IANJUTA_EDITOR_LINE_MODE_CR:
			cmd = g_strdup_printf ("set fileformat=mac");
			break;
		case IANJUTA_EDITOR_LINE_MODE_CRLF:
			cmd = g_strdup_printf ("set fileformat=dos");
			break;
		case IANJUTA_EDITOR_LINE_MODE_LF:
			cmd = g_strdup_printf ("set fileformat=unix");
			break;
	}
	vim_dbus_exec_without_reply (editor->priv->widget, cmd, err);
	g_free (cmd);
}

void 
ilinemode_iface_init (IAnjutaEditorLineModeIface *iface)
{
	iface->convert = ilinemode_convert;
	iface->fix = ilinemode_fix;
	iface->get = ilinemode_get;
	iface->set = ilinemode_set;
}

static gboolean
isearch_backward (IAnjutaEditorSearch *isearch, const gchar* search,
		gboolean case_sensitive,   IAnjutaEditorCell* start,
		IAnjutaEditorCell* end,   IAnjutaEditorCell** result_start,
		IAnjutaEditorCell** result_end, GError **err)
{
	VimEditor *editor = (VimEditor*) isearch;
	VimEditorCell* start_ = VIM_CELL (start);
	VimEditorCell* end_ = VIM_CELL (end);
	gchar *cmd;
	gint result;

	cmd = g_strdup_printf ("AnjutaRSearch(%d,'%s','b', %d, %d, %d)",
			editor->priv->bufno,
			search, 
			vim_cell_get_position(start_),
			vim_cell_get_position(end_),
			case_sensitive);
	result = vim_dbus_int_query (editor->priv->widget, cmd, err);

	if (result > 0)
	{
		*result_start = IANJUTA_EDITOR_CELL(vim_cell_new (editor, result));
		*result_end = IANJUTA_EDITOR_CELL(vim_cell_new (editor, result+strlen(search)));
		return TRUE;
	}
	else
	{
		*result_start = NULL;
		*result_end = NULL;
		return FALSE;
	}
}


static gboolean
isearch_forward (IAnjutaEditorSearch *isearch, const gchar* search,
		gboolean case_sensitive,   IAnjutaEditorCell* start,
		IAnjutaEditorCell* end,   IAnjutaEditorCell** result_start,
		IAnjutaEditorCell** result_end, GError **err)
{
	VimEditor *editor = (VimEditor*) isearch;
	VimEditorCell* start_ = VIM_CELL (start);
	VimEditorCell* end_ = VIM_CELL (end);
	gchar *cmd;
	gint result;

	cmd = g_strdup_printf ("AnjutaSearch(%d,'%s','', %d, %d, %d)",
			editor->priv->bufno,
			search, 
			vim_cell_get_position(start_),
			vim_cell_get_position(end_),
			case_sensitive);
	result = vim_dbus_int_query (editor->priv->widget, cmd, err);
	
	if (result > 0)
	{
		*result_start = IANJUTA_EDITOR_CELL(vim_cell_new (editor, result));
		*result_end = IANJUTA_EDITOR_CELL(vim_cell_new (editor, result+strlen(search)));
		return TRUE;
	}
	else
	{
		*result_start = NULL;
		*result_end = NULL;
		return FALSE;
	}
}


void 
isearch_iface_init (IAnjutaEditorSearchIface *iface)
{
	iface->backward = isearch_backward;
	iface->forward = isearch_forward;
}

static gchar*
iselection_get (IAnjutaEditorSelection *iselection, GError **err)
{
	VimEditor *editor = (VimEditor*) iselection;
	gchar *result;
	gchar *selection;

	result = vim_dbus_query (editor->priv->widget, "AnjutaSelectionGet()", err);
	if (!strlen(result))
	{
		g_free(result);
		return NULL;
	}
	parse_vim_arr (result, NULL, NULL, &selection);
	g_free (result);
	return selection;
}

static IAnjutaIterable*
iselection_get_end (IAnjutaEditorSelection *iselection, GError **err)
{
	VimEditor *editor = (VimEditor*) iselection;
	gint end_pos;
	gchar* result;
	result = vim_dbus_query (editor->priv->widget, "AnjutaSelectionGet()", err);
	if (!strlen(result))
	{
		g_free(result);
		return NULL;
	}
	parse_vim_arr (result, NULL, &end_pos, NULL);
	g_free (result);
	return IANJUTA_ITERABLE(vim_cell_new (editor, end_pos));
}

static IAnjutaIterable*
iselection_get_start (IAnjutaEditorSelection *iselection, GError **err)
{
	VimEditor *editor = (VimEditor*) iselection;
	gint start_pos;
	gchar* result;
	result = vim_dbus_query (editor->priv->widget, "AnjutaSelectionGet()", err);
	if (!strlen(result))
	{
		g_free(result);
		return NULL;
	}
	parse_vim_arr (result, &start_pos, NULL, NULL);
	g_free (result);
	return IANJUTA_ITERABLE(vim_cell_new (editor, start_pos));
}

static gboolean
iselection_has_selection (IAnjutaEditorSelection *iselection, GError **err)
{
	VimEditor *editor = (VimEditor*) iselection;
	gchar *result;

	result = vim_dbus_query (editor->priv->widget, "mode()", err);
	if ((strcmp (result, "v") == 0) ||
			(strcmp (result, "V") ==0 ))
		return TRUE;
	else
		return FALSE;
}

static void
iselection_replace (IAnjutaEditorSelection *iselection, const gchar *text,
		gint length, GError **err)
{
	VimEditor *editor = (VimEditor*) iselection;
	gchar *text_ = str_substr (text, 0, length); 
	gchar *cmd = g_strdup_printf ("call AnjutaSelectionReplace('%s')", text_);
			
	vim_dbus_exec_without_reply (editor->priv->widget, cmd, err);
	g_free (cmd);
	g_free (text_);
}

static void
iselection_select_all (IAnjutaEditorSelection *iselection, GError **err)
{
	VimEditor *editor = (VimEditor*) iselection;

	vim_dbus_exec_without_reply (editor->priv->widget, 
			"call AnjutaSelectionMake('a')", 
			err);
	g_return_if_reached ();
}

static void
iselection_select_block (IAnjutaEditorSelection *iselection, GError **err)
{
	VimEditor *editor = (VimEditor*) iselection;
	vim_dbus_exec_without_reply (editor->priv->widget, 
			"call AnjutaSelectionMake('b')", 
			err);
}

static void
iselection_select_function (IAnjutaEditorSelection *iselection, GError **err)
{
	VimEditor *editor = (VimEditor*) iselection;
	vim_dbus_exec_without_reply (editor->priv->widget, 
			"call AnjutaSelectionMake('f')", 
			err);
}

static void
iselection_select_to_brace (IAnjutaEditorSelection *iselection, GError **err)
{
	VimEditor *editor = (VimEditor*) iselection;
	vim_dbus_exec_without_reply (editor->priv->widget, 
			"call AnjutaSelectionMake('b')", 
			err);
}

static void
iselection_set (IAnjutaEditorSelection *iselection, IAnjutaIterable* start,   IAnjutaIterable* end, gboolean scroll, GError **err)
{
	VimEditor *editor = (VimEditor*) iselection;
	VimEditorCell *start_ = VIM_CELL (start);
	VimEditorCell *end_ = VIM_CELL (end);
	gchar *cmd = g_strdup_printf ("call AnjutaSelectionMakePos(%d, %d)",
			vim_cell_get_position(start_),
			vim_cell_get_position(end_));
	vim_dbus_exec_without_reply (editor->priv->widget, cmd, err);
	g_free (cmd);
}

void 
iselection_iface_init (IAnjutaEditorSelectionIface *iface)
{
	iface->get = iselection_get;
	iface->get_end = iselection_get_end;
	iface->get_start = iselection_get_start;
	iface->has_selection = iselection_has_selection;
	iface->replace = iselection_replace;
	iface->select_all = iselection_select_all;
	iface->select_block = iselection_select_block;
	iface->select_function = iselection_select_function;
	iface->set = iselection_set;
}

struct _VimEditorMark {
    guint id;
    guint location;
    IAnjutaMarkableMarker marker;
};

typedef struct _VimEditorMark VimEditorMark;

VimEditorMark* vim_editor_mark_new (IAnjutaMarkableMarker marker, guint location, guint id)
{
    VimEditorMark *mark_ = g_new0(VimEditorMark, 1);
    mark_->location = location;
    mark_->marker = marker;
    mark_->id = id;
    return mark_;
}

gint
vim_editor_mark_compare_id (VimEditorMark *mark1, gint *id)
{
    if (mark1->id == *id)
        return 0;
    else if (mark1->id > *id)
        return 1;
    else if (mark1->id < *id)
        return -1;
}

gint
vim_editor_mark_compare_loc (VimEditorMark *mark1, gint *location)
{
    if (mark1->location == *location)
        return 0;
    else if (mark1->location > *location)
        return 1;
    else if (mark1->location < *location)
        return -1;
}

/* For the debugger only */
vim_editor_mark_changed (gint bufno, gint id, gint type)
{
}

static void
imarkable_unmark (IAnjutaMarkable *imarkable, gint location,   IAnjutaMarkableMarker marker, GError **err)
{
	VimEditor *editor = (VimEditor*) imarkable;
    VimEditorMark *mark;
    GList *node = g_list_find_custom (editor->priv->marks, &location, (GCompareFunc)vim_editor_mark_compare_loc);
    if (!node) return;
    mark = (VimEditorMark*) node->data;
    editor->priv->marks = g_list_remove (editor->priv->marks, node);
    gchar *cmd = g_strdup_printf ("call AnjutaMarkRemove(%d, %d)",
            editor->priv->bufno,
            mark->id);
    vim_dbus_exec_without_reply (editor->priv->widget, cmd, err);
}

static void
imarkable_delete_all_markers (IAnjutaMarkable *imarkable, IAnjutaMarkableMarker marker, GError **err)
{
	VimEditor *editor = (VimEditor*) imarkable;
    GList* node;
    for (node = editor->priv->marks; node != NULL ; node = g_list_next(node))
    {
        VimEditorMark *mark_ = (VimEditorMark*) node->data;
        if (mark_->marker == marker) imarkable_unmark (imarkable, mark_->location, marker, err);
    }
}

static gboolean
imarkable_is_marker_set (IAnjutaMarkable *imarkable, gint location,   IAnjutaMarkableMarker marker, GError **err)
{
	VimEditor *editor = (VimEditor*) imarkable;

    if (g_list_find_custom (editor->priv->marks, &location, (GCompareFunc)vim_editor_mark_compare_loc))
        return TRUE;
    else
        return FALSE;
}

static gint
imarkable_location_from_handle (IAnjutaMarkable *imarkable, gint handle, GError **err)
{
	VimEditor *editor = (VimEditor*) imarkable;
    VimEditorMark *mark;
    GList *node = g_list_find_custom (editor->priv->marks, &handle, (GCompareFunc)vim_editor_mark_compare_id);
    if (!node) return -1;
    mark = (VimEditorMark*) node->data;
    gchar *cmd = g_strdup_printf ("AnjutaMarkGetLine(%d, %d)",
            editor->priv->bufno,
            handle);
    mark->location = vim_dbus_int_query (editor->priv->widget, cmd, err);
    return mark->location;
}

static gint
imarkable_mark (IAnjutaMarkable *imarkable, gint location,   IAnjutaMarkableMarker marker, GError **err)
{
	VimEditor *editor = (VimEditor*) imarkable;
    static gint id = 1;
    gint type;
    switch (marker)
    {
        case IANJUTA_MARKABLE_LINEMARKER:
            type = 1;
            break;
        case IANJUTA_MARKABLE_BREAKPOINT_ENABLED:
            type = 2;
            break;
        case IANJUTA_MARKABLE_BREAKPOINT_DISABLED:
            type = 3;
            break;
        case IANJUTA_MARKABLE_PROGRAM_COUNTER:
            type = 4;
            break;
        case IANJUTA_MARKABLE_BOOKMARK:
            type = 5;
            break;
        default:
            return;
    }
    editor->priv->marks = g_list_prepend (editor->priv->marks, vim_editor_mark_new (marker, location, id));
    gchar *cmd = g_strdup_printf ("AnjutaMarkSet(%d, %d, %d, %d)",
            editor->priv->bufno,
            id,
            type,
            location);
    vim_dbus_int_query (editor->priv->widget, cmd, err);
    return id++;
}

void 
imarkable_iface_init (IAnjutaMarkableIface *iface)
{
	iface->delete_all_markers = imarkable_delete_all_markers;
	iface->is_marker_set = imarkable_is_marker_set;
	iface->location_from_handle = imarkable_location_from_handle;
	iface->mark = imarkable_mark;
	iface->unmark = imarkable_unmark;
}

/* Vim already provides this feature. */
static GList*
iassist_get_suggestions (IAnjutaEditorAssist *iassist, const gchar *context, GError **err)
{
	VimEditor *editor = (VimEditor*) iassist;
    return NULL;
}

static void
iassist_suggest (IAnjutaEditorAssist *iassist, GList* choices, IAnjutaIterable* ipos,
				 int char_alignment, GError **err)
{
	VimEditor *editor = (VimEditor*) iassist;
    gchar *cmd;
    GList* node;
    GString *str = g_string_new ("");

    for (node = choices; node != NULL; node = g_list_next(node))
        g_string_append_printf (str, "%s,", (gchar*)node->data);

    cmd = g_strdup_printf ("call AnjutaAssistSuggest ([%s])", g_string_free(str, FALSE));
    vim_dbus_exec_without_reply (editor->priv->widget, cmd, err);
}

static void
iassist_hide_suggestions (IAnjutaEditorAssist* iassist, GError** err)
{
	VimEditor *editor = (VimEditor*) iassist;
}

static void 
iassist_show_tips (IAnjutaEditorAssist *iassist, GList* tips, IAnjutaIterable* ipos,
				   gint char_alignment, GError **err)
{
	VimEditor *editor = (VimEditor*) iassist;
    gchar *cmd;
    GList* node;
    GString *str = g_string_new ("");

    for (node = tips; node != NULL; node = g_list_next(node))
        g_string_append_printf (str, "%s,", (gchar*)node->data);

    cmd = g_strdup_printf ("call AnjutaAssistSuggest ([%s])", g_string_free(str, FALSE));
    vim_dbus_exec_without_reply (editor->priv->widget, cmd, err);
}

static void
iassist_cancel_tips (IAnjutaEditorAssist* iassist, GError** err)
{
	VimEditor *editor = (VimEditor*) iassist;
}

void
iassist_iface_init (IAnjutaEditorAssistIface* iface)
{
	iface->suggest = iassist_suggest;
	iface->hide_suggestions = iassist_hide_suggestions;
	iface->get_suggestions = iassist_get_suggestions;
	iface->show_tips = iassist_show_tips;
	iface->cancel_tips = iassist_cancel_tips;
}

