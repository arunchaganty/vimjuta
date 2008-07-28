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

#include <gio/gio.h>
#include <libanjuta/interfaces/ianjuta-file.h>
#include <libanjuta/interfaces/ianjuta-file-savable.h>
#include <libanjuta/anjuta-debug.h>
#include "vim-widget.h"
#include "vim-editor.h"
#include "vim-widget-priv.h"
#include "vim-editor-priv.h"
#include "vim-dbus.h"

static GFile* 
ifile_get_file (IAnjutaFile *obj, GError **err) {
	VimEditor *editor = (VimEditor*) obj;
	return g_object_ref(editor->priv->file);
}

static void
ifile_open (IAnjutaFile *obj, GFile *file, GError **err) {
	VimEditor* editor = VIM_EDITOR(obj);
	gchar *cmd;
	g_assert (file != NULL);

	cmd = g_strdup_printf (":edit! %s", g_file_get_path(file));
	g_message ("Executing: %s", cmd);

	vim_dbus_exec_without_reply (editor->priv->widget, cmd, err);
}

void 
ifile_iface_init (IAnjutaFileIface *iface)
{
	iface->get_file = ifile_get_file;
	iface->open = ifile_open;
}

/* IAnjutaFileSavable Interface */

static gboolean
isave_is_dirty (IAnjutaFileSavable *obj, GError **err)
{
	VimEditor* editor = VIM_EDITOR (obj);
	return vim_dbus_int_query (editor->priv->widget, "&modified", err);
}

static gboolean
isave_is_read_only (IAnjutaFileSavable *obj, GError **err)
{
	VimEditor* editor = VIM_EDITOR (obj);
	return (gboolean) vim_dbus_int_query (editor->priv->widget, "&readonly", err);
}

static void
isave_save (IAnjutaFileSavable *obj, GError **err)
{
	VimEditor* editor = VIM_EDITOR (obj);
	vim_dbus_exec_without_reply (editor->priv->widget, ":w", err);
	g_signal_emit_by_name (obj,
			"saved",
			editor->priv->file);
}

static void
isave_save_as (IAnjutaFileSavable *obj, GFile *file, GError **err)
{
	VimEditor* editor = VIM_EDITOR (obj);
	gchar *cmd = NULL;

	cmd = g_strdup_printf (":write %s", g_file_get_path(file));
	vim_dbus_exec_without_reply (editor->priv->widget, cmd, err);

	g_free (cmd);
}

static void
isave_set_dirty (IAnjutaFileSavable *obj, gboolean dirty, GError **err)
{
	VimEditor* editor = VIM_EDITOR (obj);
	gchar *cmd = NULL;

	if (dirty) 
		cmd = g_strdup_printf (":set modified");
	else
		cmd = g_strdup_printf (":unset modified");
	vim_dbus_exec_without_reply (editor->priv->widget, cmd, err);

	g_free (cmd);
}

void isave_iface_init (IAnjutaFileSavableIface *iface)
{
	iface->is_dirty = isave_is_dirty;
	iface->is_read_only = isave_is_read_only;
	iface->save = isave_save;
	iface->save_as = isave_save_as;
	iface->set_dirty = isave_set_dirty;
}

