qt_cflags=$(shell pkg-config --cflags Qt6Core Qt6Gui Qt6Widgets)
qt_libs=$(shell pkg-config --libs Qt6Core Qt6Gui Qt6Widgets)

zerokara: sm_parser.o zerokara.o platform.o
	g++ $^ $(qt_libs) -o $@

sm_parser.o: sm_parser.cpp
	g++ -c -std=gnu++23 $^ -o $@

platform.o: platform.cpp
	g++ -c -std=gnu++23 $^ -o $@

zerokara.o: zerokara.cpp
	g++ -c -std=gnu++23 $^ $(qt_cflags) -o $@

clean:
	rm *.o
