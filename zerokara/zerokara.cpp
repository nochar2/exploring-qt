// @includes -----------------------------------------------------------------------------------------------------------------

#include <string>
using std::string;

#include "sm_parser.h"
#include "qt_includes.h"
void __please(){qt_includes_suppress_bogus_unused_warning=0;};
using namespace Qt::Literals::StringLiterals;


// reload hacks
#include <sys/inotify.h>
#include <unistd.h>

// abstract away some parts of C++ STL, for testing compilation times
#include "dumb_stdlib_linux.h"

#pragma GCC poison printf
#define eprintf(...) fprintf(stderr, __VA_ARGS__)

#include <ranges>
using std::ranges::views::enumerate;

// @structures ----------------------------------------------------------------------------------------------------
struct NoteDisplayWidget;
struct StatusBar;
struct SmFileView;

struct NoteDisplayWidget : public QWidget {
  SmFileView *sm_file_view;
  StatusBar *status_bar;

  NoteDisplayWidget() {
    setFocusPolicy(Qt::StrongFocus);
  }

  bool downscroll = false;
  int cmod = 400;
  const double PX_VISUAL_OFFSET_FROM_HORIZ_LINE = 30.0;
  double px_of_measure_zero = PX_VISUAL_OFFSET_FROM_HORIZ_LINE;

  // here for now
  double px_per_smtick();
  double px_per_current_snap();

  void redraw();
  void wheelEvent(QWheelEvent *event) override;
  void paintEvent(QPaintEvent *event) override;
  void keyPressEvent(QKeyEvent *event) override;
  void onCmodChange(int value);
  void onDownscrollChange(Qt::CheckState state);
};



struct StatusBar : public QWidget {
  SmFileView *sm_file_view;

  NoteDisplayWidget *note_display_widget;

  QHBoxLayout container;
  QLabel label_pos;
  QPushButton btn_reset_weird_snap;
  QLabel label_snap;

  StatusBar(QWidget *parent, SmFileView *sm_file_view);

  void redraw();
};

struct NoteRowRects {
  std::vector<QRectF> rects; // 0 up to 4
  QColor color;
};

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
    double deviation = round(pos.smticks) - pos.smticks;
    if (fabs(deviation) < 0.0001) { pos.smticks += deviation; }
    return pos;
  }
  // again, this assumes 4/4 everywhere
  double total_smticks() {
    return this->measures * 192 + this->beats * 48 + this->smticks;
  }
};



// @globals ---------------------------------------------------------------------------------------------------------------

// @functions -----------------------------------------------------------------------------------------------------------



QColor qcolor_from_difftype(DiffType dt)
{
  switch (dt) {
    case DiffType::Beginner:  return QColorConstants::DarkCyan;
    case DiffType::Easy:      return QColorConstants::DarkGreen;
    case DiffType::Medium:    return QColorConstants::Svg::darkorange;
    case DiffType::Hard:      return QColorConstants::DarkRed;
    case DiffType::Challenge: return QColorConstants::Svg::purple;
    case DiffType::Edit:      return QColorConstants::DarkGray;
  }
  assert(false);
}

const char *cstr_color_from_snap (int snap);

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
const char *cstr_from_smticks(double smticks) {
  if (std::fmod(smticks, 48/1) == 0) return "red";
  if (std::fmod(smticks, 48/2) == 0) return "blue";
  if (std::fmod(smticks, 48/3) == 0) return "green";
  if (std::fmod(smticks, 48/4) == 0) return "goldenrod";
  if (std::fmod(smticks, 48/6) == 0) return "magenta";
  if (std::fmod(smticks, 48/8) == 0) return "orange";
  if (std::fmod(smticks, 48/12) == 0) return "deepskyblue";
  return "grey";
}

bool smtick_is_sane(double smtick, int cur_snap_nths) {
  // -- false on gray notes
  // for (auto div : {1,2,3,4,6,8,12}) {
  //   if (std::fmod(smtick, 48/div) == 0.) return true;
  // }
  // return false;

  // -- false if you can no longer step over red notes
  // -- (includes stuff like offbeat triplets)
  return std::fmod(smtick, 192/cur_snap_nths) == 0;
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



// who cares, everyone uses 4/4
double smticks_in_1_(int subdiv){return 192./subdiv;};







class Cell : public QStandardItem {
  public:
  // -- so I can use setEditable(false) here if needed ???
  Cell() : QStandardItem() {}
  Cell(QString str) : QStandardItem(str) {
    // -- TODO: second columns of rows like [Difficulties] or [0]
    // -- are editable anyway, this should be set elsewhere probably maybe idk
    // this->setEditable(false);
  };
};
using TimeKVs      = std::vector<TimeKV>;
using Difficulties = std::vector<Difficulty>;
enum class SmDoubleKind { Millis, Beats, Bpm };
struct DoubleField  { double *data; SmDoubleKind kind; };
struct KVFields     { std::vector<TimeKV> *data; SmDoubleKind kind; };
using TreeValue = std::variant<
  std::string *
  , DoubleField
  , DiffType
  , uint32_t
  , SmRelativePos // measure/beat/smticks
  , NoteRow       // particular note data without pos info
>;

Q_DECLARE_METATYPE(TreeValue);

struct TreeItem { QString key; TreeValue value; };

struct KVTreeModel : public QStandardItemModel {
  SmFileView *sm_file_view;
  void push_model_to_view();
  KVTreeModel(SmFileView *sm_file_view) : QStandardItemModel(), sm_file_view(sm_file_view) {
  }
};


struct KVTreeViewDelegate : public QStyledItemDelegate {
  KVTreeModel *model;
  KVTreeViewDelegate(KVTreeModel *model) : model(model) {}

  // for HTML rendering
  void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

  QWidget* createEditor (QWidget* parent, const QStyleOptionViewItem& /*option*/,  const QModelIndex& index) const override;
};


struct SmFileView : public QWidget {

  // data
  SmFile smfile;
  SmRelativePos cur_chart_pos = {0};
  int cur_snap_nths = 16;

  KVTreeModel *smfile_model;
  KVTreeViewDelegate *smfile_view_delegate;

  // widgets (roughly how they're nested)
  // (maybe some of this can just leak and not be here)
  QHBoxLayout *layout_;
    QSplitter *resizable_layout;
      QTreeView *tree_view;
      QWidget *right_chunk;
        QVBoxLayout *preview_tile;
          QFrame *why; // useless wrapper around status bar (maybe unnecessary)
            StatusBar *status_bar;
          NoteDisplayWidget *preview_actual;
          QHBoxLayout *preview_controls;
            QCheckBox *downscroll_chk;
            QLabel *cmod_spinbox_label;
            QSpinBox *cmod_spinbox;

  // constructor
  SmFileView(const char *path);

};


StatusBar::StatusBar(QWidget *parent, SmFileView *sm_file_view) : sm_file_view(sm_file_view) /*: QWidget(parent) */ {

  this->container.addWidget(&this->label_pos, 0);
  this->container.addWidget(&this->btn_reset_weird_snap, 0);
  this->container.addWidget(&this->label_snap, 0);
  // container.setParent(this);
  this->setLayout(&this->container);
  this->setParent(parent);
  QObject::connect(
    &this->btn_reset_weird_snap,
    &QPushButton::clicked,
    this,
    [&](){
      this->sm_file_view->cur_chart_pos.smticks = 0;
      /* TODO redraw a bunch of stuff */
      this->redraw();
      this->note_display_widget->update();
    }
  );
  this->redraw();
};

void StatusBar::redraw() {
  auto v = this->sm_file_view;
  
  this->label_pos.setText(
    QString("Measure %1, beat %2, smtick %3")
    .arg(v->cur_chart_pos.measures)
    .arg(v->cur_chart_pos.beats)
    .arg(QString::number(v->cur_chart_pos.smticks, 'g', 4))
  );

  this->btn_reset_weird_snap.setText("(Reset)");
  // -- this doesn't work, and I'm doing the styleditemdelegate dance again
  // this->btn_reset_weird_snap.setText("(<span style='text-decoration: underline;'>Reset</span>)");
  this->btn_reset_weird_snap.setStyleSheet("margin: 0em; border: 0em; font-weight: 600;");

  // -- getters/setters were a mistake...
  // QFont font = this->btn_reset_weird_snap.font();
  // font.setPointSize(8);
  // this->btn_reset_weird_snap.setFont(font);

  this->label_snap.setText(
    QString("| Snap <span style='color: %1; font-weight: 600;'>%2</span>")
    .arg(cstr_color_from_snap(v->cur_snap_nths))
    .arg(v->cur_snap_nths)
  );

  // -- for now comment out
  if (!smtick_is_sane(v->cur_chart_pos.smticks, v->cur_snap_nths)) {
    this->btn_reset_weird_snap.show();
  } else {
    this->btn_reset_weird_snap.hide();
  }
}

void NoteDisplayWidget::keyPressEvent(QKeyEvent *event) {
  auto pos = this->sm_file_view->cur_chart_pos;

  eprintf("Key press %d\n", event->key());
  NoteRow *loc = &(this
    ->sm_file_view
    ->smfile.diffs[0]
    .measures[pos.measures]
    .beats[pos.beats]
    .note_rows[pos.smticks]
  );

  // XXX: Oh yeah. There should be a pointer to currently active Difficulty.
  // XXX: Oh yeah, you need to locate the actual note, which might not even
  // be in the list. Why not just do the stupidest thing and have a sparse matrix?
  // this->sm_file_view->smfile.diffs[0]

  auto k = event->key(); // this is a keysym, maybe we want a keycode for some keys
  if ('1' <= k && k <= '4') {
    loc->notes[k-'1'] = loc->notes[k-'1'] == NoteType::None ? NoteType::Tap : NoteType::None;
  }

  // this->sm_file_view->update();
  // this->redraw();
  this->update();
  this->sm_file_view->smfile_model->push_model_to_view();
}

void NoteDisplayWidget::paintEvent(QPaintEvent */*event*/) {
  auto v = this->sm_file_view;
  
  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing);

  // visually scroll to desired place
  px_of_measure_zero = PX_VISUAL_OFFSET_FROM_HORIZ_LINE - px_per_smtick() * v->cur_chart_pos.total_smticks();

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
    SmRelativePos snap_of_this_snapline = v->cur_chart_pos;
    auto s = &snap_of_this_snapline;
    s->measures -= 1;

    // printf("---------------\n");
    for (int i = 0; i < 300; i++) {
      double y_distance = px_of_measure_zero + s->total_smticks() * px_per_smtick();
      // printf("snap of this snapline: %d:%d:%lg, which translates to %lg px\n",
      //        s->measures, s->beats, s->smticks, y_distance);

      if (y_distance < 0.0) {
        *s = SmRelativePos::incremented_by(*s, 192.0/v->cur_snap_nths);
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

      *s = SmRelativePos::incremented_by(*s, 192.0/v->cur_snap_nths);
    }
  }


  std::function<NoteRowRects(NoteRow,SmRelativePos)>
  noterow_to_rectangles = [&](NoteRow row, SmRelativePos pos)
  {
    // replace this if we encounter a non-4/4 file
    uint32_t global_smticks
      = (uint32_t)pos.smticks + (uint32_t)pos.beats * 48 + (uint32_t)pos.measures * 48 * 4;

    auto line = (NoteRowRects){
      .rects = {},
      .color = qcolor_from_smticks(pos.smticks)
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


  Vector<NoteRowRects> rectangles;
  for (auto [me_i, me] : enumerate(v->smfile.diffs[0].measures)) {
    for (auto [bt_i, bt] : enumerate(me.beats)) {
      for (auto [nr_i, nr] : enumerate(bt.note_rows)) {
        if (!noterow_is_zero(nr)) {
          auto pos = (SmRelativePos){.measures=(int32_t)me_i,.beats=(int32_t)bt_i,.smticks=(double)nr_i};
          rectangles.push_back(noterow_to_rectangles(nr, pos));
        }
      }
    }
  }

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

// -- TODO: yes I know I rely on a single bpm for now
double NoteDisplayWidget::px_per_smtick() {
  auto v = this->sm_file_view;
  double bpm = v->smfile.bpms[0].value;
  double secs_per_beat = 60./bpm;
  double secs_per_smtick = secs_per_beat / 48.;
  return secs_per_smtick * cmod;
}
double NoteDisplayWidget::px_per_current_snap() {
  auto v = this->sm_file_view;
  return 192.0 / v->cur_snap_nths * px_per_smtick();
}
  /// On mouse wheel scroll:
  /// no modifiers -> scroll,
  /// ctrl -> change snap,
  /// ctrl+shift -> change snap (fine)
void NoteDisplayWidget::wheelEvent(QWheelEvent *event) {
  auto v = this->sm_file_view;
  auto modifiers = QGuiApplication::keyboardModifiers();
  SmRelativePos new_pos = v->cur_chart_pos;

  // printf("pos before: %d %d %lf\n",
  //        cur_chart_pos.measures, cur_chart_pos.beats, cur_chart_pos.smticks);

  if (modifiers & Qt::ControlModifier) { // change snap
    if (event->angleDelta().ry() < 0) {
      v->cur_snap_nths =
        (modifiers & Qt::ShiftModifier)
        ? std::max(v->cur_snap_nths-1, 1)
        : sm_sane_snap_lower_than(v->cur_snap_nths)
      ;
    } else {
      v->cur_snap_nths =
        (modifiers & Qt::ShiftModifier)
        ? std::min(v->cur_snap_nths+1, 192)
        : sm_sane_snap_higher_than(v->cur_snap_nths)
      ;
    }
  } else { // move
    if ((event->angleDelta().ry() < 0) ^ !downscroll) {
      new_pos = SmRelativePos::incremented_by(v->cur_chart_pos, -smticks_in_1_(v->cur_snap_nths));
    } else {
      new_pos = SmRelativePos::incremented_by(v->cur_chart_pos, +smticks_in_1_(v->cur_snap_nths));
    }
  }
  // printf("pos after: %d %d %lg\n", new_pos.measures, new_pos.beats, new_pos.smticks);
  v->cur_chart_pos = (new_pos.measures < 0) ? (SmRelativePos){0} : new_pos;

  // printf("raw smticks: %d\n", chart_pos.raw_smticks());
  // 
  // -- FIXMEEEEEEEEEEEEEEEE: we need to get the pointer to status_bar from somewhere,
  // -- but that reeks with spaghetti. Or I can store a list of magical function pointers
  // -- somewhere. I don't know of a good idea to resolve this
  status_bar->redraw();
  this->update();
  // positionChanged(cur_chart_pos, cur_snap_nths);
};







void KVTreeModel::push_model_to_view() {
  auto v = this->sm_file_view;

  clear();
  
  this->setColumnCount(2);


  // smuggle a pointer in a QVariant
  #define PACK(x) QVariant::fromValue(TreeValue(x))

  // let's try dealing with the string fields separately beacause they are
  // annoying and there's many of them. Other types like floats vary too much
  // for abstraction to be useful.
  std::vector<std::tuple<const char *,string>> string_fields;
  string_fields.push_back({"#TITLE",     v->smfile.title});
  string_fields.push_back({"#SUBTITLE",  v->smfile.subtitle});
  string_fields.push_back({"#ARTIST",    v->smfile.artist});
  if (v->smfile.has_translit) {
    string_fields.push_back({"#TITLETRANSLIT",     v->smfile.titletranslit});
    string_fields.push_back({"#SUBTITLETRANSLIT",  v->smfile.subtitletranslit});
    string_fields.push_back({"#ARTISTTRANSLIT",    v->smfile.artisttranslit});
  }
  string_fields.push_back({"#CREDIT",  v->smfile.credit});
  string_fields.push_back({"#MUSIC",   v->smfile.music});

  Cell *mtdt_cell = new Cell("Metadata");
  Cell *key_cell;
  Cell *value_cell;

  auto basic_html_sanitize = [](const std::string &s) {
    std::string replaced;
    for (char c : s) {
      switch (c) {
      case '&': replaced.append("&amp;"); break;
      case '<': replaced.append("&lt;"); break;
      case '>': replaced.append("&gt;"); break;
      default:  replaced.push_back(c); break;
      }
    }
    return replaced;
  };

  for (auto [key,value] : string_fields) {
    key_cell = new Cell(key);
    value_cell = new Cell(QString::fromStdString(basic_html_sanitize(value)));
    value_cell->setData(PACK(new std::string(value)));
    mtdt_cell->appendRow({key_cell, value_cell});
  }

  // this might be a bit compressible, for now idc
  // NOTE:
  // So, there is this unfortunate thing where you have to set your data twice:
  // once as visual-only strings and second time for actual values that you can edit
  // (+ instructions for QStyledItemDelegate such as spinbox steps).
  // You have to have some mapping of SmFile fields to actual indexes of the table,
  // and for that I just set the data field into the value itself because it's more
  // convenient.
  
  {
    key_cell = new Cell("#OFFSET");
    value_cell = new Cell(QString::number(v->smfile.offset));
    DoubleField value = {&(v->smfile.offset),SmDoubleKind::Millis};
    value_cell->setData(PACK(value)); mtdt_cell->appendRow({key_cell, value_cell});
  }
  {
    key_cell = new Cell("#SAMPLESTART");
    value_cell = new Cell(QString::number(v->smfile.samplestart));
    DoubleField value = {&(v->smfile.samplestart),SmDoubleKind::Millis};
    value_cell->setData(PACK(value)); mtdt_cell->appendRow({key_cell, value_cell});
  }
  {
    key_cell = new Cell("#SAMPLELENGTH");
    value_cell = new Cell(QString::number(v->smfile.samplelength));
    DoubleField value = {&(v->smfile.samplelength),SmDoubleKind::Millis};
    value_cell->setData(PACK(value)); mtdt_cell->appendRow({key_cell, value_cell});
  }
  {
    key_cell = new Cell("#BPMS"); /*value_cell = new Cell();*/
    for (auto &pair : v->smfile.bpms) {
      auto *beat_cell = new Cell(QString::number(pair.beat_number));
      auto *bpm_cell  = new Cell(QString::number(pair.value));
      DoubleField value = {&pair.value, SmDoubleKind::Bpm};
      bpm_cell->setData(PACK(value)); key_cell->appendRow({beat_cell, bpm_cell});
    }
    mtdt_cell->appendRow(key_cell /*, value_cell */);
  }
  {
    key_cell = new Cell("#STOPS"); /*value_cell = new Cell();*/
    for (auto &pair : v->smfile.stops) {
      auto *beat_cell = new Cell(QString::number(pair.beat_number));
      auto *ms_cell   = new Cell(QString::number(pair.value)); // maybe ??????
      DoubleField value = {&pair.value, SmDoubleKind::Beats};
      ms_cell->setData(PACK(value)); key_cell->appendRow({beat_cell, ms_cell});
    }
    mtdt_cell->appendRow(key_cell/*, value_cell}*/);
  }
  this->invisibleRootItem()->appendRow(mtdt_cell);

  // ----------- diffs ---------------
  {
    auto *diffs_cell = new Cell("Difficulties");
    // diffs_cell->setColumnCount(2);

    size_t diff_i = 0;
    for (auto diff : v->smfile.diffs) {
      auto *num_cell = new Cell(QString::number(diff_i));
      num_cell->setColumnCount(1);

      auto *gt_n = new Cell("Game type");
      auto *gt_v = new Cell(cstr_from_gametype(diff.game_type));
      num_cell->appendRow({gt_n, gt_v});
      auto *ct_n = new Cell("Charter");
      auto *ct_v = new Cell(QString::fromStdString(diff.charter));
      num_cell->appendRow({ct_n, ct_v});

      auto *dt_n = new Cell("Diff type");
      auto *dt_v = new Cell(cstr_from_difftype(diff.diff_type));
      DiffType dt_ptr = diff.diff_type;
      dt_v->setData(PACK(dt_ptr));

      QBrush brush(qcolor_from_difftype(diff.diff_type));
      // QFont font; font.setBold(true); t_difftype->setFont(1, font);
      dt_v->setForeground(brush);
      num_cell->appendRow({dt_n, dt_v});

      auto *dv_n = new Cell("Diff value");
      auto *dv_v = new Cell(QString::number(diff.diff_num));
      dv_v->setData(PACK(diff.diff_num));
      num_cell->appendRow({dv_n, dv_v});
      auto *nr_n = new Cell(u"Note rows (%1)"_s.arg(diff.total_note_rows()));
      num_cell->appendRow(nr_n);
      auto *measures_n = new Cell(u"Measures (%1)"_s.arg(diff.measures.size()));
      num_cell->appendRow(measures_n);

      auto draw_noterows_for_measure = [](Cell *t_parent, Measure *me, int32_t me_i) {
        for (auto [bt_i, bt] : enumerate(me->beats)) {
          for (auto [smt_i, nr] : enumerate(bt.note_rows)) {
            if (noterow_is_zero(nr)) continue;
            
            // -- snap
            auto *t_noteline_snap = new Cell(
              u"%1/%2/<span style='color: %4; font-weight: 600;'>%3</span>"_s
              // u"%1/%2/%3"_s
              .arg(me_i).arg(bt_i).arg(smt_i)
              .arg(cstr_from_smticks(smt_i))
            );
            SmRelativePos pos = {.measures=(int32_t)me_i, .beats=(int32_t)bt_i, .smticks=(double)smt_i};
            t_noteline_snap->setData(PACK(pos));

            // -- note data
            QString line_s;
            for (auto n : nr.notes) { line_s.push_back(notetype_to_char(n)); }

            auto *t_noteline_notes = new Cell(line_s);
            t_noteline_notes->setData(PACK(nr));

            // -- alternatively, set background color:
            // QColor snap_color = qcolor_from_smticks(nl.smticks);
            // snap_color.setAlphaF(.12f); // 0 is fully transparent
            // QBrush brush(snap_color);
            // t_noteline_notes->setBackground(brush);
            t_parent->appendRow({t_noteline_snap, t_noteline_notes});
          }
        }
      };

      // -- all noterows in one pile
      for (auto [me_i, me] : enumerate(diff.measures)) {
        draw_noterows_for_measure(nr_n, &me, me_i);
      }

      // -- noterows in each measure
      for (auto [me_i, me] : enumerate(diff.measures)) {
        auto *measure_n = new Cell(u"Measure %1"_s.arg(me_i));

        // -- so I can double click on the measure cell too
        auto pos = (SmRelativePos){(int32_t)me_i, 0, 0};
        measure_n->setData(PACK(pos));

        draw_noterows_for_measure(measure_n, &me, (int32_t)me_i);
        measures_n->appendRow(measure_n);
      }
    
      diffs_cell->appendRow(num_cell);
      diff_i += 1;
    } // -- end of diff loop
    this->invisibleRootItem()->appendRow(diffs_cell);
  } // -- end of all diffs


  // -- XXX: this is a hack, maybe I should do something else
  v->tree_view->setHeaderHidden(true);
  // -- XXX: we need to memorize the expand state when we repaint all of this
  // (or this is completely stupid way to do this and you should update a specific cell?)
  v->tree_view->expandToDepth(2);
  // -- why is this so wide???
  // v->tree_view->resizeColumnToContents(0);
  v->tree_view->setColumnWidth(0, 200);
  v->tree_view->setColumnWidth(1, 200);

};


  
void NoteDisplayWidget::onCmodChange(int value) {
  auto v = this->sm_file_view;
  this->cmod = value;
  px_of_measure_zero = PX_VISUAL_OFFSET_FROM_HORIZ_LINE - px_per_smtick() * v->cur_chart_pos.total_smticks();
  this->update();
}
void NoteDisplayWidget::onDownscrollChange(Qt::CheckState state) {
  this->downscroll = state == Qt::Checked; this->update();
}





SmFileView::SmFileView(const char *path) {
  // -- parse the file
  std::string smfile_str = read_entire_file(path);
  auto smfile_opt = smfile_from_string_opt(smfile_str);

  if (std::holds_alternative<SmFile>(smfile_opt)) {
    smfile = std::get<SmFile>(smfile_opt);
  } else {
    auto err = std::get<SmParseError>(smfile_opt);
    eprintf("%s\n", err.msg.c_str());
    assert(false);
  }

  // ----------------------- build UI ------------------------------------------------
  layout_ = new QHBoxLayout(); // -- required so that it fills the entire space
  this->setLayout(layout_);

  resizable_layout = new QSplitter(Qt::Horizontal);
  layout_->addWidget(resizable_layout);

  
  // -- tree model of the whole file for a tree widget
  // (&smfile_model); // XXX: I don't know what was here before
  smfile_model = new KVTreeModel(this);
  smfile_view_delegate = new KVTreeViewDelegate(smfile_model);

  // -- tree widget

  tree_view = new QTreeView();
  tree_view->setModel(smfile_model);
  tree_view->setItemDelegate(smfile_view_delegate);

  smfile_model->push_model_to_view();
  resizable_layout->addWidget(tree_view);


  right_chunk = new QWidget();
  preview_tile = new QVBoxLayout(right_chunk);

    preview_actual = new NoteDisplayWidget();
      preview_actual->sm_file_view = this;

      preview_actual->setAutoFillBackground(true);
      auto pal = preview_actual->palette();
      pal.setColor(QPalette::Window, Qt::black);
      preview_actual->setPalette(pal);
    preview_tile->addWidget(preview_actual, 8);

    // -- this shows up
    // QLabel aaaaa;
    // aaaaa.setText("aaaaaaaaaaaaaaaaaaa");
    // preview_tile.addWidget(&aaaaa, 0);
  
    // another fake widget, maye this is not necessary
    why = new QFrame();
    this->status_bar = new StatusBar(why, this); // wrong, but let's see what happens
    preview_tile->addWidget(why, 0);
  
    // -- so I can draw status_bar.redraw() inside NoteDisplayWidget later;
    // -- spaghetti, I know, but don't have a much better idea rn
    preview_actual->status_bar = status_bar;
    status_bar->note_display_widget = preview_actual;


    // -- .show() spawns it in a new window, we don't want that

    QObject::connect(
      tree_view,
      &QTreeView::doubleClicked,
      preview_actual,
      // somehow get the item I'm clicking on. That item should store a ref
      // to actual pos. Tell NoteDisplayWidget to seek to that pos and
      // refresh itself.
      [&](const QModelIndex &index){
        if (!index.isValid()) return;
        QStandardItem *qitem = smfile_model->itemFromIndex(index);
        QVariant qvar = qitem->data();
        // ew! but works for now
        TreeValue row_var = *(TreeValue *)qvar.data();
        if (0) {
        } else if (std::holds_alternative<NoteRow>(row_var)) { // clicked on note data
          // nothing for now
        } else if (std::holds_alternative<SmRelativePos>(row_var)) { // clicked on a measure
          auto new_pos = std::get<SmRelativePos>(row_var);
          cur_chart_pos = new_pos;
        }
        status_bar->redraw();
        preview_actual->update();
      }
    );

    preview_tile->addWidget(status_bar, 0);
    preview_tile->setAlignment(status_bar, Qt::AlignCenter);
  
    preview_controls = new QHBoxLayout;

      downscroll_chk = new QCheckBox("Downscroll");
        QObject::connect(
          downscroll_chk,
          &QCheckBox::checkStateChanged,
          preview_actual,
          &NoteDisplayWidget::onDownscrollChange
        );
        downscroll_chk->setCheckState(Qt::Checked);
      preview_controls->addWidget(downscroll_chk, 6);
      preview_controls->setAlignment(downscroll_chk, Qt::AlignCenter);

      cmod_spinbox_label = new QLabel();
      preview_controls->addWidget(cmod_spinbox_label, 1);
      preview_controls->setAlignment(cmod_spinbox_label, Qt::AlignCenter);      

      cmod_spinbox = new QSpinBox();
        cmod_spinbox->setMinimum(50);
        cmod_spinbox->setMaximum(1000);
        cmod_spinbox->setValue(700);
        cmod_spinbox->setSingleStep(15);
        QObject::connect(
          cmod_spinbox,
          &QSpinBox::valueChanged,
          preview_actual,
          &NoteDisplayWidget::onCmodChange
        );
        cmod_spinbox_label->setText("CMOD");
        preview_actual->onCmodChange(cmod_spinbox->value());
      preview_controls->addWidget(cmod_spinbox, 1);
      preview_controls->setAlignment(cmod_spinbox, Qt::AlignCenter);
    preview_tile->addLayout(preview_controls, 0);

  resizable_layout->addWidget(right_chunk);
}


// -- Copied from StackOverflow, no idea. Needed if you want rich text rendering
// -- for field text where different words have different colors
void KVTreeViewDelegate::paint
(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{

  QStyleOptionViewItem option_ = option;
  initStyleOption(&option_, index);

  painter->save();

  QTextDocument doc;
  doc.setHtml(option_.text);

  option_.text = "";
  option_.widget->style()->drawControl(QStyle::CE_ItemViewItem, &option_, painter);

  painter->translate(option_.rect.left(), option_.rect.top());
  QRect clip(0, 0, option_.rect.width(), option_.rect.height());
  doc.drawContents(painter, clip);

  painter->restore();
}

QWidget* KVTreeViewDelegate::createEditor
(QWidget* parent, const QStyleOptionViewItem& /*option*/,  const QModelIndex& index) const
{
  eprintf("createEditor called | getting item from index %d\n", index.row());
  QStandardItem *item = model->itemFromIndex(index);
  eprintf("it has %d columns, and ", item->columnCount());

  assert(item);
  QVariant qdata = item->data();
  TreeValue data = qdata.value<TreeValue>();

  // another non-exhaustive dispatch
  if (0) {
  } else if (std::holds_alternative<DiffType>(data)) {
    eprintf("seems like difftype to me\n");
    QComboBox *combo = new QComboBox(parent);
    for (const char *cs : difftype_cstrs) { combo->addItem(cs); }
    return combo;
  } else if (std::holds_alternative<std::string *>(data)) {
    std::string *s = std::get<std::string *>(data);
    // in fields where there are no data, it thinks it's
    // a std::string * (but null)
    if (s == nullptr) {
      eprintf("seems like nothing to me\n"); return nullptr;
    }
    eprintf("seems like string %s to me\n", s->c_str());
    std::string str = *s;
    auto *lineEdit = new QLineEdit(parent);
    lineEdit->setText(QString::fromStdString(str));
    return lineEdit;
  } else if (std::holds_alternative<DoubleField>(data)) {
    eprintf("seems like double to me\n");
    auto ff = std::get<DoubleField>(data);
    auto *doubleSpinBox = new QDoubleSpinBox(parent);

    // TODO: I don't know how to deal with Shift here
    switch (ff.kind) {
      case SmDoubleKind::Millis:
        doubleSpinBox->setSingleStep(0.01);
        doubleSpinBox->setDecimals(3);
        doubleSpinBox->setMinimum(-50000);
        doubleSpinBox->setMaximum(100000); // -- this depends
        break;
      case SmDoubleKind::Beats:
        doubleSpinBox->setSingleStep(1);
        doubleSpinBox->setDecimals(2);
        doubleSpinBox->setMinimum(0);
        doubleSpinBox->setMaximum(100000); // -- this depends
        break;
      case SmDoubleKind::Bpm:
        doubleSpinBox->setSingleStep(1);
        doubleSpinBox->setDecimals(2);
        doubleSpinBox->setMinimum(1);     // don't care about negbpms
        doubleSpinBox->setMaximum(10000); // -- this depends
        break;
    }
    return doubleSpinBox;
  } else if (std::holds_alternative<uint32_t>(data)) {
    eprintf("seems like uint32_t to me\n");
    auto *spinBox = new QSpinBox(parent);
    // -- it wants int, UINT32_MAX would be -1
    // -- TODO: check if Etterna needs minimum 1? never seen a chart with a 0
    spinBox->setMaximum(INT32_MAX);
    return spinBox;
  } else {
    eprintf("seems like some unhandled type variant\n");
    // assert(false && "non-exhaustive variant");
    return nullptr; // for now
  }
}

struct MainWindow : public QMainWindow {
  QTabWidget w_tabs_root;

  MainWindow() {
    // -- menu bar
    QMenuBar *menuBar = this->menuBar();
    QMenu *fileMenu = menuBar->addMenu("File");

    QAction *exitAction = fileMenu->addAction("Exit");
    connect(exitAction, &QAction::triggered, this, &QWidget::close);

    // -- XXX: you still need to handle the event somehow
    w_tabs_root.setTabsClosable(true);
    w_tabs_root.setMovable(true);
    
    const char *path1 = "ext/Shannon's Theorem.sm";
    SmFileView *file1 = new SmFileView(path1);
    w_tabs_root.addTab(file1, path1);
    // const char *path2 = "ext/psychology.sm"; // -- this is multi bpm
    const char *path2 = "ext/Yatsume Ana.sm";
    SmFileView *file2 = new SmFileView(path2);
    w_tabs_root.addTab(file2, path2);

    // w_tabs_root.show();
    this->setCentralWidget(&w_tabs_root);

  }
};


// --------------------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------------------
// ----------------------------------------------- M A I N ------------------------------------------
// --------------------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------------------
__attribute__((noreturn))
void *exec_yourself(void *arg) {
  int inotify_fd = *(int *)arg;
  char dontcare[1];
  eprintf("\nI: The app will be restarted when the ./zerokara binary changes.\n");
  while(1) { // -- spam while the binary is momentarily gone
    ssize_t read_bytes = read(inotify_fd, dontcare, 1);             (void)(read_bytes);
    int minusone_on_fail = execl("./zerokara", "./zerokara", NULL); (void)(minusone_on_fail);
  }
}
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
  MainWindow window;
  window.show();

  int ret = app.exec();
  return ret;
}
