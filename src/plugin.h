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

extern GType anjuta_gvim_plugin_get_type ();
#define ANJUTA_TYPE_PLUGIN_GVIM         (anjuta_gvim_plugin_get_type ())
#define ANJUTA_PLUGIN_GVIM(o)           (G_TYPE_CHECK_INSTANCE_CAST ((o), ANJUTA_TYPE_PLUGIN_GVIM, VimPlugin))
#define ANJUTA_PLUGIN_GVIM_CLASS(k)     (G_TYPE_CHECK_CLASS_CAST ((k), ANJUTA_TYPE_PLUGIN_GVIM, VimPluginClass))
#define ANJUTA_IS_PLUGIN_GVIM(o)        (G_TYPE_CHECK_INSTANCE_TYPE ((o), ANJUTA_TYPE_PLUGIN_GVIM))
#define ANJUTA_IS_PLUGIN_GVIM_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE ((k), ANJUTA_TYPE_PLUGIN_GVIM))
#define ANJUTA_PLUGIN_GVIM_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), ANJUTA_TYPE_PLUGIN_GVIM, VimPluginClass))

typedef struct _VimPlugin VimPlugin;
typedef struct _VimPluginClass VimPluginClass;

struct _VimPlugin{
	AnjutaPlugin parent;
	GtkWidget *widget;


};

struct _VimPluginClass{
	AnjutaPluginClass parent_class;
};

#endif
