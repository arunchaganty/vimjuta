
anjuta_gvim_filesdir = $(anjuta_data_dir)/gvim
anjuta_gvim_files_DATA = \
	anjuta-gvim.vim\
	anjuta.gvimrc\
	vim-dbus.py

edit = sed \
	-e 's,\@datadir\@,$(anjuta_gvim_filesdir),g'

anjuta.gvimrc: anjuta.gvimrc.in
	rm -f create_global_tags.sh
	$(edit) $(srcdir)/anjuta.gvimrc.in > anjuta.gvimrc

EXTRA_DIST = $(anjuta_gvim_files_DATA) anjuta.gvimrc.in

