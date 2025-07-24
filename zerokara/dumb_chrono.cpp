#include <ctime>
#include <cassert>

namespace Chrono {
  struct TimePoint {
    struct timespec raw;
    TimePoint operator-(TimePoint tb) {
      // this is a bit broken but good enough for me
      TimePoint ret = {
        this->raw.tv_sec - tb.raw.tv_sec,
        this->raw.tv_nsec - tb.raw.tv_nsec
      };
      // while (ret.raw.tv_nsec < 0) {
        long seconds_to_borrow = (-ret.raw.tv_nsec + 999999999L) / 1000000000L;
        ret.raw.tv_nsec += 1000000000L * seconds_to_borrow;
        ret.raw.tv_sec  -= seconds_to_borrow;
      // }
      return ret;
    };
    long microseconds() {
      return (raw.tv_sec * 1000000000L + raw.tv_nsec) / 1000;
    }
  };
  // yeah, I got tired of trying to replicate the STL, bye
  TimePoint now() {
    struct timespec ts;
    int e = clock_gettime(CLOCK_REALTIME, &ts);
    assert(e == 0); // idk how else to check this :(
    TimePoint tp = {.raw = ts };
    return tp;
  }
}
