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

#include <gio/gio.h>
#include <libanjuta/anjuta-debug.h>
#include <libanjuta/interfaces/ianjuta-document.h>
#include <libanjuta/interfaces/ianjuta-editor.h>
#include <libanjuta/interfaces/ianjuta-editor-multiple.h>
#include <libanjuta/interfaces/ianjuta-editor-folds.h>
#include <libanjuta/interfaces/ianjuta-editor-goto.h>
#include <libanjuta/interfaces/ianjuta-editor-language.h>
#include <libanjuta/interfaces/ianjuta-editor-line-mode.h>
#include <libanjuta/interfaces/ianjuta-editor-search.h>
#include <libanjuta/interfaces/ianjuta-editor-selection.h>
#include <libanjuta/interfaces/ianjuta-editor-assist.h>
#include <libanjuta/interfaces/ianjuta-file.h>
#include <libanjuta/interfaces/ianjuta-file-savable.h>
#include <libanjuta/interfaces/ianjuta-markable.h>
#include "vim-widget.h"
#include "vim-editor.h"
#include "vim-widget-priv.h"
#include "vim-editor-priv.h"
#include <string.h>

extern void ieditor_iface_init (IAnjutaEditorIface *iface);
extern void imultiple_iface_init (IAnjutaEditorMultipleIface *iface);
extern void ifolds_iface_init (IAnjutaEditorFoldsIface *iface);
extern void igoto_iface_init (IAnjutaEditorGotoIface *iface);
extern void ilanguage_iface_init (IAnjutaEditorLanguageIface *iface);
extern void ilinemode_iface_init (IAnjutaEditorLineModeIface *iface);
extern void isearch_iface_init (IAnjutaEditorSearchIface *iface);
extern void iselection_iface_init (IAnjutaEditorSelectionIface *iface);
extern void iassist_iface_init (IAnjutaEditorAssistIface *iface);

extern void idocument_iface_init (IAnjutaDocumentIface *iface);
extern ifile_iface_init (IAnjutaFileIface *iface);
extern isave_iface_init (IAnjutaFileSavableIface *iface);

extern imarkable_iface_init (IAnjutaMarkableIface *iface);

#define VIM_EDITOR_PRIVATE(o)  (G_TYPE_INSTANCE_GET_PRIVATE ((o), VIM_TYPE_EDITOR, VimEditorPrivate))

static GObjectClass* parent_class = NULL;

VimEditor*
vim_editor_new (AnjutaPlugin *plugin, GFile* file)
{
	VimEditor *editor;
	VimWidget *widget;
	GError *err = NULL;
	
	if (!file)
		file = g_file_new_for_path (UNTITLED_FILE);

	DEBUG_PRINT ("VimPlugin: Creating new editor ...");

	widget = VIM_WIDGET (g_object_new(VIM_TYPE_WIDGET, NULL));

	/* Make sure that editors aren't repeated */
	if (widget &&
		(editor = vim_widget_get_document_file (widget, file, NULL)))
	{
		g_object_unref (widget);
		return g_object_ref (editor);
	}

	editor = VIM_EDITOR (g_object_new(VIM_TYPE_EDITOR, NULL));
	editor->priv->file = file;
	vim_widget_add_document (widget, editor, NULL);
	g_object_unref (widget);

	/* Add to the documents */
	return g_object_ref_sink (editor);
}

void
vim_editor_update_variables (VimEditor *editor)
{
	editor->priv->length = vim_dbus_int_query (editor->priv->widget, 
			"AnjutaPos('$')", NULL);
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
vim_editor_dispose (GObject *object)
{
	VimEditor *editor = VIM_EDITOR (object);
	VimWidget *widget = editor->priv->widget;

	if (vim_widget_has_editor(widget,editor))
		vim_widget_remove_document_complete (widget, editor);

	parent_class->dispose (object);
}

static void
vim_editor_finalize (GObject *object)
{
	VimEditor *editor = VIM_EDITOR (object);
	g_object_unref (editor->priv->file);
	g_object_unref (editor->priv->widget);
	g_free (editor->priv);
	
	parent_class->finalize (object);
}

static void
vim_editor_class_init (VimEditorClass *klass)
{
	static gboolean initialized = FALSE;
	GObjectClass* object_class = G_OBJECT_CLASS (klass);
	parent_class = g_type_class_peek_parent (klass);

	/*
	if (!initialized)
	{
		initialized = TRUE;

		g_signal_new ("destroy",
			VIM_TYPE_EDITOR,
			G_SIGNAL_RUN_CLEANUP|G_SIGNAL_NO_RECURSE|G_SIGNAL_NO_HOOKS,
			0,
			NULL, NULL,
			g_cclosure_marshal_VOID__VOID,
			G_TYPE_NONE,
			0);
	}
	*/


	object_class->dispose = vim_editor_dispose;
	object_class->finalize = vim_editor_finalize;
}

ANJUTA_TYPE_BEGIN (VimEditor, vim_editor, GTK_TYPE_OBJECT);
ANJUTA_TYPE_ADD_INTERFACE(ieditor, IANJUTA_TYPE_EDITOR);
ANJUTA_TYPE_ADD_INTERFACE(imultiple, IANJUTA_TYPE_EDITOR_MULTIPLE);
ANJUTA_TYPE_ADD_INTERFACE(ifolds, IANJUTA_TYPE_EDITOR_FOLDS);
ANJUTA_TYPE_ADD_INTERFACE(igoto, IANJUTA_TYPE_EDITOR_GOTO);
ANJUTA_TYPE_ADD_INTERFACE(ilanguage, IANJUTA_TYPE_EDITOR_LANGUAGE);
ANJUTA_TYPE_ADD_INTERFACE(ilinemode, IANJUTA_TYPE_EDITOR_LINE_MODE);
//ANJUTA_TYPE_ADD_INTERFACE(isearch, IANJUTA_TYPE_EDITOR_SEARCH);
//ANJUTA_TYPE_ADD_INTERFACE(iselection, IANJUTA_TYPE_EDITOR_SELECTION);
ANJUTA_TYPE_ADD_INTERFACE(iassist, IANJUTA_TYPE_EDITOR_ASSIST);
ANJUTA_TYPE_ADD_INTERFACE(idocument, IANJUTA_TYPE_DOCUMENT);
ANJUTA_TYPE_ADD_INTERFACE(ifile, IANJUTA_TYPE_FILE);
ANJUTA_TYPE_ADD_INTERFACE(isave, IANJUTA_TYPE_FILE_SAVABLE);
ANJUTA_TYPE_ADD_INTERFACE(imarkable, IANJUTA_TYPE_MARKABLE);
ANJUTA_TYPE_END;
