#include <QApplication>
#include <QMainWindow>
#include <QPushButton>
#include <QHBoxLayout>

int main(int argc, char **argv) {
    QApplication app(argc, argv);
    QMainWindow window;

    auto mainWidget = std::make_unique<QWidget>();
    QHBoxLayout layout(mainWidget.get());

    QPushButton b1("first button");
    QPushButton b2("second button");
    QPushButton b3("third button");
    layout.addWidget(&b1, 1);
    layout.addWidget(&b2, 2);
    layout.addWidget(&b3, 1);

    window.setCentralWidget(mainWidget.get());
    window.show();
    app.exec();
}
