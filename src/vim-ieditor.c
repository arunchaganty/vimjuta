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
#include "vim-editor.h"
#include "vim-editor-priv.h"
#include "vim-widget-priv.h"
#include "vim-dbus.h"
#include "vim-cell.h"

static gint
ieditor_get_tabsize (IAnjutaEditor *ieditor, GError **err)
{
	VimEditor *editor = (VimEditor*) ieditor;
	g_return_val_if_fail (VIM_PLUGIN_IS_READY(editor->priv->widget), 4);
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
	g_return_val_if_fail (VIM_PLUGIN_IS_READY(editor->priv->widget), TRUE);
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
	query = g_strdup_printf (":%d ", line);

	vim_dbus_exec_without_reply (editor->priv->widget, query, err);
	
	// TODO: Error Handling...

	g_free (query);
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
	query = g_strdup_printf (":goto %d ", ianjuta_iterable_get_position (icell, err));

	vim_dbus_exec_without_reply (editor->priv->widget, query, err);
	
	// TODO: Error Handling...

	g_free (query);
}

static gchar* 
ieditor_get_text (IAnjutaEditor* ieditor, 
				IAnjutaIterable* start,
				IAnjutaIterable* end, GError **err)
{
	VimEditor *editor = (VimEditor*) ieditor;
	gchar* query = NULL;
	gchar* reply = NULL;
	g_return_val_if_fail (VIM_PLUGIN_IS_READY(editor->priv->widget), NULL);

	g_assert (err == NULL);
	// Create query string
	query = g_strdup_printf ("AnjutaGetBuf(%d, %d, %d)", 
			editor->priv->buf_id,
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

	g_return_val_if_fail (VIM_PLUGIN_IS_READY(editor->priv->widget),NULL);
	g_assert (err == NULL);
	query = g_strdup_printf ("AnjutaGetBuf(%d, 0, '$')", 
			editor->priv->buf_id);
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
	g_return_val_if_fail (VIM_PLUGIN_IS_READY(editor->priv->widget), NULL);

	g_assert (err == NULL);
	position = vim_dbus_int_query(editor->priv->widget, "AnjutaGetPos()", err);

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
	g_return_val_if_fail (VIM_PLUGIN_IS_READY(editor->priv->widget), 0);
	return vim_dbus_int_query (editor->priv->widget, "line ('.')", err);
}

/* Return the length of the text in the buffer */
/* FIXME: Returns the bytes in file. Doesn't match with the number of
 * characters in a multibyte file */
static gint 
ieditor_get_length(IAnjutaEditor *ieditor, GError **err)
{
	VimEditor *editor = (VimEditor*) ieditor;
	g_return_val_if_fail (VIM_PLUGIN_IS_READY(editor->priv->widget), 0);
	return vim_dbus_int_query (editor->priv->widget, "AnjutaPos('$')", err);
}

/* Return word on cursor position */
static gchar*
ieditor_get_current_word(IAnjutaEditor *ieditor, GError **err)
{
	VimEditor *editor = (VimEditor*) ieditor;
	g_return_val_if_fail (VIM_PLUGIN_IS_READY(editor->priv->widget), NULL);
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
	query = g_strdup_printf ("AnjutaInsert ('%s',%d)", text, position-1); /* the default insert is infact an append */

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
	query = g_strdup_printf ("AnjutaInsert (%d,'%s', '$')", editor->priv->buf_id, text);

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
	g_return_if_fail (VIM_PLUGIN_IS_READY(editor->priv->widget));

	g_assert (err == NULL);
	// Create query string
	query = g_strdup_printf ("AnjutaErase(%d, %d)", 
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
	g_return_val_if_fail (VIM_PLUGIN_IS_READY(editor->priv->widget), 0);

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
	g_return_val_if_fail (VIM_PLUGIN_IS_READY(editor->priv->widget), 0);

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
	g_return_val_if_fail (VIM_PLUGIN_IS_READY(editor->priv->widget), NULL);

	query = g_strdup_printf ("line2byte(%d)", line);
	position = vim_dbus_int_query (editor->priv->widget, query, err);

	vim_cell_new (editor, position);
	
	return IANJUTA_ITERABLE(cell);
}

static IAnjutaIterable*
ieditor_get_line_end_position(IAnjutaEditor *ieditor,
											gint line, GError **err)
{
	VimEditor *editor = (VimEditor*) ieditor;
	VimEditorCell *cell = NULL;
	gint position = 0;
	gchar* query = NULL;
	g_return_val_if_fail (VIM_PLUGIN_IS_READY(editor->priv->widget), NULL);

	query = g_strdup_printf ("line2byte(%d+1)", line);
	position = vim_dbus_int_query (editor->priv->widget, query, err) - 1;

	vim_cell_new (editor, position);
	
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
	g_return_val_if_fail (VIM_PLUGIN_IS_READY(editor->priv->widget), IANJUTA_ITERABLE(cell));

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

static void
imultiple_add_document (IAnjutaEditorMultiple *obj, IAnjutaDocument *document, GError **err)
{
	VimEditor* editor = VIM_EDITOR (obj);
	editor->priv->widget->priv->documents = g_list_append (editor->priv->widget->priv->documents, document);

	ianjuta_file_open (IANJUTA_FILE (editor), ianjuta_file_get_uri(IANJUTA_FILE (editor), NULL), NULL);
}

static IAnjutaDocument*
imultiple_get_current_document (IAnjutaEditorMultiple *obj, GError **err)
{
	VimEditor* editor = VIM_EDITOR (obj);
	gint buf_id;
	GList* node = editor->priv->widget->priv->documents; 
	
	buf_id = vim_dbus_int_query (editor->priv->widget, "bufnr('%')", err);

	for (;node != NULL; node = g_list_next(node))
	{
		VimEditor *editor_ = VIM_EDITOR (node->data);
		if (editor_->priv->buf_id == buf_id)
			return IANJUTA_DOCUMENT(editor_);
	}

	g_return_val_if_reached (NULL);
}

static GtkWidget*
imultiple_get_widget (IAnjutaEditorMultiple *obj, GError **err)
{
	VimEditor* editor = VIM_EDITOR (obj);

	return GTK_WIDGET(editor->priv->widget);
}

static gboolean
imultiple_has_document (IAnjutaEditorMultiple *obj, IAnjutaDocument *document, GError **err)
{
	VimEditor* editor = VIM_EDITOR (obj);
	GList* node = editor->priv->widget->priv->documents; 

	if (g_list_find (node, document))
		return TRUE;
	else
		return FALSE;
}

static GList*
imultiple_list_documents (IAnjutaEditorMultiple *obj, GError **err)
{
	VimEditor* editor = VIM_EDITOR (obj);
	return g_list_copy (editor->priv->widget->priv->documents);
}

static void
imultiple_remove_document (IAnjutaEditorMultiple *obj, IAnjutaDocument *document, GError **err)
{
	VimEditor* editor = VIM_EDITOR (obj);
	editor->priv->widget->priv->documents = g_list_remove (editor->priv->widget->priv->documents, document);

	vim_dbus_exec_without_reply (editor->priv->widget, ":bd", err);
}

static void
imultiple_set_current_document (IAnjutaEditorMultiple *obj, IAnjutaDocument *document, GError **err)
{
	VimEditor* editor = VIM_EDITOR (obj);
	gint buf_id = VIM_EDITOR (document)->priv->buf_id;
	gchar* cmd = NULL;
	
	cmd = g_strdup_printf (":buffer %d", buf_id);
	vim_dbus_exec_without_reply (editor->priv->widget, cmd, err);
	g_free (cmd);
}

void 
imultiple_iface_init (IAnjutaEditorMultipleIface *iface)
{
	iface->add_document = imultiple_add_document;
	iface->get_current_document = imultiple_get_current_document;
	iface->get_widget = imultiple_get_widget;
	iface->has_document = imultiple_has_document;
	iface->list_documents = imultiple_list_documents;
	iface->remove_document = imultiple_remove_document;
	iface->list_documents = imultiple_list_documents;
	iface->remove_document = imultiple_remove_document;
	iface->set_current_document = imultiple_set_current_document;
}
