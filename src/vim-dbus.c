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

static void
vim_dbus_connect_cb (DBusGProxy *proxy,
		const gchar* NameOwner,
		const gchar* NameLost,
		const gchar* NameAcquired,
		VimEditor *vim)
{
	if (strcmp (NameOwner,"org.anjuta") == 0)
	{
		// Stop listening
		dbus_g_proxy_disconnect_signal (proxy,
								"NameOwnerChanged",
								G_CALLBACK(vim_dbus_connect_cb),
								vim);

		vim->proxy = dbus_g_proxy_new_for_name (vim->conn,
				DBUS_NAME_ANJUTA,
				DBUS_PATH_VIM,
				DBUS_IFACE_EDITOR_REMOTE);

		// TODO: hook up signals

	 }
}

gboolean 
vim_dbus_init (VimEditor *vim, GError **error)
{	
	DBusGConnection *conn;
	GError *err = NULL;

	vim->conn = dbus_g_bus_get (DBUS_BUS_SESSION, &err);
	if (err)
	{
		DEBUG_PRINT ("Error connecting to DBus: %s\n", err);
		if (error) *error = err; // Propogate error
		else g_error_free (err);
		return FALSE;
	}

	/* Connect to the DBus */
	vim->dbus_proxy = dbus_g_proxy_new_for_name (vim->conn,
			DBUS_SERVICE_DBUS,
			DBUS_PATH_DBUS,
			DBUS_INTERFACE_DBUS);

	/* Listen for a service on org.anjuta */
	dbus_g_proxy_add_signal (vim->dbus_proxy,
								"NameOwnerChanged",
								G_TYPE_STRING,
								G_TYPE_STRING,
								G_TYPE_STRING,
								G_TYPE_INVALID);
	dbus_g_proxy_connect_signal (vim->dbus_proxy,
								"NameOwnerChanged",
								G_CALLBACK(vim_dbus_connect_cb),
								vim,
								NULL);
	return TRUE;
}



gchar*
vim_dbus_query (VimEditor *vim, gchar* query, GError **error) 
{
	gchar *reply;
	GError *err = NULL;

	g_return_val_if_fail (VIM_PLUGIN_IS_READY(vim), NULL);

	dbus_g_proxy_call (vim->proxy,
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
vim_dbus_int_query (VimEditor *vim, gchar* query, GError **error) 
{
	gchar* reply = NULL;
	gint result;

	g_return_val_if_fail (VIM_PLUGIN_IS_READY(vim), 0);
	g_assert (error == NULL);

	reply = vim_dbus_query (vim, query, error);
	
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
vim_dbus_exec (VimEditor* vim, gchar* cmd, GError **error) 
{
	gchar *reply;
	GError *err = NULL;

	g_return_val_if_fail (VIM_PLUGIN_IS_READY(vim), NULL);
	
	dbus_g_proxy_call (vim->proxy,
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

vim_dbus_int_exec (VimEditor *vim, gchar* query, GError **error) 
{
	gchar* reply = NULL;
	gint result;

	g_return_val_if_fail (VIM_PLUGIN_IS_READY(vim), 0);
	g_assert (error == NULL);

	reply = vim_dbus_exec (vim, query, error);
	
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
vim_dbus_exec_without_reply (VimEditor* vim, gchar* cmd, GError **error) 
{
	g_return_if_fail (VIM_PLUGIN_IS_READY(vim));
	dbus_g_proxy_call_no_reply (vim->proxy,
			"ExecuteCmd",
			G_TYPE_STRING, cmd,
			G_TYPE_INVALID);

}

gchar*
vim_dbus_get_buf (VimEditor *vim, guint start, guint end, GError **error) 
{
	gchar *reply;
	GError *err = NULL;

	g_return_val_if_fail (VIM_PLUGIN_IS_READY(vim), NULL);
	dbus_g_proxy_call (vim->proxy,
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
vim_dbus_get_buf_full (VimEditor *vim, GError **error) 
{
	gchar *reply;
	GError *err = NULL;

	g_return_val_if_fail (VIM_PLUGIN_IS_READY(vim), NULL);
	dbus_g_proxy_call (vim->proxy,
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

