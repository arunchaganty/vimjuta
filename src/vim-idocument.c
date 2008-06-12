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
 */

#include <libanjuta/interfaces/ianjuta-document.h>
#include "vim-editor.h"

static void idocument_begin_undo_action (IAnjutaDocument *doc, GError **err)
{
	/* Support for undo blocks is hard */
}

static gboolean 
idocument_can_redo (IAnjutaDocument *doc, GError **err)
{
	return TRUE;
}

static gboolean 
idocument_can_undo (IAnjutaDocument *doc, GError **err)
{
	return TRUE;
}

static void 
idocument_clear (IAnjutaDocument *doc, GError **err)
{
	VimEditor *vim = (VimEditor*) doc;
	g_return_if_fail (VIM_PLUGIN_IS_READY(vim));

	g_assert (err == NULL);
	// Create query string
	vim_dbus_exec_without_reply (vim, "<ESC><ESC>", err);
	
	// TODO: Error Handling...

}

static void 
idocument_copy (IAnjutaDocument *doc, GError **err)
{
	VimEditor *vim = (VimEditor*) doc;
	g_return_if_fail (VIM_PLUGIN_IS_READY(vim));

	g_assert (err == NULL);
	// Create query string
	vim_dbus_exec_without_reply (vim, ":yank \"*", err);
	
	// TODO: Error Handling...
}

static void 
idocument_cut (IAnjutaDocument *doc, GError **err)
{
	VimEditor *vim = (VimEditor*) doc;
	g_return_if_fail (VIM_PLUGIN_IS_READY(vim));

	g_assert (err == NULL);
	// Create query string
	vim_dbus_exec_without_reply (vim, ":yank \"*", err);
	
	// TODO: Error Handling...
}

static void 
idocument_end_undo_action (IAnjutaDocument *doc, GError **err)
{
	/* Support for undo blocks is hard */
}

static const gchar* 
idocument_get_filename (IAnjutaDocument *doc, GError **err)
{
	VimEditor* vim = (VimEditor*) doc;
	return g_strdup(vim->filename);
}

static void 
idocument_grab_focus (IAnjutaDocument *doc, GError **err)
{
	VimEditor* vim = (VimEditor*) doc;
	gtk_widget_grab_focus (GTK_WIDGET(&vim->parent));
}

static void 
idocument_paste (IAnjutaDocument *doc, GError **err)
{
	VimEditor *vim = (VimEditor*) doc;
	g_return_if_fail (VIM_PLUGIN_IS_READY(vim));

	g_assert (err == NULL);
	// Create query string
	vim_dbus_exec_without_reply (vim, ":put \"*", err);
	
	// TODO: Error Handling...
}

static void 
idocument_redo (IAnjutaDocument *doc, GError **err)
{
	VimEditor *vim = (VimEditor*) doc;
	g_return_if_fail (VIM_PLUGIN_IS_READY(vim));

	g_assert (err == NULL);
	// Create query string
	vim_dbus_exec_without_reply (vim, ":redo", err);
	
	// TODO: Error Handling...

}

static void 
idocument_undo (IAnjutaDocument *doc, GError **err)
{
	VimEditor *vim = (VimEditor*) doc;
	g_return_if_fail (VIM_PLUGIN_IS_READY(vim));

	g_assert (err == NULL);
	// Create query string
	vim_dbus_exec_without_reply (vim, ":undo", err);
	
	// TODO: Error Handling...

}

void 
idocument_iface_init (IAnjutaDocumentIface *iface)
{
	iface->begin_undo_action = idocument_begin_undo_action;
	iface->can_redo = idocument_can_redo;
	iface->can_undo = idocument_can_undo;
	iface->clear = idocument_clear;
	iface->copy = idocument_copy;
	iface->cut = idocument_cut;
	iface->end_undo_action = idocument_end_undo_action;
	iface->get_filename = idocument_get_filename;
	iface->grab_focus = idocument_grab_focus;
	iface->paste = idocument_paste;
	iface->redo = idocument_redo;
	iface->undo = idocument_undo;
}

