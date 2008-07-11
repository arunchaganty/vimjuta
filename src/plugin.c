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
#include "vim-test.h"
#include "plugin.h"

#define UI_FILE ANJUTA_DATA_DIR"/ui/vim-test.ui"
#define GLADE_FILE ANJUTA_DATA_DIR"/glade/anjuta-gvim.glade"

static gpointer parent_class;

static GtkActionEntry actions_test[] = {
	{
		"TestBegin",                       /* Action name */
		GTK_STOCK_NEW,                            /* Stock icon, if any */
		N_("Begin Testing"),                     /* Display label */
		NULL,                                     /* short-cut */
		N_("Tests"),                      /* Tooltip */
		G_CALLBACK (vim_test_begin)   /* action callback */
	}
};

static gboolean
anjuta_gvim_activate (AnjutaPlugin *plugin)
{
	VimPlugin *vim_plugin = ANJUTA_PLUGIN_GVIM (plugin);
	AnjutaUI *ui;
	DEBUG_PRINT ("VimPlugin: Activating VimPlugin plugin ...");
	ui = anjuta_shell_get_ui (plugin->shell, NULL);
	anjuta_ui_add_action_group_entries (ui, "ActionGroupVimFile",
										_("VimPlugin Tests"),
										actions_test,
										G_N_ELEMENTS (actions_test),
										GETTEXT_PACKAGE, TRUE, plugin);
	vim_plugin->uiid = anjuta_ui_merge (ui, UI_FILE);
	return TRUE;
}

static gboolean
anjuta_gvim_deactivate (AnjutaPlugin *plugin)
{
	DEBUG_PRINT ("VimPlugin: Dectivating VimPlugin plugin ...");
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

