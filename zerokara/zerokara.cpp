#include "sm_parser.cpp"

// C++
#include "precompiled.h"
#include <QSplitter>
#include <QTabWidget>
#include <QStandardItemModel>
#include <sys/inotify.h>
#include <unistd.h>
#include <variant>

SmFile smfile;

using namespace Qt::Literals::StringLiterals;

// -- I might want to unify these two maybe ???
QColor qcolor_from_smticks(double smticks) {
  assert(smticks <= 48.0);
  if (smticks == 48.0) smticks = 0.0; // can happen due to rounding up

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
    if (std::fmod(smticks, s) == 0) return c;
  }
  return QColorConstants::Gray;
}
// this is for the Snap xx highlight
const char *cstr_color_from_snap (int snap) {
  switch (snap) {
    case 1: case 2: case 4: return "red";
    case 8: return "blue";
    case 3: case 6: case 12: return "green"; // limegreen
    case 16: return "goldenrod";
    case 24: return "magenta";
    case 32: return "orange";
    case 48: return "deepskyblue";
    case 20: case 28: case 36: case 64: return "grey";
    default: return "";
  }
}

std::array<NoteType, 4> notes_from_string(const char str[4]) {
  std::array<NoteType, 4> ret = {};
  ret[0] = static_cast<NoteType>(str[0]);
  ret[1] = static_cast<NoteType>(str[1]);
  ret[2] = static_cast<NoteType>(str[2]);
  ret[3] = static_cast<NoteType>(str[3]);
  return ret;
}

// 20 and 28 are still useful sometimes, but maybe that should be in some checkbox
int sm_sane_snap_higher_than(int snap) {
  for (auto bound : {1,2,4,8,12,16,24,32,36,48,64,96,192}) {
    if (snap < bound) return bound;
  }
  return 192;
}
int sm_sane_snap_lower_than(int snap) {
  for (auto bound : {192,96,64,48,36,32,24,16,12,8,4,2,1}) {
    if (snap > bound) return bound;
  }
  return 1;
}

struct NoteRowRects {
  std::vector<QRectF> rects; // 0 up to 4
  QColor color;
};


// who cares, everyone uses 4/4
double smticks_in_1_(int subdiv){return 192./subdiv;};


struct SmRelativePos {
  int32_t measures = 0;
  int32_t beats = 0;
  double smticks = 0;

  static SmRelativePos incremented_by(SmRelativePos pos, double how_many_smticks) {
    pos.smticks += how_many_smticks;
    if (pos.smticks < 0) {
      double n_borrows_from_beats = std::ceil(-(pos.smticks / 48.));
      pos.beats -= (int)n_borrows_from_beats;
      pos.smticks += 48.0 * n_borrows_from_beats;
      assert(pos.smticks >= 0);
    }
    if (pos.smticks >= 48.) {
      pos.beats += (int)(pos.smticks / 48.);
      pos.smticks = std::fmod(pos.smticks, 48.);
      assert(pos.smticks < 48.);
    }

    if (pos.beats >= 4) {
      pos.measures += pos.beats / 4;
      pos.beats = pos.beats % 4;
    } else if (pos.beats < 0) {
      int n_borrows = ((-pos.beats + 3) / 4);
      pos.measures -= n_borrows;
      pos.beats += 4 * n_borrows;
    }

    // Try to cancel out floating point errors like .00...1 or .99...9
    double deviation = pos.smticks - round(pos.smticks);
    if (abs(deviation) < 0.0001) { pos.smticks -= deviation; }
    return pos;
  }
  // again, this assumes 4/4 everywhere
  double total_smticks() {
    return this->measures * 192 + this->beats * 48 + this->smticks;
  }
};

class NoteDisplayWidget : public QWidget {
Q_OBJECT

public:
  bool downscroll = false;
  int cmod = 400;
  const double PX_VISUAL_OFFSET_FROM_HORIZ_LINE = 30.0;
  double px_of_measure_zero = PX_VISUAL_OFFSET_FROM_HORIZ_LINE;

  // instead of incrementing / decrementing pixel offset directly,
  // keep track of the precise relative position and then recalculate the pixels on scroll
  SmRelativePos cur_chart_pos = {0};
  
  // let's say this means 16ths, again 4/4 yada yada
  int current_snap_nths = 16;


  // -- TODO: yes I know I rely on a single bpm for now
  double px_per_smtick() {
    double bpm = smfile.bpms[0].value;
    double secs_per_beat = 60./bpm;
    double secs_per_smtick = secs_per_beat / 48.;
    return secs_per_smtick * cmod;
  }
  double px_per_current_snap() {
    return 192.0 / current_snap_nths * px_per_smtick();
  }


  public:
  void onDownscrollCheckboxClick(Qt::CheckState ds_state) { this->downscroll = ds_state == Qt::Checked; this->update(); }
  void onCmodChange(int value) {
    this->cmod = value;
    px_of_measure_zero = PX_VISUAL_OFFSET_FROM_HORIZ_LINE - px_per_smtick() * cur_chart_pos.total_smticks();
    this->update();
  }
  
  signals:
  void positionChanged(SmRelativePos new_pos, int snap);
  

  protected:
  /// On mouse wheel scroll:
  /// no modifiers -> scroll,
  /// ctrl -> change snap,
  /// ctrl+shift -> change snap (fine)
  void wheelEvent(QWheelEvent *event) override {
    auto modifiers = QGuiApplication::keyboardModifiers();
    SmRelativePos new_pos;

    if (modifiers & Qt::ControlModifier) { // change snap
      if (event->angleDelta().ry() < 0) {
        current_snap_nths =
          (modifiers & Qt::ShiftModifier)
          ? std::max(current_snap_nths-1, 1)
          : sm_sane_snap_lower_than(current_snap_nths)
        ;
      } else {
        current_snap_nths =
          (modifiers & Qt::ShiftModifier)
          ? std::min(current_snap_nths+1, 192)
          : sm_sane_snap_higher_than(current_snap_nths)
        ;
      }
    } else { // move
      if ((event->angleDelta().ry() < 0) ^ !downscroll) {
        new_pos = SmRelativePos::incremented_by(cur_chart_pos, -smticks_in_1_(current_snap_nths));
      } else {
        new_pos = SmRelativePos::incremented_by(cur_chart_pos, +smticks_in_1_(current_snap_nths));
      }
    }
    cur_chart_pos = (new_pos.measures < 0) ? (SmRelativePos){0} : new_pos;

    // printf("raw smticks: %d\n", chart_pos.raw_smticks());
    px_of_measure_zero = PX_VISUAL_OFFSET_FROM_HORIZ_LINE - px_per_smtick() * cur_chart_pos.total_smticks();
    this->update();
    emit positionChanged(cur_chart_pos, current_snap_nths);
  };

  void paintEvent(QPaintEvent *event) override {
    (void) event;
    
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);


    int32_t note_width  = 60;
    int32_t note_height = 20;
    int32_t px_judge_line_off = 30;
    int32_t left_start = this->width() / 2 - (int32_t)(2 * note_width); // for centering

    QPen pen;
 
    // draw judge line
    {
      auto judge_line = QLine(0, px_judge_line_off, this->width()-1, px_judge_line_off);
      if (downscroll) {
        int new_y = this->height() - judge_line.y1();
        judge_line.setLine(0, new_y, this->width()-1, new_y);
      }
      pen.setWidth(5); pen.setColor(Qt::white); painter.setPen(pen);
      painter.drawLine(judge_line);
    }

    // draw corner beams around the left and right edge of playfield
    {
      auto left_edge  = QLineF(left_start,              0, left_start,               this->height()-1);
      auto right_edge = QLineF(left_start+note_width*4, 0, left_start+note_width*4,  this->height()-1);
      pen.setWidth(2); pen.setColor(Qt::gray); painter.setPen(pen);
      painter.drawLine(left_edge);
      painter.drawLine(right_edge);
    }

    // draw snaplines, starting from previous measure
    {
      SmRelativePos snap_of_this_snapline = cur_chart_pos;
      auto s = &snap_of_this_snapline;
      s->measures -= 1;

      // printf("---------------\n");
      for (int i = 0; i < 300; i++) {
        double y_distance = px_of_measure_zero + s->total_smticks() * px_per_smtick();
        // printf("snap of this snapline: %d:%d:%lg, which translates to %lg px\n",
        //        s->measures, s->beats, s->smticks, y_distance);

        if (y_distance < 0.0) {
          *s = SmRelativePos::incremented_by(*s, 192.0/current_snap_nths);
          continue;
        }
        if (y_distance > this->height()) { break; }
        double y = downscroll ? (double)this->height() - y_distance : y_distance;

        // -- In SM/Etterna, actual notes will have weird colors on weird snaps
        // -- because they are snapped to the nearest smtick (at least that's how
        // -- they are stored in most files).
        // -- There should be some checkbox somewhere if we want smticks to be int
        // -- or float and only quantized on save. We don't want to break existing
        // -- 28th colored streams for example.
        // QColor color = qcolor_from_smticks((int32_t)roundl(s->smticks));
        QColor color = qcolor_from_smticks(s->smticks);
        color.setAlphaF(.6f);
        pen.setColor(color);
        pen.setWidth(
          std::fmod(s->smticks, 48.) == 0 ? 4 :
          std::fmod(s->smticks, 24.) == 0 ? 3 :
          std::fmod(s->smticks, 16.) == 0 ? 2 :
          std::fmod(s->smticks, 12.) == 0 ? 2 : 1
         );
        painter.setPen(pen);
        auto snap_line = QLineF(left_start, y, left_start + 4 * note_width, y);
        painter.drawLine(snap_line);

        *s = SmRelativePos::incremented_by(*s, 192.0/current_snap_nths);
      }
    }


    std::function<NoteRowRects(NoteRow)>
    rectangles_at_smtick_pos = [&](NoteRow row)
    {
      uint32_t global_smticks = row.smticks + row.beat * 48 + row.measure * 48 * 4;

      auto line = (NoteRowRects){
        .rects = {},
        .color = qcolor_from_smticks(row.smticks)
      };

      for (size_t i = 0; i < 4; i++) {
        if (row.notes[i] == NoteType::Tap) {
          line.rects.push_back(
            QRect((int32_t)(left_start + note_width * (int32_t)i),
                  (int32_t)(px_of_measure_zero + px_per_smtick() * global_smticks),
                  note_width, note_height)
          );
        }
      }
      return line;
    };


    // might be very slow for now
    std::vector<NoteRow> note_rows = smfile.diffs[0].note_rows();

    std::vector<NoteRowRects> rectangles;
    std::transform(note_rows.begin(), note_rows.end(), std::back_inserter(rectangles), [&](NoteRow nr) {return rectangles_at_smtick_pos(nr);});

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


__attribute__((noreturn))
void *exec_yourself(void *arg) {
  int inotify_fd = *(int *)arg;
  char dontcare[1];
  // should block until the binary changes
  printf("\nI: The app will be restarted when the ./zerokara binary changes.\n");
  // spam if the binary is gone for a moment
  while(1) {
    ssize_t read_bytes = read(inotify_fd, dontcare, 1); (void)(read_bytes);
    int minusone_on_fail = execl("./zerokara", "./zerokara", NULL); (void)(minusone_on_fail);
    // perror("I couldn't exec myself");
  }
  // if we ever got here, we failed.
  assert(false);
}


// this is messed up :(
using TimeKVs      = std::vector<TimeKV>;
using Difficulties = std::vector<Difficulty>;

using TreeValue = std::variant<
  std::string,
  double,
  std::vector<TimeKV>,
  std::vector<Difficulty>
>;

// just so I don't have to spam setEditable(false) everywhere
class Cell : public QStandardItem {
  public:
  Cell(QString str) : QStandardItem(str) {
    this->setEditable(false);
  };
};

Q_DECLARE_METATYPE(TreeValue);
struct TreeItem {
  QString key;
  TreeValue value;
};
struct KVTreeModel : public QStandardItemModel {
  Q_OBJECT
public:

  KVTreeModel(const SmFile &smfile) : QStandardItemModel() {
    
    this->setColumnCount(2);

    Cell *mtdt_cell = new Cell("Metadata");

    // HACK: throw all SmFile fields on one temporary pile so the code is shorter.
    QList<TreeItem> mtdt_fields;
    mtdt_fields.append({("#TITLE"),        (smfile.title)});
    mtdt_fields.append({("#MUSIC"),        (smfile.music)});
    mtdt_fields.append({("#ARTIST"),       (smfile.artist)});
    mtdt_fields.append({("#OFFSET"),       (smfile.offset)});
    mtdt_fields.append({("#SAMPLESTART"),  (smfile.samplestart)});
    mtdt_fields.append({("#SAMPLELENGTH"), (smfile.samplelength)});
    mtdt_fields.append({("#BPMS"),         (smfile.bpms)});
    mtdt_fields.append({("#STOPS"),        (smfile.stops)});
    // ...

    for (auto l : mtdt_fields) {
      auto *key_cell = new Cell(l.key);
      Cell *value_cell;

      QString value_str;
      if (0) {
      } else if (std::holds_alternative<std::string>(l.value)) { // most fields
        const std::string &str = std::get<std::string>(l.value);
        value_str = QString::fromStdString(str);
        value_cell = new Cell(value_str);
      } else if (std::holds_alternative<double>(l.value)) { // #OFFSET, #SAMPLESTART, #SAMPLELENGTH
        value_str = QString::number(std::get<double>(l.value));
        value_cell = new Cell(value_str);
      } else if (std::holds_alternative<std::vector<TimeKV>>(l.value)) { // #BPMS, #STOPS
        value_cell = new Cell(""); // no text on second column
        for (auto timekv : std::get<TimeKVs>(l.value)) {
          auto *time_cell  = new Cell(QString::number(timekv.beat_number));
          auto *value_cell = new Cell(QString::number(timekv.value));
          key_cell->appendRow({time_cell, value_cell});
        }
      } else {
        assert(false);
      }
      value_cell->setEditable(false);
      mtdt_cell->appendRow({key_cell, value_cell});
    }
    appendRow(mtdt_cell); // probably the root item?


    Cell *diff_cell = new Cell("Difficulties");
    int diff_i = 0;
    for (auto diff : smfile.diffs) {
      auto *num_cell = new Cell(QString::number(diff_i));

      auto *gt_n = new Cell("Game type");
      auto *gt_v = new Cell(cstr_from_gametype(diff.game_type));
      num_cell->appendRow({gt_n, gt_v});
      auto *ct_n = new Cell("Charter");
      auto *ct_v = new Cell(QString::fromStdString(diff.charter));
      num_cell->appendRow({ct_n, ct_v});
      auto *dt_n = new Cell("Diff type");
      auto *dt_v = new Cell(cstr_from_difftype(diff.diff_type));
      QBrush brush(qcolor_from_difftype(diff.diff_type));
      // QFont font; font.setBold(true); t_difftype->setFont(1, font);
      dt_v->setForeground(brush);

      num_cell->appendRow({dt_n, dt_v});
      auto *nr_n = new Cell(u"Note rows (%1)"_s.arg(diff.total_note_rows()));
      num_cell->appendRow(nr_n);
      auto *mrs_n = new Cell(u"Measures (%1)"_s.arg(diff.measures.size()));
      num_cell->appendRow(mrs_n);

      auto draw_note_rows = [](Cell *t_parent, const std::vector<NoteRow>& note_rows) {
        for (auto nl : note_rows) {
          auto *t_noteline_snap = new Cell(
            (QString("%1/%2/%3").arg(nl.measure).arg(nl.beat).arg(nl.smticks))
          );
          QString line_s;
          for (auto n : nl.notes) { line_s.push_back(static_cast<char>(n)); }
          auto *t_noteline_notes = new Cell(line_s);

          QColor snap_color = qcolor_from_smticks(nl.smticks);
          snap_color.setAlphaF(.12f); // 0 is fully transparent
          QBrush brush(snap_color);
          t_noteline_notes->setBackground(brush);
          t_parent->appendRow({t_noteline_snap, t_noteline_notes});
        }
      };
      draw_note_rows(nr_n, diff.note_rows());
      int mr_i = 0;
      for (auto m : diff.measures) {
        auto *mr_n = new Cell(u"Measure %1"_s.arg(mr_i));
        draw_note_rows(mr_n, m.note_rows);
        mrs_n->appendRow(mr_n);

        mr_i += 1;
      }
      
      diff_cell->appendRow(num_cell);
      diff_i += 1;
    }
    appendRow(diff_cell);
  }
};


int main(int argc, char **argv) {
  // -- make float parsing not break in Czech locale
  // -- XXX: why does this not work? For now, I'll set it nearby float parsing.
  // QLocale locale("C");
  // QLocale::setDefault(locale);

  // -- let's try the hot restart thing.
  int inotify_fd = inotify_init1(IN_CLOEXEC);
  inotify_add_watch(inotify_fd, argv[0], IN_ATTRIB);
  pthread_t inotify_reader;
  pthread_create(&inotify_reader, NULL, exec_yourself, &inotify_fd);

  // -- QGuiApplication doesn't work if you want widgets
  QApplication app(argc, argv);

  // -- parse a sample smfile
  const char *path = "ext/Shannon's Theorem.sm";
  std::ifstream file(path);
  std::ostringstream ss;
  ss << file.rdbuf();
  auto smfile_opt = smfile_from_string_opt(ss.str());

  if (std::holds_alternative<SmFile>(smfile_opt)) {
    smfile = std::get<SmFile>(smfile_opt);
  } else {
    auto err = std::get<SmParseError>(smfile_opt);
    printf("%s\n", err.msg.c_str());
    assert(false);
  }

  // -- build UI
  QWidget *w_root = new QWidget();

  QTabWidget w_tabs_root;
  w_tabs_root.addTab(w_root, path);

    // -- required so that it fills the entire space
  QHBoxLayout layout_(w_root);

  QSplitter resizable_layout(Qt::Horizontal);
  layout_.addWidget(&resizable_layout);

  // -- TODO: at some point, I would like QTreeView, so that I can change
  // diff type with a dropdown. But I would need to scrap all of this
  // and learn idk abstract qt model stuff.

  KVTreeModel smfile_model(smfile);
  QTreeView *tree_view = new QTreeView();
  tree_view->setModel(&smfile_model);
  tree_view->setHeaderHidden(true);
  tree_view->expandToDepth(1); // hacky :( but luckily works here
  tree_view->resizeColumnToContents(0);
  tree_view->show();

  // QTreeWidget *tree = new QTreeWidget();
  // tree->setColumnCount(2);
  // tree->setHeaderHidden(true);
  // auto t_meta = new QTreeWidgetItem(tree); {
  //   t_meta->setText(0, "Metadata");
  //   t_meta->setExpanded(true);
  // }
  // auto t_title = new QTreeWidgetItem(t_meta); {
  //   t_title->setText(0, "#TITLE"); t_title->setText(1, QString(smfile.title.c_str()));
  // }
  // auto t_music = new QTreeWidgetItem(t_meta); {
  //   t_music->setText(0, "#MUSIC"); t_music->setText(1, QString(smfile.music.c_str()));
  // }
  // auto t_artist = new QTreeWidgetItem(t_meta); {
  //   t_artist->setText(0, "#ARTIST"); t_artist->setText(1, QString(smfile.artist.c_str()));
  // }
  // auto t_offset = new QTreeWidgetItem(t_meta); {
  //   t_offset->setText(0, "#OFFSET"); t_offset->setText(1, QString::number(smfile.offset));
  // }
  // auto t_samplestart = new QTreeWidgetItem(t_meta); {
  //   t_samplestart->setText(0, "#SAMPLESTART"); t_samplestart->setText(1, QString::number(smfile.samplestart));
  // }
  // auto t_samplelength = new QTreeWidgetItem(t_meta); {
  //   t_samplelength->setText(0, "#SAMPLELENGTH"); t_samplelength->setText(1, QString::number(smfile.samplelength));
  // }

  // QList<QTreeWidgetItem *> t_bpm_list;
  // auto t_bpms = new QTreeWidgetItem(t_meta); {
  //   t_bpms->setText(0, "#BPMS"); t_bpms->setExpanded(true);
  //   for (auto time_bpm : smfile.bpms) {
  //     auto *t_bpm = new QTreeWidgetItem();
  //     t_bpm->setText(0, QString::number(time_bpm.beat_number));
  //     t_bpm->setText(1, QString::number(time_bpm.value)); // 'g' (default) or 'f', '3'
  //     t_bpm_list.push_back(t_bpm);
  //   }
  //   t_bpms->addChildren(t_bpm_list);
  // }

  // QList<QTreeWidgetItem *> t_stop_list;
  // auto t_stops = new QTreeWidgetItem(t_meta); {
  //   t_stops->setText(0, "#STOPS"); t_stops->setExpanded(true);
  //   for (auto time_stop : smfile.stops) {
  //     auto *t_stop = new QTreeWidgetItem();
  //     t_stop->setText(0, QString::number(time_stop.beat_number));
  //     t_stop->setText(1, QString::number(time_stop.value)); // 'g' (default) or 'f', '3'
  //     t_stop_list.push_back(t_stop);
  //   }
  //   t_bpms->addChildren(t_stop_list);
  // }
  
  // auto t_diffs = new QTreeWidgetItem(tree);
  // t_diffs->setText(0, "Difficulties"); t_diffs->setExpanded(true);

  // size_t i = 0;
  // for (auto diff : smfile.diffs) {
  //   auto *t_diff = new QTreeWidgetItem(t_diffs);
  //   t_diff->setText(0, QString::number(i));
  //   t_diff->setExpanded(true);

  //   auto *t_gametype = new QTreeWidgetItem(t_diff);
  //   t_gametype->setText(0, "Game type");
  //   t_gametype->setText(1, cstr_from_gametype(diff.game_type));

  //   auto *t_charter = new QTreeWidgetItem(t_diff);
  //   t_charter->setText(0, "Charter");
  //   t_charter->setText(1, diff.charter.c_str());

  //   auto *t_difftype = new QTreeWidgetItem(t_diff);
  //   t_difftype->setText(0, "Diff type");
  //   t_difftype->setText(1, cstr_from_difftype(diff.diff_type));
  //   QBrush brush(qcolor_from_difftype(diff.diff_type));
  //   // QFont font; font.setBold(true); t_difftype->setFont(1, font);
  //   t_difftype->setForeground(1, brush);

  //   auto *t_diffval = new QTreeWidgetItem(t_diff);
  //   t_diffval->setText(0, "Diff value");
  //   t_diffval->setText(1, QString::number(diff.diff_num));

  //   auto *t_notes = new QTreeWidgetItem(t_diff);
  //   t_notes->setText(0, QString("Note rows (%1)").arg(diff.total_note_rows()));

  //   auto *t_as_measures = new QTreeWidgetItem(t_diff);
  //   t_as_measures->setText(0, QString("Measures (%1)").arg(diff.measures.size()));

  //   auto draw_note_rows = [](QTreeWidgetItem *t_parent, std::vector<NoteRow> const& note_rows) {
  //     for (auto nl : note_rows) {
  //       auto *t_noteline = new QTreeWidgetItem(t_parent);
  //       t_noteline->setText(0, QString("%1/%2/%3")
  //                           .arg(nl.measure).arg(nl.beat).arg(nl.smticks));
  //       QString line_s;
  //       for (auto n : nl.notes) { line_s.push_back(static_cast<char>(n)); }
  //       t_noteline->setText(1, line_s);
        // -- stupid:
        // -- * there is no verification
        // -- * not per-column
        // -- * I want to actually double-click to seek to that pos
        // -- * it's not ergonomic anyway so ?????
        // t_noteline->setFlags(t_noteline->flags() | Qt::ItemIsEditable);

  //       QColor snap_color = qcolor_from_smticks(nl.smticks);
  //       snap_color.setAlphaF(.12f); // 0 is fully transparent
  //       QBrush brush(snap_color);
  //       t_noteline->setBackground(1, brush);
  //     }
  //   };

  //   int measure_i = 0;
  //   for (auto m : diff.measures) {
  //     auto *t_measure = new QTreeWidgetItem(t_as_measures);
  //     t_measure->setText(0, QString("%1 (%2 row%3)")
  //                        .arg(measure_i)
  //                        .arg(m.note_rows.size())
  //                        .arg(m.note_rows.size() == 1 ? "" : "s")
  //     );
  //     draw_note_rows(t_measure, m.note_rows);
  //     measure_i += 1;
  //   }
  //   draw_note_rows(t_notes, diff.note_rows());

  //   i++;
  // }
  // tree->expandAll();
  // tree->resizeColumnToContents(0);



  // -- must be already in the QTreeWidget (https://doc.qt.io/qt-6/qtreewidgetitem.html#setExpanded)

  // QHBoxLayout layout(&w_root);
  

  
  // QHBoxLayout resizable_layout;
  // layout_.addLayout(&resizable_layout);
  
  // layout.addWidget(&tree, 5);
  // resizable_layout.addWidget(tree);
  resizable_layout.addWidget(tree_view);

  // QTextEdit viewer;
  // viewer.setReadOnly(true);
  // viewer.setText("A bunch of random text\nthat spans\na couple lines");
  // layout.addWidget(&viewer, 2);


  // -- You can't scope these, they need to live somehow (on stack or heap).
  // -- Also, if you make them static, it aborts on exit for some reason
  // -- leaving just the indentation for now
  QWidget right_chunk;
  QVBoxLayout preview_tile(&right_chunk);

    NoteDisplayWidget preview_actual;
      preview_actual.setAutoFillBackground(true);
      auto pal = preview_actual.palette();
      pal.setColor(QPalette::Window, Qt::black);
      preview_actual.setPalette(pal);
    preview_tile.addWidget(&preview_actual, 8);

    // TODO: this guy needs to be updated when I scroll
    // QLabel status_bar(QString("Measure %1, beat %2, smtick %3  |  Snap %4")
    //                   .arg(preview_actual.chart_pos.measures)
    //                   .arg(preview_actual.chart_pos.beats)
    //                   .arg(preview_actual.chart_pos.smticks)
    //                   .arg(preview_actual.current_snap_nths)
    //                 );
    QLabel status_bar;

    auto on_pos_change =
      [&](SmRelativePos pos, int snap){status_bar.setText(
        QString("Measure %1, beat %2, smtick %3  |  "
                "Snap <span style='color: %4; font-weight: 600;'>%5</span>")
          .arg(pos.measures)
          .arg(pos.beats)
          .arg(QString::number(pos.smticks, 'g', 4))
          .arg(cstr_color_from_snap(snap))
          .arg(snap)
      );};
    on_pos_change(preview_actual.cur_chart_pos, preview_actual.current_snap_nths);

    QObject::connect(
      &preview_actual,
      &NoteDisplayWidget::positionChanged,
      on_pos_change
    );
    preview_tile.addWidget(&status_bar, 0);
    preview_tile.setAlignment(&status_bar, Qt::AlignCenter);
    
    QHBoxLayout preview_controls;

      QCheckBox downscroll_chk("Downscroll");
        downscroll_chk.setCheckState(Qt::Checked);
        QObject::connect(
          &downscroll_chk,
          &QCheckBox::checkStateChanged,
          &preview_actual,
          &NoteDisplayWidget::onDownscrollCheckboxClick
        );
        preview_actual.onDownscrollCheckboxClick(downscroll_chk.checkState());  // set initial
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

      // doesn't look super aesthetic, but it's ok for now
      // TODO: replace with QSpinBox
      QSlider cmod_slider(Qt::Vertical, nullptr);
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

  // layout.addLayout(&preview_tile, 4);
  
  // -- idk if this is correct
  // 
  // QWidget *right_chunk = new QWidget();
  // right_chunk->setLayout(&preview_tile);
  // resizable_layout.addWidget(right_chunk);
  resizable_layout.addWidget(&right_chunk);

  w_tabs_root.show();
  int ret = app.exec();
  // delete tree;
  // tree = nullptr;
  return ret;
}

#include "zerokara.moc"
