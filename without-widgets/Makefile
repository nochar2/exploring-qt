qml=$(shell pkg-config --cflags --libs Qt6Core Qt6Gui)

wwid: wwid.cpp
	g++ $^ $(qml) -o $@
	
