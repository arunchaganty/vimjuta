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
#include <libanjuta/interfaces/ianjuta-editor-master.h>
#include <libanjuta/interfaces/ianjuta-editor-multiple.h>
#include <libanjuta/interfaces/ianjuta-file.h>
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
	g_assert (editor != NULL);
	if (!g_list_find (widget->priv->documents, editor))
	{
		widget->priv->documents = g_list_prepend (widget->priv->documents, editor);
		/* Buffer is assigned a buf_no on load (by vim) */
		if (!editor->priv->bufno)
			ianjuta_file_open (IANJUTA_FILE (editor), ianjuta_file_get_uri(IANJUTA_FILE (editor), NULL), NULL);
		g_signal_emit_by_name (IANJUTA_EDITOR_MASTER(widget),
				"document-added",
				G_OBJECT(editor));
	}
}

void
vim_widget_remove_document (VimWidget *widget, VimEditor *editor, GError **err)
{
	g_return_if_fail (editor != NULL);
	if (!g_list_find (widget->priv->documents, editor))
	{
		widget->priv->documents = g_list_remove (widget->priv->documents, editor);
		g_signal_emit_by_name (IANJUTA_EDITOR_MASTER(widget),
				"document-removed",
				G_OBJECT(editor));
	}
}

VimEditor*
vim_widget_get_document_bufno (VimWidget *widget, const guint bufno, GError **err)
{
	GList* node = NULL;
	for (node = widget->priv->documents; node != NULL; node = g_list_next (node))
	{
		VimEditor *editor = VIM_EDITOR(node->data);
		if (editor->priv->bufno == bufno)
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
vim_widget_connect_plug (VimWidget *widget) 
{
	gchar *cmd;
	GError *err = NULL;
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

/* IAnjutaEditorMaster */

static void
imaster_add_document (IAnjutaEditorMaster *obj, IAnjutaDocument *document, GError **err)
{
	VimWidget* widget = VIM_WIDGET (obj);
	VimEditor* editor = VIM_EDITOR (document);
	vim_widget_add_document (widget, editor, err);
}

static IAnjutaDocument*
imaster_get_current_document (IAnjutaEditorMaster *obj, GError **err)
{
	VimWidget* widget = VIM_WIDGET (obj);
	return IANJUTA_DOCUMENT(widget->priv->current_editor);
}

static gboolean
imaster_has_document (IAnjutaEditorMaster *obj, IAnjutaDocument *document, GError **err)
{
	VimWidget* widget = VIM_WIDGET (obj);
	GList* node = widget->priv->documents; 

	if (g_list_find (node, document))
		return TRUE;
	else
		return FALSE;
}

static gboolean
imaster_is_registered (IAnjutaEditorMaster *obj, GError **err)
{
	VimWidget* widget = VIM_WIDGET (obj);
	return widget->priv->registered;
}

static GList*
imaster_list_documents (IAnjutaEditorMaster *obj, GError **err)
{
	VimWidget* widget = VIM_WIDGET (obj);
	return g_list_copy (widget->priv->documents);
}

static void
imaster_remove_document (IAnjutaEditorMaster *obj, IAnjutaDocument *document, GError **err)
{
	VimWidget* widget = VIM_WIDGET (obj);
	widget->priv->documents = g_list_remove (widget->priv->documents, document);

	vim_dbus_exec_without_reply (widget, ":bd", err);
}

static void
imaster_set_current_document (IAnjutaEditorMaster *obj, IAnjutaDocument *document, GError **err)
{
	VimWidget* widget = VIM_WIDGET (obj);
	VimEditor* editor_ = VIM_EDITOR (document);
	gint bufno = editor_->priv->bufno;
	gchar* cmd = NULL;
	
	cmd = g_strdup_printf (":buffer %d", bufno);
	vim_dbus_exec_without_reply (widget, cmd, err);
	g_free (cmd);
}

static void
imaster_set_registered (IAnjutaEditorMaster *obj, gboolean state, GError **err)
{
	VimWidget* widget = VIM_WIDGET (obj);
	widget->priv->registered = state;
}

void 
imaster_iface_init (IAnjutaEditorMasterIface *iface)
{
	iface->add_document = imaster_add_document;
	iface->get_current_document = imaster_get_current_document;
	iface->has_document = imaster_has_document;
	iface->is_registered = imaster_is_registered;
	iface->list_documents = imaster_list_documents;
	iface->remove_document = imaster_remove_document;
	iface->list_documents = imaster_list_documents;
	iface->set_current_document = imaster_set_current_document;
	iface->set_registered = imaster_set_registered;
}

/* Signal Callbacks */
void 
vim_signal_buf_new_file_cb (DBusGProxy *proxy, const guint bufno, 
		VimWidget *widget)
{
	VimEditor* editor = vim_editor_new (NULL, "", "");
	vim_widget_add_document (widget, editor, NULL);
}

void 
vim_signal_buf_read_cb (DBusGProxy *proxy, const guint bufno, 
		const gchar* filename, VimWidget *widget)
{
	
	VimEditor* editor = vim_widget_get_document_bufno (widget, bufno, NULL);
	if (!editor)
		editor = vim_editor_new (NULL, "", filename);
	vim_widget_add_document (widget, editor, NULL);
	g_message ("f00 %d %s\n", bufno, filename);
}

void 
vim_signal_buf_write_cb (DBusGProxy *proxy, const guint bufno, 
		const gchar* filename, VimWidget *widget)
{
	/* Nothing needs to be done here */
}

void 
vim_signal_buf_add_cb (DBusGProxy *proxy, const guint bufno, 
		const gchar* filename, VimWidget *widget)
{
	VimEditor *editor = vim_editor_new (NULL,  "", filename);
	editor->priv->bufno = bufno;
	vim_widget_add_document (widget, editor, NULL);
	g_message ("f00 %d %s\n", bufno, filename);
}


void 
vim_signal_buf_delete_cb (DBusGProxy *proxy, const guint bufno, 
		VimWidget *widget)
{
	VimEditor *editor = vim_widget_get_document_bufno (widget, bufno, NULL);
	vim_widget_remove_document (widget, editor, NULL);
}

/* Filename may have changed */
void 
vim_signal_buf_file_post_cb (DBusGProxy *proxy, const guint bufno, 
		const gchar* filename, VimWidget *widget)
{
	VimEditor *editor = vim_widget_get_document_bufno (widget, bufno, NULL);
	if (strcmp (editor->priv->filename, filename) != 0)
	{
		g_free (editor->priv->filename);
		editor->priv->filename = g_strdup (filename);
	}
}

void 
vim_signal_buf_enter_cb (DBusGProxy *proxy, const guint bufno, 
		const gchar* filename, VimWidget *widget)
{
	VimEditor *editor = vim_widget_get_document_bufno (widget, bufno, NULL);
	/* Sometimes vim opens new files with BufEnter */
	if (!editor)
	{
		editor = vim_editor_new (NULL, "", filename);
		editor->priv->bufno = bufno;
		vim_widget_add_document (widget, editor, NULL);
	}
	g_signal_emit_by_name (IANJUTA_EDITOR_MASTER(widget),
			"current-document-changed",
			G_OBJECT(editor));
	widget->priv->current_editor = editor;
}

void 
vim_signal_buf_leave_cb (DBusGProxy *proxy, const guint bufno, 
		VimWidget *widget)
{
	VimEditor *editor = vim_widget_get_document_bufno (widget, bufno, NULL);
	/* Do nothing */
}

void 
vim_signal_vim_leave_cb (DBusGProxy *proxy, VimWidget *widget)
{
	g_object_run_dispose (G_OBJECT(widget));
	/* Do nothing */
}

void 
vim_signal_menu_popup_cb (DBusGProxy *proxy, const guint bufno, 
		VimWidget *widget)
{
	VimEditor *editor = vim_widget_get_document_bufno (widget, bufno, NULL);
	/* Do nothing, yet */
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
						"realize",
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
vim_widget_instance_init (VimWidget *widget)
{
	widget->priv = g_new0 (VimWidgetPrivate, 1);
	widget->priv->documents = NULL; /* NULL is the empty GList */
}

static void
vim_widget_dispose (GObject *object)
{
	/* TODO: Add deinitalization code here */

	VimWidget* widget = VIM_WIDGET (object);
    VimWidgetClass* klass = VIM_WIDGET_CLASS (g_type_class_peek (VIM_TYPE_WIDGET));
	VimWidgetPrivate *priv = VIM_WIDGET_PRIVATE(widget);
	GList* node = priv->documents;
	
	for (;node != NULL; node = g_list_next (node))
		g_free (node->data);
	g_list_free (priv->documents);

	priv->socket_id = 0;
	g_free (priv->socket);


	g_free (priv->conn);
	g_free (priv->dbus_proxy);
	g_free (priv->proxy);

	priv->current_editor = NULL;
	priv->registered = FALSE;
	gboolean registered;

	g_object_unref (klass->widget);
	parent_class->dispose (object);
}

static void
vim_widget_finalize (GObject *object)
{
	/* TODO: Add deinitalization code here */
}

static void
vim_widget_class_init (VimWidgetClass *klass)
{
    parent_class = G_OBJECT_CLASS (g_type_class_peek_parent (klass));
	GObjectClass* object_class = G_OBJECT_CLASS (klass);

	g_type_class_add_private (klass, sizeof (VimWidgetPrivate));
	object_class->constructor = vim_widget_constructor;
	object_class->dispose = vim_widget_dispose;
	object_class->finalize = vim_widget_finalize;

	klass->widget = NULL; /* Singleton hasn't been initialized yet */

}

ANJUTA_TYPE_BEGIN (VimWidget, vim_widget, GTK_TYPE_FRAME);
ANJUTA_TYPE_ADD_INTERFACE(imaster, IANJUTA_TYPE_EDITOR_MASTER);
ANJUTA_TYPE_END;

