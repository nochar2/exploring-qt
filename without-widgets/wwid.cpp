// #include "qpropertyanimation.h"
#include <QGuiApplication>
#include <QWindow>
#include <QBackingStore>
#include <QPainter>
#include <cassert>
#include <cstdio>
// #include <iostream>
// #include <print>

struct MyRectangle : QObject {
    QRect area;
    QPoint pos() { return  { area.x(), area.y() }; }
    
    MyRectangle(QRect pos) : area(pos) { }
    MyRectangle(MyRectangle &other) { this->area = other.area; }
};


struct SimpleWindow : QWindow {
    // this is stupid design but whatever
    SimpleWindow(MyRectangle rect) : rectObj(rect) {
        setTitle("No Widgets Example");
        resize(400, 300);
    }

protected:
    void exposeEvent(QExposeEvent *) override {
        if (isExposed()) {
            renderNow();
        }
    }

private:
    QBackingStore m_backingStore{this};
    MyRectangle rectObj;

    void renderNow() {
        printf("paint happened!\n"); fflush(stdout);

        // -- Create a backing surface the same size as the window
        QSize size = this->size();
        m_backingStore.resize(size);

        auto rect = rectObj.area;
        QRegion whole_screen(rect);

        m_backingStore.beginPaint(whole_screen); {
            // -- WARNING: the QPaintDevice pointer is only valid between calls
            // -- to beginPaint and endPaint. (Qt docs) (it segfaults otherwise,
            // -- but *isn't* nullptr!)
            QPaintDevice *device = m_backingStore.paintDevice();
            QPainter painter(device);

            // -- Fill background
            painter.fillRect(0, 0, size.width(), size.height(), Qt::gray);

            // -- Draw something
            painter.setBrush(Qt::red);
            painter.drawRect(rect.x(), rect.y(), rect.width(), rect.height());
            // std::print("[{} {} {} {}]", rect.x(), rect.y(), rect.width(), rect.height());
        } m_backingStore.endPaint();

        // -- this doesn't work, idk why
        // bool b = m_backingStore.scroll({0, 0, 20, 20}, 0, 10);
        // printf("scroll returned %d\n", b);

        m_backingStore.flush(QRect(QPoint(), size));
    }
};

int main(int argc, char **argv) {
    QGuiApplication app(argc, argv);

    QRect rect(10, 10, 50, 50);
    MyRectangle visRect(rect);
    SimpleWindow window(visRect);

    // -- todo: animations?

    window.show();
    return app.exec();
}
