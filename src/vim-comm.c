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

#include "vim-comm.h"
#include <gdk/gdk.h>
#include <gdk/gdkx.h>
#include <string.h>

static guint
vim_comm_gen_message (VimWidget *widget, gchar **cmd, gboolean isexpr);
static gint 
vim_comm_property_get (VimWidget *widget, const gchar *prop, guchar **reply);

gboolean 
vim_comm_init (VimWidget *widget, GError **error)
{	
	GdkWindow *window = gtk_widget_get_root_window (GTK_WIDGET(widget));
	GdkAtom prop_vim = gdk_atom_intern ("Vim", FALSE);
	GdkAtom type_string = gdk_atom_intern ("STRING", FALSE);
    guchar *reply, *str; 
    gchar **tmp;
    guint len;
    guint len_;
    glong wid;

	gdk_property_change (window,
			prop_vim, type_string,
			8, GDK_PROP_MODE_REPLACE,
			"6.0", 4);

    /* get the vim window */
    len = vim_comm_property_get (widget, "VimRegistry", &str);
    len_ = 0;
    do
    {
        tmp = g_strsplit(str, " ", 2);
        if (tmp[0] && tmp[1] && g_str_equal(tmp[1], widget->priv->servername))
            break;
        len_ += strlen(str)+1;
        str += strlen(str)+1;
        
    } while (len_ < len);

    if (len_ >= len) return FALSE;

    sscanf (tmp[0],"%x", &wid);

    widget->priv->vim = gdk_window_foreign_new (wid);

	return TRUE;
}

static gint 
vim_comm_property_get (VimWidget *widget, const gchar *prop, guchar **reply)
{
    GdkWindow *window = gtk_widget_get_root_window (GTK_WIDGET(widget));
    GdkAtom prop_atom = gdk_atom_intern (prop, FALSE);
    GdkAtom type_string = gdk_atom_intern ("STRING", FALSE);
    GdkAtom type_actual;
    gint len, format;
    int i;
    gdk_property_get (window,
                prop_atom,
                type_string,
                0,
                500,
                FALSE,
                &type_actual,
                &format,
                &len,
                reply);
    
    return len;
}

/* Change the string to a message */
static guint
vim_comm_gen_message (VimWidget *widget, gchar **cmd, gboolean isexpr)
{
    static guint serial = 0;
	GdkWindow *window = gtk_widget_get_root_window (GTK_WIDGET(widget));
	XID xid = GDK_WINDOW_XID(window);
    gchar *tmp, *cmd_str;
    gint len;
    serial++;
	len = 1+(1+1)+(3+strlen(widget->priv->servername)+1)+(3+strlen(*cmd)+1)+(3+8+8+1);
    cmd_str = tmp = (gchar*) malloc (sizeof(gchar)*len);
    tmp += sprintf (tmp,"") + 1;
    tmp += sprintf (tmp,"%c", (isexpr)?'c':'k') + 1;
    tmp += sprintf (tmp,"-n %s", widget->priv->servername) + 1;
    tmp += sprintf (tmp,"-s %s", *cmd) + 1;
    tmp += sprintf (tmp,"-r %x %d", xid, len) + 1;
    //tmp += sprintf (tmp,"-r %x %d", xid, serial);
    *cmd = cmd_str;
    len = tmp - cmd_str;
    return len;
}

void
vim_comm_send_keys (VimWidget* widget, gchar* keys, GError **error) 
{
	GdkWindow *window = gtk_widget_get_root_window (GTK_WIDGET(widget));
	XID xid = GDK_WINDOW_XID(window);
	GdkAtom prop_comm = gdk_atom_intern ("Comm", FALSE);
	GdkAtom type_string = gdk_atom_intern ("STRING", FALSE);
	gchar *cmd_str = NULL;
	gint len;
	/* generate command string */
    len = vim_comm_gen_message (widget, &keys, FALSE);

	gdk_property_change (widget->priv->vim,
			prop_comm, type_string,
			8, GDK_PROP_MODE_APPEND,
			keys, len);
}

void
vim_comm_exec_without_reply (VimWidget* widget, gchar* cmd, GError **error) 
{
    gchar *str = g_strdup_printf ("<C-\\><C-N>:%s<CR>", cmd);
    vim_comm_send_keys (widget, str, error);
    g_free (str);
}

