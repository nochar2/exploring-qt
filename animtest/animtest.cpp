#include "qeasingcurve.h"
#include "qapplication.h"
#include "qpushbutton.h"
#include "qpropertyanimation.h"

class MyButtonWidget : public QWidget
{
public:
    MyButtonWidget(QWidget *parent = nullptr);
};

MyButtonWidget::MyButtonWidget(QWidget *parent) : QWidget(parent)
{
    QPushButton *button = new QPushButton(tr("Animated Button"), this);

    QPropertyAnimation *anim = new QPropertyAnimation(button, "size", this);
    // QPropertyAnimation *anim = new QPropertyAnimation(button, "pos", this);
    // anim->setStartValue(QPoint(0, 0));
    // anim->setEndValue(QPoint(400, 500));
    
    anim->setStartValue (QSize(50, 50));
    anim->setEndValue   (QSize(500,50));

    anim->setEasingCurve(QEasingCurve::OutQuad);
    anim->setDuration   (1000);
    anim->start();
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MyButtonWidget buttonAnimWidget;
    buttonAnimWidget.resize(QSize(800, 600));
    buttonAnimWidget.show();
    return a.exec();
}
