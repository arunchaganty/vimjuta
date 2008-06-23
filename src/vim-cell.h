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

#ifndef _VIM_CELL_H_
#define _VIM_CELL_H_

#include <glib-object.h>
#include "vim-editor.h"

G_BEGIN_DECLS

#define VIM_TYPE_CELL             (vim_cell_get_type ())
#define VIM_CELL(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), VIM_TYPE_CELL, VimEditorCell))
#define VIM_CELL_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), VIM_TYPE_CELL, VimEditorCellClass))
#define VIM_IS_CELL(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), VIM_TYPE_CELL))
#define VIM_IS_CELL_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), VIM_TYPE_CELL))
#define VIM_CELL_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), VIM_TYPE_CELL, VimEditorCellClass))

typedef struct _VimEditorCellClass VimEditorCellClass;
typedef struct _VimEditorCell VimEditorCell;
typedef struct _VimEditorCellPrivate VimEditorCellPrivate;

struct _VimEditorCellClass
{
	GObjectClass parent_class;
};

struct _VimEditorCell
{
	GObject parent_instance;

	/* private */
	VimEditorCellPrivate *priv;
};

GType vim_cell_get_type (void) G_GNUC_CONST;
VimEditorCell* vim_cell_new (VimEditor* editor, gint position);
VimEditor* vim_cell_get_editor (VimEditorCell *cell);
void vim_cell_set_position (VimEditorCell *cell, gint position);
gint vim_cell_get_position (VimEditorCell *cell);

G_END_DECLS

#endif /* _VIM_CELL_H_ */
