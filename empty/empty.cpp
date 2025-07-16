#include <QApplication>
// #include <QGuiApplication>
#include <QLabel>
int main(int argc, char **argv) {
  // QGuiApplication app(argc, argv);
  QApplication app(argc, argv);
  QLabel text;
  text.setText("Hello, world.");
  text.show();

  return app.exec();
}
