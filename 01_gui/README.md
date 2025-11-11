# without-widgets

Turns out you don't need to use Qt Widgets.

If you want, there is QGuiApplication, you can draw to a `QBackingStore`,
use `QPainter` commands directly on the window. (Don't know how to do anything yet,
but the possibility is there.)

---

|version|time|
|-------|----|
|c++17|1.9 seconds|
|c++23|2.6 seconds|
|c++23 with a single std::print call|4 seconds|
