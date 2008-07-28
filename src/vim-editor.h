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

#ifndef _VIM_EDITOR_H_
#define _VIM_EDITOR_H_

#include <glib-object.h>
#include <gio/gio.h>
#include <libanjuta/anjuta-plugin.h>
#include "vim-widget.h"

G_BEGIN_DECLS

#define VIM_TYPE_EDITOR             (vim_editor_get_type ())
#define VIM_EDITOR(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), VIM_TYPE_EDITOR, VimEditor))
#define VIM_EDITOR_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), VIM_TYPE_EDITOR, VimEditorClass))
#define VIM_IS_EDITOR(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), VIM_TYPE_EDITOR))
#define VIM_IS_EDITOR_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), VIM_TYPE_EDITOR))
#define VIM_EDITOR_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), VIM_TYPE_EDITOR, VimEditorClass))

typedef struct _VimEditorClass VimEditorClass;
typedef struct _VimEditor VimEditor;
typedef struct _VimEditorPrivate VimEditorPrivate;

struct _VimEditorClass
{
	GtkObjectClass parent_class;
};

struct _VimEditor
{
	GtkObject parent;
	VimEditorPrivate *priv;
};

GType vim_editor_get_type (void) G_GNUC_CONST;
VimEditor* vim_editor_new (AnjutaPlugin *plugin, GFile* file); 
void vim_editor_update_variables (VimEditor *editor); 

G_END_DECLS

#endif /* _VIM_EDITOR_H_ */
