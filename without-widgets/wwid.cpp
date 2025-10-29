#include <QGuiApplication>
#include <QWindow>

int main(int argc, char **argv) {
    QGuiApplication app(argc, argv);
    QWindow window;
    window.show();
    return app.exec();
}
