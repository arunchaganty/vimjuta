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

/**
 *
 * Following Anjuta's conventions, this is actually a document.
 * The actual vim instance is stored in VimWidget.
 *
 */

#include <libanjuta/anjuta-debug.h>
#include <libanjuta/interfaces/ianjuta-document.h>
#include <libanjuta/interfaces/ianjuta-editor.h>
#include <libanjuta/interfaces/ianjuta-editor-multiple.h>
#include <libanjuta/interfaces/ianjuta-file.h>
#include <libanjuta/interfaces/ianjuta-file-savable.h>
#include "vim-editor.h"
#include "vim-widget.h"
#include "vim-util.h"
#include <string.h>

#define GLADE_FILE ANJUTA_DATA_DIR"/glade/anjuta_gvim.glade"

extern void ieditor_iface_init (IAnjutaEditorIface *iface);
extern void imultiple_iface_init (IAnjutaEditorIface *iface);
extern void idocument_iface_init (IAnjutaDocumentIface *iface);
extern ifile_iface_init (IAnjutaFileIface *iface);
extern isave_iface_init (IAnjutaFileSavableIface *iface);

#define VIM_EDITOR_PRIVATE(o)  (G_TYPE_INSTANCE_GET_PRIVATE ((o), VIM_TYPE_EDITOR, VimEditorPrivate))

static GObjectClass* parent_class = NULL;

VimEditor*
vim_editor_new (AnjutaPlugin *plugin, const gchar* uri, const gchar* filename)
{
	VimEditor *editor;
	GError *err = NULL;
	
	DEBUG_PRINT ("VimPlugin: Creating new editor ...");

	editor = VIM_EDITOR (g_object_new(VIM_TYPE_EDITOR, NULL));

	/* TODO: Handle filenames/uri's properly */
	if (strcmp(uri, "") != 0) editor->priv->uri = convert2uri(uri, "file");
	else if (strcmp(filename, "") != 0) editor->priv->uri = convert2uri(filename, "file");
	else editor->priv->uri = NULL;

	if (strcmp(filename, "") != 0) editor->priv->filename = convert2filename(filename);
	else if (strcmp(filename, "") != 0) editor->priv->filename = convert2filename(filename);
	else editor->priv->filename = NULL;

	if (filename) DEBUG_PRINT ("URI:%s\n FILENAME:%s\n", editor->priv->uri, editor->priv->filename);

	/* Open in Vim. And create a buf id */

	return editor;
}

/* GObject */
static void
vim_editor_instance_init (GObject *object)
{
	VimEditor *editor = VIM_EDITOR (object);
	editor->priv = g_new0 (VimEditorPrivate, 1);
	editor->priv->widget = g_object_new (VIM_TYPE_WIDGET, NULL);
}

static void
vim_editor_finalize (GObject *object)
{
	/* TODO: Add deinitalization code here */	
	VimEditor *editor = VIM_EDITOR (object);
	
	parent_class->finalize (object);
}

static void
vim_editor_class_init (VimEditorClass *klass)
{
	GObjectClass* object_class = G_OBJECT_CLASS (klass);
	parent_class = g_type_class_peek_parent (klass);

	object_class->finalize = vim_editor_finalize;
}

ANJUTA_TYPE_BEGIN (VimEditor, vim_editor, G_TYPE_OBJECT);
ANJUTA_TYPE_ADD_INTERFACE(ieditor, IANJUTA_TYPE_EDITOR);
ANJUTA_TYPE_ADD_INTERFACE(imultiple, IANJUTA_TYPE_EDITOR_MULTIPLE);
ANJUTA_TYPE_ADD_INTERFACE(idocument, IANJUTA_TYPE_DOCUMENT);
ANJUTA_TYPE_ADD_INTERFACE(ifile, IANJUTA_TYPE_FILE);
ANJUTA_TYPE_ADD_INTERFACE(isave, IANJUTA_TYPE_FILE_SAVABLE);
ANJUTA_TYPE_END;