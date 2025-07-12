### kind of implies `core gui widgets`
QT = widgets

TARGET = zerokara

# SOURCES = zerokara.cpp sm_parser.cpp
SOURCES = zerokara.cpp

# CONFIG += c++2b
# CONFIG += c++2b force_debug_info
CONFIG += c++2a precompile_header force_debug_info
# CONFIG += c++2b precompile_header force_debug_info

PRECOMPILED_HEADER = precompiled.h
