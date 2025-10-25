#include "qtreewidget.h"
#include <QApplication>
#include <QMainWindow>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QLabel>

using std::make_unique;
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    auto mainWindow = make_unique<QMainWindow>();

    mainWindow->setWindowTitle("Partial Highlight Example");
    mainWindow->resize(400, 200);

    auto treeWidget = make_unique<QTreeWidget>(mainWindow.get());
    // auto *treeWidget = new QTreeWidget(mainWindow.get());

    treeWidget->setColumnCount(1);
    treeWidget->setHeaderLabel("Items"); // Optional: set a header

    auto topLevelItem = make_unique<QTreeWidgetItem>(treeWidget.get());

    auto richTextLabel = make_unique<QLabel>();
    // auto *richTextLabel = new QLabel();
    
    richTextLabel->setText("This word is <b style='color:red'>highlighted</b> in the item.");

    treeWidget->setItemWidget(topLevelItem.get(), 0, richTextLabel.get());
    // treeWidget->setItemWidget(topLevelItem, 0, richTextLabel);

    mainWindow->setCentralWidget(treeWidget.get());
    // mainWindow->setCentralWidget(treeWidge);
    
    mainWindow->show();

    return app.exec();
}
