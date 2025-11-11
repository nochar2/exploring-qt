#ifndef EXPONENTIALSPINBOX_H
#define EXPONENTIALSPINBOX_H

#include <QDoubleSpinBox>
#include <QWheelEvent>
#include <QKeyEvent>
#include <cmath>

class ExponentialSpinBox : public QDoubleSpinBox
{
public:
    explicit ExponentialSpinBox(QWidget *parent = nullptr)
        : QDoubleSpinBox(parent)
        , m_factor(1.05)
        , m_minValue(100.0)
        , m_maxValue(1000.0)
    {
        // Set the range and initial value
        setRange(m_minValue, m_maxValue);
        setValue(m_minValue);
        
        // Set reasonable precision
        setDecimals(2);
        
        // Disable the built-in stepping - we'll handle it ourselves
        setSingleStep(0); // This disables the default linear stepping
    }
    
    // Set the exponential factor (e.g., 1.05 for 5% increase per step)
    void setExponentialFactor(double factor) {
        // -- stupid Claude, what is this "let's silently suppress odd but perfectly valid values"?
        // if (factor > 1.0) {
        if (factor >= 0) {
            m_factor = factor;
        } else {
            assert(false && "we don't support complex numbers in a DoubleSpinBox, sorry");
        }
    }
    
    double exponentialFactor() const {
        return m_factor;
    }
    
    // Set custom min/max bounds
    void setExponentialRange(double min, double max) {
        if (min < max && min > 0) {
            m_minValue = min;
            m_maxValue = max;
            setRange(min, max);
        }
    }

protected:
    // Override stepBy to implement exponential stepping
    void stepBy(int steps) override {
        double currentVal = value();
        double newVal = currentVal;
        
        if (steps > 0) {
            // Step up exponentially
            for (int i = 0; i < steps; ++i) {
                newVal *= m_factor;
                if (newVal > m_maxValue) {
                    newVal = m_maxValue;
                    break;
                }
            }
        } else if (steps < 0) {
            // Step down exponentially  
            for (int i = 0; i < -steps; ++i) {
                newVal /= m_factor;
                if (newVal < m_minValue) {
                    newVal = m_minValue;
                    break;
                }
            }
        }
        
        setValue(newVal);
    }
    
    // Handle wheel events for exponential scrolling
    void wheelEvent(QWheelEvent *event) override {
        if (!isEnabled()) {
            return;
        }
        
        // Get the wheel delta and convert to steps
        int numDegrees = event->angleDelta().y() / 8;
        int numSteps = numDegrees / 15; // Standard wheel step
        
        if (numSteps != 0) {
            stepBy(numSteps);
            event->accept();
        } else {
            event->ignore();
        }
    }
    
    // Handle key events
    void keyPressEvent(QKeyEvent *event) override {
        switch (event->key()) {
        case Qt::Key_Up:
            stepBy(1);
            event->accept();
            break;
        case Qt::Key_Down:
            stepBy(-1);
            event->accept();
            break;
        case Qt::Key_PageUp:
            stepBy(5); // Bigger jumps with Page Up/Down
            event->accept();
            break;
        case Qt::Key_PageDown:
            stepBy(-5);
            event->accept();
            break;
        default:
            // Let the base class handle other keys (editing, etc.)
            QDoubleSpinBox::keyPressEvent(event);
            break;
        }
    }

private:
    double m_factor;
    double m_minValue;
    double m_maxValue;
};
#endif
