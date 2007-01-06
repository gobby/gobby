gobby:
	g++ src/window.cpp src/main.cpp -o gobby -Iinc `pkg-config --cflags --libs gtkmm-2.4 lobby-1.0`
