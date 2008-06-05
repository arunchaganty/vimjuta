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

/* DBus Helper Functions */
#ifndef VIM_DBUS_H_
#define VIM_DBUS_H_

#include "vim-editor.h"

#define DBUS_NAME_ANJUTA "org.anjuta"
#define DBUS_PATH_VIM "/org/anjuta/vim/daemon"
#define DBUS_IFACE_EDITOR_REMOTE "org.editors.remote"

gboolean vim_dbus_init (VimEditor *editor, GError **error);
gchar* vim_dbus_query (VimEditor *editor, gchar* query, GError **error);
gchar* vim_dbus_exec (VimEditor* editor, gchar* cmd, GError **error);
void vim_dbus_exec_without_reply (VimEditor* editor, gchar* cmd, GError **error);
gchar* vim_dbus_get_buf (VimEditor *editor, guint start, guint end, GError **error);
gchar* vim_dbus_get_buf_full (VimEditor *editor, GError **error);

#endif /* VIM-DBUS_H_ */
