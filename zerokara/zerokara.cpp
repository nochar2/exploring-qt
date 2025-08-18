 // #define SPLITTER

// @includes -----------------------------------------------------------------------------------------------------------------

// generated from include-what-you-use
// -- Qt includes
#include "qabstractitemmodel.h"   // for QModelIndex
#include "qaction.h"              // for QAction
#include "qapplication.h"         // for QApplication
#include "qboxlayout.h"           // for QHBoxLayout, QVBoxLayout
#include "qbrush.h"               // for QBrush
#include "qbytearray.h"           // for QByteArray
#include "qchar.h"                // for QChar
#include "qcheckbox.h"            // for QCheckBox
#include "qcolor.h"               // for QColor, darkorange, Blue, DarkMagenta
#include "qcombobox.h"            // for QComboBox
#include "qdatastream.h"          // for operator<<, operator>>
#include "qdebug.h"               // for operator<<
#include "qevent.h"               // for QKeyEvent, QWheelEvent
#include "qfiledialog.h"          // for QFileDialog
#include "qflags.h"               // for QFlags
#include "qframe.h"               // for QFrame
#include "qguiapplication.h"      // for QGuiApplication
#include "qlabel.h"               // for QLabel
#include "qline.h"                // for QLineF, QLine
#include "qlineedit.h"            // for QLineEdit
#include "qlist.h"                // for QList
#include "qmainwindow.h"          // for QMainWindow
#include "qmenu.h"                // for QMenu
#include "qmenubar.h"             // for QMenuBar
#include "qmetatype.h"            // for checkTypeIsSuitableForMetaType, typ...
#include "qnamespace.h"           // for AlignmentFlag, CheckState, qt_getEn...
#include "qobject.h"              // for qobject_cast, QObject
#include "qpainter.h"             // for QPainter
#include "qpalette.h"             // for QPalette
#include "qpen.h"                 // for QPen
#include "qpoint.h"               // for QPoint
#include "qpushbutton.h"          // for QPushButton
#include "qrect.h"                // for QRectF, QRect
#include "qregularexpression.h"   // for QRegularExpression
#include "qspinbox.h"             // for QDoubleSpinBox, QSpinBox
#ifdef SPLITTER
#include "qsplitter.h"            // for QSplitter
#endif
#include "qstandarditemmodel.h"   // for QStandardItem, QStandardItemModel
#include "qstring.h"              // for QString, operator""_s, operator==
#include "qstyle.h"               // for QStyle
#include "qstyleditemdelegate.h"  // for QStyledItemDelegate
#include "qstyleoption.h"         // for QStyleOptionViewItem
#include "qtabwidget.h"           // for QTabWidget
#include "qtextdocument.h"        // for QTextDocument
#include "qtreeview.h"            // for QTreeView
#include "qvalidator.h"           // for QRegularExpressionValidator
#include "qvariant.h"             // for QVariant
#include "qwidget.h"              // for QWidget

// -- stdlib mostly
#include <QtCore/qobjectdefs.h>   // for FunctionPointer<>::ArgumentCount
#include <assert.h>               // for assert
#include <stdint.h>               // for int32_t, uint32_t, INT32_MAX
#include <stdio.h>                // for fprintf, stderr, size_t, NULL
#include <sys/types.h>            // for ssize_t
#include <algorithm>              // for max, min
#include <array>                  // for array
#include <cmath>                  // for fmod, ceil, fabs, round
#include <functional>             // for function
#include <initializer_list>       // for initializer_list
#include <string>                 // for basic_string, string
#include <tuple>                  // for tuple, get
#include <utility>                // for pair, get
#include <variant>                // for get, holds_alternative, variant, tuple
#include <vector>                 // for vector
struct DoubleField;
struct SmRelativePos;

#include "claude/exp_spinbox.h"   // for ExponentialSpinBox
#include "dumb_stdlib_linux.h"    // for Array, read_entire_file, Vector
#include "sm_parser.h"            // for Difficulty, TimeKV, SmFile, NoteRow

#include <libgen.h>               // for basename
#include <ranges>


// @using --------------------------------------------------------------------------
using std::string;
using namespace Qt::Literals::StringLiterals;
using std::ranges::views::enumerate;

// @macros ----------------------------------------------------------------------------------
#pragma GCC poison printf
#define eprintf(...)   fprintf(stderr, __VA_ARGS__)
#define eprintfln(x, ...) eprintf(x "\n" __VA_OPT__(,) __VA_ARGS__)

// make C++ variants less disgusting
#define WHEN(T,bod) if (std::holds_alternative<T>(*_vrnt)) { T _unpacked = std::get<T>(*_vrnt); bod }
#define MATCH(val) typeof(val) *_vrnt = &val;

// @forward-declarations -----------------------------------------------------------------------
struct NoteDisplayWidget;
struct State;
struct KVTreeModel;

// @shortfns
QString qs(std::string const &s) { return QString::fromStdString(s); }

// @structures ----------------------------------------------------------------------------------------------------
struct TextStatusBar : public QWidget {
  State *state;
  NoteDisplayWidget *note_display_widget;

  QHBoxLayout container;
  QLabel label_pos;
  QPushButton btn_to_reset_weird_snap;
  QLabel label_snap;

  TextStatusBar(QWidget *parent, State *state, NoteDisplayWidget *note_display_widget);
  void redraw();
};

struct NoteDisplayWidget : public QWidget {
  State *state;
  TextStatusBar *status_bar;
  KVTreeModel *model;

  NoteDisplayWidget(QWidget *parent, State *state, TextStatusBar *status_bar, KVTreeModel *model)
  : QWidget(parent)
  , state(state)
  , status_bar(status_bar)
  , model(model)
  {
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



struct NoteRowRect {
  QRectF rect; // 0 up to 4
  QColor color;
};

struct SmRelativePos {
  int32_t measures = 0;
  int32_t beats = 0;
  double smticks = 0;

  static SmRelativePos incremented_by(SmRelativePos pos, double how_many_smticks) {
    pos.smticks += how_many_smticks;
    if (pos.smticks < 0) {
      double beats_to_n_borrows = std::ceil(-(pos.smticks / 48.));
      pos.beats -= (int)beats_to_n_borrows;
      pos.smticks += 48.0 * beats_to_n_borrows;
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



QColor difftype_to_qcolor(DiffType dt)
{
  using enum DiffType;
  using namespace QColorConstants;
  switch (dt) {
    case Beginner:  return DarkCyan;
    case Easy:      return DarkGreen;
    case Medium:    return Svg::darkorange;
    case Hard:      return DarkRed;
    case Challenge: return Svg::purple;
    case Edit:      return DarkGray;
  }
  assert(false);
}


// convert "4th", "8th", "16th" ... to smticks
double smticks_in_1_(int subdiv){return 192./subdiv;};

QColor smticks_to_qcolor(double smticks) {
  using namespace QColorConstants;
  assert(smticks <= 48.0);
  if (smticks == 48.0) smticks = 0.0; // can happen due to rounding up

  std::vector<std::pair<uint32_t,QColor>> snap_to_color = {
    { smticks_in_1_(4),  Red },
    { smticks_in_1_(8),  Blue },
    { smticks_in_1_(12), Svg::lawngreen },   // less eyesore than green
    { smticks_in_1_(16), Svg::gold },        // less eyesore than yellow
    { smticks_in_1_(24), DarkMagenta },
    { smticks_in_1_(32), Svg::darkorange },  // orange and gold are too similar
    { smticks_in_1_(48), Svg::deepskyblue }, // less eyesore than cyan
  };
  for (auto [s,c] : snap_to_color) {
    if (std::fmod(smticks, s) == 0) return c;
  }
  return Gray;
}

const char *smticks_to_cstr_color(double smticks) {
  if (std::fmod(smticks, smticks_in_1_(4)) == 0) return "red";
  if (std::fmod(smticks, smticks_in_1_(8)) == 0) return "blue";
  if (std::fmod(smticks, smticks_in_1_(12)) == 0) return "green";
  if (std::fmod(smticks, smticks_in_1_(16)) == 0) return "goldenrod";
  if (std::fmod(smticks, smticks_in_1_(24)) == 0) return "magenta";
  if (std::fmod(smticks, smticks_in_1_(32)) == 0) return "orange";
  if (std::fmod(smticks, smticks_in_1_(48)) == 0) return "deepskyblue";
  return "grey";
}

bool smticks_are_sane(double smticks, int cur_snap_nths) {
  // -- false on gray notes
  // for (auto div : {1,2,3,4,6,8,12}) {
  //   if (std::fmod(smtick, 48/div) == 0.) return true;
  // }
  // return false;

  // -- false if you can no longer land on red notes
  // -- (stuff like offbeat triplets)
  return std::fmod(smticks, 192/cur_snap_nths) == 0;
}

// this is for the Snap xx highlight
const char *snap_to_cstr_color (int snap) {
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












using TimeKVs      = std::vector<TimeKV>;
using Difficulties = std::vector<Difficulty>;
enum class SmDoubleKind { Millis, Beats, Bpm };
struct DoubleField  { double *value; const SmDoubleKind kind; };
struct KVFields     { std::vector<TimeKV> *data; const SmDoubleKind kind; };
// -- pointers to actual underlying SmFile fields
using TreeValue = std::variant<
  std::string *
  , DoubleField
  , GameType *
  , DiffType *
  , uint32_t *
  , SmRelativePos  // measure/beat/smticks
  , NoteRow *      // particular note data without pos info
>;

Q_DECLARE_METATYPE(TreeValue);

struct TreeItem { QString key; TreeValue value; };

struct MainWindow;
struct KVTreeModel : public QStandardItemModel {
  // ground truth (more like in SmFile, XXX why is this not just SmFile?)
  State *state;   
  QTreeView *view;
  
  /* like why does it want a parent widget? it's not even visual, why is it its business? */
  KVTreeModel(QWidget *parent) : QStandardItemModel(parent) {};

  void rebuild_the_entire_model_from_ground_truth();

  // -- no longer needed
  // bool setItemData(const QModelIndex &index, const QMap<int,QVariant> &values) override;
  // bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
};



struct KVTreeViewDelegate : public QStyledItemDelegate {
  KVTreeModel *model;
  KVTreeViewDelegate(KVTreeModel *model) : model(model) {}

  // for HTML rendering
  void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

  QWidget* createEditor (QWidget* parent, const QStyleOptionViewItem& /*option*/,  const QModelIndex& index) const override;
  // void setEditorData(QWidget *editor, const QModelIndex &index) const override;
  void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;
};





// TODO! These widgets should be created only once. I will still new() them,
// but we definitely don't want the case where each file has its own set
// of widgets because 
struct SmFileState {
  SmFile smfile;
  std::string path;
  // these two should be definitely file specific,
  // not really diff specific
  SmRelativePos cur_chart_pos = {0};
  int cur_snap_nths = 16;
};

// ground truth data
struct State {
  std::vector<SmFileState> loaded_files;
  size_t cur_tab;
  SmFileState *cur_file_state() {return &loaded_files[cur_tab];}
};





TextStatusBar::TextStatusBar(QWidget *parent, State *state, NoteDisplayWidget *note_display_widget)
: QWidget(parent)
, state(state)
, note_display_widget(note_display_widget)
{
  auto s = this->state;

  this->container.addWidget(&this->label_pos, 0);
  this->container.addWidget(&this->btn_to_reset_weird_snap, 0);
  this->container.addWidget(&this->label_snap, 0);
  // container.setParent(this);
  this->setLayout(&this->container);
  this->setParent(parent);

  QObject::connect(
    &this->btn_to_reset_weird_snap,
    &QPushButton::clicked,
    this,
    [&](){
      s->cur_file_state()->cur_chart_pos.smticks = 0;
      /* TODO redraw a bunch of stuff */
      this->redraw();
      this->note_display_widget->update();
    }
  );
  this->redraw();
};

void TextStatusBar::redraw() {
  // XXX: this is not correct.  We want different positions for different files.
  auto f = this->state->cur_file_state();
  
  this->label_pos.setText(
    QString("Measure %1, beat %2, smtick %3")
    .arg(f->cur_chart_pos.measures)
    .arg(f->cur_chart_pos.beats)
    .arg(QString::number(f->cur_chart_pos.smticks, 'g', 4))
  );

  this->btn_to_reset_weird_snap.setText("(Reset)");
  // -- this doesn't work, and I'm doing the styleditemdelegate dance again
  // this->btn_reset_weird_snap.setText("(<span style='text-decoration: underline;'>Reset</span>)");
  this->btn_to_reset_weird_snap.setStyleSheet("margin: 0em; border: 0em; font-weight: 600;");

  // -- getters/setters were a mistake...
  // QFont font = this->btn_reset_weird_snap.font();
  // font.setPointSize(8);
  // this->btn_reset_weird_snap.setFont(font);

  // TODO: There should be a snap policy, which is either of:
  // - exact (some number of smticks)
  // - nearest note
  this->label_snap.setText(
    QString("| Snap <span style='color: %1; font-weight: 600;'>%2</span>")
    .arg(snap_to_cstr_color(f->cur_snap_nths))
    .arg(f->cur_snap_nths)
  );

  if (!smticks_are_sane(f->cur_chart_pos.smticks, f->cur_snap_nths)) {
    this->btn_to_reset_weird_snap.show();
  } else {
    this->btn_to_reset_weird_snap.hide();
  }
}

void NoteDisplayWidget::keyPressEvent(QKeyEvent *event) {
  auto f   = this->state->cur_file_state()->smfile;
  auto pos = this->state->cur_file_state()->cur_chart_pos;

  eprintfln("Key press %d", event->key());

  // -- XXX: There should be a pointer to currently active Difficulty, not 0.
  // -- That's not implemented yet.
  NoteRow *loc = &(f.diffs[0]
    .measures[pos.measures]
    .beats[pos.beats]
    .beat_rows[pos.smticks]
  );

  // -- this is a keysym, maybe a keycode for number bar keys would be nice
  auto k = event->key();
  if ('1' <= k && k <= '4') {
    using enum NoteType;
    loc->notes[k-'1'] = loc->notes[k-'1'] == None ? Tap : None;
  }

  // this->sm_file_view->update();
  // this->redraw();
  this->update();
  this->model->rebuild_the_entire_model_from_ground_truth();
}

void NoteDisplayWidget::paintEvent(QPaintEvent */*event*/) {
  auto fs = this->state->cur_file_state();
  auto f = fs->smfile;
  
  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing);

  // visually scroll to desired place
  px_of_measure_zero = PX_VISUAL_OFFSET_FROM_HORIZ_LINE - px_per_smtick() * fs->cur_chart_pos.total_smticks();

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
    SmRelativePos snap_of_this_snapline = fs->cur_chart_pos;
    auto s = &snap_of_this_snapline;
    s->measures -= 1;

    // printf("---------------\n");
    for (int i = 0; i < 300; i++) {
      double y_distance = px_of_measure_zero + s->total_smticks() * px_per_smtick();
      // printf("snap of this snapline: %d:%d:%lg, which translates to %lg px\n",
      //        s->measures, s->beats, s->smticks, y_distance);

      if (y_distance < 0.0) {
        *s = SmRelativePos::incremented_by(*s, 192.0/fs->cur_snap_nths);
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
      // QColor color = smticks_to_qcolor((int32_t)roundl(s->smticks));
      QColor color = smticks_to_qcolor(s->smticks);
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

      *s = SmRelativePos::incremented_by(*s, 192.0/fs->cur_snap_nths);
    }
  }


  // NOTE: temporarily
  using NoteRowRects = std::array<NoteRowRect, 4>;
  std::function<NoteRowRects(NoteRow,SmRelativePos)>
  noterow_to_rectangles = [&](NoteRow row, SmRelativePos pos)
  {
    using enum NoteType;
    
    // replace this if we encounter a non-4/4 file
    uint32_t global_smticks
      = (uint32_t)pos.smticks + (uint32_t)pos.beats * 48 + (uint32_t)pos.measures * 48 * 4;

    // this assumption is wrong, we want semi-transparent fakes
    NoteRowRects line;

    for (size_t i = 0; i < 4; i++) {
      int x,y,w,h;

      auto notetype_start_looks_like_tap = [](NoteType nt) {
        return nt == Tap || nt == Hold || nt == Rolld || nt == Fake;
      };

      if (notetype_start_looks_like_tap(row.notes[i])) {
        x = (int32_t)(left_start + note_width * (int32_t)i);
        y = (int32_t)(px_of_measure_zero + px_per_smtick() * global_smticks);
        w = note_width;
        h = note_height;
        line[i].rect = QRect(x,y,w,h);
        line[i].color = smticks_to_qcolor(pos.smticks);
        if (row.notes[i] == Fake) {
          line[i].color.setAlphaF(0.25);
        }
      } else {
        // TODO: other objects
        // assert(false);
      }
    }
    return line;
  };


  Vector<NoteRowRects> rectangle_rows;
  for (auto [me_i, me] : enumerate(f.diffs[0].measures)) {
    for (auto [bt_i, bt] : enumerate(me.beats)) {
      for (auto [nr_i, nr] : enumerate(bt.beat_rows)) {
        if (!noterow_is_zero(nr)) {
          auto pos = (SmRelativePos){.measures=(int32_t)me_i,.beats=(int32_t)bt_i,.smticks=(double)nr_i};
          rectangle_rows.push_back(noterow_to_rectangles(nr, pos));
        }
      }
    }
  }

  QRectF cont_rect = this->contentsRect();

  for (auto rectangle_row : rectangle_rows) {
    for (auto rectangle : rectangle_row) {
      if (downscroll) {
        // printf("cont_rect h: %d, rect_y: %g, rect_h: %g\n",
        //        cont_rect.height(), rect.y(), rect.height());
        rectangle.rect.moveBottom(cont_rect.height() - rectangle.rect.top());
        // printf("top y is now %g\n", rect.y());
      }
      painter.fillRect(rectangle.rect, rectangle.color);
    }
  }
}

// -- TODO: yes I know I rely on a single bpm for now
double NoteDisplayWidget::px_per_smtick() {
  auto f = this->state->cur_file_state()->smfile;
  double bpm = f.bpms[0].value;
  double secs_per_beat = 60./bpm;
  double secs_per_smtick = secs_per_beat / 48.;
  return secs_per_smtick * cmod;
}
double NoteDisplayWidget::px_per_current_snap() {
  auto fs = this->state->cur_file_state();
  return 192.0 / fs->cur_snap_nths * px_per_smtick();
}
  /// On mouse wheel scroll:
  /// no modifiers -> scroll,
  /// ctrl -> change snap,
  /// ctrl+shift -> change snap (fine)
void NoteDisplayWidget::wheelEvent(QWheelEvent *event) {
  auto fs = this->state->cur_file_state();
  auto modifiers = QGuiApplication::keyboardModifiers();
  SmRelativePos new_pos = fs->cur_chart_pos;


  // printf("pos before: %d %d %lf\n",
  //        cur_chart_pos.measures, cur_chart_pos.beats, cur_chart_pos.smticks);

  if (modifiers & Qt::ControlModifier) { // change snap
    if (event->angleDelta().ry() < 0) {
      fs->cur_snap_nths =
        (modifiers & Qt::ShiftModifier)
        ? std::max(fs->cur_snap_nths-1, 1)
        : sm_sane_snap_lower_than(fs->cur_snap_nths)
      ;
    } else {
      fs->cur_snap_nths =
        (modifiers & Qt::ShiftModifier)
        ? std::min(fs->cur_snap_nths+1, 192)
        : sm_sane_snap_higher_than(fs->cur_snap_nths)
      ;
    }
  } else { // move
    if ((event->angleDelta().ry() < 0) ^ !downscroll) {
      new_pos = SmRelativePos::incremented_by(fs->cur_chart_pos, -smticks_in_1_(fs->cur_snap_nths));
    } else {
      new_pos = SmRelativePos::incremented_by(fs->cur_chart_pos, +smticks_in_1_(fs->cur_snap_nths));
    }
  }
  // printf("pos after: %d %d %lg\n", new_pos.measures, new_pos.beats, new_pos.smticks);
  fs->cur_chart_pos = (new_pos.measures < 0) ? (SmRelativePos){0} : new_pos;

  // printf("raw smticks: %d\n", chart_pos.raw_smticks());
  // 
  // -- FIXMEEEEEEEEEEEEEEEE: we need to get the pointer to status_bar from somewhere,
  // -- but that reeks with spaghetti. Or I can store a list of magical function pointers
  // -- somewhere. I don't know of a good idea to resolve this
  status_bar->redraw();
  this->update();
  // positionChanged(cur_chart_pos, cur_snap_nths);
};



// class Cell : public QStandardItem {
//   public:
//   Cell() : QStandardItem() {}
//   Cell(QString str) : QStandardItem(str) { };
// };
#define Cell QStandardItem


void KVTreeModel::rebuild_the_entire_model_from_ground_truth() {
  // -- I construct a Cell (QStandardItem) the following way:
  // -- * the value in constructor is merely a visual string
  // -- * the data is a pointer to actual ground truth (or wrapper with extra info)
  auto f = this->state->cur_file_state()->smfile;

  this->clear();
  
  this->setColumnCount(2);

  // -- T <-> std::variant <-> QVariant allows you to smuggle a pointer,
  // -- which QVariant doesn't like for some reason.
  #define PACK(x)   QVariant::fromValue(TreeValue(x))
  #define UNPACK(x) x.value<TreeValue>()

  // -- process the string fields separately beacause they are
  // -- annoying and there's many of them. Other types like floats
  // -- vary too much for abstraction to be useful.
  std::vector<std::tuple<const char *,string *>> string_fields;
  string_fields.push_back({"#TITLE",     &f.title});
  string_fields.push_back({"#SUBTITLE",  &f.subtitle});
  string_fields.push_back({"#ARTIST",    &f.artist});
  // -- TODO: some way to say "I want to also consider these translit fields".
  // -- Maybe a right-click menu on Metadata.
  if (f.has_translit) {
    string_fields.push_back({"#TITLETRANSLIT",     &f.titletranslit});
    string_fields.push_back({"#SUBTITLETRANSLIT",  &f.subtitletranslit});
    string_fields.push_back({"#ARTISTTRANSLIT",    &f.artisttranslit});
  }
  string_fields.push_back({"#CREDIT",  &f.credit});
  string_fields.push_back({"#MUSIC",   &f.music});

  Cell *mtdt_cell = new Cell("Metadata");
  Cell *key_cell;
  Cell *value_cell;

  for (auto [key,p_value] : string_fields) {
    key_cell = new Cell(key);
    value_cell = new Cell(qs(*p_value)); value_cell->setData(PACK(p_value));
    mtdt_cell->appendRow({key_cell, value_cell});
  }
  
  // NOTE: Numeric values are not just a pointer to data. I also store the information
  // of what type of field it is so I can decide on more reasonable step amounts for QSpinBoxes.
  {
    key_cell = new Cell("#OFFSET");
    value_cell = new Cell(QString::number(f.offset));
    DoubleField value = {&(f.offset),SmDoubleKind::Millis};
    value_cell->setData(PACK(value)); mtdt_cell->appendRow({key_cell, value_cell});
  }
  {
    key_cell = new Cell("#SAMPLESTART");
    value_cell = new Cell(QString::number(f.samplestart));
    DoubleField value = {&(f.samplestart),SmDoubleKind::Millis};
    value_cell->setData(PACK(value));
    mtdt_cell->appendRow({key_cell, value_cell});
  }
  {
    key_cell = new Cell("#SAMPLELENGTH");
    value_cell = new Cell(QString::number(f.samplelength));
    DoubleField value = {&(f.samplelength),SmDoubleKind::Millis};
    value_cell->setData(PACK(value)); mtdt_cell->appendRow({key_cell, value_cell});
  }
  {
    key_cell = new Cell("#BPMS"); /*value_cell = new Cell();*/
    for (auto &pair : f.bpms) {
      auto *beat_cell = new Cell(QString::number(pair.beat_number));
      auto *bpm_cell  = new Cell(QString::number(pair.value));
      DoubleField value = {&pair.value, SmDoubleKind::Bpm};
      bpm_cell->setData(PACK(value));
      key_cell->appendRow({beat_cell, bpm_cell});
    }
    mtdt_cell->appendRow(key_cell /*, value_cell */);
  }
  {
    key_cell = new Cell("#STOPS"); /*value_cell = new Cell();*/
    for (auto &pair : f.stops) {
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
    for (auto &diff : f.diffs) {
      auto *num_cell = new Cell(QString::number(diff_i));
      num_cell->setColumnCount(1);

      auto *gt_n = new Cell("Game type");
      QString gt_str = u"%1 (%2 keys)"_s
        .arg(gametype_to_cstr(diff.game_type))
        .arg(gametype_to_keycount(diff.game_type))
      ;
      auto *gt_v = new Cell(gt_str);
      gt_v->setData(PACK(&diff.game_type));
      num_cell->appendRow({gt_n, gt_v});
      auto *ct_n = new Cell("Charter");
      auto *ct_v = new Cell(qs(diff.charter));
      ct_v->setData(PACK(&diff.charter));
      num_cell->appendRow({ct_n, ct_v});

      auto *dt_n = new Cell("Diff type");
      // eprintfln("I'm storing this value: %d, as a pointer %p",
      //   (int)diff.diff_type,
      //   &diff.diff_type);
      auto *dt_v = new Cell(difftype_to_cstr(diff.diff_type)); dt_v->setData(PACK(&diff.diff_type));

      QBrush brush(difftype_to_qcolor(diff.diff_type));
      // QFont font; font.setBold(true); t_difftype->setFont(1, font);
      dt_v->setForeground(brush);
      num_cell->appendRow({dt_n, dt_v});

      auto *dv_n = new Cell("Diff value");
      auto *dv_v = new Cell(QString::number(diff.diff_num)); dv_v->setData(PACK(&diff.diff_num));
      num_cell->appendRow({dv_n, dv_v});
      auto *nr_n = new Cell(u"Note rows (%1)"_s.arg(diff.total_note_rows()));
      num_cell->appendRow(nr_n);
      auto *measures_n = new Cell(u"Measures (%1)"_s.arg(diff.measures.size()));
      num_cell->appendRow(measures_n);

      auto draw_noterows_for_measure = [](Cell *t_parent, Measure *me, int32_t me_i) {
        // -- I don't know how to use enumerate() here. C for loop it is.
        for (size_t bt_i = 0; bt_i < me->beats.size(); bt_i++) {
          Beat *bt = &me->beats[bt_i];
          for (size_t smt_i = 0; smt_i < bt->beat_rows.size(); smt_i++) {
            NoteRow *nr = &bt->beat_rows[smt_i];
            
            if (noterow_is_zero(*nr)) continue;
            
            // -- snap
            auto *t_noteline_snap = new Cell(
              u"%1/%2/<span style='color: %4; font-weight: 600;'>%3</span>"_s
              // u"%1/%2/%3"_s
              .arg(me_i).arg(bt_i).arg(smt_i)
              .arg(smticks_to_cstr_color(smt_i))
            );
            SmRelativePos pos = {.measures=(int32_t)me_i, .beats=(int32_t)bt_i, .smticks=(double)smt_i};
            t_noteline_snap->setData(PACK(pos));

            // -- note data
            QString line_visual_string;
            for (auto n : nr->notes) { line_visual_string.push_back(notetype_to_char(n)); }
            auto *t_noteline_notes = new Cell(line_visual_string);
            t_noteline_notes->setData(PACK(nr));

            // -- alternatively, set background color:
            // QColor snap_color = smticks_to_qcolor(nl.smticks);
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


#if 0
  // -- XXX: this is a hack, maybe I should do something else
  this->tree_view->setHeaderHidden(true);
  // -- XXX: we need to memorize the expand state when we repaint all of this
  // (or this is completely stupid way to do this and you should update a specific cell?)
  this->widgets->tree_view->expandToDepth(2);
  // -- why is this so wide???
  // f->tree_view->resizeColumnToContents(0);
  this->tree_view->setColumnWidth(0, 200);
  this->tree_view->setColumnWidth(1, 200);
  this->tree_view->setExpandsOnDoubleClick(true);
#endif

};


  
void NoteDisplayWidget::onCmodChange(int value) {
  auto fs = this->state->cur_file_state();
  this->cmod = value;
  this->px_of_measure_zero = PX_VISUAL_OFFSET_FROM_HORIZ_LINE - px_per_smtick() * fs->cur_chart_pos.total_smticks();
  this->update();
}
void NoteDisplayWidget::onDownscrollChange(Qt::CheckState state) {
  this->downscroll = state == Qt::Checked; this->update();
}


std::variant<SmFile,SmParseError> 
filepath_to_smfile_opt(const char *path) {
  std::string smfile_str = read_entire_file(path);
  auto smfile_opt = string_to_smfile_opt(smfile_str);
  return smfile_opt;
}

/*
SmFileState::SmFileState(const char *path) {
  // -- parse the file
  std::string smfile_str = read_entire_file(path);
  auto smfile_opt = string_to_smfile_opt(smfile_str);

  MATCH (smfile_opt) {
    WHEN (SmFile,
      smfile = _unpacked;
    )
    else WHEN (SmParseError, 
      eprintfln("%s", _unpacked.msg.c_str());
      assert(false);
    )
    else assert(false && "unhandled variant");
  }
}  
*/



void KVTreeViewDelegate::paint
(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
  // eprintf("In paint(), ");

  QStandardItem *item = model->itemFromIndex(index);

  QStyleOptionViewItem option_ = option;
  initStyleOption(&option_, index);
  QVariant qdata = item->data();
  TreeValue data = UNPACK(qdata);

  MATCH (data) {
    WHEN (SmRelativePos, {
      (void)_unpacked;
      // -- Copied from StackOverflow, no idea. Needed if you want rich text rendering
      // -- for field text where different words have different colors
      painter->save();

      QTextDocument doc;
      doc.setHtml(option_.text);

      option_.text = "";
      option_.widget->style()->drawControl(QStyle::CE_ItemViewItem, &option_, painter);

      painter->translate(option_.rect.left(), option_.rect.top());
      QRect clip(0, 0, option_.rect.width(), option_.rect.height());
      doc.drawContents(painter, clip);

      painter->restore();
    })
    else WHEN (DiffType *, {
      // -- draw DiffType colored
      DiffType *dt = _unpacked;
    
      painter->save();
      QPen pen(difftype_to_qcolor(*dt));
      painter->setPen(pen);
      QStyledItemDelegate::paint(painter, option, index);
      painter->restore();
    })
    else {
      // eprintfln("we're painting something else");
      // -- we don't want to render everything as html, the subtitle of Yatsume Ana is <!>
      // -- which would be an html fragment (and you can't easily escape this)
      QStyledItemDelegate::paint(painter, option, index);
    }
  }
}

struct MyValidator : public QRegularExpressionValidator {
  explicit MyValidator(const QRegularExpression &re, QObject *parent = nullptr)
  : QRegularExpressionValidator(re, parent) {}
  State validate(QString &str, int &pos) const override {
    str = str.toUpper();
    return QRegularExpressionValidator::validate(str, pos);
  }
};


// -- xxx: I thought this works??? why does it not?
Q_DECLARE_METATYPE(GameType);
Q_DECLARE_METATYPE(DiffType);
Q_DECLARE_METATYPE(NoteType);

QWidget* KVTreeViewDelegate::createEditor
(QWidget* parent, const QStyleOptionViewItem& /*option*/,  const QModelIndex& index) const
{
  eprintfln("createEditor called | getting item from index %d", index.row());
  QStandardItem *item = model->itemFromIndex(index);
  eprintf("it has %d columns, and ", item->columnCount());

  assert(item);
  QVariant qdata = item->data();
  TreeValue data = UNPACK(qdata);

  MATCH (data) {
    if (0) {}
    else WHEN (GameType *,
      eprintfln("... seems like gametype to me");
      QComboBox *combo = new QComboBox(parent);
      for (GameType gt : gametypes) {
        QString str = u"%1 (%2 keys)"_s
          .arg(gametype_to_cstr(gt))
          .arg(gametype_to_keycount(gt))
        ;
        combo->addItem(str, QVariant::fromValue(gt));
      }
      combo->setCurrentIndex((int)*_unpacked);
      return combo;
    )
    else WHEN (DiffType *,
      eprintfln("... seems like difftype to me");
      QComboBox *combo = new QComboBox(parent);
      for (DiffType dt : difftypes) {
        const char *cstr = difftype_to_cstr(dt);
        combo->addItem(QString(cstr), QVariant::fromValue(dt));
      }
      combo->setCurrentIndex((int)*_unpacked);
      return combo;
    )
    else WHEN(std::string *, 
      // in some fields there are no data, it thinks it's a std::string * (but null)
      if (_unpacked == nullptr) {
        eprintfln("... seems like nothing to me"); return nullptr;
      }
      eprintfln("seems like string %s to me", _unpacked->c_str());
      std::string str = *_unpacked;
      auto *lineEdit = new QLineEdit(parent);
      lineEdit->setText(qs(str));
      return lineEdit;
    )
    else WHEN (DoubleField, 
      eprintfln("seems like double to me");
      auto *doubleSpinBox = new QDoubleSpinBox(parent);

      // TODO: I don't know how to deal with Shift here
      switch (_unpacked.kind) {
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
    )
    else WHEN(uint32_t *, 
      (void)_unpacked;
      eprintfln("seems like uint32_t to me");

      auto *spinBox = new QSpinBox(parent);
      // -- it wants int, UINT32_MAX would be -1
      // -- TODO: check if Etterna needs minimum 1? never seen a chart with a 0
      spinBox->setMaximum(INT32_MAX);
      return spinBox;
    )
    // -- TODO row editing
    else WHEN(NoteRow *,
      (void)_unpacked;
      eprintfln("seems like a noterow to me");

      auto *editor = new QLineEdit(parent);
      editor->setMaxLength(4);

      QString rx_s = u"[01234MLF]{%1}"_s.arg(4);
      QRegularExpression rx(rx_s);
      auto *validator = new MyValidator(rx, nullptr);
      // TODO: it treats 0 as nothing. What if I want to delete to zero
      // and still keep the zeroes, instead of it pretending that they
      // don't count?
      // editor->setInputMask(">NNNN;_");
      editor->setValidator(validator);
      return editor;
    )
    else {
      eprintfln("seems like some unhandled type variant");
      // assert(false && "non-exhaustive variant");
      return nullptr; // for now
    }
  } // MATCH
  assert(false);
}


NoteRow qstr_to_noterow(const QString &str)
{
  eprintfln("length is %lld", str.size());
  if (str.size() != 4) assert(false);
  NoteRow nr;
  for (auto [i,c] : enumerate(str)) {
    nr.notes[i] = char_to_notetype((char)c.unicode());
  }
  return nr;
}


void KVTreeViewDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const {
  (void)model;
  eprintfln("in setModelData()");
  QStandardItem *item = this->model->itemFromIndex(index);
  QVariant target_qv = item->data();
  TreeValue target_v = target_qv.value<TreeValue>();

  MATCH(target_v) {
    if (0) {}
    else WHEN(std::string *, {
      QLineEdit *lineEdit = qobject_cast<QLineEdit *>(editor);
      *_unpacked = lineEdit->text().toStdString();
    })
    else WHEN(GameType *, {
      QComboBox *comboBox = qobject_cast<QComboBox *>(editor);
      GameType edit_result =  comboBox->itemData(comboBox->currentIndex()).value<GameType>();
      *_unpacked = edit_result;
    })
    else WHEN(DiffType *,
      QComboBox *comboBox = qobject_cast<QComboBox *>(editor);
      DiffType edit_result = comboBox->itemData(comboBox->currentIndex()).value<DiffType>();
      *_unpacked = edit_result;
    )
    else WHEN(DoubleField,
      QDoubleSpinBox *spinBox = qobject_cast<QDoubleSpinBox *>(editor);
      *(_unpacked.value) = spinBox->value();
    )
    else WHEN(uint32_t *,
      QSpinBox *spinBox = qobject_cast<QSpinBox *>(editor);
      *_unpacked = spinBox->value();
    )
    else WHEN(NoteRow *,
      QLineEdit *lineEdit = qobject_cast<QLineEdit *>(editor);
      NoteRow nr = qstr_to_noterow(lineEdit->text());
      *_unpacked = nr;
    )
    else {
      eprintfln("Warning: unhandled edit type, the new value will not be set");
    }
  }
  this->model->rebuild_the_entire_model_from_ground_truth();
}


struct MainWindow : public QMainWindow {
  State state;
 
  QTabBar *tab_bar;

  KVTreeModel        *tree_model;
  QTreeView          *tree_view;
  KVTreeViewDelegate *tree_view_delegate;

#ifdef SPLITTER
  QHBoxLayout *root_layout_just_to_fill_entire_space;
    QSplitter *resizable_layout;
#else
  QWidget *root;
  QHBoxLayout *root_hlayout;
#endif
      QWidget *left_button_stripe;
      QWidget *right_column;
        // QVBoxLayout *preview_tile;
          QFrame *why; // useless wrapper around status bar (maybe unnecessary)
            TextStatusBar *status_bar;
          NoteDisplayWidget *preview_actual;
          QWidget *preview_controls;
            QCheckBox *downscroll_chk;
            QLabel *cmod_spinbox_label;
            ExponentialSpinBox *cmod_spinbox;



  void load_file_unchecked_for_now(const char *path) {
    // XXX: this constructor is a bit dubious but I'll keep it for now
    auto smfile_opt = filepath_to_smfile_opt(path);
    // TODO: if fail, write error on statusbar at bottom of window instead of crashing
    auto smfile = std::get<SmFile>(smfile_opt);

    const char *base_path = basename((char *)path);
    this->tab_bar->addTab(qs(smfile.title));
    this->tab_bar->setTabToolTip(tab_bar->count()-1, qs(base_path));
    SmFileState state;
    state.smfile = smfile;
    state.path = std::string(path);
    this->state.loaded_files.push_back(state);
  };


  MainWindow() {

    eprintfln("Building up the widget hierarchy..."); 
    {
    #ifdef SPLITTER
      this->root = new QWidget(this);
      // just trying to make it span the window width
      // also this is mostly broken
      this->root_hlayout_dummy = new QHBoxLayout();
      this->root->setLayout(root_hlayout_dummy);

      this->splitter = new QSplitter(Qt::Horizontal);
      this->splitter->setHandleWidth(6);
      this->root_hlayout_dummy->addWidget(resizable_layout);
    #else
      this->root = new QWidget(this);
      this->root_hlayout = new QHBoxLayout();
      this->root->setLayout(root_hlayout);
    #endif

      eprintfln("Column 1...");
      this->left_button_stripe = new QWidget(root);
      {
        auto vbox = new QVBoxLayout();
        this->left_button_stripe->setLayout(vbox);
        auto b1 = new QPushButton("B1"); this->left_button_stripe->layout()->addWidget(b1);
        auto b2 = new QPushButton("B2"); this->left_button_stripe->layout()->addWidget(b2);
        auto b3 = new QPushButton("B3"); this->left_button_stripe->layout()->addWidget(b3);
      }

      eprintfln("Column 2...");
      this->tree_view = new QTreeView(root);
      {
        #ifdef SPLITTER
          resizable_layout->addWidget(tree_view);
        #else
          root_hlayout->addWidget(tree_view);
          root_hlayout->setStretchFactor(tree_view, 1);
        #endif
      }

      eprintfln("Column 3...");
      this->right_column = new QWidget(root);
      {
        auto vbox = new QVBoxLayout(); this->right_column->setLayout(vbox);


        eprintfln("Tab bar...");
        {
          this->tab_bar = new QTabBar(this->right_column);
          vbox->addWidget(tab_bar);
          QWidget::connect(
            tab_bar, &QTabBar::tabCloseRequested,
            this, [&](int index){ this->tab_bar->removeTab(index); /* TODO: prompt to save */ }
          );
          // -- XXX: these are unhandled, and it should be a tabwidget for fucks sake (at least for now)
          // tab_bar->setTabsClosable(true);
          // tab_bar->setMovable(true);
          // tab_bar->setStyleSheet("QTabBar::tab {max-width: 100px;}");
          // tab_bar->setElideMode(Qt::ElideRight);
        }

        eprintfln("Actual preview...");
        {
          this->preview_actual = new NoteDisplayWidget(
            this->right_column,
            &this->state,
            this->status_bar,
            this->tree_model // NOTE: this is still NULL at this moment!!!
          );
          {
            auto pal = preview_actual->palette();
            pal.setColor(QPalette::Window, Qt::black);
            this->preview_actual->setPalette(pal);
            this->preview_actual->setAutoFillBackground(true);
          }
          vbox->addWidget(this->preview_actual, 8);
        }

        eprintfln("Status bar text...");
        {
          // another fake widget, maye this is not necessary
          this->why        = new QFrame(this->right_column);
          this->status_bar = new TextStatusBar(this->why, &this->state, this->preview_actual);
          // CHK: there was an additional add of the inner widget, check this again
          // it might be broken
          vbox->addWidget(this->why, 0);
          vbox->setAlignment(this->why, Qt::AlignCenter);

          // -- so I can draw status_bar.redraw() inside NoteDisplayWidget later;
          // -- spaghetti, I know, but don't have a much better idea because of
          // -- how everything has to have pointers to other stuff
          this->preview_actual->status_bar = this->status_bar;
          this->status_bar->note_display_widget = this->preview_actual;
        }

        eprintfln("Preview controls...");
        {
          this->preview_controls = new QWidget(this->right_column);
          auto hbox = new QHBoxLayout();
          this->preview_controls->setLayout(hbox);

          
          eprintfln("Checkbox...");
          {
            this->downscroll_chk = new QCheckBox("Downscroll");
            QObject::connect(
              this->downscroll_chk,
              &QCheckBox::checkStateChanged,
              this->preview_actual,
              &NoteDisplayWidget::onDownscrollChange
            );
            this->downscroll_chk->setCheckState(Qt::Checked);

            hbox->addWidget(this->downscroll_chk, 6);
            hbox->setAlignment(this->downscroll_chk, Qt::AlignLeft);
          }

          eprintfln("CMOD spinbox...");
          {
            cmod_spinbox_label = new QLabel("CMOD");
            hbox->addWidget(cmod_spinbox_label, 1);
            hbox->setAlignment(cmod_spinbox_label, Qt::AlignCenter);
            this->cmod_spinbox = new ExponentialSpinBox();
            this->cmod_spinbox->setDecimals(0);
            this->cmod_spinbox->setMinimum(50);
            this->cmod_spinbox->setMaximum(1000);
            this->cmod_spinbox->setValue(700);
            this->cmod_spinbox->setExponentialFactor(1.05);
            QObject::connect(
              this->cmod_spinbox,
              &ExponentialSpinBox::valueChanged,
              this->preview_actual,
              &NoteDisplayWidget::onCmodChange
            );
            this->preview_actual->onCmodChange(cmod_spinbox->value());
            hbox->addWidget(cmod_spinbox, 1);
            hbox->setAlignment(cmod_spinbox, Qt::AlignCenter);
          }
          vbox->addWidget(this->preview_controls, 0);
          
        }

        // UGHHHHHHHHHHHHHHHHHHHHHHHHHHHHHH
      #ifdef SPLITTER
        this->resizable_layout->addWidget(right_chunk);
      #else
        this->root_hlayout->addWidget(right_column);
        this->root_hlayout->setStretchFactor(right_column, 2);
      #endif
      }

      eprintfln("Menubar...");
      {
        QMenuBar *menuBar = this->menuBar();

        eprintfln("File menu...");
        {
          QMenu *fileMenu = menuBar->addMenu("File");
          QAction *openAction = fileMenu->addAction("Open");
          connect(openAction, &QAction::triggered, this, [&](){
            QString fileName = QFileDialog::getOpenFileName(
              this,
              "Open File",
              nullptr, // current dir
              "Stepmania files (*.sm)"
            );
            if (fileName != nullptr) {
              std::string std_fileName = fileName.toStdString();
              const char *c_fileName = std_fileName.c_str();
              eprintfln("chosen file name: %s", c_fileName);
              load_file_unchecked_for_now(c_fileName);
            }
          });
          QAction *exitAction = fileMenu->addAction("Exit");
          connect(exitAction, &QAction::triggered, this, &QWidget::close);
        }

        // w_tabs_root.show();
        // this->setCentralWidget(&w_tabs_root);
      }
    }


    eprintfln("Loading files..."); 
    {
      load_file_unchecked_for_now("ext/Shannon's Theorem.sm");
      load_file_unchecked_for_now("ext/Yatsume Ana.sm");
      // load_file_unchecked_for_now("ext/psychology.sm"); // -- this is multi bpm, also boo find a more interesting file
    }


    eprintfln("Setting up model/view things..."); 
    {
      this->tree_model = new KVTreeModel(nullptr);
      this->tree_model->state = &this->state;
      this->tree_model->view  = this->tree_view;
      this->tree_view_delegate = new KVTreeViewDelegate(tree_model);

      this->tree_view->setModel(tree_model);
      this->tree_view->setItemDelegate(tree_view_delegate);

      this->tree_model->rebuild_the_entire_model_from_ground_truth();
    } 


    eprintfln("Wiring up callbacks...");
    {
      QObject::connect(
        this->tree_view,
        &QTreeView::doubleClicked,
        this->preview_actual,

        [&](const QModelIndex &index){
          if (!index.isValid()) return;
          QStandardItem *qitem = this->tree_model->itemFromIndex(index);
          QVariant qvar = qitem->data();
          TreeValue row_var = UNPACK(qvar);

          // If a row I'm double clicking on is a line with measure info,
          // jump to the position which I stored there
          MATCH (row_var) {
            WHEN(SmRelativePos,
              this->state.cur_file_state()->cur_chart_pos = _unpacked;
            )
          }      
          // And refresh itself (XXX hacky for now, there should be a unified way)
          status_bar->redraw();
          preview_actual->update();
        }
      );
    }

    // ----------------------- @build_ui ------------------------------------------------
  }
};


// --------------------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------------------
// ----------------------------------------------- M A I N ------------------------------------------
// --------------------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------------------

int main(int argc, char **argv) {
  // -- make float parsing not break in Czech locale
  // -- XXX: why does this not work? For now, I'll set it nearby float parsing.
  // QLocale locale("C");
  // QLocale::setDefault(locale);

void run_auto_restarter(int argc, char **argv);

  // -- QGuiApplication doesn't work if you want widgets
  QApplication app(argc, argv);
  MainWindow window;
  window.show();

  int ret = app.exec();
  return ret;
}
