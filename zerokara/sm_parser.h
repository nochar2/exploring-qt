#include <string>
#include <cstdint>
#include <array>
#include <vector>
#include <variant>

extern const std::string CHART;


enum class GameType { DanceSingle, DanceDouble };
enum class DiffType { Beginner, Easy, Medium, Hard, Challenge, Edit };
std::string string_from_difftype(DiffType dt);
std::string string_from_gametype(GameType gt);

enum class NoteType : char {
  None   = '0',
  Tap    = '1',
  Hold   = '2',
  HRLift = '3',
  Rolld  = '4',
  Mine   = 'M',
  Lift   = 'L',
  Fake   = 'F',
};

// -- this doesn't work :(
// -- and there's some other variant with throw which works but the error msg is garbage
// template <uint8_t max>
// struct Fin {
//   uint8_t n;
//   consteval Fin(uint8_t n_raw) : n(n_raw) {
//     if (n_raw >= max) { static_assert(false, "Value is outside of the Fin range"); }
//   }
// };
struct NoteRow {
  uint32_t measure;
  uint32_t beat; // 0..3, possibly more
  uint32_t smticks; // 0..47
  // actually, idk if this should be with offset or without
  double sec_zero_offset;
  std::array<NoteType, 4> notes;
};

struct Measure {
  std::vector<NoteRow> note_rows;
};

struct Difficulty {
  GameType game_type;
  std::string charter;
  DiffType diff_type;
  uint32_t diff_num;
  std::vector<double> groove_values;
  std::vector<Measure> measures;
  std::vector<NoteRow> note_rows();
  size_t total_note_rows();
};

struct TimeKV {
  double beat_number;
  double value;
};

struct SmFile {
  std::string title;
  std::string subtitle;
  std::string artist;
  std::string background;
  std::string banner;
  std::string cdtitle;
  std::string credit;
  std::string music;
  double offset;
  double samplestart;
  double samplelength;

  std::vector<TimeKV> bpms;
  std::vector<TimeKV> stops;
  std::vector<Difficulty> diffs;
};

enum class SmField {
  Title,
  /* the #NOTES: field, but that is misleading since there's still metadata */
  Difficulty
};

struct SmParseError {
  std::string msg; // for now, just this.
};

std::variant<SmFile, SmParseError>
smfile_from_string_opt(std::string const& str);
