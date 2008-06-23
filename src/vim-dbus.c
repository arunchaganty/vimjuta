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

/* DBus Helper Functions */
#include <libanjuta/anjuta-debug.h>
#include "vim-dbus.h"
#include <dbus/dbus.h>
#include <dbus/dbus-glib.h>
#include <string.h>

/* An implementation to queue up some commands to be run at later time  Mainly
 * used before DBus initializes*/

static GList* cmd_list = NULL;

/**
 * vim_queue_add : Adds commands to the queue. Commands are run
 * without replying
 *
 * @widget: The instance that holds the DBus Session
 * @cmd: The command
 *
 * Returns: 
 *
 */
static void
vim_queue_add (VimWidget* widget, const gchar* cmd)
{
	cmd_list = g_list_append (cmd_list, (gpointer) cmd);
}

/**
 * vim_queue_exec : Executes the list of commands. Removes them
 * from the command list as it does so.
 *
 * @widget: The instance that holds the DBus Session
 *
 * Returns: 
 *
 */
static void 
vim_queue_exec (VimWidget* widget)
{
	GList* node;
	for (node=cmd_list; node != NULL; node = g_list_next (node))
	{
		vim_dbus_exec_without_reply (widget, (gchar*) node->data, NULL);
		cmd_list = g_list_remove (cmd_list, (const gpointer) node->data);
	}
}

static void
vim_dbus_connect_cb (DBusGProxy *proxy,
		const gchar* NameOwner,
		const gchar* NameLost,
		const gchar* NameAcquired,
		VimWidget *widget)
{
	if (strcmp (NameOwner,DBUS_NAME_ANJUTA) == 0)
	{
		// Stop listening
		dbus_g_proxy_disconnect_signal (proxy,
								"NameOwnerChanged",
								G_CALLBACK(vim_dbus_connect_cb),
								widget);

		widget->priv->proxy = dbus_g_proxy_new_for_name (widget->priv->conn,
				DBUS_NAME_ANJUTA,
				DBUS_PATH_VIM,
				DBUS_IFACE_EDITOR_REMOTE);

		// TODO: hook up signals
		
		// Execute all pending commands
		vim_queue_exec (widget);

	 }
}

gboolean 
vim_dbus_init (VimWidget *widget, GError **error)
{	
	DBusGConnection *conn;
	GError *err = NULL;

	widget->priv->conn = dbus_g_bus_get (DBUS_BUS_SESSION, &err);
	if (err)
	{
		DEBUG_PRINT ("Error connecting to DBus: %s\n", err);
		if (error) *error = err; // Propogate error
		else g_error_free (err);
		return FALSE;
	}

	/* Connect to the DBus */
	widget->priv->dbus_proxy = dbus_g_proxy_new_for_name (widget->priv->conn,
			DBUS_SERVICE_DBUS,
			DBUS_PATH_DBUS,
			DBUS_INTERFACE_DBUS);

	/* Listen for a service on org.anjuta */
	dbus_g_proxy_add_signal (widget->priv->dbus_proxy,
								"NameOwnerChanged",
								G_TYPE_STRING,
								G_TYPE_STRING,
								G_TYPE_STRING,
								G_TYPE_INVALID);
	dbus_g_proxy_connect_signal (widget->priv->dbus_proxy,
								"NameOwnerChanged",
								G_CALLBACK(vim_dbus_connect_cb),
								widget,
								NULL);
	return TRUE;
}



gchar*
vim_dbus_query (VimWidget *widget, gchar* query, GError **error) 
{
	gchar *reply;
	GError *err = NULL;

	g_return_val_if_fail (VIM_PLUGIN_IS_READY(widget), NULL);

	dbus_g_proxy_call (widget->priv->proxy,
			"QueryExpr", &err,
			G_TYPE_STRING, query,
			G_TYPE_INVALID,
			G_TYPE_STRING, &reply,
			G_TYPE_INVALID);

	if (err)
	{
		if ( err->domain == DBUS_GERROR && err->code == DBUS_GERROR_REMOTE_EXCEPTION)
			DEBUG_PRINT ("Caught a remote exception \n");
		else if (err)
			DEBUG_PRINT ("Error calling QueryExpr: %s\n", err->message);
		if (error) *error = err;
		else g_error_free (err);
		return NULL;
	}
		
	return reply;
}

gint
vim_dbus_int_query (VimWidget *widget, gchar* query, GError **error) 
{
	gchar* reply = NULL;
	gint result;

	g_return_val_if_fail (VIM_PLUGIN_IS_READY(widget), 0);
	g_assert (error == NULL);

	reply = vim_dbus_query (widget, query, error);
	
	if (reply)
	{
		result = (gint)g_ascii_strtod (reply, NULL);
		g_free (reply);
		return result;	
	}
	else
	{
		return 0;
	}
}

gchar*
vim_dbus_exec (VimWidget* widget, gchar* cmd, GError **error) 
{
	gchar *reply;
	GError *err = NULL;

	g_return_val_if_fail (VIM_PLUGIN_IS_READY(widget), NULL);
	
	dbus_g_proxy_call (widget->priv->proxy,
			"ExecuteCmd", &err,
			G_TYPE_STRING, cmd,
			G_TYPE_INVALID,
			G_TYPE_STRING, &reply,
			G_TYPE_INVALID);

	if (err)
	{
		if ( err->domain == DBUS_GERROR && err->code == DBUS_GERROR_REMOTE_EXCEPTION)
			DEBUG_PRINT ("Caught a remote exception \n");
		else if (err)
			DEBUG_PRINT ("Error calling ExecuteCmd: %s\n", err->message);
		if (error) *error = err;
		else g_error_free (err);
		return NULL;
	}
		
	return reply;
}

vim_dbus_int_exec (VimWidget *widget, gchar* query, GError **error) 
{
	gchar* reply = NULL;
	gint result;

	g_return_val_if_fail (VIM_PLUGIN_IS_READY(widget), 0);
	g_assert (error == NULL);

	reply = vim_dbus_exec (widget, query, error);
	
	if (reply)
	{
		result = (gint)g_ascii_strtod (reply, NULL);
		g_free (reply);
		return result;	
	}
	else
	{
		return 0;
	}
}
void 
vim_dbus_exec_without_reply (VimWidget* widget, gchar* cmd, GError **error) 
{
	if (!VIM_PLUGIN_IS_READY(widget))
	{
		DEBUG_PRINT ("Plugin not fully initialized. Adding %s to process queue\n", cmd);
		vim_queue_add (widget, cmd);
	}
	else	
		dbus_g_proxy_call_no_reply (widget->priv->proxy,
				"ExecuteCmd",
				G_TYPE_STRING, cmd,
				G_TYPE_INVALID);
}

gchar*
vim_dbus_get_buf (VimWidget *widget, guint start, guint end, GError **error) 
{
	gchar *reply;
	GError *err = NULL;

	g_return_val_if_fail (VIM_PLUGIN_IS_READY(widget), NULL);
	dbus_g_proxy_call (widget->priv->proxy,
			"GetBufContents", &err,
			G_TYPE_UINT, start,
			G_TYPE_UINT, end,
			G_TYPE_INVALID,
			G_TYPE_STRING, &reply,
			G_TYPE_INVALID);

	if (err)
	{
		if ( err->domain == DBUS_GERROR && err->code == DBUS_GERROR_REMOTE_EXCEPTION)
			DEBUG_PRINT ("Caught a remote exception \n");
		else if (err)
			DEBUG_PRINT ("Error calling GetBufContents: %s\n", err->message);
		if (error) *error = err;
		else g_error_free (err);
		return NULL;
	}
		
	return reply;
}

gchar* 
vim_dbus_get_buf_full (VimWidget *widget, GError **error) 
{
	gchar *reply;
	GError *err = NULL;

	g_return_val_if_fail (VIM_PLUGIN_IS_READY(widget), NULL);
	dbus_g_proxy_call (widget->priv->proxy,
			"GetBufContentsFull", &err,
			G_TYPE_INVALID,
			G_TYPE_STRING, &reply,
			G_TYPE_INVALID);

	if (err)
	{
		if ( err->domain == DBUS_GERROR && err->code == DBUS_GERROR_REMOTE_EXCEPTION)
			DEBUG_PRINT ("Caught a remote exception \n");
		else if (err)
			DEBUG_PRINT ("Error calling GetBufContentsFull: %s\n", err->message);
		if (error) *error = err;
		else g_error_free (err);
		return NULL;
	}
	
	return reply;
}

