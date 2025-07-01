// #include <QtWidgets> exists but makes compilation slower
#include <QtWidgets/QWidget>
#include <QtWidgets/QFrame>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QLabel>
#include <QtGui/QWheelEvent>
#include <QtGui/QPainter>
#include <QtGui/QPaintEvent>
#include <QApplication>

class DrawingArea : public QWidget
{
public:
    DrawingArea(QWidget *parent = nullptr) : QWidget(parent)
    {
        setFixedSize(200, 50);
    }

protected:
    void paintEvent(QPaintEvent *event) override
    {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);
        
        // outlines
        QPen pen(Qt::black);
        pen.setWidth(2);
        painter.setPen(pen);
        // fill
        painter.setBrush(Qt::black);
        
        int centerY = height() / 2;
        int squareSize = 8;
        int lineLength = width() - 2 * squareSize - 20;
        int startX = 10 + squareSize;
        
        // Draw left square
        painter.drawRect(10, centerY - squareSize/2, squareSize, squareSize);
        
        // Draw horizontal line
        painter.drawLine(startX, centerY, startX + lineLength, centerY);
        
        // Draw right square
        painter.drawRect(startX + lineLength, centerY - squareSize/2, squareSize, squareSize);
    }
};


// you usually want QWidget
// QFrame just gets you a couple more methods for drawing outline around the whole thing
class ClickCounter : public QFrame
{
    Q_OBJECT

public:
    ClickCounter(QFrame *parent = nullptr) : QFrame(parent), clickCount(0)
    {
        // Create the button
        QVBoxLayout *layout = new QVBoxLayout(this);

        drawingArea = new DrawingArea(this);
        button = new QPushButton(this); this->updateButtonText();
        label = new QLabel("some text here", this); label->setFrameStyle(QFrame::Box);

        layout->addWidget(button);
        layout->addWidget(label, 0, Qt::AlignCenter);
        layout->setAlignment(Qt::AlignCenter);

        // this->setFrameShape(QFrame::Panel);
        // this->setFrameShape(QFrame::HLine);
        this->setFrameShadow(Shadow::Plain);
        // frame->setWidget(this);

        // the fact that these two are methods on qwidget and not global makes me sad
        this->setWindowTitle("Click Counter");
        this->setFixedSize(300, 150);

        this->connect(button, &QPushButton::clicked, this, &ClickCounter::onButtonClicked);     
    }

// protected:
    // void paintEvent(QPaintEvent *event) override {
        // QPainter painter(this);
        // painter.setPen();
    // }

protected:
    void wheelEvent(QWheelEvent *event) override
    {
        // Check if mouse is over the button
        if (button->underMouse()) {
            QPoint angleDelta = event->angleDelta();
            if (!angleDelta.isNull()) {
                // Scroll up = increment, scroll down = decrement
                if (angleDelta.y() > 0) {
                    clickCount++;
                } else if (angleDelta.y() < 0 && clickCount > 0) {
                    clickCount--;
                }
                updateButtonText();
            }
            event->accept();
        } else {
            event->ignore();
        }
    }

private slots:
    void onButtonClicked()
    {
        clickCount++;
        updateButtonText();
    }
    
    void updateButtonText()
    {
        button->setText(QString("Clicked %1 times").arg(clickCount));
    }

private:
    DrawingArea *drawingArea;
    QPushButton *button;
    QLabel *label;
    int clickCount;
};


int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    ClickCounter window;
    window.show();

    return app.exec();
}

// required when using Q_OBJECT
#include "claude.moc"
