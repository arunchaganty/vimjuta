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
#include <libanjuta/anjuta-shell.h>
#include <libanjuta/anjuta-debug.h>
#include <libanjuta/interfaces/ianjuta-document-manager.h>

#include "plugin.h"


#define UI_FILE ANJUTA_DATA_DIR"/ui/anjuta_gvim.ui"


#define GLADE_FILE ANJUTA_DATA_DIR"/glade/anjuta_gvim.glade"


static gpointer parent_class;

static gboolean
anjuta_gvim_activate (AnjutaPlugin *plugin)
{
	GtkWidget *wid = NULL;
	GtkWidget *sock = NULL;
	GladeXML *gxml = NULL;
	gchar *cmd = NULL;
	GError *err = NULL;

	VimPlugin *anjuta_gvim;
	
	DEBUG_PRINT ("VimPlugin: Activating VimPlugin plugin ...");
	anjuta_gvim = (VimPlugin*) plugin;
	/* Add plugin widgets to Shell */
	gxml = glade_xml_new (GLADE_FILE, "top_widget", NULL);
	wid = glade_xml_get_widget (gxml, "top_widget");
	sock = glade_xml_get_widget (gxml, "vim_socket");
	/* Test X11 socket */
	g_assert (sock != NULL && wid != NULL);
	g_print ("%d %d", sock, wid);
	anjuta_gvim->widget = wid;
	anjuta_shell_add_widget (plugin->shell, wid,
							 "VimPluginWidget", _("VimPlugin widget"), NULL,
							 ANJUTA_SHELL_PLACEMENT_CENTER, NULL);

	anjuta_gvim->socket_id = gtk_socket_get_id ((GtkSocket *) sock);
	g_print ("%d",anjuta_gvim->socket_id);
	//g_assert (anjuta_gvim->socket_id != 0);

	cmd = g_malloc (32 * sizeof(gchar));
	g_sprintf (cmd, "gvim --socketid %d", anjuta_gvim->socket_id);
	g_print (cmd);

	g_spawn_command_line_async (cmd, &err);
	if (err) {
		DEBUG_PRINT ("VimPlugin: Error: %s", err);
		return FALSE;
	}

g_object_unref (gxml);

	return TRUE;
}

static gboolean
anjuta_gvim_deactivate (AnjutaPlugin *plugin)
{

	AnjutaUI *ui;

	DEBUG_PRINT ("VimPlugin: Dectivating VimPlugin plugin ...");

	anjuta_shell_remove_widget (plugin->shell, ((VimPlugin*)plugin)->widget,
								NULL);

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
	plugin->socket_id = 0;

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

ANJUTA_PLUGIN_BOILERPLATE (VimPlugin, anjuta_gvim);
ANJUTA_SIMPLE_PLUGIN (VimPlugin, anjuta_gvim);
