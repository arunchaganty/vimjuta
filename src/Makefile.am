# Sample Makefile for a anjuta plugin.

# Plugin UI file
anjuta_gvim_uidir = $(anjuta_ui_dir)
anjuta_gvim_ui_DATA =  anjuta_gvim.ui


# Plugin Glade file
anjuta_gvim_gladedir = $(anjuta_glade_dir)
anjuta_gvim_glade_DATA =  anjuta_gvim.glade

# Plugin Icon file
anjuta_gvim_pixmapsdir = $(anjuta_image_dir)
anjuta_gvim_pixmaps_DATA = anjuta_gvim.png

# Plugin description file
plugin_in_files = anjuta_gvim.plugin.in
%.plugin: %.plugin.in $(INTLTOOL_MERGE) $(wildcard $(top_srcdir)/po/*po) ; $(INTLTOOL_MERGE) $(top_srcdir)/po $< $@ -d -u -c $(top_builddir)/po/.intltool-merge-cache

anjuta_gvim_plugindir = $(anjuta_plugin_dir)
anjuta_gvim_plugin_DATA = $(plugin_in_files:.plugin.in=.plugin) \
	vim-editor.c vim-editor.h	\
	vim-idocument.c \
	vim-ieditor.c \
	vim-ifile.c 


# NOTE :
# The naming convention is very intentional
# We are forced to use the prefix 'lib' by automake and libtool
#    There is probably a way to avoid it but it is not worth to effort
#    to find out.
# The 'anjuta_' prfix is a safety measure to avoid conflicts where the
#    plugin 'libpython.so' needs to link with the real 'libpython.so'

# Include paths
AM_CPPFLAGS = \
	-DPACKAGE_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\" \
	-DANJUTA_DATA_DIR=\"$(anjuta_data_dir)\" \
	-DANJUTA_PLUGIN_DIR=\"$(anjuta_plugin_dir)\" \
	-DANJUTA_IMAGE_DIR=\"$(anjuta_image_dir)\" \
	-DANJUTA_GLADE_DIR=\"$(anjuta_glade_dir)\" \
	-DANJUTA_UI_DIR=\"$(anjuta_ui_dir)\" \
	-DPACKAGE_DATA_DIR=\"$(datadir)\" \
	-DPACKAGE_SRC_DIR=\"$(srcdir)\" \
	$(LIBANJUTA_CFLAGS) 

# Where to install the plugin
plugindir = $(anjuta_plugin_dir)

# The plugin
plugin_LTLIBRARIES = libanjuta_gvim.la

# Plugin sources
libanjuta_gvim_la_SOURCES = plugin.c plugin.h \
	vim-editor.h  vim-editor.c \
	vim-ifile.c \
	vim-idocument.c \
	vim-ieditor.c 


# Plugin dependencies
libanjuta_gvim_la_LIBADD = \
	$(LIBANJUTA_LIBS) 

EXTRA_DIST = \
	$(plugin_in_files) \
	$(anjuta_gvim_plugin_DATA) \
	$(anjuta_gvim_ui_DATA) \
	$(anjuta_gvim_glade_DATA) \
	$(anjuta_gvim_pixmaps_DATA)
