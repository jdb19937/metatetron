#ifndef __REPLACEMENT_HH__
#define __REPLACEMENT_HH__ 1

#include "cell.hh"
#include "blob.hh"

struct Replacement {
  Replacement(const Cell &_c, BlobId _b) : c(_c), b(_b), err(0) { }
  Replacement(const Replacement &r) :
    c(r.c), b(r.b), err(r.err) { }

  Cell c;
  BlobId b;
  double err;

  void compute_err(class Field *f);
  void apply(class Field *f);
};

#endif

