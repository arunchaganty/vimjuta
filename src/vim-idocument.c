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

#include <libanjuta/interfaces/ianjuta-document.h>
#include "vim-editor.h"

static void idocument_update_ui (IAnjutaDocument *obj)
{

}

static void idocument_begin_undo_action (IAnjutaDocument *obj, GError **err)
{

}

static gboolean 
idocument_can_redo (IAnjutaDocument *obj, GError **err)
{
	return TRUE;
}

static gboolean 
idocument_can_undo (IAnjutaDocument *obj, GError **err)
{
	return TRUE;
}

static void 
idocument_clear (IAnjutaDocument *obj, GError **err)
{

}

static void 
idocument_copy (IAnjutaDocument *obj, GError **err)
{

}

static void 
idocument_cut (IAnjutaDocument *obj, GError **err)
{

}

static void 
idocument_end_undo_action (IAnjutaDocument *obj, GError **err)
{
}

static const gchar* 
idocument_get_filename (IAnjutaDocument *obj, GError **err)
{
	return NULL;
}

static void 
idocument_grab_focus (IAnjutaDocument *obj, GError **err)
{

}

static void 
idocument_paste (IAnjutaDocument *obj, GError **err)
{
}

static void 
idocument_redo (IAnjutaDocument *obj, GError **err)
{

}

static void 
idocument_undo (IAnjutaDocument *obj, GError **err)
{

}

void 
idocument_iface_init (IAnjutaDocumentIface *iface)
{
	/* signal */
	iface->update_ui = idocument_update_ui;

	iface->begin_undo_action = idocument_begin_undo_action;
	iface->can_redo = idocument_can_redo;
	iface->can_undo = idocument_can_undo;
	iface->clear = idocument_clear;
	iface->copy = idocument_copy;
	iface->cut = idocument_cut;
	iface->end_undo_action = idocument_end_undo_action;
	iface->get_filename = idocument_get_filename;
	iface->grab_focus = idocument_grab_focus;
	iface->paste = idocument_paste;
	iface->redo = idocument_redo;
	iface->undo = idocument_undo;

}

