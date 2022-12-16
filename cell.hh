#ifndef __CELL_HH__
#define __CELL_HH__ 1

#include "useful.hh"
#include "point.hh"

struct Cell : AnyPoint<int32_t> {
  Cell() : AnyPoint<int32_t>() { }
  Cell(const Cell &c) : AnyPoint<int32_t>(c) { }
  Cell(int32_t _x, int32_t _y) : AnyPoint<int32_t>(_x, _y) { }

  double d2(const Point &p) const;

  bool in_range(int32_t w, int32_t h) const
    { return (x >= 0 && y >= 0 && x < w && y < h); }
  void neighbors(std::map<Cell,uint32_t> *) const;
};

template <class T> struct CellMap {
  T *value;
  uint32_t n, w, h;

  CellMap(uint32_t _w, uint32_t _h) : w(_w), h(_h), n(_w * _h) {
    value = new T[n];
  }

  ~CellMap() {
    delete[] value;
  }

  T& operator [] (const Cell &c) const {
    return value[c.y * w + c.x];
  }
};

#endif
