#include <QApplication>

// widgets
#include <QTreeWidget>
#include <QCheckBox>
#include <QSlider>
#include <QBoxLayout>
#include <QWindow>
#include <QFrame>
#include <QRadioButton>
#include <QLabel>
#include <QTextEdit>
#include <QSlider>

// painter stuff
#include <QPainter>
#include <QPalette>
#include <QRect>


class NoteDisplayWidget : public QWidget {
  // upscroll is natural to think about in normal coordinates
  // (screen y grows downwards)

  bool downscroll = false;
  int cmod = 400;

  public:
  void onDownscrollClick(Qt::CheckState ds_state) { this->downscroll = ds_state == Qt::Checked; this->update(); }
  void onCmodChange(int value) { this->cmod = value; this->update(); }
  
  struct RectSpec {
    QRectF rect;
    QColor color;
  };

  protected:
  void paintEvent(QPaintEvent *event) override {
    (void) event;
    
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
  
    // draw a judge line
    int px_judge_line_off = 20;
    auto judge_line = QLineF(0, px_judge_line_off, this->width()-1, px_judge_line_off);

    if (downscroll) {
      int new_y = this->height() - judge_line.y1();
      judge_line.setLine(0, new_y, this->width()-1, new_y);
    }
    painter.setPen(Qt::white);
    painter.drawLine(judge_line);


    
    // this is just outline
    // QPen pen(Qt::red);
    // painter.setPen(pen);

    QBrush brush(Qt::red);
    painter.setBrush(brush);

    // who cares, everyone uses 4/4
    auto ticks_per_1_ = [](int subdiv){return 192./subdiv;};

    // A bunch of 16th notes for an example. Let's assume BPM = 180, and let's assume CMOD
    // means pixels per second.
    float bpm = 180.;
    float secs_per_beat = 60./bpm;
    float secs_per_smtick = secs_per_beat / 48.; // smallest subdivision in stepmania games
    float px_per_smtick = secs_per_smtick * cmod;

    int rwidth = 60;
    auto rss = {
      (RectSpec){ QRectF(10+rwidth*0, px_judge_line_off + px_per_smtick * (int)(ticks_per_1_(16))*0, rwidth, 20), Qt::red },
      (RectSpec){ QRectF(10+rwidth*1, px_judge_line_off + px_per_smtick * (int)(ticks_per_1_(16))*1, rwidth, 20), Qt::yellow },
      (RectSpec){ QRectF(10+rwidth*2, px_judge_line_off + px_per_smtick * (int)(ticks_per_1_(16))*2, rwidth, 20), Qt::blue },
      (RectSpec){ QRectF(10+rwidth*3, px_judge_line_off + px_per_smtick * (int)(ticks_per_1_(16))*3, rwidth, 20), Qt::yellow },

      // all of this will have to be generalized away into a lambda
      (RectSpec){ QRectF(10+rwidth*0, px_judge_line_off + px_per_smtick * (int)(ticks_per_1_(24))*6,  rwidth, 20), Qt::red },
      (RectSpec){ QRectF(10+rwidth*1, px_judge_line_off + px_per_smtick * (int)(ticks_per_1_(24))*7,  rwidth, 20), Qt::magenta },
      (RectSpec){ QRectF(10+rwidth*2, px_judge_line_off + px_per_smtick * (int)(ticks_per_1_(24))*8,  rwidth, 20), Qt::green },
      (RectSpec){ QRectF(10+rwidth*3, px_judge_line_off + px_per_smtick * (int)(ticks_per_1_(24))*9,  rwidth, 20), Qt::blue },
      (RectSpec){ QRectF(10+rwidth*2, px_judge_line_off + px_per_smtick * (int)(ticks_per_1_(24))*10, rwidth, 20), Qt::green },
      (RectSpec){ QRectF(10+rwidth*1, px_judge_line_off + px_per_smtick * (int)(ticks_per_1_(24))*11, rwidth, 20), Qt::magenta },
    };

    QRectF cont_rect = this->contentsRect();

    for (auto rs : rss) {
      if (downscroll) {
        // printf("cont_rect h: %d, rect_y: %g, rect_h: %g\n",
        //        cont_rect.height(), rect.y(), rect.height());
        rs.rect.moveBottom(cont_rect.height() - rs.rect.top());
        // printf("top y is now %g\n", rect.y());
      }
      painter.fillRect(rs.rect, rs.color);
    }

    // printf("%d %d\n", cont_rect.width(), cont_rect.height());

    // printf("rect xywh is now: %g %g %g %g\n",
           // rect.x(), rect.y(), rect.width(), rect.height());


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


  // You can't scope these, they need to live. Also, if you make them static, it aborts on exit
  // leaving just the indentation for now
  QVBoxLayout preview_tile;
    NoteDisplayWidget preview_actual;
      preview_actual.setAutoFillBackground(true);
      auto pal = preview_actual.palette();
      pal.setColor(QPalette::Window, Qt::black);
      preview_actual.setPalette(pal);
    preview_tile.addWidget(&preview_actual, 6);

    QHBoxLayout preview_controls;
      QCheckBox downscroll_chk("Downscroll");
        downscroll_chk.setCheckState(Qt::Checked);
        preview_controls.addWidget(&downscroll_chk);
        preview_controls.addWidget(&downscroll_chk);
        QObject::connect(
          &downscroll_chk,
          &QCheckBox::checkStateChanged,
          &preview_actual,
          &NoteDisplayWidget::onDownscrollClick
        );
        preview_actual.onDownscrollClick(downscroll_chk.checkState());  // set initial
      preview_controls.addWidget(&downscroll_chk);

      QSlider cmod_slider(Qt::Horizontal, nullptr); // this is 100 % wrong
        cmod_slider.setMinimum(100);
        cmod_slider.setMaximum(1000);
        cmod_slider.setPageStep(200);
        cmod_slider.setSingleStep(20);
        QObject::connect(
          &cmod_slider,
          &QSlider::valueChanged,
          &preview_actual,
          &NoteDisplayWidget::onCmodChange
        );
        preview_actual.onCmodChange(cmod_slider.value()); // set initial
      preview_controls.addWidget(&cmod_slider);
    preview_tile.addLayout(&preview_controls, 1);
  layout.addLayout(&preview_tile, 2);






  root.show();

  // -- they are all like 40 bytes
  // printf("root takes %zu bytes\n", sizeof root);
  // printf("tree takes %zu bytes\n", sizeof tree);
  // printf("label takes %zu bytes\n", sizeof label);
  // printf("layout takes %zu bytes\n", sizeof layout);
  return app.exec();
}
