project('filescanner', 'c', version: '0.0.1')

compiler = meson.get_compiler('c')


fs_sources = ['src/main.c',
	'src/file_scanner.c',
	'src/file_scanner.h',
	'src/directories.c',
	'src/directories.h',
	'src/debug.h',
	'src/directory_hblk.c',
	'src/directory_hblk.h',
	'src/database.h',
	'src/database.c']

cc = meson.get_compiler('c')
sqlite_dep = cc.find_library('SQLite3', required : true)
rb_dep = cc.find_library('rb', required: true)

deps = [
    sqlite_dep,   
	rb_dep 
]

executable('filescanner', fs_sources, c_args: ['-maes', '-mavx2'], dependencies: deps, install: true)
