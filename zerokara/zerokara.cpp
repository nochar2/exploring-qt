#include <fstream>
#include <string>
using std::string;

// reload hacks
#include <sys/inotify.h>
#include <unistd.h>

#include "sm_parser.h"
#include "qt_includes.h"

#include "dumb_stdlib_linux.h"


void __please(){suppress_the_spurious_include_warning=0;};

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

SmFile smfile;

using namespace Qt::Literals::StringLiterals;

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
    SmRelativePos new_pos = cur_chart_pos;

    // printf("pos before: %d %d %lf\n",
    //        cur_chart_pos.measures, cur_chart_pos.beats, cur_chart_pos.smticks);

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
    // printf("pos after: %d %d %lg\n", new_pos.measures, new_pos.beats, new_pos.smticks);
    cur_chart_pos = (new_pos.measures < 0) ? (SmRelativePos){0} : new_pos;

    // printf("raw smticks: %d\n", chart_pos.raw_smticks());
    this->update();
    emit positionChanged(cur_chart_pos, current_snap_nths);
  };

  void paintEvent(QPaintEvent */*event*/) override {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // visually scroll to desired place
    px_of_measure_zero = PX_VISUAL_OFFSET_FROM_HORIZ_LINE - px_per_smtick() * cur_chart_pos.total_smticks();

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
    Vector<NoteRow> note_rows = smfile.diffs[0].note_rows();

    Vector<NoteRowRects> rectangles;
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
  printf("\nI: The app will be restarted when the ./zerokara binary changes.\n");
  while(1) { // -- spam while the binary is momentarily gone
    ssize_t read_bytes = read(inotify_fd, dontcare, 1);             (void)(read_bytes);
    int minusone_on_fail = execl("./zerokara", "./zerokara", NULL); (void)(minusone_on_fail);
  }
}



// -- sadly this has to be out here in global scope
// Q_DECLARE_METATYPE(TreeValue);

class Cell : public QStandardItem {
  public:
  // -- just for setEditable(false) and less typing
  Cell() : QStandardItem() {
  }
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
  // , std::vector<Difficulty> *
  , DiffType
  , uint32_t
  , SmRelativePos // measure
  , NoteRow       // particular noterow
>;

Q_DECLARE_METATYPE(TreeValue);

struct TreeItem { QString key; TreeValue value; };

struct KVTreeModel : public QStandardItemModel {

  Q_OBJECT
public:

  // so, idk, this should be modified on change but idk how to pass the pointers
  // through hell and back and contorted through QVariants which don't like double * for some reason
  SmFile &smfile;

  KVTreeModel(SmFile &smfile) : QStandardItemModel(), smfile(smfile) {
    redraw_yourself();
  }

  // XXXXXXXXXXXXXXXXX: this really should not be here I believe
  void redraw_yourself() {
    clear();
    
    this->setColumnCount(2);


    // smuggle a pointer in a QVariant
    #define PACK(x) QVariant::fromValue(TreeValue(x))

    // let's try dealing with the string fields separately beacause they are
    // annoying and there's many of them. Other types like floats vary too much
    // for abstraction to be useful.
    std::vector<std::tuple<const char *,string>> string_fields;
    string_fields.push_back({"#TITLE",  smfile.title});
    string_fields.push_back({"#MUSIC",  smfile.music});
    string_fields.push_back({"#ARTIST", smfile.artist});

    Cell *mtdt_cell = new Cell("Metadata");
    Cell *key_cell;
    Cell *value_cell;

    for (auto [key,value] : string_fields) {
      key_cell = new Cell(key);
      value_cell = new Cell(QString::fromStdString(value));
      value_cell->setData(PACK(&value));
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
      value_cell = new Cell(QString::number(smfile.offset));
      DoubleField value = {&(smfile.offset),SmDoubleKind::Millis};
      value_cell->setData(PACK(value)); mtdt_cell->appendRow({key_cell, value_cell});
    }
    {
      key_cell = new Cell("#SAMPLESTART");
      value_cell = new Cell(QString::number(smfile.samplestart));
      DoubleField value = {&(smfile.samplestart),SmDoubleKind::Millis};
      value_cell->setData(PACK(value)); mtdt_cell->appendRow({key_cell, value_cell});
    }
    {
      key_cell = new Cell("#SAMPLELENGTH");
      value_cell = new Cell(QString::number(smfile.samplelength));
      DoubleField value = {&(smfile.samplelength),SmDoubleKind::Millis};
      value_cell->setData(PACK(value)); mtdt_cell->appendRow({key_cell, value_cell});
    }
    {
      key_cell = new Cell("#BPMS"); /*value_cell = new Cell();*/
      for (auto &pair : smfile.bpms) {
        auto *beat_cell = new Cell(QString::number(pair.beat_number));
        auto *bpm_cell  = new Cell(QString::number(pair.value));
        DoubleField value = {&pair.value, SmDoubleKind::Bpm};
        bpm_cell->setData(PACK(value)); key_cell->appendRow({beat_cell, bpm_cell});
      }
      mtdt_cell->appendRow(key_cell /*, value_cell */);
    }
    {
      key_cell = new Cell("#STOPS"); /*value_cell = new Cell();*/
      for (auto &pair : smfile.stops) {
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
      for (auto diff : smfile.diffs) {
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
        dv_v->setData(diff.diff_num);
        num_cell->appendRow({dv_n, dv_v});
        auto *nr_n = new Cell(u"Note rows (%1)"_s.arg(diff.total_note_rows()));
        num_cell->appendRow(nr_n);
        auto *measures_n = new Cell(u"Measures (%1)"_s.arg(diff.measures.size()));
        num_cell->appendRow(measures_n);

        auto draw_note_rows = [](Cell *t_parent, const Vector<NoteRow>& note_rows) {
          for (auto &nl : note_rows) {
            auto *t_noteline_snap = new Cell(
              u"%1/%2/<span style='color: %4; font-weight: 600;'>%3</span>"_s
              // u"%1/%2/%3"_s
              .arg(nl.measure).arg(nl.beat).arg(nl.smticks)
              .arg(cstr_from_smticks(nl.smticks))
            );
            QString line_s;
            for (auto n : nl.notes) { line_s.push_back((char)n); }
            auto *t_noteline_notes = new Cell(line_s);
            t_noteline_snap->setData(PACK(nl));
            t_noteline_notes->setData(PACK(nl));

            // -- alternatively, set background color:
            // QColor snap_color = qcolor_from_smticks(nl.smticks);
            // snap_color.setAlphaF(.12f); // 0 is fully transparent
            // QBrush brush(snap_color);
            // t_noteline_notes->setBackground(brush);
            t_parent->appendRow({t_noteline_snap, t_noteline_notes});
          }
        };
        draw_note_rows(nr_n, diff.note_rows());
        int32_t mr_i = 0;
        for (auto m : diff.measures) {
          auto *measure_n = new Cell(u"Measure %1"_s.arg(mr_i));
          auto pos = (SmRelativePos){mr_i, 0, 0};
          measure_n->setData(PACK(pos));
          draw_note_rows(measure_n, m.note_rows);
          measures_n->appendRow(measure_n);

          mr_i += 1;
        }
      
        diffs_cell->appendRow(num_cell);
        diff_i += 1;
      } // end of diff loop
      this->invisibleRootItem()->appendRow(diffs_cell);
    } // end of all diffs
  }
};


struct KVTreeViewDelegate : public QStyledItemDelegate {
  KVTreeModel *model;
  KVTreeViewDelegate(KVTreeModel *model) : model(model) {}

  // Copied from StackOverflow, no idea. Goal is to provide rich text rendering
  // for field text
  void paint(QPainter *painter, const QStyleOptionViewItem &option,
             const QModelIndex &index) const override {

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

  QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& /*option*/, 
                     const QModelIndex& index) const override {


    // nullptr -> segfault, parent class -> free invalid size
    // if (!index.isValid())
    // return QStyledItemDelegate::createEditor(parent, option, index);

    // TreeValue val;
    printf("createEditor called | getting item from index %d\n", index.row());
    // this can't work, the data is tree-like and the index has just a row ????
    QStandardItem *item = model->itemFromIndex(index);
    printf("it has %d columns, and ", item->columnCount());

    assert(item);
    QVariant qdata = item->data();
    // XXX: really nasty, there must be some proper way
    TreeValue data = *(TreeValue *) qdata.data();

    // another non-exhaustive dispatch
    if (0) {
    } else if (std::holds_alternative<DiffType>(data)) {
      printf("seems like difftype to me\n");
      QComboBox *combo = new QComboBox(parent);
      for (const char *cs : difftype_cstrs) { combo->addItem(cs); }
      return combo;
    } else if (std::holds_alternative<std::string *>(data)) {
      printf("seems like string to me\n");
      auto *lineEdit = new QLineEdit(parent);
      return lineEdit;
    } else if (std::holds_alternative<DoubleField>(data)) {
      printf("seems like double to me\n");
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
          doubleSpinBox->setMinimum(1);     // idk what negbpm really means
          doubleSpinBox->setMaximum(10000); // -- this depends
          break;
      }
      return doubleSpinBox;
    } else if (std::holds_alternative<uint32_t>(data)) {
      printf("seems like uint32_t to me\n");
      auto *spinBox = new QSpinBox(parent);
      // -- TODO set this to whatever Etterna supports as maximum
      spinBox->setMaximum(UINT32_MAX);
      return spinBox;
    } else {
      printf("I have no idea what this is\n");
      // assert(false && "non-exhaustive variant");
      return nullptr; // for now
    }

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

  QHBoxLayout layout_(w_root); // -- required so that it fills the entire space

  QSplitter resizable_layout(Qt::Horizontal);
  layout_.addWidget(&resizable_layout);

  // -- tree model of the whole file for a tree widget
  KVTreeModel        smfile_model(smfile);
  KVTreeViewDelegate smfile_view_delegate(&smfile_model);
  smfile_view_delegate.model = &smfile_model;

  // -- tree widget
  QTreeView *tree_view = new QTreeView();
  tree_view->setModel(&smfile_model);
  tree_view->setItemDelegate(&smfile_view_delegate);
  tree_view->setHeaderHidden(true);
  tree_view->expandToDepth(1); // hacky :( but luckily works here
  tree_view->resizeColumnToContents(0);
  tree_view->show();
  resizable_layout.addWidget(tree_view);

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


    QLabel status_bar;
    auto redraw_statusbar =
      [&](SmRelativePos pos, int snap){status_bar.setText(
        QString("Measure %1, beat %2, smtick %3  |  "
                "Snap <span style='color: %4; font-weight: 600;'>%5</span>")
          .arg(pos.measures)
          .arg(pos.beats)
          .arg(QString::number(pos.smticks, 'g', 4))
          .arg(cstr_color_from_snap(snap))
          .arg(snap)
      );};
    // set initial value
    redraw_statusbar(preview_actual.cur_chart_pos, preview_actual.current_snap_nths);

    QObject::connect(
      &preview_actual,
      &NoteDisplayWidget::positionChanged,
      [&](SmRelativePos pos, int snap){
        redraw_statusbar(pos, snap);
        preview_actual.update();
      }
    );

    QObject::connect(
      tree_view,
      &QTreeView::doubleClicked,
      &preview_actual,
      // somehow get the item I'm clicking on. That item should store a ref
      // to actual pos. Tell NoteDisplayWidget to seek to that pos and
      // refresh itself.
      [&](const QModelIndex &index){
        if (!index.isValid()) return;
        QStandardItem *qitem = smfile_model.itemFromIndex(index);
        QVariant qvar = qitem->data();
        // ew! but works for now
        TreeValue row_var = *(TreeValue *)qvar.data();
        if (0) {
        } else if (std::holds_alternative<NoteRow>(row_var)) { // clicked on a noterow
          NoteRow row = std::get<NoteRow>(row_var);
          // NOTE: maybe put SmRelativePos into the sm_parser already
          SmRelativePos new_pos = {(int32_t)row.measure, (int32_t)row.beat, (double)row.smticks};

          // XXX: this really shouldn't be three statements like this
          preview_actual.cur_chart_pos = new_pos;
          // preview_actual.update();
          emit preview_actual.positionChanged(new_pos, preview_actual.current_snap_nths);
        } else if (std::holds_alternative<SmRelativePos>(row_var)) { // clicked on a measure
          auto new_pos = std::get<SmRelativePos>(row_var);
          preview_actual.cur_chart_pos = new_pos;
          // preview_actual.update();
          emit preview_actual.positionChanged(new_pos, preview_actual.current_snap_nths);
        }
      }
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

      QSpinBox cmod_spinbox;
        cmod_spinbox.setMinimum(50);
        cmod_spinbox.setMaximum(1000);
        cmod_spinbox.setValue(700);
        cmod_spinbox.setSingleStep(15);
        QObject::connect(
          &cmod_spinbox,
          &QSpinBox::valueChanged,
          &preview_actual,
          &NoteDisplayWidget::onCmodChange
        );
        cmod_value.setText("CMOD");
        preview_actual.onCmodChange(cmod_spinbox.value());
      preview_controls.addWidget(&cmod_spinbox, 1);
      preview_controls.setAlignment(&cmod_spinbox, Qt::AlignCenter);
    preview_tile.addLayout(&preview_controls, 1);

  resizable_layout.addWidget(&right_chunk);

  w_tabs_root.show();
  int ret = app.exec();
  // delete tree;
  // tree = nullptr;
  return ret;
}

#include "zerokara.moc"
