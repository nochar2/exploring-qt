#include <QApplication>
#include <QLabel>
#include <QTreeWidget>

int main(int argc, char **argv) {
  QApplication app(argc, argv);
  QLabel label("Hello, Qt!");
  label.setAlignment(Qt::AlignCenter);
  label.show();

  switch (argc) {
    case 1:
      return app.exec();
    default:
      return 1;
  }
}
