#include <QApplication>
#include <QMainWindow>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QLabel>

int main(int argc, char *argv[])
{
    // 1. Basic Application Setup
    QApplication app(argc, argv);
    QMainWindow mainWindow;
    mainWindow.setWindowTitle("Partial Highlight Example");
    mainWindow.resize(400, 200);

    // 2. Create the Tree Widget
    auto *treeWidget = new QTreeWidget(&mainWindow);
    treeWidget->setColumnCount(1);
    treeWidget->setHeaderLabel("Items"); // Optional: set a header

    // 3. Create the Tree Item
    // We don't set text on the item itself, as we will use a custom widget.
    auto *topLevelItem = new QTreeWidgetItem(treeWidget);

    // 4. Create the QLabel with Rich Text (HTML)
    // This is the core of the solution. We use HTML tags like <b> for bold
    // and a style attribute for the color.
    auto *richTextLabel = new QLabel();
    richTextLabel->setText("This word is <b style='color:red'>highlighted</b> in the item.");

    // 5. Assign the QLabel as the widget for our tree item
    // Parameters are: the item, the column index (0 for the first column), and the widget.
    treeWidget->setItemWidget(topLevelItem, 0, richTextLabel);

    // 6. Finalize and Show
    mainWindow.setCentralWidget(treeWidget);
    mainWindow.show();

    return app.exec();
}
