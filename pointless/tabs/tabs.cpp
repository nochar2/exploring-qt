#include "qapplication.h"
#include "qtabbar.h"
#include "qboxlayout.h"
#include "qmainwindow.h"
#include "qtoolbar.h"
#include "qlabel.h"

struct MainWindow : public QMainWindow {
  // I can't do this->setLayout, or something, I just don't know
  QWidget *root; QVBoxLayout *resize;
  QTabBar *tab_bar;
  QLabel *label;

  MainWindow () {
    root = new QWidget();
    resize = new QVBoxLayout();
    root->setLayout(resize);

    tab_bar = new QTabBar(root);
    resize->addWidget(tab_bar);
    tab_bar->addTab("First");
    tab_bar->addTab("Second");
    label = new QLabel("Label text.");
    resize->addWidget(label);

    this->setCentralWidget(root);
  }
};

int main(int argc, char **argv) {
  QApplication app(argc, argv);
  MainWindow win;
  win.show();
  return app.exec();
}
