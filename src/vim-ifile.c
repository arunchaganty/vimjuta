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

#include <libanjuta/interfaces/ianjuta-file.h>
#include <libanjuta/interfaces/ianjuta-file-savable.h>
#include "vim-editor.h"

static gchar* 
ifile_get_uri (IAnjutaFile *obj, GError **err) {
	VimEditor *vim = (VimEditor*) obj;
	return g_strdup(vim->uri);
}

static void
ifile_open (IAnjutaFile *obj, const gchar *uri, GError **err) {

}

void 
ifile_iface_init (IAnjutaFileIface *iface)
{
	iface->get_uri = ifile_get_uri;
	iface->open = ifile_open;
}

/* IAnjutaFileSavable Interface */

static gboolean
isave_is_dirty (IAnjutaFileSavable *obj, GError **err)
{
	VimEditor* vim = VIM_EDITOR (obj);
	g_return_val_if_fail (VIM_PLUGIN_IS_READY(vim), TRUE);
	return vim_dbus_int_query (vim, "&modified", err);
}

static void
isave_save (IAnjutaFileSavable *obj, GError **err)
{
	VimEditor* vim = VIM_EDITOR (obj);
	g_return_if_fail (VIM_PLUGIN_IS_READY(vim));
	vim_dbus_exec_without_reply (vim, ":w", err);
}

static void
isave_save_as (IAnjutaFileSavable *obj, const gchar *uri, GError **err)
{
	VimEditor* vim = VIM_EDITOR (obj);
	gchar *cmd = NULL;
	g_return_if_fail (VIM_PLUGIN_IS_READY(vim));

	cmd = g_strdup_printf (cmd, ":w %s", uri);
	vim_dbus_exec_without_reply (vim, cmd, err);

	g_free (cmd);
}

static void
isave_set_dirty (IAnjutaFileSavable *obj, gboolean dirty, GError **err)
{
	VimEditor* vim = VIM_EDITOR (obj);
	gchar *cmd = NULL;
	g_return_if_fail (VIM_PLUGIN_IS_READY(vim));

	if (dirty) 
		cmd = g_strdup_printf (cmd, ":set modified");
	else
		cmd = g_strdup_printf (cmd, ":unset modified");
	vim_dbus_exec_without_reply (vim, cmd, err);

	g_free (cmd);
}

void isave_iface_init (IAnjutaFileSavableIface *iface)
{
	iface->is_dirty = isave_is_dirty;
	iface->save = isave_save;
	iface->save_as = isave_save_as;
	iface->set_dirty = isave_set_dirty;
}

