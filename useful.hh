#ifndef __USEFUL_HH__
#define __USEFUL_HH__ 1

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/time.h>

#include <math.h>

#include <pthread.h>

#include <string>
#include <set>
#include <map>
#include <vector>
#include <list>
#include <utility>

#define foreach(xi, xl) for (typeof((xl).begin()) xi = (xl).begin(); xi != (xl).end(); ++xi)

#define EXTRA_COMPARATORS(T) \
  int operator <= (const T &p) const \
    { return (*this < p || *this == p); } \
  int operator > (const T &p) const \
    { return (p < *this); } \
  int operator >= (const T &p) const \
    { return (p < *this || *this == p); } \
  int operator != (const T &p) const \
    { return (!(*this == p)); }

#define EXTRA_SUMMATORS(T) \
  T operator + (const T &p) const \
    { T x = *this; return (x += p); } \
  T &operator -= (const T &p) \
    { *this += -p; return *this; } \
  T operator - (const T &p) const \
    { T x = *this; return (x -= p); }

#define pair(x, y)	std::make_pair(x, y)

template <class T, class U> static inline void mm_erase_pair(std::multimap<T,U> &m, const std::pair<T, U>& tu) {
  const T &t = tu.first;
  const U &u = tu.second;

  std::pair<typeof(m.begin()), typeof(m.begin())> er = m.equal_range(t);
  typeof(m.begin()) i = er.first;
  while (i != er.second) {
    if (i->second == u) {
      m.erase(i++);
    } else {
      ++i;
    }
  }
}

static inline std::string str(int x) {
  char buf[16];
  sprintf(buf, "%d", x);
  return std::string(buf);
}

static inline double now() {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return (tv.tv_sec + tv.tv_usec / 1000000.0);
}

#endif
