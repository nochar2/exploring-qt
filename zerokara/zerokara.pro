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

# CONFIG = qt import_qpa_plugin debug import_plugins c++20 precompile_header force_debug_info
CONFIG += debug c++20 precompile_header force_debug_info
# QMAKE_CXXFLAGS_DEBUG = -O0

### (kind of implies `core gui` somehow)
QT = widgets

TARGET = zerokara
SOURCES = zerokara.cpp

PRECOMPILED_HEADER = precompiled.h
