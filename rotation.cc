#include "rotation.hh"
#include "cell.hh"
#include "field.hh"

using namespace std;

void Rotation::compute_err(Field *f) {
  err = 0;

  if (dir == 0)
    return;

  Cell cc[4] = {c, c, c, c};
  ++cc[1].x;
  ++cc[2].y;
  ++cc[3].x;
  ++cc[3].y;

  BlobId bi[4] = {
    f->cell_blob[cc[0]],
    f->cell_blob[cc[1]],
    f->cell_blob[cc[2]],
    f->cell_blob[cc[3]]
  };

  Blob *bb[4] = {
    f->blobs + bi[0],
    f->blobs + bi[1],
    f->blobs + bi[2],
    f->blobs + bi[3]
  };

  const static int p[4][4] = {
    {0, 1, 2, 3},
    {1, 3, 0, 2},
    {2, 0, 3, 1},
    {3, 2, 1, 0}
  };

  if (dir >= 4)
    throw "bad dir";
  const int *q = p[dir];

  err += bb[0]->move_cost(cc[0], cc[q[0]]);
  err += bb[0]->move_cost(cc[1], cc[q[1]]);
  err += bb[0]->move_cost(cc[2], cc[q[2]]);
  err += bb[0]->move_cost(cc[3], cc[q[3]]);
}

void Rotation::apply(Field *f) {
  if (dir == 0)
    return;

  Cell cc[4] = {c, c, c, c};
  ++cc[1].x;
  ++cc[2].y;
  ++cc[3].x;
  ++cc[3].y;

  BlobId bi[4] = {
    f->cell_blob[cc[0]],
    f->cell_blob[cc[1]],
    f->cell_blob[cc[2]],
    f->cell_blob[cc[3]]
  };

  Blob *bb[4] = {
    f->blobs + bi[0],
    f->blobs + bi[1],
    f->blobs + bi[2],
    f->blobs + bi[3]
  };

  const static int p[4][4] = {
    {0, 1, 2, 3},
    {1, 3, 0, 2},
    {2, 0, 3, 1},
    {3, 2, 1, 0}
  };

  if (dir >= 4)
    throw "bad dir";
  const int *q = p[dir];

  bb[0]->do_move(cc[0], cc[q[0]]); f->cell_blob[cc[q[0]]] = bi[0];
  bb[1]->do_move(cc[1], cc[q[1]]); f->cell_blob[cc[q[1]]] = bi[1];
  bb[2]->do_move(cc[2], cc[q[2]]); f->cell_blob[cc[q[2]]] = bi[2];
  bb[3]->do_move(cc[3], cc[q[3]]); f->cell_blob[cc[q[3]]] = bi[3];
}
