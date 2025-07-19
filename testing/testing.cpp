#include <QApplication>
#include <QTreeWidget>
int main(int argc, char **argv) {
  QApplication app(argc, argv);
  QTreeWidget *shit = new QTreeWidget();
  QTreeWidgetItem it(shit);
  shit->show();
  int ret = app.exec();
  delete shit;
  return ret;
}
