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

/* General Helper Functions */

#include <glib-object.h>
#include <string.h>
#include "vim-util.h"

/* Parses return strings of the form [start,end,string] */
void 
parse_vim_arr (gchar* str, int *start, int *end, gchar** ret_str)
{
	gchar* tmp_str;
	gchar** str_array;
	tmp_str = str_substr (str, 1, strlen(str)-1);
	str_array = g_strsplit (tmp_str, ",", 3);
	if (start) *start = atoi (str_array[0]);
	if (end) *end = atoi (str_array[1]);
	if (ret_str) *ret_str = str_substr(str_array[2], 2, strlen(str_array[2]) -1);
	g_free (tmp_str);
	g_strfreev (str_array);
}

gchar* 
str_substr (const gchar* str, const gint start, const gint end)
{
	gchar *new_str = NULL;
	gint i, j = 0;

	g_assert (end >= start && end < strlen(str));
	if (end == start)
		return g_strdup ("");
	new_str = g_new0 (gchar, end - start);

	memcpy (new_str, &str[start], (end-start)*sizeof(gchar));

	return new_str;
}

gchar*
convert2uri (const gchar* file, const gchar* schema) {
	gchar *tmp;

	tmp = g_strdup_printf ("%s://",schema);
	if (!g_str_has_prefix (file, tmp))
	{
		g_free(tmp);
		tmp = g_strdup_printf("%s://%s",schema,file);
	} 
	else 
	{
		g_free(tmp);
		tmp = g_strdup(file);
	}

	return tmp;
}

gchar*
convert2filename (const gchar* uri) {
	gchar *tmp;

	tmp = g_uri_parse_scheme (uri);
	if (!tmp)
		tmp = g_strdup (uri);
	else if (strcmp (tmp, "file") == 0)
	{
		tmp = str_substr (uri, 7, strlen (uri)); 
	}
	else
		/* Throw an error ror something */
		tmp = NULL;
	return tmp;
}


