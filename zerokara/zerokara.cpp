#include <QApplication>
#include <QTreeWidget>
#include <QHBoxLayout>
#include <QWindow>
#include <QFrame>
#include <QRadioButton>
#include <QLabel>
#include <QTextEdit>
#include <QPalette>
#include <QColor>
#include <QPainter>
#include <QPaintEvent>
#include <QRect>
#include <QPaintEvent>
#include <QPalette>
#include <QCheckBox>

class NoteDisplayWidget : public QWidget {
  // upscroll is natural to think about in normal coordinates
  // (screen y grows downwards)

  // an issue is that this has to be in sync with the button which
  // toggles it :( let's start with false for now
  bool downscroll = false;

  public:
  void onDownscrollClick() {
    this->downscroll = !this->downscroll;
    this->update();
  }
  
  protected:
  void paintEvent(QPaintEvent *event) override {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
  
    // this is just outline
    // QPen pen(Qt::red);
    // painter.setPen(pen);

    QBrush brush(Qt::red);
    painter.setBrush(brush);

    QRectF rect(10, 10, 100, 20);

    auto cont_rect = this->contentsRect();
    // printf("%d %d\n", cont_rect.width(), cont_rect.height());

    if (downscroll) {
      // printf("cont_rect h: %d, rect_y: %g, rect_h: %g\n",
      //        cont_rect.height(), rect.y(), rect.height());
      rect.moveBottom(cont_rect.height() - rect.top());
      // printf("top y is now %g\n", rect.y());
    }
    // printf("rect xywh is now: %g %g %g %g\n",
           // rect.x(), rect.y(), rect.width(), rect.height());

    painter.fillRect(rect, brush);

  }
};


int main(int argc, char **argv) {
  QApplication app(argc, argv);
  // -- this alone works
  // QFrame frame; frame.show();
  // -- even this alone works
  // QRadioButton button; button.show();
  // -- this shows running thing on taskbar but there is no window
  // QWindow win; win.show();

  QWidget root;

  QTreeWidget tree;
  tree.setColumnCount(1);
  tree.setHeaderHidden(true);
  QTreeWidgetItem item(&tree); item.setText(0, "Parent");
  QTreeWidgetItem child(&item); child.setText(0, "Child");
  // -- must be already in the QTreeWidget,
  // -- https://doc.qt.io/qt-6/qtreewidgetitem.html#setExpanded
  item.setExpanded(true);

  QHBoxLayout layout(&root);
  layout.addWidget(&tree, 1);

  // -- QLabel label; label.setText("Hi world"); label.setAlignment(Qt::AlignCenter);
  // -- layout.addWidget(&label, 3);
  QTextEdit viewer;
  viewer.setReadOnly(true);
  viewer.setText("A bunch of random text\nthat spans\na couple lines");
  layout.addWidget(&viewer, 2);


  QVBoxLayout preview_tile;
  NoteDisplayWidget preview_actual;
  preview_actual.setAutoFillBackground(true);
  auto pal = preview_actual.palette();
  pal.setColor(QPalette::Window, Qt::black);
  preview_actual.setPalette(pal);
  preview_tile.addWidget(&preview_actual, 6);

  QCheckBox downscroll_cbox("Downscroll");
  preview_tile.addWidget(&downscroll_cbox, 1);

  layout.addLayout(&preview_tile, 2);

  QObject::connect(
    &downscroll_cbox,
    &QCheckBox::checkStateChanged,
    &preview_actual,
    &NoteDisplayWidget::onDownscrollClick
  );

  root.show();

  // -- they are all like 40 bytes
  // printf("root takes %zu bytes\n", sizeof root);
  // printf("tree takes %zu bytes\n", sizeof tree);
  // printf("label takes %zu bytes\n", sizeof label);
  // printf("layout takes %zu bytes\n", sizeof layout);
  return app.exec();
}
