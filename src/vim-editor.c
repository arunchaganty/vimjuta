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

#include <libanjuta/anjuta-debug.h>
#include <libanjuta/interfaces/ianjuta-document.h>
#include <libanjuta/interfaces/ianjuta-editor.h>
#include <libanjuta/interfaces/ianjuta-file.h>
#include <libanjuta/interfaces/ianjuta-file-savable.h>
#include "vim-editor.h"
#include "vim-dbus.h"
#include <string.h>

#define GLADE_FILE ANJUTA_DATA_DIR"/glade/anjuta_gvim.glade"
#define VIM_DBUS_FILE "/home/teju/Projects/anjuta-gvim/src/vim-dbus.py"
#define GVIMRC_FILE "/home/teju/Projects/anjuta-gvim/misc/anjuta.gvimrc"

static GObjectClass* parent_class = NULL;

extern void ieditor_iface_init (IAnjutaEditorIface *iface);
extern void idocument_iface_init (IAnjutaDocumentIface *iface);
extern ifile_iface_init (IAnjutaFileIface *iface);
extern isave_iface_init (IAnjutaFileSavableIface *iface);

gchar* str_substr (const gchar* str, const gint start, const gint end)
{
	gchar *new_str = NULL;
	gint i, j = 0;

	g_assert (end > start);
	new_str = g_new0 (gchar, end - start);

	for (i = 0; i < start; i++)
		g_return_if_fail (str[i] != '\0');

	for (;i < end; i++ && j++) {
		if (str[i] == '\0')
			break;
		new_str[j] = str[i];
	}

	return new_str;
}

gchar*
uri_to_file (const gchar* uri) {
	gchar *tmp;
	gchar **list;

	tmp = g_uri_parse_scheme (uri);
	if (strcmp (tmp, "file") == 0)
	{
		tmp = str_substr (uri, 7, strlen (uri)); 
	}
	else
		tmp = g_strdup (uri);
	return tmp;
}

/* 
 * Once the window is ready, call this function to embed vim
 */
static void 
vim_editor_connect_plug (VimEditor *vim, GParamSpec *param) {
	gchar *cmd;
	GError *err = NULL;
	GtkContainer *parent;

	g_object_get (vim,
			"parent", &parent,
			NULL);

	if (parent != NULL) {
		vim->socket_id = gtk_socket_get_id ((GtkSocket *) vim->socket);
		g_assert (vim->socket_id != 0);
		
		if (vim->filename)
		{
			cmd = g_strdup_printf ("gvim -U %s --socketid %d %s \n",
					GVIMRC_FILE,vim->socket_id, vim->filename);
		}
		else 
		{
			cmd = g_strdup_printf ("gvim --socketid %d \n",
					GVIMRC_FILE,vim->socket_id);
		}
		g_print (cmd);
		
		// Run vim
		g_spawn_command_line_async (cmd, &err);
		if (err)
		{
			DEBUG_PRINT ("VimPlugin: Error: %s", err);
			g_object_unref (err);
			err = NULL;
		}
	}

}

VimEditor*
vim_editor_new (AnjutaPlugin *plugin, const gchar* uri, const gchar* filename)
{
	VimEditor *vim;
	GtkSocket *sock = NULL;
	GError *err = NULL;
	
	DEBUG_PRINT ("VimPlugin: Creating new editor ...");

	vim = VIM_EDITOR (g_object_new(VIM_TYPE_EDITOR, NULL));
	vim->uri = g_strdup(filename);
	if (uri && strcmp (uri,"") != 0) vim->filename = uri_to_file (uri);
	else if (filename && strcmp (filename,"") != 0) vim->filename = uri_to_file (filename);
	else vim->filename = NULL;

	if (filename) g_print ("%s \n", vim->filename);
	
	/* Socket Impl. */
	vim->socket = (GtkSocket*) gtk_socket_new ();
	g_object_set (vim->socket,
			"visible", TRUE,
			NULL);
	g_assert (vim != NULL && vim->socket != NULL);
	gtk_container_add (GTK_CONTAINER(vim), GTK_WIDGET(vim->socket));

	g_signal_connect_after (vim,
						"notify::parent",
						G_CALLBACK(vim_editor_connect_plug),
						NULL);
	
	// Now hook in DBus
	vim_dbus_init(vim, &err);
	if (err) 
	{
		DEBUG_PRINT ("VimPlugin: Error: %s", err);
		g_object_unref (err);
		err = NULL;
	}

	return vim;
}

/* GObject */
static void
vim_editor_instance_init (VimEditor *object)
{
	object->socket_id = 0;
	
	// TODO: Make a list
	object->filename = NULL;
	object->buf_id = 0;
	
	object->conn = NULL;
	object->proxy = NULL;
	object->dbus_proxy = NULL;

}

static void
vim_editor_finalize (GObject *object)
{
	/* TODO: Add deinitalization code here */	
	
	parent_class->finalize (object);
}

static void
vim_editor_class_init (VimEditorClass *klass)
{
	GObjectClass* object_class = G_OBJECT_CLASS (klass);
	parent_class = g_type_class_peek_parent (klass);

	object_class->finalize = vim_editor_finalize;
}

ANJUTA_TYPE_BEGIN (VimEditor, vim_editor, GTK_TYPE_FRAME);
ANJUTA_TYPE_ADD_INTERFACE(ieditor, IANJUTA_TYPE_EDITOR);
ANJUTA_TYPE_ADD_INTERFACE(idocument, IANJUTA_TYPE_DOCUMENT);
ANJUTA_TYPE_ADD_INTERFACE(ifile, IANJUTA_TYPE_FILE);
ANJUTA_TYPE_ADD_INTERFACE(isave, IANJUTA_TYPE_FILE_SAVABLE);
ANJUTA_TYPE_END;
