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
#include <libanjuta/interfaces/ianjuta-document-manager.h>
#include <libanjuta/interfaces/ianjuta-editor-master.h>
#include <libanjuta/interfaces/ianjuta-editor-multiple.h>
#include <libanjuta/interfaces/ianjuta-file.h>
#include <gtk/gtk.h>
#include <gio/gio.h>
#include "plugin.h"
#include "vim-widget.h"
#include "vim-editor.h"
#include "vim-widget-priv.h"
#include "vim-editor-priv.h"
#include "vim-dbus.h"

#define VIM_WIDGET_PRIVATE(o)  (G_TYPE_INSTANCE_GET_PRIVATE ((o), VIM_TYPE_WIDGET, VimWidgetPrivate))
#define GVIMRC_FILE ANJUTA_DATA_DIR"/gvim/anjuta.gvimrc"

static GObjectClass* parent_class;

enum {
    PROP_0,
    PROP_VIM_PLUGIN
};

static gint
g_ptr_array_find (GPtrArray *array, gpointer ptr)
{
    int i,n;
    n = array->len;
    for (i=0; i<n; i++)
        if (g_ptr_array_index (array, i) == ptr) return i;
    return -1;
}

void
vim_widget_add_document (VimWidget *widget, VimEditor *editor, GError **err)
{
	g_assert (editor != NULL);
	if (!vim_widget_has_editor (widget, editor)) 
	{
		if (g_ptr_array_find (widget->priv->unloaded, editor) == -1)
            g_ptr_array_add (widget->priv->unloaded, editor);
		ianjuta_file_open (IANJUTA_FILE (editor), editor->priv->file, NULL);
	}
}

/* Complete the document addition process */
void
vim_widget_add_document_complete (VimWidget *widget, VimEditor *editor)
{
    IAnjutaDocumentManager *docman;
    if (g_ptr_array_find (widget->priv->unloaded, editor) != -1)
        g_ptr_array_remove (widget->priv->unloaded, editor);

    g_ptr_array_add (widget->priv->documents, editor);
    editor->priv->loaded = TRUE;
	vim_editor_update_variables (editor);
    docman = anjuta_shell_get_interface (ANJUTA_PLUGIN(widget->priv->plugin)->shell, 
            IAnjutaDocumentManager, NULL);
    ianjuta_document_manager_add_document (docman, IANJUTA_DOCUMENT(editor), NULL);
}


void
vim_widget_remove_document (VimWidget *widget, VimEditor *editor, GError **err)
{
    unsigned int i;
	g_return_if_fail (editor != NULL);
	if (vim_widget_has_editor (widget, editor))
	{
		gchar* cmd = g_strdup_printf (":bd! %d", editor->priv->bufno);
		vim_dbus_exec_without_reply (widget, cmd, err);
		g_free (cmd);
	}
}

/* Complete the document removal process */
void
vim_widget_remove_document_complete (VimWidget *widget, VimEditor *editor)
{
    IAnjutaDocumentManager *docman;
	GFile *file = g_file_new_for_path (UNTITLED_FILE);
	/* The phantom "Untitled document" */
	VimEditor *null_editor = vim_widget_get_document_file (widget, file, NULL);
	VimEditor *next_editor = NULL;

	g_return_if_fail (editor != NULL);

	/* Check for the phantom "Untitled" document */
    g_ptr_array_remove (widget->priv->documents, editor);
    if (null_editor && widget->priv->documents->len == 1)
        g_ptr_array_remove (widget->priv->documents, null_editor);

	/* Set this to null until vim signals the change */
    if (widget->priv->documents->len == 0)
        widget->priv->current_editor = NULL;
    else
        widget->priv->current_editor = g_ptr_array_index (widget->priv->documents, 0);

    docman = anjuta_shell_get_interface (ANJUTA_PLUGIN(widget->priv->plugin)->shell, 
            IAnjutaDocumentManager, NULL);
    ianjuta_document_manager_remove_document (docman, IANJUTA_DOCUMENT(editor), TRUE, NULL);
	gtk_object_destroy (GTK_OBJECT(editor));

	/*
	g_signal_emit_by_name (editor,
			"destroy");
	*/

	g_object_unref (file);
    /* One for the master's reference */
	g_object_unref (editor);
}


VimEditor*
vim_widget_get_document_bufno (VimWidget *widget, const guint bufno, GError **err)
{
    int i, n;
    n = widget->priv->documents->len;
	for (i=0; i < n; i++)
	{
		VimEditor *editor = g_ptr_array_index (widget->priv->documents, i);
		if (editor->priv->bufno == bufno)
			return editor;
	}
	return NULL;
}

VimEditor*
vim_widget_get_document_file (VimWidget *widget, GFile* file, GError **err)
{
    int i, n;
    n = widget->priv->documents->len;
	for (i=0; i < n; i++)
	{
		VimEditor *editor = g_ptr_array_index (widget->priv->documents, i);
		if (g_file_equal(editor->priv->file, file))
			return editor;
	}
    n = widget->priv->unloaded->len;
	for (i=0; i < n; i++)
	{
		VimEditor *editor = g_ptr_array_index (widget->priv->unloaded, i);
		if (g_file_equal(editor->priv->file, file))
			return editor;
	}
	return NULL;
}

VimEditor*
vim_widget_get_document_filename (VimWidget *widget, const gchar* filename, GError **err)
{
	GFile* file = g_file_new_for_path (filename);
    VimEditor *editor = vim_widget_get_document_file (widget, file, err);
    g_object_unref (file);
    return editor;
}

VimEditor*
vim_widget_get_document_uri (VimWidget *widget, const gchar* uri, GError **err)
{
	GFile* file = g_file_new_for_uri (uri);
    VimEditor *editor = vim_widget_get_document_file (widget, file, err);
    g_object_unref (file);
    return editor;
}

/* 
 * Once the window is realized, call this function to embed vim
 */

static void
vim_widget_plug_added_cb (GtkSocket *socket, VimWidget *widget)
{
    vim_comm_init(widget, NULL);
}

static void 
vim_widget_connect_plug (VimWidget *widget) 
{
	gchar *cmd;
    gchar *VIMRC_FILE = anjuta_util_get_user_config_file_path ("gvim", "anjuta.vimrc", NULL);
	GError *err = NULL;
	widget->priv->socket_id = gtk_socket_get_id ((GtkSocket *) widget->priv->socket);
	g_assert (widget->priv->socket_id != 0);
    /* TODO: Make unique */
	widget->priv->servername = g_strdup_printf ("ANJUTA%d", widget->priv->socket_id);

	g_signal_connect_after (widget->priv->socket,
						"plug-added",
						G_CALLBACK(vim_widget_plug_added_cb),
						widget);
	
	cmd = g_strdup_printf ("gvim -u %s -S %s --socketid %d --servername %s \n",
            VIMRC_FILE, GVIMRC_FILE, widget->priv->socket_id, widget->priv->servername);
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
}

static gboolean
vim_widget_focus_cb (VimWidget *widget, GtkDirectionType dir)
{
    switch (dir)
    {
        case GTK_DIR_TAB_FORWARD:
            g_print ("f");
            break;
        case GTK_DIR_TAB_BACKWARD:
            g_print ("b");
            break;
        case GTK_DIR_UP:
            g_print ("u");
            break;
        case GTK_DIR_DOWN:
            g_print ("d");
            break;
        case GTK_DIR_LEFT:
            g_print ("l");
            break;
        case GTK_DIR_RIGHT:
            g_print ("r");
            break;
    }
    g_print ("\n");
    return TRUE;
}

gboolean
vim_widget_has_editor (VimWidget *widget, VimEditor *editor)
{
    int i,n;
    n = widget->priv->documents->len;
    for (i=0; i<n; i++) 
        if (g_ptr_array_index (widget->priv->documents, i) == editor)
            return TRUE;
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

vim_widget_close_all (VimWidget *widget)
{
    int i,n;
    n = widget->priv->documents->len;

    for (i = n; i > 0; i--)
	{
		VimEditor *editor = g_ptr_array_index(widget->priv->documents, i-1);
		vim_widget_remove_document_complete (widget, editor);
	}
}

void vim_widget_grab_focus (VimWidget *widget) {
	gtk_widget_grab_focus (GTK_WIDGET(widget));
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
	/* Anjuta sometimes tries to switch to this document right before 
	 * it is loaded */
	if (!vim_widget_has_editor (widget, editor))
		return;
	vim_widget_set_current_editor (widget, editor, err);
}

void 
imaster_iface_init (IAnjutaEditorMasterIface *iface)
{
	iface->add_document = imaster_add_document;
	iface->get_current_document = imaster_get_current_document;
	iface->remove_document = imaster_remove_document;
	iface->set_current_document = imaster_set_current_document;
}

/* Signal Callbacks */
void 
vim_signal_buf_new_file_cb (DBusGProxy *proxy, const guint bufno, 
		VimWidget *widget)
{
	GFile *file = g_file_new_for_path (UNTITLED_FILE);
	VimEditor *editor;
	if (!(editor = vim_widget_get_document_file (widget, file, NULL)))
        editor = vim_editor_new (NULL, file);
	editor->priv->bufno = bufno;
	if (!vim_widget_has_editor (widget, editor))
		vim_widget_add_document_complete (widget, editor);
}

void 
vim_signal_buf_enter_cb (DBusGProxy *proxy, const guint bufno, 
		const gchar* filename, VimWidget *widget)
{
    IAnjutaDocumentManager *docman;
	VimEditor *editor = vim_widget_get_document_bufno (widget, bufno, NULL);
	if (!editor)
		return;
	editor->priv->bufno = bufno;
	if (widget->priv->current_editor != editor)
    {
	    widget->priv->current_editor = editor;
        /* Set the current editor */
        docman = anjuta_shell_get_interface (ANJUTA_PLUGIN(widget->priv->plugin)->shell, 
                IAnjutaDocumentManager, NULL);
        ianjuta_document_manager_set_current_document (docman, IANJUTA_DOCUMENT(editor), NULL);
    }
}


void 
vim_signal_buf_read_cb (DBusGProxy *proxy, const guint bufno, 
		const gchar* filename, VimWidget *widget)
{
	/* Sometimes, Vim opens new files this way too. *sigh* */
	VimEditor *editor = vim_widget_get_document_bufno (widget, bufno, NULL);
	if (!editor)
	{
		GFile* file;
		if (strcmp (filename, "") != 0)
			file = g_file_new_for_path (filename);
		else
			file = g_file_new_for_path (UNTITLED_FILE);
        if (!(editor = vim_widget_get_document_file (widget, file, NULL)))
            editor = vim_editor_new (NULL, file);
		editor->priv->bufno = bufno;
		editor->priv->file = file;
		vim_widget_add_document_complete (widget, editor);
        vim_signal_buf_enter_cb (proxy, bufno, NULL, widget);
	}
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
	VimEditor *editor;
	if (!(editor = vim_widget_get_document_file (widget, file, NULL)))
        editor = vim_editor_new (NULL, file);
	editor->priv->bufno = bufno;
	editor->priv->file = file;
	if (!vim_widget_has_editor (widget, editor))
		vim_widget_add_document_complete (widget, editor);
}


void 
vim_signal_buf_delete_cb (DBusGProxy *proxy, const guint bufno, 
		VimWidget *widget)
{
	VimEditor *editor = vim_widget_get_document_bufno (widget, bufno, NULL);
	if (!editor) return;
	vim_widget_remove_document_complete (widget, editor);
}

/* Filename may have changed */
void 
vim_signal_buf_file_post_cb (DBusGProxy *proxy, const guint bufno, 
		const gchar* filename, VimWidget *widget)
{
	VimEditor *editor = vim_widget_get_document_bufno (widget, bufno, NULL);
	if (!editor) return;
	GFile *file = g_file_new_for_path (filename);
	if (g_file_equal(editor->priv->file, file))
	{
		g_object_unref (editor->priv->file);
		editor->priv->file = file;
	}
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
	vim_widget_close_all (widget);
	gtk_object_destroy (GTK_OBJECT(widget));
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

void vim_signal_file_type_cb (DBusGProxy *proxy, const guint bufno, 
		const gchar* filetype, VimWidget *widget)
{
	VimEditor *editor = vim_widget_get_document_bufno (widget, bufno, NULL);
	/* Do nothing, yet */
}

/* TODO: Send more data */
void vim_signal_insert_leave_cb (DBusGProxy *proxy, const guint bufno, 
		VimWidget *widget)
{
	VimEditor *editor = vim_widget_get_document_bufno (widget, bufno, NULL);
	vim_editor_update_variables (editor);
}

void vim_signal_cursor_hold_cb (DBusGProxy *proxy, const guint bufno, 
		const gchar* word, VimWidget *widget)
{
	VimEditor *editor = vim_widget_get_document_bufno (widget, bufno, NULL);
	/* Do nothing, yet */
}

static GObject*
vim_widget_constructor (GType   type,
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
	g_signal_connect_after (widget->priv->socket,
						"focus",
						G_CALLBACK(vim_widget_focus_cb),
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
	widget->priv->documents = g_ptr_array_new();
	widget->priv->unloaded = g_ptr_array_new();
}

static void
vim_widget_dispose (GObject *object)
{
	VimWidget* widget = VIM_WIDGET (object);
	VimWidgetPrivate *priv = VIM_WIDGET_PRIVATE(widget);
	
	priv->registered = FALSE;

    if (priv->unloaded) {
        g_ptr_array_foreach (priv->unloaded, (GFunc) g_object_unref, NULL);
        g_ptr_array_free (priv->unloaded, TRUE);
        priv->unloaded = NULL;
    }

    if (priv->documents) {
        g_ptr_array_foreach (priv->documents, (GFunc) g_object_unref, NULL);
        g_ptr_array_free (priv->documents, TRUE);
        priv->documents = NULL;
    }

	priv->current_editor = NULL;

	parent_class->dispose (object);
}

static void
vim_widget_finalize (GObject *object)
{
	VimWidget* widget = VIM_WIDGET (object);
    VimWidgetClass* klass = VIM_WIDGET_CLASS (g_type_class_peek (VIM_TYPE_WIDGET));
	VimWidgetPrivate *priv = widget->priv;
	
	dbus_g_connection_unref (priv->conn);
	g_object_unref (priv->dbus_proxy);
	g_object_unref (priv->proxy);

	g_free (widget->priv);
	klass->widget = NULL;
	parent_class->finalize (object);
}

static void vim_widget_get_property (GObject *object, guint property_id, 
        const GValue *value, GParamSpec *pspec)
{
    VimWidget *widget = VIM_WIDGET (object); 
    switch (property_id) {
        case PROP_VIM_PLUGIN:
            widget->priv->plugin = ANJUTA_PLUGIN_GVIM (g_value_get_object (value));
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (widget, property_id, pspec);
    }
}

static void vim_widget_set_property (GObject *object, guint property_id, 
        GValue *value, GParamSpec *pspec)
{
    VimWidget *widget = VIM_WIDGET (object); 
    switch (property_id) {
        case PROP_VIM_PLUGIN:
            g_value_set_object (value, widget->priv->plugin);
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (widget, property_id, pspec);
    }
}

static void
vim_widget_class_init (VimWidgetClass *klass)
{
    GParamSpec *pspec;
	GObjectClass* object_class = G_OBJECT_CLASS (klass);
    parent_class = G_OBJECT_CLASS (g_type_class_peek_parent (klass));

	g_type_class_add_private (klass, sizeof (VimWidgetPrivate));
    /*
	object_class->get_property = vim_widget_get_property;
	object_class->set_property = vim_widget_set_property;
    */
	object_class->constructor = vim_widget_constructor;
	object_class->dispose = vim_widget_dispose;
	object_class->finalize = vim_widget_finalize;

    /*
    pspec = g_param_spec_object (
            "vim-plugin",
            "Vim Plugin",
            "Set Vim Plugin",
            ANJUTA_TYPE_PLUGIN_GVIM,
            G_PARAM_CONSTRUCT_ONLY|G_PARAM_READWRITE);
    g_object_class_install_property (object_class, PROP_VIM_PLUGIN, pspec);
    */

	klass->widget = NULL; /* Singleton hasn't been initialized yet */
}

ANJUTA_TYPE_BEGIN (VimWidget, vim_widget, GTK_TYPE_FRAME);
ANJUTA_TYPE_ADD_INTERFACE(imaster, IANJUTA_TYPE_EDITOR_MASTER);
ANJUTA_TYPE_END;

