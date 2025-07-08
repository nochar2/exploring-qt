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

// event
#include <QScrollEvent>

// painter stuff
#include <QPainter>
#include <QPalette>
#include <QRect>

// #include <chrono>




class NoteDisplayWidget : public QWidget {
  // upscroll is natural to think about in normal coordinates
  // (screen y grows downwards)

  bool downscroll = false;
  int cmod = 400;
  int px_chart_start_off = 30;

  public:
  void onDownscrollClick(Qt::CheckState ds_state) { this->downscroll = ds_state == Qt::Checked; this->update(); }
  void onCmodChange(int value) { this->cmod = value; this->update(); }
  
  struct RectSpec {
    QList<QRectF> rects; // actually 1 .. 4 of them
    QColor color;
  };

  protected:
  void wheelEvent(QWheelEvent *event) override {
    if (event->angleDelta().ry() > 0) {
      // printf("scrollev, delta > 0\n");
      px_chart_start_off += 30;
    } else {
      // printf("scrollev, delta < 0\n");
      px_chart_start_off -= 30;
    }
    this->update();
  };

  void paintEvent(QPaintEvent *event) override {
    // auto tb = std::chrono::system_clock().now();

    (void) event;
    
    int note_width  = 60;
    int note_height = 20;
    int px_judge_line_off = 30;
    int left_start = this->width() / 2 - 2 * note_width; // for centering

    // who cares, everyone uses 4/4
    auto ticks_per_1_ = [](int subdiv){return 192./subdiv;};

    // 16th notes for an example. Let's assume BPM = 180, and let's assume CMOD
    // means pixels per second (which is not entirely true, it is more like assuming h == 480)
    float bpm = 180.;
    float secs_per_beat = 60./bpm;
    float secs_per_smtick = secs_per_beat / 48.; // smallest subdivision in stepmania games
    float px_per_smtick = secs_per_smtick * cmod;


    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
  
    // draw judge line
    auto judge_line = QLineF(0, px_judge_line_off, this->width()-1, px_judge_line_off);
    if (downscroll) {
      int new_y = this->height() - judge_line.y1();
      judge_line.setLine(0, new_y, this->width()-1, new_y);
    }
    QPen pen;
    pen.setWidth(5); pen.setColor(Qt::white); painter.setPen(pen);
    painter.drawLine(judge_line);

    // draw corner beams around the left and right edge of playfield
    auto left_edge  = QLineF(left_start,              0, left_start,               this->height()-1);
    auto right_edge = QLineF(left_start+note_width*4, 0, left_start+note_width*4,  this->height()-1);
    pen.setWidth(2); pen.setColor(Qt::gray); painter.setPen(pen);
    painter.drawLine(left_edge);
    painter.drawLine(right_edge);

    // draw snap lines (for now, do 4ths)
    for (int i = 0; i < 10; i++) {
      int y = px_chart_start_off + i * (px_per_smtick * ticks_per_1_(4));
      if (downscroll) y = this->height() - y;
      auto snap_line = QLineF(left_start, y, left_start + 4 * note_width, y);
      pen.setWidth(1); pen.setColor(Qt::red); painter.setPen(pen);
      painter.drawLine(snap_line);
    }


    auto rectangles_at_smtick_pos = [=](int n_beats, int n_smticks, int column_mask){
      n_smticks += n_beats * 48;

      assert(0 <= column_mask && column_mask <= 15);
      QList<QPair<int,QColor>> snap_pairs = {
        QPair((192/4),  QColorConstants::Red),
        QPair((192/8),  QColorConstants::Blue),
        QPair((192/12), QColorConstants::Green),
        QPair((192/16), QColorConstants::Yellow),
        QPair((192/24), QColorConstants::DarkMagenta),
        QPair((192/32), QColorConstants::Svg::orange),
        QPair((192/48), QColorConstants::Cyan),
      };

      auto line = (RectSpec){
        .rects = {},
        .color = QColorConstants::Gray
      };

      for (int i = 8, column_i = 0; i > 0; i /= 2, column_i += 1) {
        if (column_mask & i) {
          line.rects.push_back(
            QRectF(left_start + note_width * column_i, px_chart_start_off + px_per_smtick * n_smticks, note_width, note_height)
          );
        }
      }

      for (auto [snap, col] : snap_pairs) {
        if (n_smticks % snap == 0) { line.color = col; return line; }
      }
      return line;
    };


    // whatever name for 1..4 notes at one place
    auto patterns = {
      rectangles_at_smtick_pos(0, (int)(ticks_per_1_(16))*0,  0b1000),
      rectangles_at_smtick_pos(0, (int)(ticks_per_1_(16))*1,  0b0100),
      rectangles_at_smtick_pos(0, (int)(ticks_per_1_(16))*2,  0b0010),
      rectangles_at_smtick_pos(0, (int)(ticks_per_1_(16))*3,  0b0001),

      rectangles_at_smtick_pos(1, (int)(ticks_per_1_(24))*0,  0b1000),
      rectangles_at_smtick_pos(1, (int)(ticks_per_1_(24))*1,  0b0100),
      rectangles_at_smtick_pos(1, (int)(ticks_per_1_(24))*2,  0b0010),
      rectangles_at_smtick_pos(1, (int)(ticks_per_1_(24))*3,  0b0100),
      rectangles_at_smtick_pos(1, (int)(ticks_per_1_(24))*4,  0b0010),
      rectangles_at_smtick_pos(1, (int)(ticks_per_1_(24))*5,  0b0001),

      rectangles_at_smtick_pos(2, (int)(ticks_per_1_(32))*0,  0b1100),
      rectangles_at_smtick_pos(2, (int)(ticks_per_1_(32))*1,  0b0001),
      rectangles_at_smtick_pos(2, (int)(ticks_per_1_(32))*2,  0b0010),
      rectangles_at_smtick_pos(2, (int)(ticks_per_1_(32))*3,  0b0100),
      rectangles_at_smtick_pos(2, (int)(ticks_per_1_(32))*4,  0b1000),
      rectangles_at_smtick_pos(2, (int)(ticks_per_1_(32))*5,  0b0001),
      rectangles_at_smtick_pos(2, (int)(ticks_per_1_(32))*6,  0b0010),
      rectangles_at_smtick_pos(2, (int)(ticks_per_1_(32))*7,  0b1000),

      rectangles_at_smtick_pos(3, (int)(ticks_per_1_(32))*0,  0b0110),
    };

    QRectF cont_rect = this->contentsRect();

    for (auto pat : patterns) {
      for (auto rect : pat.rects) {
        if (downscroll) {
          // printf("cont_rect h: %d, rect_y: %g, rect_h: %g\n",
          //        cont_rect.height(), rect.y(), rect.height());
          rect.moveBottom(cont_rect.height() - rect.top());
          // printf("top y is now %g\n", rect.y());
        }
        painter.fillRect(rect, pat.color);
      }
    }


    // auto te = std::chrono::system_clock().now();
    // auto tdiff = std::chrono::duration_cast<std::chrono::microseconds>(te-tb).count();
    // printf("took %ld microseconds", tdiff);
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
    preview_tile.addWidget(&preview_actual, 8);

    QLabel status_bar("Beat x, Sm-tick x  |  Snap x");
    preview_tile.addWidget(&status_bar, 0);
    preview_tile.setAlignment(&status_bar, Qt::AlignCenter);
    
    QHBoxLayout preview_controls;

      QCheckBox downscroll_chk("Downscroll");
        downscroll_chk.setCheckState(Qt::Checked);
        QObject::connect(
          &downscroll_chk,
          &QCheckBox::checkStateChanged,
          &preview_actual,
          &NoteDisplayWidget::onDownscrollClick
        );
        preview_actual.onDownscrollClick(downscroll_chk.checkState());  // set initial
      preview_controls.addWidget(&downscroll_chk, 6);
      preview_controls.setAlignment(&downscroll_chk, Qt::AlignCenter);

      QLabel cmod_value;
      preview_controls.addWidget(&cmod_value, 1);
      preview_controls.setAlignment(&cmod_value, Qt::AlignCenter);      

      // -- TODO: When I ctrl + scrollwheel over the preview area, do the following:
      // * maybe just printf "hit!"
      // * redraw snap lines
      // * update snap value text (snap: 4th, 8th etc.)
      // * (bonus: on ctrl + shift + scrollwheel, do fine snap increments)
      // QLabel snap_text;
      // preview_controls.addWidget(&snap_text, 1);

      QSlider cmod_slider(Qt::Vertical, nullptr); // doesn't look super aesthetic, but it's ok for now
        cmod_slider.setMinimum(300);
        cmod_slider.setMaximum(1000);
        cmod_slider.setValue(700);
        cmod_slider.setPageStep(200);
        cmod_slider.setSingleStep(15);
        QObject::connect(
          &cmod_slider,
          &QSlider::valueChanged,
          &preview_actual,
          &NoteDisplayWidget::onCmodChange
        );
        QObject::connect(
          &cmod_slider,
          &QSlider::valueChanged,
          &cmod_value,
          [&](int value){ cmod_value.setText(QString("CMOD: \n%1").arg(value)); }
        );
          
        cmod_value.setText(QString("CMOD: \n%1").arg(cmod_slider.value())); // set initial
        preview_actual.onCmodChange(cmod_slider.value());
      preview_controls.addWidget(&cmod_slider, 1);
      preview_controls.setAlignment(&cmod_slider, Qt::AlignCenter);
    preview_tile.addLayout(&preview_controls, 1);
  layout.addLayout(&preview_tile, 4);






  root.show();

  // -- they are all like 40 bytes
  // printf("root takes %zu bytes\n", sizeof root);
  // printf("tree takes %zu bytes\n", sizeof tree);
  // printf("label takes %zu bytes\n", sizeof label);
  // printf("layout takes %zu bytes\n", sizeof layout);
  return app.exec();
}
