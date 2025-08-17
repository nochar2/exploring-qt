linux {
	CONFIG = qt import_qpa_plugin \
		c++2b warn_on rtti_off exceptions_off \
		debug force_debug_info 
	QMAKE_LFLAGS += -fuse-ld=mold
}
win32 {
	# idk how to make it run windeployqt in the debug dir yet
	CONFIG += c++2b debug force_debug_info
}


# QMAKE_CXXFLAGS_DEBUG += -Wconversion

QT = widgets                  # (kind of implies `core gui` somehow)

TARGET = zerokara
SOURCES = zerokara.cpp sm_parser.cpp platform.cpp


##### NOTES ####################################
### (https://blog.shantanu.io/2016/06/25/printing-a-list-of-qmake-variables/)
### you can print all variables like this:
###
### for(var, $$list($$enumerate_vars())) {
###     message($$var)
###     message($$eval($$var))
### }
###
### The default value of CONFIG is this monstrosity:
### lex yacc debug exceptions depend_includepath testcase_targets import_plugins import_qpa_plugin file_copies qmake_use qt warn_on release link_prl incremental release shared plugin_manifest intelcet glibc_fortify_source trivial_auto_var_init_pattern stack_protector stack_clash_protection libstdcpp_assertions relro_now_linker linux unix posix gcc
###### dumb shit
### -fno-exceptions just disables try ... catch, not actual exceptions
### CONFIG -= exceptions
### precompile_header
### PRECOMPILED_HEADER = precompiled.h
