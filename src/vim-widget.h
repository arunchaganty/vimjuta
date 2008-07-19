/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * anjuta-gvim
 * Copyright (C) Arun Tejasvi Chaganty 2008 <arunchaganty@gmail.com>
 * 
 * anjuta-gvim is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * anjuta-gvim is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _VIM_WIDGET_H_
#define _VIM_WIDGET_H_

#include <glib-object.h>
#include <gtk/gtk.h>
#include "vim-editor.h"

G_BEGIN_DECLS

#define VIM_TYPE_WIDGET             (vim_widget_get_type ())
#define VIM_WIDGET(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), VIM_TYPE_WIDGET, VimWidget))
#define VIM_WIDGET_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), VIM_TYPE_WIDGET, VimWidgetClass))
#define VIM_IS_WIDGET(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), VIM_TYPE_WIDGET))
#define VIM_IS_WIDGET_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), VIM_TYPE_WIDGET))
#define VIM_WIDGET_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), VIM_TYPE_WIDGET, VimWidgetClass))
#define VIM_PLUGIN_IS_READY(widget)	((widget->priv->socket_id != 0) && (widget->priv->proxy != NULL))

typedef struct _VimWidgetClass VimWidgetClass;
typedef struct _VimWidget VimWidget;
typedef struct _VimWidgetPrivate VimWidgetPrivate;

struct _VimWidgetClass
{
	GtkFrameClass parent_class;
	VimWidget *widget; // Singleton
};

struct _VimWidget
{
	GtkFrame parent_instance;
	VimWidgetPrivate* priv;
};

GType vim_widget_get_type (void) G_GNUC_CONST;
void vim_widget_add_document (VimWidget *widget, VimEditor* editor, GError **err);
void vim_widget_add_document_complete (VimWidget *widget, VimEditor *editor);
void vim_widget_remove_document (VimWidget *widget, VimEditor* editor, GError **err);
void vim_widget_remove_document_complete (VimWidget *widget, VimEditor *editor);
VimEditor* vim_widget_get_document_bufno (VimWidget *widget, const guint bufno, GError **err);
VimEditor* vim_widget_get_document_file (VimWidget *widget, GFile* file, GError **err);
VimEditor* vim_widget_get_document_filename (VimWidget *widget, const gchar* filename, GError **err);
VimEditor* vim_widget_get_document_uri (VimWidget *widget, const gchar* uri, GError **err);
void vim_widget_set_current_editor (VimWidget *widget, VimEditor *editor, GError **err);

G_END_DECLS

#endif /* _VIM_WIDGET_H_ */
