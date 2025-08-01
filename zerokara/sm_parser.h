#ifndef SM_PARSER_H
#define SM_PARSER_H
#include <cstdint>
// #include <array>
// #include <vector>
#include <variant>
#include "dumb_stdlib_linux.h"

#include <string>
// class std::string;

extern const std::string CHART;


enum class GameType { DanceSingle, DanceDouble };
enum class DiffType { Beginner, Easy, Medium, Hard, Challenge, Edit };
std::string string_from_difftype(DiffType dt);
std::string string_from_gametype(GameType gt);


// I could store '0', '1' etc., but then what would other memory
// values mean?
enum class NoteType : char { None,Tap,Hold,HRLift,Rolld,Mine,Lift,Fake };
NoteType char_to_notetype(char c);
char notetype_to_char(NoteType nt);



// -- not good
// struct NoteRow {
//   uint32_t measure;
//   uint32_t beat; // 0..3, possibly more
//   uint32_t smticks; // 0..47
  // actually, idk if this should be with offset or without
  // double sec_zero_offset;
  // Array<NoteType, 4> notes;
// };

constexpr uint32_t BEATS_PER_MEASURE = 4; // 99.99999 % of all charts

// -- memory inefficient, but we don't care. It's convenient for now!
struct NoteRow { Array<NoteType, 4> notes; };
struct Beat    { Array<NoteRow,48> note_rows; };
struct Measure { Array<Beat, BEATS_PER_MEASURE> beats; };

bool noterow_is_zero(NoteRow nr);


struct Difficulty {
  GameType game_type;
  std::string charter;
  DiffType diff_type;
  uint32_t diff_num;
  Vector<double> groove_values;
  Vector<Measure> measures;
  // Vector<NoteRow> note_rows();
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

  Vector<TimeKV> bpms;
  Vector<TimeKV> stops;
  Vector<Difficulty> diffs;
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

const char *cstr_from_gametype(GameType gt);
const char *cstr_from_difftype(DiffType gt);
extern Array<const char *, 6> difftype_cstrs;

#endif
