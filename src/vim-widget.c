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
#include <gio/gio.h>
#include "vim-widget.h"
#include "vim-editor.h"
#include "vim-widget-priv.h"
#include "vim-editor-priv.h"
#include "vim-dbus.h"

#define VIM_WIDGET_PRIVATE(o)  (G_TYPE_INSTANCE_GET_PRIVATE ((o), VIM_TYPE_WIDGET, VimWidgetPrivate))
#define VIM_DBUS_FILE "/home/teju/Projects/anjuta-gvim/src/vim-dbus.py"
#define GVIMRC_FILE "/home/teju/Projects/anjuta-gvim/misc/anjuta.gvimrc"

static GObjectClass* parent_class;

void
vim_widget_add_document (VimWidget *widget, VimEditor *editor, GError **err)
{
	g_assert (editor != NULL);
	if (!vim_widget_has_editor (widget, editor))
		ianjuta_file_open (IANJUTA_FILE (editor), editor->priv->file, NULL);
}

/* Complete the document addition process */
void
vim_widget_add_document_complete (VimWidget *widget, VimEditor *editor)
{
	widget->priv->documents = g_list_prepend (widget->priv->documents, editor);
	g_signal_emit_by_name (IANJUTA_EDITOR_MASTER(widget),
			"document-added",
			G_OBJECT(editor));
}


void
vim_widget_remove_document (VimWidget *widget, VimEditor *editor, GError **err)
{
	g_return_if_fail (editor != NULL);
	if (vim_widget_has_editor (widget, editor))
	{
		gchar* cmd = g_strdup_printf (":bd %d", editor->priv->bufno);
		vim_dbus_exec_without_reply (widget, cmd, err);
		g_free (cmd);
	}
}

/* Complete the document removal process */
void
vim_widget_remove_document_complete (VimWidget *widget, VimEditor *editor)
{
	g_return_if_fail (editor != NULL);
	/* If the last element from the list is being removed, delete the list */
	if (g_list_length (widget->priv->documents) == 1)
	{
		g_list_free (widget->priv->documents);
		widget->priv->documents = NULL;
	}
	else
		widget->priv->documents = g_list_remove (widget->priv->documents, editor);
	g_signal_emit_by_name (IANJUTA_EDITOR_MASTER(widget),
			"document-removed",
			G_OBJECT(editor));
	g_object_unref (editor);
}


VimEditor*
vim_widget_get_document_bufno (VimWidget *widget, const guint bufno, GError **err)
{
	GList* node = NULL;
	for (node = widget->priv->documents; node != NULL; node = g_list_next (node))
	{
		VimEditor *editor = VIM_EDITOR(node->data);
		if (editor->priv->bufno == bufno)
			return g_object_ref(editor);
	}
	return NULL;
}

VimEditor*
vim_widget_get_document_file (VimWidget *widget, GFile* file, GError **err)
{
	GList* node = NULL;
	for (node = widget->priv->documents; node != NULL; node = g_list_next (node))
	{
		VimEditor *editor = VIM_EDITOR(node->data);
		if (g_file_equal(editor->priv->file, file))
			return g_object_ref(editor);
	}
	return NULL;
}

VimEditor*
vim_widget_get_document_filename (VimWidget *widget, const gchar* filename, GError **err)
{
	GList* node = NULL;
	GFile* file = g_file_new_for_path (filename);
	for (node = widget->priv->documents; node != NULL; node = g_list_next (node))
	{
		VimEditor *editor = VIM_EDITOR(node->data);
		if (g_file_equal(editor->priv->file, file))
		{
			g_object_unref (file);
			return g_object_ref(editor);
		}
	}
	return NULL;
}

VimEditor*
vim_widget_get_document_uri (VimWidget *widget, const gchar* uri, GError **err)
{
	GList* node = NULL;
	GFile* file = g_file_new_for_uri (uri);
	for (node = widget->priv->documents; node != NULL; node = g_list_next (node))
	{
		VimEditor *editor = VIM_EDITOR(node->data);
		if (g_file_equal(editor->priv->file, file))
		{
			g_object_unref (file);
			return g_object_ref(editor);
		}
	}
	return NULL;
}

/* 
 * Once the window is realized, call this function to embed vim
 */

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

gboolean
vim_widget_has_editor (VimWidget *widget, VimEditor *editor)
{
	GList* node = widget->priv->documents; 

	if (g_list_find (node, editor))
		return TRUE;
	else
		return FALSE;
}

void
vim_widget_set_current_editor (VimWidget *widget, VimEditor *editor, GError **err)
{
	gchar* cmd = NULL;

	if (widget->priv->current_editor != editor)
	{
		g_return_if_fail(editor->priv->bufno != 0);
		cmd = g_strdup_printf (":buffer %d", editor->priv->bufno);
		vim_dbus_exec_without_reply (widget, cmd, err);
		g_free (cmd);
	}
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
	return IANJUTA_DOCUMENT(g_object_ref(widget->priv->current_editor));
}

static gboolean
imaster_has_document (IAnjutaEditorMaster *obj, IAnjutaDocument *document, GError **err)
{
	VimWidget* widget = VIM_WIDGET (obj);
	VimEditor* editor = VIM_EDITOR (document);
	return vim_widget_has_editor (widget, editor);
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
	VimEditor* editor = VIM_EDITOR (document);
	vim_widget_remove_document(widget, editor, err);
}

static void
imaster_set_current_document (IAnjutaEditorMaster *obj, IAnjutaDocument *document, GError **err)
{
	VimWidget* widget = VIM_WIDGET (obj);
	VimEditor* editor = VIM_EDITOR (document);
	vim_widget_set_current_editor (widget, editor, err);
}

static void
imaster_set_registered (IAnjutaEditorMaster *obj, gboolean state, GError **err)
{
	VimWidget* widget = VIM_WIDGET (obj);
	widget->priv->registered = state;
}

static gint
imaster_get_page (IAnjutaEditorMaster *obj, GError **err)
{
	VimWidget* widget = VIM_WIDGET (obj);
	return widget->priv->page_no;
}

static void
imaster_set_page (IAnjutaEditorMaster *obj, gint page, GError **err)
{
	VimWidget* widget = VIM_WIDGET (obj);
	widget->priv->page_no = page;
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
	iface->get_page = imaster_get_page;
	iface->set_page = imaster_set_page;
}

/* Signal Callbacks */
void 
vim_signal_buf_new_file_cb (DBusGProxy *proxy, const guint bufno, 
		VimWidget *widget)
{
	VimEditor* editor = vim_editor_new (NULL, NULL);
	editor->priv->bufno = bufno;
	if (!vim_widget_has_editor (widget, editor))
		vim_widget_add_document_complete (widget, editor);
	else
		g_object_unref (editor);
}

void 
vim_signal_buf_read_cb (DBusGProxy *proxy, const guint bufno, 
		const gchar* filename, VimWidget *widget)
{
	/* Nothing needs to be done here */
}

void 
vim_signal_buf_write_cb (DBusGProxy *proxy, const guint bufno, 
		const gchar* filename, VimWidget *widget)
{
	/* Nothing needs to be done here */
	/* Save signal */
}

void 
vim_signal_buf_add_cb (DBusGProxy *proxy, const guint bufno, 
		const gchar* filename, VimWidget *widget)
{
	GFile *file = g_file_new_for_path (filename);
	VimEditor *editor = vim_editor_new (NULL, file);
	editor->priv->bufno = bufno;
	editor->priv->file = file;
	if (!vim_widget_has_editor (widget, editor))
		vim_widget_add_document_complete (widget, editor);
	else 
		g_object_unref (editor);
}


void 
vim_signal_buf_delete_cb (DBusGProxy *proxy, const guint bufno, 
		VimWidget *widget)
{
	VimEditor *editor = vim_widget_get_document_bufno (widget, bufno, NULL);
	vim_widget_remove_document_complete (widget, editor);
}

/* Filename may have changed */
void 
vim_signal_buf_file_post_cb (DBusGProxy *proxy, const guint bufno, 
		const gchar* filename, VimWidget *widget)
{
	VimEditor *editor = vim_widget_get_document_bufno (widget, bufno, NULL);
	GFile *file = g_file_new_for_path (filename);
	if (g_file_equal(editor->priv->file, file))
	{
		g_object_unref (editor->priv->file);
		editor->priv->file = file;
	}
	g_object_unref (editor);
}

void 
vim_signal_buf_enter_cb (DBusGProxy *proxy, const guint bufno, 
		const gchar* filename, VimWidget *widget)
{
	VimEditor *editor = vim_widget_get_document_bufno (widget, bufno, NULL);
	/* Sometimes vim opens new files with BufEnter */
	if (!editor)
	{
		GFile *file = g_file_new_for_path (filename);
		editor = vim_editor_new (NULL, file);
		editor->priv->bufno = bufno;
		editor->priv->file = file;
		vim_widget_add_document_complete (widget, editor);
		widget->priv->current_editor = editor;
		g_signal_emit_by_name (IANJUTA_EDITOR_MASTER(widget),
				"current-document-changed",
				G_OBJECT(editor));
	}
	else
	{
		editor->priv->bufno = bufno;
		widget->priv->current_editor = editor;
		g_signal_emit_by_name (IANJUTA_EDITOR_MASTER(widget),
				"current-document-changed",
				G_OBJECT(editor));
		g_object_unref (editor);
	}
	/* Set the current editor */
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
	g_object_unref (widget);
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

	return G_OBJECT(g_object_ref(widget));
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
	parent_class->dispose (object);
}

static void
vim_widget_finalize (GObject *object)
{
	VimWidget* widget = VIM_WIDGET (object);
    VimWidgetClass* klass = VIM_WIDGET_CLASS (g_type_class_peek (VIM_TYPE_WIDGET));
	VimWidgetPrivate *priv = VIM_WIDGET_PRIVATE(widget);
	GList* list = priv->documents;
	
	priv->registered = FALSE;

	g_list_foreach (list, (GFunc) g_object_unref, NULL);
	g_list_free (priv->documents);
	priv->documents = NULL;
	priv->current_editor = NULL;

	g_object_unref (priv->socket);
	priv->socket_id = 0;

	g_object_unref (priv->conn);
	g_object_unref (priv->dbus_proxy);
	g_object_unref (priv->proxy);

	g_object_unref (klass->widget);
	parent_class->finalize (object);
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

