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

#include <libanjuta/interfaces/ianjuta-iterable.h>
#include "vim-cell.h"
#include "vim-dbus.h"

static GObjectClass* parent_class = NULL;

struct _VimEditorCellPrivate {
	VimEditor *editor;
	gint position;
};


/* IAnjutaIterable implementation */
static gboolean
iiter_first (IAnjutaIterable* iter, GError** e)
{
	VimEditorCell* cell = VIM_CELL(iter);
	cell->priv->position = 1;
	return TRUE;
}

static gboolean
iiter_next (IAnjutaIterable* iter, GError** e)
{
	VimEditorCell* cell = VIM_CELL(iter);
	gint final_pos;

	// FIXME: Seems expensive
	final_pos = vim_dbus_int_query (cell->priv->editor->priv->widget, "AnjutaPos('$')", e);
	if (cell->priv->position >= final_pos) 
	{
		cell->priv->position = final_pos;
		return FALSE;
	}
	cell->priv->position++;

	return TRUE;
}

static gboolean
iiter_previous (IAnjutaIterable* iter, GError** e)
{
	VimEditorCell* cell = VIM_CELL(iter);
	gint begin_pos;
	// FIXME: Seems highly sub-optimal
	begin_pos = vim_dbus_int_query (cell->priv->editor->priv->widget, "AnjutaPos('0')", e);
	if (cell->priv->position >= begin_pos) 
	{
		cell->priv->position = begin_pos;
		return FALSE;
	}
	cell->priv->position++;
	return TRUE;
}

static gboolean
iiter_last (IAnjutaIterable* iter, GError** e)
{
	VimEditorCell* cell = VIM_CELL(iter);
	gint final_pos;
	// FIXME: Seems highly sub-optimal
	final_pos = vim_dbus_int_query (cell->priv->editor->priv->widget, "AnjutaPos('$')", e);
	if (cell->priv->position == final_pos) 
		return FALSE;
	cell->priv->position = final_pos;
	return TRUE;
}

static void
iiter_foreach (IAnjutaIterable* iter, GFunc callback, gpointer data, GError** e)
{
	VimEditorCell* cell = VIM_CELL(iter);
	gint saved;
	
	/* Save current position */
	saved = cell->priv->position;
	cell->priv->position = 0;
	while (ianjuta_iterable_next (iter, NULL))
	{
		(*callback)(cell, data);
	}
	
	/* Restore current position */
	cell->priv->position = saved;
}

static gboolean
iiter_set_position (IAnjutaIterable* iter, gint position, GError** e)
{
	gchar* cmd;
	gint final_pos;
	VimEditorCell* cell = VIM_CELL(iter);

	final_pos = vim_dbus_int_query (cell->priv->editor->priv->widget, "AnjutaPos('$')", e);
	
	if (position < 0)
	{
		/* Set to end-iter (length of the doc) */
		cell->priv->position = final_pos;
		return TRUE;
	}

	if (position > final_pos) 
	{
		cell->priv->position = position;
		return  TRUE;
	}
	else
		return FALSE;

}

static gint
iiter_get_position (IAnjutaIterable* iter, GError** e)
{
	gint char_position = 0;
	
	VimEditorCell* cell = VIM_CELL(iter);

	return cell->priv->position;
}

static gint
iiter_get_length (IAnjutaIterable* iter, GError** e)
{
	VimEditorCell* cell = VIM_CELL(iter);
	return vim_dbus_int_query (cell->priv->editor->priv->widget, "AnjutaPos('$')", e);
}

static IAnjutaIterable *
iiter_clone (IAnjutaIterable *iter, GError **e)
{
	VimEditorCell *src = VIM_CELL (iter);
	VimEditorCell *cell = vim_cell_new (src->priv->editor,
												 src->priv->position);
	return IANJUTA_ITERABLE (cell);
}

static void
iiter_assign (IAnjutaIterable *iter, IAnjutaIterable *src_iter, GError **e)
{
	VimEditorCell *cell = VIM_CELL (iter);
	VimEditorCell *src = VIM_CELL (src_iter);
	cell->priv->editor = src->priv->editor;
	cell->priv->position = src->priv->position;
}

static gint
iiter_compare (IAnjutaIterable *iter, IAnjutaIterable *iter2, GError **e)
{
	gint delta;
	VimEditorCell *cell = VIM_CELL (iter);
	VimEditorCell *cell2 = VIM_CELL (iter2);
	delta = cell->priv->position - cell2->priv->position;
	return (delta == 0)? 0 : ((delta > 0)? 1 : -1);
}

static gint
iiter_diff (IAnjutaIterable *iter, IAnjutaIterable *iter2, GError **e)
{
	gint diff = 0;
	VimEditorCell *cell = VIM_CELL (iter);
	VimEditorCell *cell2 = VIM_CELL (iter2);
	
	return cell->priv->position - cell2->priv->position;	
}

/* Class Implementation */

VimEditorCell*
vim_cell_new (VimEditor* editor, gint position)
{
	VimEditorCell *cell;
	
	g_return_val_if_fail (VIM_IS_EDITOR (editor), NULL);
	g_return_val_if_fail (position >= 0, NULL);
	
	cell = VIM_CELL (g_object_new(VIM_TYPE_CELL, NULL));
	
	g_object_ref (editor);
	cell->priv->editor = editor;
	vim_cell_set_position (cell, position);
	return cell;
}

VimEditor*
vim_cell_get_editor (VimEditorCell *cell)
{
	g_return_val_if_fail (VIM_IS_CELL(cell), NULL);
	return cell->priv->editor;
}

void
vim_cell_set_position (VimEditorCell *cell, gint position)
{
	g_return_if_fail (VIM_IS_CELL(cell));
	g_return_if_fail (position >= 0);
	
	iiter_set_position (IANJUTA_ITERABLE(cell), position, NULL);
}

gint
vim_cell_get_position (VimEditorCell *cell)
{
	g_return_val_if_fail (VIM_IS_CELL(cell), -1);
	return 	iiter_get_position (IANJUTA_ITERABLE(cell), NULL);
}


static void
vim_cell_instance_init (VimEditorCell *cell)
{
	/* TODO: Add initialization code here */

	cell->priv = g_new0 (VimEditorCellPrivate, 1);
}

static void
vim_cell_finalize (GObject *object)
{
	/* TODO: Add deinitalization code here */

	parent_class->finalize (object);
}

static void
vim_cell_class_init (VimEditorCellClass *klass)
{
	GObjectClass* object_class = G_OBJECT_CLASS (klass);
	parent_class = g_type_class_peek_parent (klass);

	object_class->finalize = vim_cell_finalize;
}

static void
iiter_iface_init(IAnjutaIterableIface* iface)
{
	iface->first = iiter_first;
	iface->next = iiter_next;
	iface->previous = iiter_previous;
	iface->last = iiter_last;
	iface->foreach = iiter_foreach;
	iface->set_position = iiter_set_position;
	iface->get_position = iiter_get_position;
	iface->get_length = iiter_get_length;
	iface->clone = iiter_clone;
	iface->assign = iiter_assign;
	iface->compare = iiter_compare;
	iface->diff = iiter_diff;
}

ANJUTA_TYPE_BEGIN (VimEditorCell, vim_cell, G_TYPE_OBJECT);
ANJUTA_TYPE_ADD_INTERFACE(iiter, IANJUTA_TYPE_ITERABLE);
ANJUTA_TYPE_END;

