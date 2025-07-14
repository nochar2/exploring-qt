#include <QApplication>
#include <QHBoxLayout>
#include <QWidget>
#include <QLabel>
#include <QPainter>
#include <QLinearGradient>
#include <QSpinBox>

struct MyPaintWidget : public QWidget {
  int gradient_length = 100;
  const int max_gradient_length = 500;

  void paintEvent(QPaintEvent *event) override {
    (void)event;
    QPainter p(this);
    // we know you'd like to use designated initializers,
    // but we hate you, here's a constructor with unlabeled
    // parameters
    QRect solid_rect(10, 10, 100, 20);
    p.fillRect(solid_rect, Qt::red);


    if (gradient_length > 0) /* otherwise, it would fill the whole rect red */ {
      QRect gradient_rect;
      gradient_rect.setLeft   (solid_rect.left());
      gradient_rect.setTop    (solid_rect.bottom());
      gradient_rect.setRight  (solid_rect.right());
      gradient_rect.setBottom (solid_rect.bottom() + max_gradient_length);

      QLinearGradient grad;
      grad.setStart     (gradient_rect.topLeft());
      grad.setFinalStop (gradient_rect.left(), gradient_rect.top()+gradient_length);
      grad.setColorAt(0, Qt::red);
      grad.setColorAt(1, Qt::transparent);

      p.fillRect(gradient_rect, grad);
    }
  }
};

int main(int argc, char **argv) {
  QApplication app(argc, argv);

  QWidget root;
  QHBoxLayout cont(&root);

  QSpinBox spinbox;
  spinbox.setMinimum(0);
  spinbox.setMaximum(500);
  spinbox.setSingleStep(10);
  cont.addWidget(&spinbox);

  QLabel w_text;
  w_text.setText("asdfjlkasdkljfasdjk");
  w_text.setTextInteractionFlags(Qt::TextInteractionFlag::TextSelectableByMouse);
  w_text.setAlignment(Qt::AlignCenter);
  cont.addWidget(&w_text);

  MyPaintWidget drawarea;
  {
    auto pal = drawarea.palette();
    pal.setColor(QPalette::Window, Qt::black);
    drawarea.setPalette(pal);
  }
  cont.addWidget(&drawarea);

  QWidget::connect(
              &spinbox,
              &QSpinBox::valueChanged,
              &drawarea,
              [&](int value){
                drawarea.gradient_length = value;
                drawarea.update();
              }
  );


  root.show();

  return app.exec();
}
