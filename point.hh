#ifndef __POINT_HH__
#define __POINT_HH__ 1

#include "useful.hh"

template <class T> struct AnyPoint {
  const static AnyPoint Zero;

  AnyPoint() : x(0), y(0) { }
  AnyPoint(const AnyPoint &p) : x(p.x), y(p.y) { }
  AnyPoint(T _x, T _y) : x(_x), y(_y) { }

  T x, y;

  T d2(const AnyPoint &p) const { 
    T dx = x - p.x;
    T dy = y - p.y;
    return (dx * dx + dy * dy);
  }

  AnyPoint &operator += (const AnyPoint &p)
    { x += p.x; y += p.y; return *this; }
  AnyPoint operator - () const
    { return AnyPoint(-x, -y); }

  EXTRA_SUMMATORS(AnyPoint)

  int operator < (const AnyPoint &p) const
    { return ((y < p.y) || (y == p.y && x < p.x)); }
  int operator == (const AnyPoint &p) const
    { return (y == p.y && x == p.x); }

  AnyPoint &operator *=(const T &z) {
    x *= z;
    y *= z;
    return *this;
  }

  AnyPoint operator *(const T &z) {
    AnyPoint w = *this;
    w.x *= z;
    w.y *= z;
    return w;
  }

  EXTRA_COMPARATORS(AnyPoint)

  T length2() const {
    return (x * x + y * y);
  }
};

typedef AnyPoint<double> Point;

#endif
