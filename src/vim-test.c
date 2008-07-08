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

#include "vim-widget.h"
#include "vim-editor.h"
#include "vim-widget-priv.h"
#include "vim-editor-priv.h"
#include "vim-test.h"
#include "plugin.h"
#include <libanjuta/interfaces/ianjuta-document-manager.h>
#include <libanjuta/interfaces/ianjuta-editor.h>
#include <libanjuta/interfaces/ianjuta-file.h>
#include <libanjuta/interfaces/ianjuta-document.h>

void vim_test_begin (GtkAction *action, VimPlugin *plugin)
{
	VimEditor* vim = NULL;
	VimWidget* widget = NULL;
	IAnjutaEditor* editor = NULL;
	IAnjutaFile* file = NULL;
	IAnjutaDocument* doc= NULL;
	IAnjutaDocumentManager* docman = NULL;

	docman = (IAnjutaDocumentManager *) anjuta_shell_get_object 
		(ANJUTA_PLUGIN (plugin)->shell, "IAnjutaDocumentManager", NULL);
	doc = ianjuta_document_manager_get_current_document (docman, NULL);

	editor = IANJUTA_EDITOR (doc);
	file = IANJUTA_FILE (doc);
	vim = VIM_EDITOR (doc);
	widget = vim->priv->widget;

	g_print ("Vim Plugin Tests\n");

	g_print ("VimEditor: %d\n", vim);
	g_print ("socket: %d\n", widget->priv->socket_id);
	g_print ("filename: %s\n", g_file_get_path(vim->priv->file));
	g_print ("proxy: \n\t%s \n\t%s \n\t%s\n", 
			dbus_g_proxy_get_bus_name(widget->priv->proxy),
			dbus_g_proxy_get_path(widget->priv->proxy),
			dbus_g_proxy_get_interface(widget->priv->proxy)
			);

	g_print ("IAnjutaEditor: %d\n", editor);
	ianjuta_editor_set_tabsize (editor, 3, NULL);
	g_print ("tabsize: %d\n", ianjuta_editor_get_tabsize (editor, NULL));
	ianjuta_editor_set_use_spaces (editor, TRUE, NULL);
	g_print ("use_spaces: %d\n", ianjuta_editor_get_use_spaces (editor, NULL));
	ianjuta_editor_set_auto_indent (editor, FALSE, NULL);
	ianjuta_editor_set_auto_indent (editor, TRUE, NULL);
	ianjuta_editor_goto_line (editor, 10, NULL);
	//ianjuta_editor_goto_position (editor, 10, NULL);
	//ianjuta_editor_get_text
	//ianjuta_editor_get_text_all
	ianjuta_editor_get_position (editor, NULL);
	//ianjuta_editor_get_length (editor, NULL);
	//ianjuta_editor_insert 
	//ianjuta_editor_append
	//ianjuta_editor_erase
	g_print ("get_column: %d\n", ianjuta_editor_get_column (editor, NULL));
	//g_print ("get_overwrite: %d\n", ianjuta_editor_get_overwrite (editor, NULL));
	//ianjuta_editor_set_popup_menu
	//ianjuta_editor_get_line_from_position
	//ianjuta_editor_get_line_end_position
	//ianjuta_editor_get_line_begin_position
	//ianjuta_editor_get_position_from_offset
	ianjuta_editor_goto_start (editor, NULL);
	ianjuta_editor_goto_end (editor, NULL);
	ianjuta_editor_erase_all (editor, NULL);
}


