qml=$(shell pkg-config --cflags --libs Qt6Core Qt6Gui Qt6Widgets)
# qml=$(shell pkg-config --cflags --libs Qt6Core Qt6Gui)
ubsan=-fsanitize=address,undefined

wwid: wwid.cpp
	g++ -std=c++17 -O0 -g $^ $(qml) $(ubsan) -o $@
.PHONY: wwid
	
# ai: ai_wwid.cpp
# 	g++ -O1 -g $^ $(qml) $(ubsan) -o $@

