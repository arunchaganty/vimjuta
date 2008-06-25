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

/**
 * This actually takes care of all transactions with Vim.
 */

#include <libanjuta/anjuta-debug.h>
#include <gtk/gtk.h>
#include "vim-widget.h"
#include "vim-editor.h"
#include "vim-widget-priv.h"
#include "vim-editor-priv.h"
#include "vim-dbus.h"

#define VIM_WIDGET_PRIVATE(o)  (G_TYPE_INSTANCE_GET_PRIVATE ((o), VIM_TYPE_WIDGET, VimWidgetPrivate))
#define VIM_DBUS_FILE "/home/teju/Projects/anjuta-gvim/src/vim-dbus.py"
#define GVIMRC_FILE "/home/teju/Projects/anjuta-gvim/misc/anjuta.gvimrc"

/* 
 * Once the window is parented, call this function to embed vim
 */

static GObjectClass* parent_class;

void
vim_widget_add_document (VimWidget *widget, VimEditor *editor, GError **err)
{
	if (!g_list_find (widget->priv->documents, editor))
		widget->priv->documents = g_list_append (widget->priv->documents, editor);
}

void
vim_widget_remove_document (VimWidget *widget, VimEditor *editor, GError **err)
{
	if (!g_list_find (widget->priv->documents, editor))
		widget->priv->documents = g_list_remove (widget->priv->documents, editor);
}

VimEditor*
vim_widget_get_document_bufno (VimWidget *widget, const guint bufno, GError **err)
{
	GList* node = NULL;
	for (node = widget->priv->documents; node != NULL; node = g_list_next (node))
	{
		VimEditor *editor = VIM_EDITOR(node->data);
		if (editor->priv->bufno = bufno)
			return editor;
	}
	return NULL;
}

VimEditor*
vim_widget_get_document_filename (VimWidget *widget, const gchar* filename, GError **err)
{
	GList* node = NULL;
	for (node = widget->priv->documents; node != NULL; node = g_list_next (node))
	{
		VimEditor *editor = VIM_EDITOR(node->data);
		if (strcmp (editor->priv->filename, filename) == 0)
			return editor;
	}
	return NULL;
}

VimEditor*
vim_widget_get_document_uri (VimWidget *widget, const gchar* uri, GError **err)
{
	GList* node = NULL;
	for (node = widget->priv->documents; node != NULL; node = g_list_next (node))
	{
		VimEditor *editor = VIM_EDITOR(node->data);
		if (strcmp (editor->priv->uri, uri) == 0)
			return editor;
	}
	return NULL;
}

static void 
vim_widget_connect_plug (VimWidget *widget, GParamSpec *param) 
{
	gchar *cmd;
	GError *err = NULL;
	GtkNotebook *parent;

	g_object_get (widget,
			"parent", &parent,
			NULL);


	if (parent != NULL)
	{
		widget->priv->socket_id = gtk_socket_get_id ((GtkSocket *) widget->priv->socket);
		g_assert (widget->priv->socket_id != 0);
		
		cmd = g_strdup_printf ("gvim -U %s --socketid %d \n",
				GVIMRC_FILE, widget->priv->socket_id);
		g_message ("Executing %s\n", cmd);
		/* Run vim */
		g_spawn_command_line_async (cmd, &err);
		g_free (cmd);

		if (err)
		{
			DEBUG_PRINT ("VimPlugin: Error: %s", err);
			g_object_unref (err);
			err = NULL;
		}

		/* Connect callbacks */
	}

}

static GObject*
vim_widget_constructor ( GType   type,
						guint   n_construct_properties,
						GObjectConstructParam *construct_properties)
{
    GObject* obj;
	GError *err = NULL;
    VimWidgetClass* klass;
	VimWidget* widget;
	VimWidgetPrivate *priv;
	
    klass = VIM_WIDGET_CLASS (g_type_class_peek (VIM_TYPE_WIDGET));

	if (klass->widget)
		return g_object_ref (klass->widget);

	widget = VIM_WIDGET (parent_class->constructor (type,
						   n_construct_properties,
						   construct_properties));
	klass->widget = widget;
	priv = widget->priv;

	/* Create a socket */
	priv->socket = (GtkSocket*) g_object_new (GTK_TYPE_SOCKET,
			"visible", TRUE,
			NULL);
	g_assert (widget != NULL && priv->socket != NULL);
	gtk_container_add (GTK_CONTAINER(widget), GTK_WIDGET(priv->socket));
	g_signal_connect_after (widget,
						"notify::parent",
						G_CALLBACK(vim_widget_connect_plug),
						NULL);
	
	// Now hook in DBus
	vim_dbus_init(widget, &err);
	if (err) 
	{
		DEBUG_PRINT ("VimPlugin: Error: %s", err);
		g_object_unref (err);
		err = NULL;
	}

	return G_OBJECT(widget);
}

static void
vim_widget_init (VimWidget *widget)
{
	widget->priv = g_new0 (VimWidgetPrivate, 1);
	widget->priv->documents = NULL; /* NULL is the empty GList */
}

static void
vim_widget_finalize (GObject *object)
{
	/* TODO: Add deinitalization code here */

	VimWidget* widget = VIM_WIDGET (object);
    VimWidgetClass* klass = VIM_WIDGET_CLASS (g_type_class_peek (VIM_TYPE_WIDGET));
	VimWidgetPrivate *priv = VIM_WIDGET_PRIVATE(widget);
	
	g_list_free (priv->documents);

	g_free (priv->conn);
	g_free (priv->dbus_proxy);
	g_free (priv->proxy);

	klass->widget = NULL;
	parent_class->finalize (object);
}

static void
vim_widget_class_init (VimWidgetClass *klass)
{
    parent_class = G_OBJECT_CLASS (g_type_class_peek_parent (klass));
	GObjectClass* object_class = G_OBJECT_CLASS (klass);

	g_type_class_add_private (klass, sizeof (VimWidgetPrivate));
	object_class->constructor = vim_widget_constructor;
	object_class->finalize = vim_widget_finalize;

	klass->widget = NULL; /* Singleton hasn't been initialized yet */

}

G_DEFINE_TYPE (VimWidget, vim_widget, GTK_TYPE_FRAME);

