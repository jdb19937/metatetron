#ifndef __ROTATION_HH__
#define __ROTATION_HH__ 1

#include "cell.hh"
#include "blob.hh"

struct Rotation {
  Rotation(const Cell &_c, int _dir) : c(_c), dir(_dir), err(0) { }
  Rotation(const Rotation &r) :
    c(r.c), dir(r.dir), err(r.err) { }

  Cell c;
  int dir;
  double err;

  void compute_err(class Field *f);
  void apply(class Field *f);
};

#endif

