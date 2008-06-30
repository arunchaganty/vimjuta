# Sample Makefile for a anjuta plugin.

# Plugin UI file
anjuta-gvim_uidir = $(anjuta_ui_dir)
anjuta-gvim_ui_DATA =  anjuta-gvim.ui \
	vim-test.ui


# Plugin Glade file
anjuta-gvim_gladedir = $(anjuta_glade_dir)
anjuta-gvim_glade_DATA =  anjuta-gvim.glade

# Plugin Icon file
anjuta-gvim_pixmapsdir = $(anjuta_image_dir)
anjuta-gvim_pixmaps_DATA = anjuta-gvim.png

# Plugin description file
plugin_in_files = anjuta-gvim.plugin.in
%.plugin: %.plugin.in $(INTLTOOL_MERGE) $(wildcard $(top_srcdir)/po/*po) ; $(INTLTOOL_MERGE) $(top_srcdir)/po $< $@ -d -u -c $(top_builddir)/po/.intltool-merge-cache

anjuta-gvim_plugindir = $(anjuta_plugin_dir)
anjuta-gvim_plugin_DATA = $(plugin_in_files:.plugin.in=.plugin)

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
plugin_LTLIBRARIES = libanjuta-gvim.la

# Plugin sources
libanjuta-gvim_la_SOURCES = plugin.c plugin.h \
	vim-editor.h  vim-editor.c \
	vim-ifile.c \
	vim-idocument.c \
	vim-ieditor.c  vim-dbus.c \
	vim-dbus.h  vim-cell.h \
	vim-signal.c vim-signal.h\
	vim-cell.c\
	vim-test.c\
	vim-util.c \
	vim-widget.c vim-widget.h 


# Plugin dependencies
libanjuta-gvim_la_LIBADD = \
	$(LIBANJUTA_LIBS) 

EXTRA_DIST = \
	$(plugin_in_files) \
	$(anjuta-gvim_plugin_DATA) \
	$(anjuta-gvim_ui_DATA) \
	$(anjuta-gvim_glade_DATA) \
	$(anjuta-gvim_pixmaps_DATA) \
	vim-dbus.py \
	vim-test.ui
