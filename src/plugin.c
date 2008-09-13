/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * plugin.c
 * Copyright (C) Arun Tejasvi Chaganty 2008 <arunchaganty@gmail.com>
 * 
 * plugin.c is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * plugin.c is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <config.h>
#include <gtk/gtk.h>
#include <gio/gio.h>
#include <libanjuta/anjuta-shell.h>
#include <libanjuta/anjuta-debug.h>
#include <libanjuta/interfaces/ianjuta-document-manager.h>
#include <libanjuta/interfaces/ianjuta-editor-factory.h>
#include <libanjuta/interfaces/ianjuta-editor.h>

#include "vim-widget.h"
#include "vim-editor.h"
#include "plugin.h"

#define GLADE_FILE ANJUTA_DATA_DIR"/glade/anjuta-gvim.glade"
#define VIMRC_FILE ANJUTA_DATA_DIR"/gvim/anjuta.vimrc"
#define ACCELS_FILE ANJUTA_DATA_DIR"/gvim/vimjuta-accels"
#define KEYS_FILE ANJUTA_DATA_DIR"/gvim/locked_keys"

static gpointer parent_class;

static void
anjuta_gvim_lock_keys()
{
    GDataInputStream *stream;
    GFile *file = anjuta_util_get_user_config_file ("gvim", "locked_keys", NULL);
    GList *keys = NULL;
    GList *node = NULL;
    gchar *key = NULL;
    gint len;

    if (!g_file_query_exists (file, NULL))
    {
        GFile *master = NULL;
        master = g_file_new_for_path (KEYS_FILE);
        g_file_copy (master, file,
                G_FILE_COPY_NONE,
                NULL,
                NULL,
                NULL,
                NULL);
    }

    stream = g_data_input_stream_new (G_INPUT_STREAM(g_file_read (file, NULL, NULL)));

    while (key = (gchar*) g_data_input_stream_read_line (stream, NULL, NULL, NULL))
    {
        g_print ("%s", key);
        gtk_accel_map_lock_path (key);
        g_free (key);
    }
}

static void
anjuta_gvim_unlock_keys()
{
    GDataInputStream *stream;
    GFile *file = anjuta_util_get_user_config_file ("gvim", "locked_keys", NULL);
    GList *keys = NULL;
    GList *node = NULL;
    gchar *key = NULL;
    gint len;

    if (!g_file_query_exists (file, NULL))
    {
        GFile *master = NULL;
        master = g_file_new_for_path (KEYS_FILE);
        g_file_copy (master, file,
                G_FILE_COPY_NONE,
                NULL,
                NULL,
                NULL,
                NULL);
    }

    stream = g_data_input_stream_new (G_INPUT_STREAM(g_file_read (file, NULL, NULL)));

    while (key = (gchar*)g_data_input_stream_read_line (stream, NULL, NULL, NULL))
    {
        gtk_accel_map_unlock_path (key);
        g_free (key);
    }
}

static gboolean
anjuta_gvim_activate (AnjutaPlugin *plugin)
{
	VimPlugin *vim_plugin = ANJUTA_PLUGIN_GVIM (plugin);
	AnjutaUI *ui;
    GFile *file = NULL;
	DEBUG_PRINT ("VimPlugin: Activating VimPlugin plugin ...");
	ui = anjuta_shell_get_ui (plugin->shell, NULL);
    anjuta_ui_unload_accels (ui);
    anjuta_gvim_lock_keys();
    gtk_accel_map_lock_path ("<Ctrl>w");
    gtk_accel_map_lock_path ("<Ctrl>n");

    /* Check for vimrc file. If not, copy from the package directory */
    file = anjuta_util_get_user_config_file ("gvim", "vimjuta-accels", NULL);
    if (!g_file_query_exists (file, NULL))
    {
        GFile *master = NULL;
        master = g_file_new_for_path (ACCELS_FILE);
        g_file_copy (master, file,
                G_FILE_COPY_NONE,
                NULL,
                NULL,
                NULL,
                NULL);
    }

    anjuta_ui_load_accels (g_file_get_path(file));
    g_object_unref (file);
    /* Check for vimrc file. If not, copy from the package directory */
    file = anjuta_util_get_user_config_file ("gvim", "anjuta.vimrc", NULL);
    if (!g_file_query_exists (file, NULL))
    {
        GFile *master = NULL;
        master = g_file_new_for_path (VIMRC_FILE);
        g_file_copy (master, file,
                G_FILE_COPY_NONE,
                NULL,
                NULL,
                NULL,
                NULL);
    }
    g_object_unref (file);

	return TRUE;
}

static gboolean
anjuta_gvim_deactivate (AnjutaPlugin *plugin)
{
	AnjutaUI *ui;
	VimPlugin *vim_plugin = ANJUTA_PLUGIN_GVIM (plugin);
	ui = anjuta_shell_get_ui (plugin->shell, NULL);
	GFile *file;
	DEBUG_PRINT ("VimPlugin: Dectivating VimPlugin plugin ...");
    anjuta_gvim_unlock_keys();
    gtk_accel_map_unlock_path ("<Ctrl>w");
    gtk_accel_map_unlock_path ("<Ctrl>n");

    if (vim_plugin->widget) gtk_widget_destroy (GTK_WIDGET(vim_plugin->widget));

    /* Check for vimrc file. If not, copy from the package directory */
    file = anjuta_util_get_user_config_file ("gvim", "vimjuta-accels", NULL);
    if (!g_file_query_exists (file, NULL))
    {
        GFile *master = NULL;
        master = g_file_new_for_path (ACCELS_FILE);
        g_file_copy (master, file,
                G_FILE_COPY_NONE,
                NULL,
                NULL,
                NULL,
                NULL);
    }

	ui = anjuta_shell_get_ui (plugin->shell, NULL);
    anjuta_ui_save_accels (g_file_get_path(file));
    anjuta_ui_unload_accels (ui);
    anjuta_ui_load_accels (NULL);
    g_object_unref (file);

	return TRUE;
}

static void
anjuta_gvim_finalize (GObject *obj)
{
	/* Finalization codes here */
	GNOME_CALL_PARENT (G_OBJECT_CLASS, finalize, (obj));
}

static void
anjuta_gvim_dispose (GObject *obj)
{
	/* Disposition codes */
	GNOME_CALL_PARENT (G_OBJECT_CLASS, dispose, (obj));
}

static void
anjuta_gvim_instance_init (GObject *obj)
{
	VimPlugin *plugin = (VimPlugin*)obj;

	plugin->widget = NULL;
}

static void
anjuta_gvim_class_init (GObjectClass *klass) 
{
	AnjutaPluginClass *plugin_class = ANJUTA_PLUGIN_CLASS (klass);

	parent_class = g_type_class_peek_parent (klass);

	plugin_class->activate = anjuta_gvim_activate;
	plugin_class->deactivate = anjuta_gvim_deactivate;
	klass->finalize = anjuta_gvim_finalize;
	klass->dispose = anjuta_gvim_dispose;
}

/*
 * Creates a new 'Editor', a document. The document
 * has a reference to the vim instance through a private
 * field, widget, used by the IAnjutaEditorMultiple.
 */
static IAnjutaEditor*
ieditor_factory_new_editor (IAnjutaEditorFactory* factory,
								GFile* file,
								const gchar* filename,
								GError** error)
{
	AnjutaPlugin* plugin = ANJUTA_PLUGIN (factory);
	VimEditor* editor = NULL;
	GError *err = NULL;
	editor = vim_editor_new (plugin, file);

	return IANJUTA_EDITOR (editor);
}

static void
ieditor_factory_iface_init (IAnjutaEditorFactoryIface *iface)
{
	iface->new_editor = ieditor_factory_new_editor;
}


ANJUTA_PLUGIN_BEGIN (VimPlugin, anjuta_gvim);
ANJUTA_TYPE_ADD_INTERFACE(ieditor_factory, IANJUTA_TYPE_EDITOR_FACTORY);
ANJUTA_PLUGIN_END;

ANJUTA_SIMPLE_PLUGIN (VimPlugin, anjuta_gvim);

