// #include <QString>
// #include <QList>

#include <cstdint>
#include <string>
#include <algorithm>
#include <array>
#include <vector>
#include <cstring>
#include <variant>
#include <charconv>
#include <ranges>
#include <cassert>
#include <format>
// #include <chrono>


using std::array;
using std::string;
using std::string_view;
using std::vector;
// using namespace std::chrono_literals;
using namespace std::string_literals;

// Ikik, you can only have one snap subdivision per measure.
string chart = ""
"#TITLE:Short test file;\n"
"#NOTES:dance-single:myself:Edit:15:0.000:\n"
"1000\n"
"0100\n"
"0010\n"
"0001\n"
",\n"
"1000\n"
"0100\n"
"0010\n"
"0100\n"
"0010\n"
"0001\n"
",\n"
"1100\n"
"0001\n"
"0010\n"
"0100\n"
"1000\n"
"0001\n"
"0010\n"
"1000\n"
",\n"
"0110\n"
"0000\n"
"0000\n"
"0000\n"
";";

enum class GameType { DanceSingle, DanceDouble };
enum class DiffType { Beginner, Easy, Medium, Hard, Challenge, Edit };
enum class NoteType {
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
struct NoteInfo {
  uint32_t measure;
  uint8_t beat; // 0..3, possibly more
  uint8_t subbeat_ticks; // 0..47
  std::array<NoteType, 4>  line;
  double seconds;
};

struct Difficulty {
  GameType game_type;
  std::string charter;
  DiffType diff_type;
  uint32_t diff_num;
  std::vector<double> groove_values;
  std::vector<NoteInfo> note_rows;
};

struct TimeKV {
  double beat_number;
  double value;
};

struct SmFile {
  std::string title;
  std::vector<TimeKV> bpms;
  std::vector<Difficulty> diffs;
};

enum class SmField {
  Title,
  /* the #NOTES: field, but that is misleading since there's still metadata */
  Difficulty
};

// enum class SmParserState {
//   Ignoring,
//   ReadingKey,
//   ReadingValue,
// }

struct SmParseError {
  string msg; // for now, just this.
};

// use holds_alternative for destructuring
std::variant<SmFile, SmParseError>
parse(string const& str) {
  /*
  find hash. If not, we're done. Let's permit files that don't have a diff yet (maybe warn).
  find colon. If not, panic. If yes, translate the key to an enum and a datatype repr.
  don't search the semicolon (stupid for cache). Instead, gradually read in the value according to whatever
  value type we expect. If you get end of file (without a semicolon), panic.
  */

  SmFile smfile {};

  /* let's use the C api instead */

  // const char *last_semi_pos = str.c_str();
  size_t value_len;

  std::string_view sv(str);

  while (true) {
    // const char *hash_pos = strchr(last_semi_pos, '#');
    size_t hash_pos = sv.find('#');
    if (hash_pos == string::npos) return smfile;
    size_t junk_len = hash_pos;
    sv.remove_prefix(junk_len + 1);

    size_t colon_pos = sv.find(':');
    if (colon_pos == string::npos) { return (SmParseError){.msg="While reading a new key: EOF"s}; }
    size_t key_len = colon_pos;
    if (false) {
    } else if (sv.starts_with("TITLE")) {
      sv.remove_prefix(colon_pos + 1);
      size_t semi_pos = sv.find(';');
      if (semi_pos == string::npos) { return (SmParseError){.msg="While reading #TITLE: EOF"s}; }
      value_len = semi_pos;
      smfile.title = std::string(sv.data(), value_len);
    } else if (sv.starts_with("NOTES")) {

      Difficulty diff;
      size_t sentinel_pos;
      
      {
        sentinel_pos = sv.find(':'); if (sentinel_pos == string::npos) {
          return (SmParseError) {.msg="While reading #NOTES/GameType: EOF"s};
        }
        string game_type = string(sv.data(), sentinel_pos);
        if (false) {}
        else if (game_type == "dance-single") { diff.game_type = GameType::DanceSingle; }
        else if (game_type == "dance-double") { diff.game_type = GameType::DanceDouble; }
        else {
          return (SmParseError) {.msg = "While reading #NOTES/GameType: unknown GameType "s + string(game_type)};
        };
        sv.remove_prefix(sentinel_pos + 1);
      }

      {
        sentinel_pos = sv.find(':'); if (sentinel_pos == string::npos) {
          return (SmParseError) {.msg="While reading #NOTES/Charter: EOF"s};
        }
        diff.charter = string(sv.data(), sentinel_pos);
        sv.remove_prefix(sentinel_pos + 1);
      }

      {
        sentinel_pos = sv.find(':'); if (sentinel_pos == string::npos) {
          return (SmParseError) {.msg="While reading #NOTES/DiffType: EOF"s};
        }
        string_view diff_type_s = sv.substr(0, sentinel_pos);
        if (false) {}
        else if (diff_type_s == "Beginner")  { diff.diff_type = DiffType::Beginner; }
        else if (diff_type_s == "Easy")      { diff.diff_type = DiffType::Easy; }
        else if (diff_type_s == "Medium")    { diff.diff_type = DiffType::Medium; }
        else if (diff_type_s == "Hard")      { diff.diff_type = DiffType::Hard; }
        else if (diff_type_s == "Challenge") { diff.diff_type = DiffType::Challenge; }
        else if (diff_type_s == "Edit")      { diff.diff_type = DiffType::Edit; }
        else {
          return (SmParseError) {.msg=std::format("While reading #NOTES/DiffType: Unknown DiffType {}", diff_type_s)};
          // TODO: this is not actually error. Maybe make it return a list of warnings instead.
        };
        sv.remove_prefix(sentinel_pos + 1);
      }

      {
        sentinel_pos = sv.find(':'); if (sentinel_pos == string::npos) {
          return (SmParseError) {.msg="While reading #NOTES/DiffNum: EOF"s};
        }
        string_view diff_num_s = sv.substr(0, sentinel_pos);
        uint32_t num;
        auto err = std::from_chars(diff_num_s.data(), diff_num_s.data() + diff_num_s.size(), num);
        if (err.ec != std::errc{}) {
          return (SmParseError) {.msg=std::format("While reading #NOTES/DiffNum: Not a number: {}", diff_num_s)};
        }
        diff.diff_num = num;
        sv.remove_prefix(sentinel_pos + 1);
      }

      {
        sentinel_pos = sv.find(':'); if (sentinel_pos == string::npos) {
          return (SmParseError) {.msg="While reading #NOTES/GrooveValues: EOF"s};
        }
        string_view groove_values_s = sv.substr(0, sentinel_pos);
        std::vector<double> gvs;

        for (auto gvr : groove_values_s | std::views::split(':')) {
          double gv;
          auto err = std::from_chars(gvr.begin(), gvr.end(), gv);
          if (err.ec != std::errc()) { // errc() means success
            return (SmParseError) {.msg=std::format("While reading #NOTES/GrooveValues: Not a float: {}", gvr.data())};
          }
          gvs.push_back(gv);
        }
        diff.groove_values = gvs;
        sv.remove_prefix(sentinel_pos + 1);
      }

      // Oh boy, now the #NOTES: data. Let's first at least verify that #BPMS are sane.
      // NOTE: we are doing it the hardcore way and not looking up the end ; at first because that's slow, we have to stumble upon it somewhere.
      {
        if (smfile.bpms.size() == 0) {
          return (SmParseError) {.msg="While reading #NOTES/NoteRows: BPMS are empty!"s};
        }
        if (smfile.bpms[0].beat_number != 0.0) {
          return (SmParseError) {.msg="While reading #NOTES/NoteRows: BPMS don't start at beat 0."s};
        }
        if (smfile.bpms.size() > 1) {
          return (SmParseError) {.msg="While reading #NOTES/NoteRows: More than one BPM is not implemented yet."s};
        }

        double bpm = smfile.bpms[0].value;
        double secs_per_beat = 60. / bpm ;

        // TODO: parse the #OFFSET: field and add it everywhere here. Let's say the #NOTES will already be
        // the complete thing, outside of user-specified offset of course.

        // eat_spaces()
        // eat_while_n("01234MLF", 4)


        bool end_of_diff = false;
        bool end_of_measure = false;

        for (uint32_t measure_i = 0; !end_of_diff; measure_i++) {
          std::vector<NoteInfo> current_measure_pre;

          for (uint32_t pat_i = 0; !end_of_measure; pat_i++) {
            size_t end_of_line;

            switch (sv[0]) {
            case ',':  end_of_measure = true; break;
            case ';':  end_of_measure = true; end_of_diff = true; break;
            case '/':  // pretend it's //, I don't care
              end_of_line = sv.find('\n');
              sv.remove_prefix(end_of_line);
              break;
            case '\0':
              return (SmParseError) {.msg=std::format(
                "While reading #Notes/NoteRows: Measure #{}: Unexpected EOF (no semicolon reeeee)", measure_i
              )};
            case ' ': case '\t': case '\r': case '\n':
              sv.remove_prefix(1);
              continue;
            case '0': case '1': case '2': case '3': case '4': case 'M': case 'L': case 'F':
            {
              size_t pat_len = sv.find_first_not_of("01234MLF");
              string_view line_s = sv.substr(0, pat_len);
              sv.remove_prefix(pat_len);
              if (pat_len != 4) {
                return (SmParseError) {.msg=std::format(
                   "While reading #Notes/NoteRows: Measure #{}, pattern #{}: Expected ({}) to consist of "
                   "exactly 4 symbols 01234MLF",
                   measure_i, pat_i, line_s
                  )
                };
              }
              array<NoteType,4> line = {{
                static_cast<NoteType>(line_s[0]),
                static_cast<NoteType>(line_s[1]),
                static_cast<NoteType>(line_s[2]),
                static_cast<NoteType>(line_s[3]),
              }};
              // I don't know beat / ticks / seconds because I don't know the pattern count per measure yet
              current_measure_pre.push_back((NoteInfo){ .measure=measure_i, .beat=0, .subbeat_ticks=0, .line=line, .seconds=0, });
              break;
            }
            default:
              return (SmParseError) {.msg=std::format(
                 "While reading #Notes/NoteRows: Measure #{}, pattern #{}: Garbage starting with {}",
                 measure_i, pat_i, sv.substr(0, std::min(10UL,sv.size()))
                )
              };
            
          }

          /* Although you might want to assert that everything is 4/4 by now, I don't want to bake
          the assumption into a poor implementation. So 
          */
          uint32_t beats_per_measure = 4; // 99.99999 % of all charts
          auto b = beats_per_measure;
          uint32_t pats_per_measure = (uint32_t)current_measure_pre.size();
          array<size_t, 10> common_ppm = {b, 2*b, 3*b, 4*b, 6*b, 8*b, 12*b, 16*b, 24*b, 48*b};
          if (pats_per_measure % b != 0) {
            // return (SmParseError){.msg=std::format(
              fprintf(stderr, "W: While reading #Notes/NoteRows: Measure %u has number of rows %u, which is not divisible by time signature %u/4.",
              measure_i, pats_per_measure, beats_per_measure);
            // };
          } else if (std::none_of(common_ppm.begin(), common_ppm.end(), [&](auto x){ return pats_per_measure == x; })) {
              fprintf(stderr, "W: While reading #Notes/NoteRows: Measure %u has an unusual number of rows %u",
              measure_i, pats_per_measure);
          }


          // vector<NoteInfo> current_measure;
          for (auto [i, m] : std::views::enumerate(current_measure_pre)) {
            if (!std::all_of(m.line.begin(), m.line.end(), [](auto x){return x == NoteType::None;})) {
              m.beat          = (uint8_t)(i * beats_per_measure * 48 / pats_per_measure);
              m.subbeat_ticks = (uint8_t)(i * beats_per_measure * 48 % pats_per_measure);
              // improvable
              m.seconds = secs_per_beat * (m.beat + m.subbeat_ticks / 48.);
              diff.note_rows.push_back(m);
            }
          }
        }
        fprintf(stderr, "I: Parsed a diff with %lu notes\n", diff.note_rows.size());
        smfile.diffs.push_back(diff);
      }
      fprintf(stderr, "I: Returning smfile with %lu diffs\n", smfile.diffs.size());
      return smfile;
    }
  }
}
