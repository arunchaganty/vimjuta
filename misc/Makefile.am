
anjuta_gvim_filesdir = $(anjuta_data_dir)/gvim
anjuta_gvim_files_DATA = \
	anjuta-gvim.vim\
	anjuta.gvimrc\
	anjuta.vimrc\
	vimjuta-accels\
	locked_keys\
	vim-dbus.py\
	anjuta-linemark-16.png\
	anjuta-breakpoint-enabled-16.png\
	anjuta-breakpoint-disabled-16.png\
	anjuta-pcmark-16.png\
	anjuta-bookmark-16.png

edit = sed \
	-e 's,\@datadir\@,$(anjuta_gvim_filesdir),g'

anjuta.gvimrc: anjuta.gvimrc.in
	rm -f create_global_tags.sh
	$(edit) $(srcdir)/anjuta.gvimrc.in > anjuta.gvimrc

EXTRA_DIST = $(anjuta_gvim_files_DATA) anjuta.gvimrc.in

