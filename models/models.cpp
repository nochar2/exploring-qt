#include <QTreeView>
#include <QTreeWidget>
#include <QStandardItemModel>
#include <QApplication>
#include <set>
using namespace Qt::Literals::StringLiterals;

int main(int argc, char **argv) {
  QApplication app(argc, argv);

  QAbstractItemModel abstract_model;

  QStandardItemModel model;
  model.setColumnCount(2);


  QStandardItem root_row("root");
  model.appendRow(&root_row);  
  
  QList<QStandardItem *> row;
  row.append(new QStandardItem("Priority"));
  row.append(new QStandardItem("High"));
  root_row.appendRow(row);

  QTreeView view;
  view.setModel(&model);
  view.expandAll();
  view.show();

  return app.exec();
}
