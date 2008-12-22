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

#ifndef _VIM_WIDGET_PRIV_H_
#define _VIM_WIDGET_PRIV_H_

#include <glib-object.h>
#include "vim-widget.h"
#include <dbus/dbus.h>
#include <dbus/dbus-glib.h>

struct _VimWidgetPrivate
{
	gint socket_id;
	GtkSocket* socket;
	DBusGConnection* conn;
	DBusGProxy* dbus_proxy;
	DBusGProxy* proxy;
	gchar *servername;
	GdkWindow *vim;

	GPtrArray* documents; 
	GPtrArray* unloaded; 
	gint page_no;
	gboolean registered;
	VimEditor *current_editor;
};

#endif /* _VIM_WIDGET_PRIV_H_ */
