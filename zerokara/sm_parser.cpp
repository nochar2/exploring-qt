#include "sm_parser.h"
#include "precompiled.h"

using std::array;
using std::string;
using std::string_view;
using std::vector;
using namespace std::string_literals;

#define eprintf(...) fprintf(stderr, __VA_ARGS__)

size_t Difficulty::total_note_rows() {
  size_t n = 0;
  for (auto m : this->measures) {
    for (auto _ : m.note_rows) {
      n += 1;
    }
  }
  return n;
}
// naive and slow, but let's do it for now
std::vector<NoteRow> Difficulty::note_rows() {
  std::vector<NoteRow> ret;
  for (auto m : this->measures) {
    for (auto nl : m.note_rows) {
      ret.push_back(nl);
    }
  }
  return ret;
}

bool smfile_key_is_string_type(string_view sv) {
  // possibly more
  auto str_keys = {
    "TITLE", "SUBTITLE", "ARTIST",
    "TITLETRANSLIT", "SUBTITLETRANSLIT", "ARTISTTRANSLIT",
    "CREDIT", "MUSIC", "BACKGROUND", "BANNER", "CDTITLE",
  };
  return std::any_of(str_keys.begin(), str_keys.end(), [&](auto k){return k == sv;});
}
bool smfile_key_is_double_type(string_view sv) {
  auto double_keys = { "OFFSET", "SAMPLESTART", "SAMPLELENGTH"};
  return std::any_of(double_keys.begin(),
                     double_keys.end(),
                     [&](auto k){return k == sv;});
}

const char *cstr_from_difftype(DiffType dt)
{
  switch (dt) {
    case DiffType::Beginner:  return "Beginner";
    case DiffType::Easy:      return "Easy";
    case DiffType::Medium:    return "Medium";
    case DiffType::Hard:      return "Hard";
    case DiffType::Challenge: return "Challenge";
    case DiffType::Edit:      return "Edit";
  }
  assert(false);
}

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
const char *cstr_from_gametype(GameType gt)
{
  switch (gt) {
    case GameType::DanceSingle: return "dance-single (4 keys)";
    case GameType::DanceDouble: return "dance-double (8 keys)";
  }
  assert(false);
}


// Ikik, you can only have one snap subdivision per measure.
const std::string CHART = ""
"#TITLE:Short test file;\n"
"#BPMS:0=180;\n"
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


// use holds_alternative for destructuring
std::variant<SmFile, SmParseError>
smfile_from_string_opt(string const& str)
{
  // -- HACK: make float conversions not break on non-English locales.
  // -- for some reason, setting this right after main() doesn't work, and none of
  // -- QLocale stuff works either.
  setlocale(LC_ALL, "C");

  /*
  find hash. If not, we're done. Let's permit files that don't have a diff yet (maybe warn).
  find colon. If not, panic. If yes, translate the key to an enum and a datatype repr.
  don't search the semicolon (stupid for cache). Instead, gradually read in the value according to whatever
  value type we expect. If you get end of file (without a semicolon), panic.
  */

  SmFile smfile {};
  auto tb = std::chrono::system_clock::now();

  /* let's use the C api instead */

  // const char *last_semi_pos = str.c_str();
  // size_t value_len;

  string_view sv(str);

  while (true) {
    size_t hash_pos = sv.find('#');
    if (hash_pos == string::npos) {

      auto te = std::chrono::system_clock::now();
      auto us = std::chrono::duration_cast<std::chrono::microseconds>(te-tb).count();
      // std::print(stderr, "Parsed an SmFile in {} seconds", (double)us / 1000000.);
      eprintf("Parsed an SmFile in %lf seconds", (double)us / 1000000.);
      return smfile;
    }

    size_t junk_len = hash_pos;
    sv.remove_prefix(junk_len + 1); // eat the # also

    size_t colon_pos = sv.find(':');
    if (colon_pos == string::npos) { return (SmParseError){.msg="While reading a new key: EOF"s}; }
    string_view key = sv.substr(0, colon_pos);
    sv.remove_prefix(colon_pos + 1);


    size_t semi_pos = string::npos;
    // because notes take 99 % of the file and we don't want to scan that
    if (key != "NOTES") {
      semi_pos = sv.find(';');
      if (semi_pos == string::npos) {
        return (SmParseError){.msg=std::format("While reading #{}: EOF", key)};
      }
    }

    if (false) {
    // you might say this is stupid and inefficient but I don't see a better way other than
    // some macro magic, and even then you don't have a preprocessor macro to turn things
    // upper/lower case
    } else if (smfile_key_is_string_type(key)) {
      std::string val(sv.data(), semi_pos);
      if (false) {}
      else if (key == "TITLE")      smfile.title = val;
      else if (key == "SUBTITLE")   smfile.subtitle = val;
      else if (key == "ARTIST")     smfile.artist = val;
      else if (key == "BACKGROUND") smfile.background = val;
      else if (key == "BANNER")     smfile.banner = val;
      else if (key == "CDTITLE")    smfile.cdtitle = val;
      else if (key == "CREDIT")     smfile.credit = val;
      else if (key == "MUSIC")      smfile.music = val;
      else {
        eprintf("unhandled sm key of strhing type %s\n", string(key).c_str());
        fflush(stderr); assert(false);
      }
    } else if (smfile_key_is_double_type(key)) {
      double parsed;
      std::string_view val = sv.substr(0, semi_pos);
      int matched = sscanf(val.data(), "%lf", &parsed);
      if (matched != 1) {
        eprintf("While reading #NOTES/%s: Couldn't parse float, defaulting to 0.0\n", string(key).c_str());
        parsed = 0;
      }
      // eprintf("parsed float %lf\n", parsed);
      if (false) {}
      else if (key == "OFFSET")       smfile.offset = parsed;
      else if (key == "SAMPLESTART")  smfile.samplestart = parsed;
      else if (key == "SAMPLELENGTH") smfile.samplelength = parsed;
      else {
        eprintf("unhandled sm key of double type %s\n", string(key).c_str());
        fflush(stderr); assert(false);
      }
    } else if (key == "BPMS" || key == "STOPS") {
      string_view timevals_sv = sv.substr(0, semi_pos);

      std::vector<TimeKV> timevals;
      for (auto kv_s : timevals_sv | std::views::split(',')) {
        double time, val;
        int matched = sscanf(kv_s.data(), " %lf=%lf", &time, &val);
        if (matched != 2) {
          // not super robust but let's move on for now
          eprintf("While reading #NOTES/%s: Couldn't parse float, ignoring\n", string(key).c_str());
        } else {
          timevals.push_back(TimeKV(time, val));
        }
      }
      if (false) {}
      else if (key == "BPMS")  smfile.bpms = timevals;
      else if (key == "STOPS") smfile.stops = timevals;
      else {
        eprintf("unhandled sm key of std::vector<TimeKV> type %s\n", string(key).c_str());
        fflush(stderr); assert(false);
      }
    } else if (key == "NOTES") {
      Difficulty diff;
      size_t sentinel_pos;
      
      {
        while (isspace(sv[0])) sv.remove_prefix(1);
        sentinel_pos = sv.find(':');
        if (sentinel_pos == string::npos){return (SmParseError){.msg="While reading #NOTES/GameType: EOF"s};}

        string_view game_type = sv.substr(0, sentinel_pos);
        if (false) {}
        else if (game_type == "dance-single") { diff.game_type = GameType::DanceSingle; }
        else if (game_type == "dance-double") { diff.game_type = GameType::DanceDouble; }
        else {
          return (SmParseError) {.msg = "While reading #NOTES/GameType: unknown GameType "s + string(game_type)};
        };
        sv.remove_prefix(sentinel_pos + 1);
      }

      {
        while (isspace(sv[0])) sv.remove_prefix(1);
        sentinel_pos = sv.find(':'); if (sentinel_pos == string::npos) {
          return (SmParseError) {.msg="While reading #NOTES/Charter: EOF"s};
        }

        diff.charter = string(sv.data(), sentinel_pos);
        sv.remove_prefix(sentinel_pos + 1);
      }

      {
        while (isspace(sv[0])) sv.remove_prefix(1);
        sentinel_pos = sv.find(':');
        if (sentinel_pos == string::npos) {return (SmParseError){.msg="While reading #NOTES/DiffType: EOF"s};}

        string_view diff_type_s = sv.substr(0, sentinel_pos);
        if (false) {}
        else if (diff_type_s == "Beginner")  { diff.diff_type = DiffType::Beginner; }
        else if (diff_type_s == "Easy")      { diff.diff_type = DiffType::Easy; }
        else if (diff_type_s == "Medium")    { diff.diff_type = DiffType::Medium; }
        else if (diff_type_s == "Hard")      { diff.diff_type = DiffType::Hard; }
        else if (diff_type_s == "Challenge") { diff.diff_type = DiffType::Challenge; }
        else if (diff_type_s == "Edit")      { diff.diff_type = DiffType::Edit; }
        else {
          return (SmParseError) {.msg="While reading #NOTES/DiffType: Unknown DiffType " + string(diff_type_s)};
          // TODO: this is not actually error. Maybe make it return a list of warnings instead.
        };
        sv.remove_prefix(sentinel_pos + 1);
      }

      {
        while (isspace(sv[0])) sv.remove_prefix(1);
        sentinel_pos = sv.find(':');
        if (sentinel_pos == string::npos) {return (SmParseError) {.msg="While reading #NOTES/DiffNum: EOF"s};}

        string_view diff_num_s = sv.substr(0, sentinel_pos);
        uint32_t num;
        auto err = std::from_chars(diff_num_s.data(), diff_num_s.data() + diff_num_s.size(), num);
        if (err.ec != std::errc{}) {
          return (SmParseError) {.msg="While reading #NOTES/DiffNum: Not a number: " + string(diff_num_s)};
        }
        diff.diff_num = num;
        sv.remove_prefix(sentinel_pos + 1);
      }

      {
        while (isspace(sv[0])) sv.remove_prefix(1);
        sentinel_pos = sv.find(':');
        if (sentinel_pos == string::npos) {return (SmParseError){.msg="While reading #NOTES/GrooveValues: EOF"s};}

        string_view groove_values_s = sv.substr(0, sentinel_pos);
        std::vector<double> gvs;
        for (auto gvr : groove_values_s | std::views::split(':')) {
          double gv;
          auto err = std::from_chars(gvr.begin(), gvr.end(), gv);
          if (err.ec != std::errc()) { // errc() means success
            return (SmParseError) {.msg="While reading #NOTES/GrooveValues: Not a float: " + string(gvr.data())};
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


        uint32_t measure_i = 0;

        bool end_of_difficulty = false;
        while (!end_of_difficulty) { // for each measure
          std::vector<NoteRow> current_measure_pre = {};

          bool end_of_measure = false;
          while (!end_of_measure) { // for each pattern
            const size_t &pat_i = current_measure_pre.size();
            // fflush(stdout);

            switch (sv[0]) {
            case ',':  sv.remove_prefix(1); end_of_measure = true; break;
            case ';':  sv.remove_prefix(1); end_of_measure = true; end_of_difficulty = true; break;
            case '/':  // pretend it's //, I don't care
              size_t end_of_line;
              end_of_line = sv.find('\n'); sv.remove_prefix(end_of_line + 1);
              break;
            case '\0':
              return (SmParseError) {.msg=std::format(
                "While reading #Notes/NoteRows: Measure #{}: Unexpected EOF (missing semicolon?)", measure_i
              )};
            case ' ': case '\t': case '\r': case '\n':
              sv.remove_prefix(1);
              break;
            case '0' ... '4': case 'M': case 'L': case 'F':
            {
              size_t pat_len = sv.find_first_not_of("01234MLF");
              string_view line_s = sv.substr(0, pat_len);
              sv.remove_prefix(pat_len);
              if (pat_len != 4) {
                return (SmParseError) {.msg=std::format(
                   "While reading #Notes/NoteRows: Measure #{}, pattern #{}: Expected ({}) to consist of "
                   "exactly 4 symbols 01234MLF",
                   measure_i, current_measure_pre.size(), line_s
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
              // std::println(stderr, "pushing back a line {}", line_s);
              current_measure_pre.push_back((NoteRow){ .measure=measure_i, .beat=0, .smticks=0, .sec_zero_offset=0, .line=line });
              break;
            }
            default:
              return (SmParseError) {.msg=std::format(
                 "While reading #Notes/NoteRows: Measure #{}, pattern #{}: Garbage starting with {}",
                 measure_i, pat_i, sv.substr(0, std::min(10UL,sv.size()))
                )
              }; 
            }
          } // for all patterns in measure

          // std::println(stderr, "remaining sv is {}", sv);
          // fflush(stdout);
            
          // Resolve the timing info.
          uint32_t beats_per_measure = 4; // 99.99999 % of all charts
          uint32_t pats_per_measure = (uint32_t)current_measure_pre.size();
          assert(pats_per_measure != 0); // for now

          auto b = beats_per_measure;
          array<size_t, 10> common_ppm = {b, 2*b, 3*b, 4*b, 6*b, 8*b, 12*b, 16*b, 24*b, 48*b};
          if (pats_per_measure % b != 0) {
            // return (SmParseError){.msg=std::format(
              eprintf("W: While reading #Notes/NoteRows: Measure %u has number of rows %u, which is not divisible by time signature %u/4.\n",
              measure_i, pats_per_measure, beats_per_measure);
            // };
          } else if (std::none_of(common_ppm.begin(), common_ppm.end(), [&](auto x){ return pats_per_measure == x; })) {
              eprintf("W: While reading #Notes/NoteRows: Measure %u has an unusual number of rows %u\n",
              measure_i, pats_per_measure);
          }

          size_t i = 0;
          // -- TODO: make a span of this measure
          // -- Problem (at least one): this whole thing can get reallocated, and if it does,
          // -- your span points to invalid memory and you segfault. 
          // size_t notelines_in_this_measure = 0;
          // auto begin
          Measure m;
          for (auto nl : current_measure_pre) {
            if (!std::all_of(nl.line.begin(), nl.line.end(), [](auto x){return x == NoteType::None;})) {
              nl.beat    = (uint8_t)((i * beats_per_measure * 48 / pats_per_measure) / 48);
              nl.smticks = (uint8_t)((i * beats_per_measure * 48 / pats_per_measure) % 48);
              nl.sec_zero_offset = secs_per_beat * (nl.beat + nl.smticks / 48.);
              // notelines_in_this_measure += 1;
              m.note_rows.push_back(nl);
            }
            i++;
          }
          diff.measures.push_back(m);
          // auto note_span = std::span(notelines_in_this_measure)
          // diff.as_measures.push_back(MeasureInfo(note_span, notelines_in_this_measure));

          measure_i += 1;
        } // for all measures

        eprintf("I: Parsed a diff with %lu noterows\n", diff.total_note_rows());
        smfile.diffs.push_back(diff);
      } // (NoteInfo parsing scope)
    } // else if (key == "#NOTES") {
  } // while (true)
}

// int main(void) {
//   std::ios_base::sync_with_stdio(true);

  // fflush(stdout);
  // eprintf("this is a c stream test\n");
  // std::print(stderr, "this is a c++ print test\n");
  // fflush(stdout);

//   auto smfile_opt = smfile_from_string_opt(CHART);
//   if (std::holds_alternative<SmFile>(smfile_opt)) {
//     SmFile smfile = std::get<SmFile>(smfile_opt);
//     eprintf("got map: %s\n", smfile.title.c_str());
//   } else {
//     SmParseError error = std::get<SmParseError>(smfile_opt);
//     eprintf("%s\n", error.msg.c_str());
//   }
// }
