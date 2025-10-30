#include <QGuiApplication>
#include <QWindow>
#include <QBackingStore>
#include <QPainter>
#include <cassert>
#include <cstdio>
#include <QPropertyAnimation>
#include <iostream>
#include <print>

struct MyRectangle : QObject {
    QRect area;

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

        // Create a backing surface the same size as the window
        QSize size = this->size();
        m_backingStore.resize(size);

        // QRect rect(0, 0, size.width(), size.height());

        auto rect = rectObj.area;
        QRegion whole_screen(rect);

        m_backingStore.beginPaint(whole_screen); {

            // WARNING: this pointer is only valid between calls to beginPaint
            // and endPaint. (Qt docs) (it segfaults otherwise)
            QPaintDevice *device = m_backingStore.paintDevice();
            // printf("device ptr: %p\n", device);
            // fflush(stdout);

            QPainter painter(device);

            // Fill background
            painter.fillRect(0, 0, size.width(), size.height(), Qt::gray);

            // Draw something
            painter.setBrush(Qt::red);
            
            // std::print("[{} {} {} {}]", rect.x(), rect.y(), rect.width(), rect.height());

            painter.drawRect(rect.x(), rect.y(), rect.width(), rect.height());
        } m_backingStore.endPaint();

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


    window.show();
    return app.exec();
}

// #include "ai_wwid.moc"
