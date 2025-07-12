#include "sm_parser.cpp"

// C++
#include "precompiled.h"

SmFile smfile;

QColor qcolor_from_smticks(uint32_t smticks) {
  assert(smticks < 48);

  std::vector<std::pair<uint32_t,QColor>> snap_to_color = {
    { (192/4),  QColorConstants::Red },
    { (192/8),  QColorConstants::Blue },
    { (192/12), QColorConstants::Green },
    { (192/16), QColorConstants::Yellow },
    { (192/24), QColorConstants::DarkMagenta },
    { (192/32), QColorConstants::Svg::orange },
    { (192/48), QColorConstants::Cyan },
  };
  for (auto [s,c] : snap_to_color) {
    if (smticks % s == 0) return c;
  }
  return QColorConstants::Gray;
}

std::array<NoteType, 4> notes_from_string(const char str[4]) {
  std::array<NoteType, 4> ret = {};
  ret[0] = static_cast<NoteType>(str[0]);
  ret[1] = static_cast<NoteType>(str[1]);
  ret[2] = static_cast<NoteType>(str[2]);
  ret[3] = static_cast<NoteType>(str[3]);
  return ret;
}

struct NoteRowRects {
  std::vector<QRectF> rects; // actually 1 .. 4 of them
  QColor color;
};


// NoteRowRects noterowrects_from_noterow(NoteRow note_row) {
//   NoteRowRects ret = {};
//   ret.rects.push_back(note_row.line[0]);
// };

// who cares, everyone uses 4/4
double ticks_per_1_(int subdiv){return 192./subdiv;};


class NoteDisplayWidget : public QWidget {
  // upscroll is natural to think about in normal coordinates
  // (screen y grows downwards)

  bool downscroll = false;
  int cmod = 400;
  int px_chart_start_off = 30;

  public:
  void onDownscrollClick(Qt::CheckState ds_state) { this->downscroll = ds_state == Qt::Checked; this->update(); }
  void onCmodChange(int value) { this->cmod = value; this->update(); }
  

  protected:
  void wheelEvent(QWheelEvent *event) override {
    // -- TODO: move by snap, not by 30 pixels.
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
    (void) event;
    
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);


    int32_t note_width  = 60;
    int32_t note_height = 20;
    int32_t px_judge_line_off = 30;
    int32_t left_start = this->width() / 2 - (int32_t)(2 * note_width); // for centering

    // -- TODO: again we assume one BPM only for now
    // -- TODO: also this is obsolete and should be seconds-based
    double bpm = smfile.bpms[0].value;
    double secs_per_beat = 60./bpm;
    double secs_per_smtick = secs_per_beat / 48.; // smallest subdivision in stepmania games
    double px_per_smtick = secs_per_smtick * cmod;


  
    // draw judge line
    auto judge_line = QLine(0, px_judge_line_off, this->width()-1, px_judge_line_off);
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
      int y = (int)(px_chart_start_off + i * (px_per_smtick * ticks_per_1_(4)));
      if (downscroll) y = this->height() - y;
      auto snap_line = QLineF(left_start, y, left_start + 4 * note_width, y);
      pen.setWidth(1); pen.setColor(Qt::red); painter.setPen(pen);
      painter.drawLine(snap_line);
    }


    std::function<NoteRowRects(NoteRow)>
    rectangles_at_smtick_pos = [&](NoteRow row)
    {
      /*, int32_t left_start, int32_t note_width, int32_t note_height, double px_per_smtick, double px_chart_start_off */
      uint32_t global_smticks = row.smticks + row.beat * 48 + row.measure * 48 * 4;

      auto line = (NoteRowRects){
        .rects = {},
        .color = qcolor_from_smticks(row.smticks)
      };

      for (size_t i = 0; i < 4; i++) {
        if (row.line[i] == NoteType::Tap) {
          line.rects.push_back(
            QRect((int32_t)(left_start + note_width * (int32_t)i),
                  (int32_t)(px_chart_start_off + px_per_smtick * global_smticks),
                  note_width, note_height)
          );
        }
      }
      return line;
    };


    // whatever name for 1..4 notes at one place
    std::vector<NoteRow>& note_rows = smfile.diffs[0].note_rows;
    // std::vector<NoteRow> note_rows = {
    //   (NoteRow){.measure=0, .beat=0, .smticks=(uint8_t)(ticks_per_1_(16)*0), .sec=0., .line=notes_from_string("1111")},
    //   (NoteRow){.measure=0, .beat=0, .smticks=(uint8_t)(ticks_per_1_(16)*1), .sec=0., .line=notes_from_string("0100")},
    //   (NoteRow){.measure=0, .beat=0, .smticks=(uint8_t)(ticks_per_1_(16)*2), .sec=0., .line=notes_from_string("0010")},
    //   (NoteRow){.measure=0, .beat=0, .smticks=(uint8_t)(ticks_per_1_(16)*3), .sec=0., .line=notes_from_string("0001")},
    // };

    std::vector<NoteRowRects> rectangles;
    std::transform(note_rows.begin(), note_rows.end(), std::back_inserter(rectangles), [&](NoteRow nr) {return rectangles_at_smtick_pos(nr);});

    // std::vector<NoteRowRects> rectangles = {
    //   rectangles_at_smtick_pos(0, (int)(ticks_per_1_(16))*0,  0b1111),
    //   rectangles_at_smtick_pos(0, (int)(ticks_per_1_(16))*1,  0b0100),
    //   rectangles_at_smtick_pos(0, (int)(ticks_per_1_(16))*2,  0b0010),
    //   rectangles_at_smtick_pos(0, (int)(ticks_per_1_(16))*3,  0b0001),
    // };

    QRectF cont_rect = this->contentsRect();

    for (auto pat : rectangles) {
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
  }
  
};


int main(int argc, char **argv) {
  // -- make float parsing not break in Czech locale
  // -- XXX: why does this not work?
  // QLocale locale("C");
  // QLocale::setDefault(locale);


  // -- QGuiApplication doesn't work if you want widgets
  QApplication app(argc, argv);

  // -- this alone works
  // QFrame frame; frame.show();
  // -- even this alone works
  // QRadioButton button; button.show();
  // -- this shows running thing on taskbar but there is no window
  // QWindow win; win.show();


  // parse the smfile
  
  std::ifstream file("ext/Shannon's Theorem.sm");
  std::ostringstream ss;
  ss << file.rdbuf();
  auto smfile_opt = smfile_from_string_opt(ss.str());

  if (!std::holds_alternative<SmFile>(smfile_opt)) {
    auto err = std::get<SmParseError>(smfile_opt);
    printf("%s\n", err.msg.c_str());
    assert(false);
  }

  smfile = std::get<SmFile>(smfile_opt);


  QWidget w_root;

  QTreeWidget tree;
  tree.setColumnCount(2);
  tree.setHeaderHidden(true);

  QTreeWidgetItem t_root(&tree); {
    t_root.setText(0, "SmFile");
  }
  QTreeWidgetItem t_meta(&t_root); {
    t_meta.setText(0, "Metadata");
  }
  QTreeWidgetItem t_title(&t_meta); {
    t_title.setText(0, "#TITLE"); t_title.setText(1, QString(smfile.title.c_str()));
  }
  QTreeWidgetItem t_music(&t_meta); {
    t_music.setText(0, "#MUSIC"); t_music.setText(1, QString(smfile.music.c_str()));
  }
  QTreeWidgetItem t_artist(&t_meta); {
    t_artist.setText(0, "#ARTIST"); t_artist.setText(1, QString(smfile.artist.c_str()));
  }
  QTreeWidgetItem t_offset(&t_meta); {
    t_offset.setText(0, "#OFFSET"); t_offset.setText(1, QString::number(smfile.offset));
  }
  QTreeWidgetItem t_samplestart(&t_meta); {
    t_samplestart.setText(0, "#SAMPLESTART"); t_samplestart.setText(1, QString::number(smfile.samplestart));
  }
  QTreeWidgetItem t_samplelength(&t_meta); {
    t_samplelength.setText(0, "#SAMPLELENGTH"); t_samplelength.setText(1, QString::number(smfile.samplelength));
  }

  QList<QTreeWidgetItem *> t_bpm_list;
  QTreeWidgetItem t_bpms(&t_meta); {
    t_bpms.setText(0, "#BPMS");
    for (auto time_bpm : smfile.bpms) {
      auto *t_bpm = new QTreeWidgetItem();
      t_bpm->setText(0, QString::number(time_bpm.beat_number));
      t_bpm->setText(1, QString::number(time_bpm.value)); // 'g' (default) or 'f', '3'
      t_bpm_list.push_back(t_bpm);
    }
    t_bpms.addChildren(t_bpm_list);
  }

  QList<QTreeWidgetItem *> t_stop_list;
  QTreeWidgetItem t_stops(&t_meta); {
    t_stops.setText(0, "#STOPS");
    for (auto time_stop : smfile.stops) {
      auto *t_stop = new QTreeWidgetItem();
      t_stop->setText(0, QString::number(time_stop.beat_number));
      t_stop->setText(1, QString::number(time_stop.value)); // 'g' (default) or 'f', '3'
      t_stop_list.push_back(t_stop);
    }
    t_bpms.addChildren(t_stop_list);
  }
  

  // tree.resizeColumnToContents(1);

  QTreeWidgetItem t_diffs(&t_root);
  t_diffs.setText(0, "Difficulties");

  size_t i = 0;
  for (auto diff : smfile.diffs) {
    auto *t_diff = new QTreeWidgetItem(&t_diffs);
    t_diff->setText(0, QString::number(i));

    auto *t_gametype = new QTreeWidgetItem(t_diff);
    t_gametype->setText(0, "Game type");
    t_gametype->setText(1, cstr_from_gametype(diff.game_type));

    auto *t_charter = new QTreeWidgetItem(t_diff);
    t_charter->setText(0, "Charter");
    t_charter->setText(1, diff.charter.c_str());

    auto *t_difftype = new QTreeWidgetItem(t_diff);
    t_difftype->setText(0, "Diff type");
    t_difftype->setText(1, cstr_from_difftype(diff.diff_type));
    QBrush brush(qcolor_from_difftype(diff.diff_type));
    // QFont font; font.setBold(true); t_difftype->setFont(1, font);
    t_difftype->setForeground(1, brush);

    auto *t_diffval = new QTreeWidgetItem(t_diff);
    t_diffval->setText(0, "Diff value");
    t_diffval->setText(1, QString::number(diff.diff_num));

    auto *t_notes = new QTreeWidgetItem(t_diff);
    t_notes->setText(0, "Note rows");

    for (auto note : diff.note_rows) {
      auto *t_note = new QTreeWidgetItem(t_notes);
      t_note->setText(0, QString("%1/%2/%3").arg(note.measure).arg(note.beat).arg(note.smticks));
      QString line;
      for (auto n : note.line) { line.push_back(static_cast<char>(n)); }
      t_note->setText(1, line);

      // t_note->setTextAlignment()
      // QFont font;
      // t_note->setFont(int column, const QFont &afont)
      
      // -- kinda ugly, but coloring parts of text seems nearly impossible (there is like QStyledItemDelegate with no sane examples)
      QColor snap_color = qcolor_from_smticks(note.smticks);
      snap_color.setAlpha(30); // 0 is fully transparent
      QBrush brush(snap_color);
      t_note->setBackground(1, brush);
    }
    i++;
  }
  tree.expandAll();
  tree.resizeColumnToContents(0);


  // -- must be already in the QTreeWidget (https://doc.qt.io/qt-6/qtreewidgetitem.html#setExpanded)

  QHBoxLayout layout(&w_root);
  
  layout.addWidget(&tree, 5);
  // -- QLabel label; label.setText("Hi world"); label.setAlignment(Qt::AlignCenter);
  // -- layout.addWidget(&label, 3);
  QTextEdit viewer;
  viewer.setReadOnly(true);
  viewer.setText("A bunch of random text\nthat spans\na couple lines");
  // layout.addWidget(&viewer, 2);


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


  w_root.show();




  return app.exec();
}
