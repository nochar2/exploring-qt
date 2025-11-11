#include "qnamespace.h"
#include <QApplication>
// #include "qabstractbutton.h"
#include <QPushButton>
#include <QWidget>
#include <QParallelAnimationGroup>
#include <QPropertyAnimation>
#include <QRandomGenerator>
#include <QHBoxLayout>
#include <QGraphicsOpacityEffect>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QWidget window;
    window.resize(400, 400);

    auto *button = new QPushButton("Spawn square", &window);
    button->setFixedSize(150, 40);

    QHBoxLayout layout; window.setLayout(&layout);
    layout.addWidget(button);
    layout.setAlignment(Qt::AlignCenter);

    QObject::connect(button, &QPushButton::clicked, [&]() {
        // QWidget *sq = new QWidget(&window);
        auto *square = new QPushButton(&window);
        // sq->setAutoFillBackground(false);

        // ???
        // sq->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowSystemMenuHint);
        // -- ODD: Why does this cause QWidget to disappear completely, but not QPushButton?
        // -- (I can't set any of them to be translucent)
        // sq->setAttribute(Qt::WA_TranslucentBackground);
        // sq->setWindowOpacity(1);
        

        // -- DUMB! But I don't know how to do this. There is no opacity attr, the above
        // window hints don't work
        square->setStyleSheet("background-color: rgba(255, 0, 0, " + QString::number(50) + ")");

        int sqSize = 40;
        square->resize(sqSize, sqSize);

        // Random x position
        int x = QRandomGenerator::global()->bounded(window.width() - square->width());
        int y = 0;

        square->move(x, y);
        square->show();


        // Animate from top to bottom        
        auto animPos = new QPropertyAnimation (square, "pos");
        
        animPos->setDuration(2000);
        animPos->setStartValue(QPoint(x, y));
        animPos->setEasingCurve(QEasingCurve::OutBounce);
        animPos->setEndValue(QPoint(x, window.height() - sqSize));
        animPos->start();

        // -- I can't make any of this to work...
        // auto opEff = new QGraphicsOpacityEffect(sq);
        // auto animOp = new QPropertyAnimation (opEff, "opacity");
        // auto animOp = new QPropertyAnimation (sq, "windowOpacity");
        // animOp->setDuration(2000);
        // animOp->setStartValue(1);
        // animOp->setEndValue(0);
        // animOp->start();
    });

    window.show();
    return app.exec();
}

