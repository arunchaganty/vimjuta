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

/* X11 ICCCM Communication Helper Functions */
#include "vim-widget.h"
#include "vim-widget-priv.h"

gboolean 
vim_comm_init (VimWidget *widget, GError **error);

gboolean 
vim_comm_init (VimWidget *widget, GError **error);

void
vim_comm_send_keys (VimWidget* widget, gchar* keys, GError **error);

void
vim_comm_exec_without_reply (VimWidget* widget, gchar* cmd, GError **error) ;

