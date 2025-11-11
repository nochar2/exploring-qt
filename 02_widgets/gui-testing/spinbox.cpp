#include <QApplication>
#include <QLabel>
#include <QBoxLayout>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    QWidget window;
    QVBoxLayout *layout = new QVBoxLayout(&window);
    
    QLabel *label = new QLabel("Exponential SpinBox (factor 1.05, range 100-1000):");
    ExponentialSpinBox *spinBox = new ExponentialSpinBox();
    
    // You can customize the settings:
    spinBox->setExponentialFactor(1.05);
    spinBox->setExponentialRange(100.0, 1000.0);
    
    layout->addWidget(label);
    layout->addWidget(spinBox);
    
    // Connect to see the values change
    QObject::connect(spinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
                     [](double value) {
        qDebug() << "Value changed to:" << value;
    });
    
    window.show();
    return app.exec();
}
