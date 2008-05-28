/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * plugin.h
 * Copyright (C) Arun Tejasvi Chaganty 2008 <arunchaganty@gmail.com>
 * 
 * plugin.h is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * plugin.h is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _ANJUTA_GVIM_H_
#define _ANJUTA_GVIM_H_

#include <libanjuta/anjuta-plugin.h>

typedef struct _VimPlugin VimPlugin;
typedef struct _VimPluginClass VimPluginClass;

struct _VimPlugin{
	AnjutaPlugin parent;
	GtkWidget *widget;
	guint socket_id;

};

struct _VimPluginClass{
	AnjutaPluginClass parent_class;
};

#endif
