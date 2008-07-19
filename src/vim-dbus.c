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
#include "vim-signal.h"
#include <dbus/dbus.h>
#include <dbus/dbus-glib.h>
#include <string.h>
#include "vim-widget.h"
#include "vim-widget-priv.h"

extern void vim_signal_buf_new_file_cb (DBusGProxy *proxy, const guint, 
		VimWidget *widget);
extern void vim_signal_buf_read_cb (DBusGProxy *proxy, const guint, 
		const gchar* filename, VimWidget *widget);
extern void vim_signal_buf_write_cb (DBusGProxy *proxy, const guint, 
		const gchar* filename, VimWidget *widget);
extern void vim_signal_buf_add_cb (DBusGProxy *proxy, const guint, 
		const gchar* filename, VimWidget *widget);
extern void vim_signal_buf_delete_cb (DBusGProxy *proxy, const guint, 
		VimWidget *widget);
extern void vim_signal_buf_file_post_cb (DBusGProxy *proxy, const guint, 
		const gchar* filename, VimWidget *widget);
extern void vim_signal_buf_enter_cb (DBusGProxy *proxy, const guint, 
		VimWidget *widget);
extern void vim_signal_buf_leave_cb (DBusGProxy *proxy, const guint, 
		VimWidget *widget);
extern void vim_signal_vim_leave_cb (DBusGProxy *proxy, VimWidget *widget);
extern void vim_signal_menu_popup_cb (DBusGProxy *proxy, const guint, 
		VimWidget *widget);

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
	cmd_list = g_list_append (cmd_list, g_strdup (cmd));
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
		vim_dbus_exec_without_reply (widget, (gchar*) node->data, NULL);
	g_list_foreach (cmd_list, (GFunc)g_free, NULL);
	g_list_free (cmd_list);
	cmd_list = NULL;
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

		g_print ("proxy: \n\t%s \n\t%s \n\t%s\n", 
				dbus_g_proxy_get_bus_name(widget->priv->proxy),
				dbus_g_proxy_get_path(widget->priv->proxy),
				dbus_g_proxy_get_interface(widget->priv->proxy)
				);

		// TODO: hook up signals
		/* Register Marshallers */
		dbus_g_object_register_marshaller (g_cclosure_user_marshal_VOID__UINT_STRING, 
											G_TYPE_NONE,
											G_TYPE_UINT, 
											G_TYPE_STRING,
											G_TYPE_INVALID); 


		/* Register signals */
		dbus_g_proxy_add_signal (widget->priv->proxy,
									"BufNewFile",
									G_TYPE_UINT,
									G_TYPE_INVALID);
		dbus_g_proxy_add_signal (widget->priv->proxy,
									"BufRead",
									G_TYPE_UINT,
									G_TYPE_STRING,
									G_TYPE_INVALID);
		dbus_g_proxy_add_signal (widget->priv->proxy,
									"BufWrite",
									G_TYPE_UINT,
									G_TYPE_STRING,
									G_TYPE_INVALID);
		dbus_g_proxy_add_signal (widget->priv->proxy,
									"BufAdd",
									G_TYPE_UINT,
									G_TYPE_STRING,
									G_TYPE_INVALID);
		dbus_g_proxy_add_signal (widget->priv->proxy,
									"BufDelete",
									G_TYPE_UINT,
									G_TYPE_INVALID);
		dbus_g_proxy_add_signal (widget->priv->proxy,
									"BufFilePost",
									G_TYPE_UINT,
									G_TYPE_STRING,
									G_TYPE_INVALID);
		dbus_g_proxy_add_signal (widget->priv->proxy,
									"BufEnter",
									G_TYPE_UINT,
									G_TYPE_STRING,
									G_TYPE_INVALID);
		dbus_g_proxy_add_signal (widget->priv->proxy,
									"BufLeave",
									G_TYPE_UINT,
									G_TYPE_INVALID);
		dbus_g_proxy_add_signal (widget->priv->proxy,
									"VimLeave",
									G_TYPE_INVALID);
		dbus_g_proxy_add_signal (widget->priv->proxy,
									"MenuPopup",
									G_TYPE_UINT,
									G_TYPE_INVALID);


		/* Connect Signals */
		dbus_g_proxy_connect_signal (widget->priv->proxy,
									"BufNewFile",
									G_CALLBACK(vim_signal_buf_new_file_cb),
									widget,
									NULL);
		dbus_g_proxy_connect_signal (widget->priv->proxy,
									"BufRead",
									G_CALLBACK(vim_signal_buf_read_cb),
									widget,
									NULL);
		dbus_g_proxy_connect_signal (widget->priv->proxy,
									"BufWrite",
									G_CALLBACK(vim_signal_buf_write_cb),
									widget,
									NULL);
		dbus_g_proxy_connect_signal (widget->priv->proxy,
									"BufAdd",
									G_CALLBACK(vim_signal_buf_add_cb),
									widget,
									NULL);
		dbus_g_proxy_connect_signal (widget->priv->proxy,
									"BufDelete",
									G_CALLBACK(vim_signal_buf_delete_cb),
									widget,
									NULL);
		dbus_g_proxy_connect_signal (widget->priv->proxy,
									"BufFilePost",
									G_CALLBACK(vim_signal_buf_file_post_cb),
									widget,
									NULL);
		dbus_g_proxy_connect_signal (widget->priv->proxy,
									"BufEnter",
									G_CALLBACK(vim_signal_buf_enter_cb),
									widget,
									NULL);
		dbus_g_proxy_connect_signal (widget->priv->proxy,
									"BufLeave",
									G_CALLBACK(vim_signal_buf_leave_cb),
									widget,
									NULL);
		dbus_g_proxy_connect_signal (widget->priv->proxy,
									"VimLeave",
									G_CALLBACK(vim_signal_vim_leave_cb),
									widget,
									NULL);
		dbus_g_proxy_connect_signal (widget->priv->proxy,
									"MenuPopup",
									G_CALLBACK(vim_signal_menu_popup_cb),
									widget,
									NULL);

		/* Execute all pending commands */
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
		if (error) *error = err; /* Propogate error */
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

