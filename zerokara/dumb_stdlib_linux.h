#pragma once
#include <ctime>
#include <cassert>
#include <cstdio>
#include <cstdlib>

// THIS IS BROKEN AND NAIVE, DON'T USE THIS

#include <vector>
#define Vector std::vector

// #define USE_STL
#ifdef USE_STL
#include <array>
#include <chrono>
#define Array  std::array
#define Chrono std::chrono
#else


inline void check_index(size_t idx, size_t n) {
  if (idx >= n) {
    fprintf(stderr,"FATAL: index %ld is out of range (0-%ld)\n", idx, n-1); assert(false);
  }
};

/*
template<typename T>
struct Vector
{
  Vector();
  T *storage;
  size_t n;
  size_t cap;
  inline size_t size() { return n; }
  inline void push_back(T el) {
    if (n == cap) {
      this->resize_to(2 * cap);
    }
    storage[n] = el;
    n += 1;
  }
  inline T &operator[](size_t at) {
    check_index(at, n);
    return storage[at];
  }
  inline const T *begin() const { return &storage[0]; }
  inline const T *end()   const { return &storage[n]; }
  inline       T *begin()       { return &storage[0]; }
  inline       T *end()         { return &storage[n]; }

private:
  void resize_to(size_t new_cap) {
    T *storage_new = (T *)realloc(storage, new_cap);
    if (!storage_new) {
      fprintf(stderr, "FATAL: out of memory\n"); assert(false);
    }
    this->storage = storage_new;
    this->cap = new_cap;
  }
};
template<typename T>
Vector<T>::Vector() : n(0) {
  this->resize_to(8);
}
*/

template<typename T, size_t N>
struct Array
{
  T storage[N];
  T &operator[](size_t idx);
  const T *begin() const;
  const T *end() const;
  T *begin() ;
  T *end() ;
};

// these have to be in a header file
template<typename T, size_t N>
const T *Array<T,N>::begin() const { return &storage[0]; }

template<typename T, size_t N>
const T *Array<T,N>::end() const { return &storage[N]; }

template<typename T, size_t N>
T *Array<T,N>::begin() { return &storage[0]; }

template<typename T, size_t N>
T *Array<T,N>::end()   { return &storage[N]; }

template<typename T, size_t N>
T &Array<T,N>::operator[](size_t idx) { check_index(idx, N); return storage[idx]; }



namespace Chrono {
  struct TimePoint {
    struct timespec raw;
    TimePoint operator-(TimePoint tb);
    long microseconds();
  };
  namespace system_clock {
    // emulate system_clock::now()
    inline TimePoint now() {
      struct timespec ts;
      int e = clock_gettime(CLOCK_REALTIME, &ts);
      assert(e == 0); // idk how else to check this :(
      TimePoint tp = {.raw = ts };
      return tp;
    }
  }
  // emulate end - begin
  inline TimePoint TimePoint::operator-(TimePoint tb) {
    TimePoint ret = {
      this->raw.tv_sec - tb.raw.tv_sec,
      this->raw.tv_nsec - tb.raw.tv_nsec
    };
    if (ret.raw.tv_nsec < 0) {
      long seconds_to_borrow = (-ret.raw.tv_nsec + 999999999L) / 1000000000L;
      ret.raw.tv_nsec += 1000000000L * seconds_to_borrow;
      ret.raw.tv_sec  -= seconds_to_borrow;
    }
    return ret;
  }

  // emulate duration_cast<microseconds>
  enum Duration { microseconds };
  struct AfterDurationCast { // not type safe whatsoever, you can only cast once
    long m_count;
    // emulate .count()
    inline long count() { return m_count; }
  };
  template <Duration D>
  inline AfterDurationCast duration_cast(TimePoint d);
  template <>
  inline AfterDurationCast duration_cast<Duration::microseconds>(TimePoint d) {
    return { .m_count = (d.raw.tv_sec * 1000000000L + d.raw.tv_nsec) / 1000 };
  }
};

#endif // USE_STL
