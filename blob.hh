#ifndef __BLOB_HH__
#define __BLOB_HH__ 1

#include "useful.hh"

typedef int32_t BlobId;
typedef std::pair<BlobId,BlobId> BlobIdPair;

struct Blob {
  Blob() {
    px = py = 0;
    tx = ty = 0;
    vx = vy = 0;
    n_cells = 0;
    sum_cx = sum_cy = 0;
    sum_cx2 = sum_cy2 = 0;
    sum_d2 = 0;
    color = (rand() & 0x3F3F3F) + 0x7F7F7F;
    solid = true;
    dirty = true;
  }

  int32_t px, py;
  double tx, ty;
  double vx, vy;
  int32_t n_cells;
  int64_t sum_cx, sum_cx2;
  int64_t sum_cy, sum_cy2;
  uint32_t color;
  int64_t sum_d2;
  bool solid, dirty;

  int32_t bb_x0, bb_x1, bb_y0, bb_y1;

  void add_cell(int32_t cx, int32_t cy);
  void del_cell(int32_t cx, int32_t cy);

  int64_t cost_move(int32_t ax, int32_t ay, int32_t bx, int32_t by) {
    if (!solid)
      return 0.0;

    int64_t ax2 = ax * ax;
    int64_t ay2 = ay * ay;
    int64_t bx2 = bx * bx;
    int64_t by2 = by * by;

    int64_t dx = bx - ax;
    int64_t dy = by - ay;

    int64_t dx2 = bx2 - ax2;
    int64_t dy2 = by2 - ay2;

    return (dx2 + dy2 - 2 * (px * dx + py * dy));
  }

  void apply_move(int32_t ax, int32_t ay, int32_t bx, int32_t by) {
    if (!solid)
      return;

    int64_t ax2 = ax * ax;
    int64_t ay2 = ay * ay;
    int64_t bx2 = bx * bx;
    int64_t by2 = by * by;

    int64_t dx = bx - ax;
    int64_t dy = by - ay;
    sum_cx += dx;
    sum_cy += dy;

    int64_t dx2 = bx2 - ax2;
    int64_t dy2 = by2 - ay2;
    sum_cx2 += dx2;
    sum_cy2 += dy2;

    sum_d2 += dx2 + dy2 - 2 * (px * dx + py * dy);
  }

  void set_target(double ntx, double nty) {
    if (!solid)
      return;

    tx = ntx;
    ty = nty;

    int64_t npx = floor(0.5 + tx);
    int64_t npy = floor(0.5 + ty);

    if (npx != px || npy != py) {
      px = npx;
      py = npy;

      sum_d2 = (
        sum_cx2 + px * px * n_cells - 2 * px * sum_cx +
        sum_cy2 + py * py * n_cells - 2 * py * sum_cy
      );

      dirty = 1;
    }
  }

  void add_target(double dtx, double dty) {
    if (!solid)
      return;
    set_target(tx + dtx, ty + dty);
  }

  double sum_dx() const {
    if (!solid)
      return 0.0;
    return tx * n_cells - sum_cx;
  }

  double sum_dy() const {
    if (!solid)
      return 0.0;
    return ty * n_cells - sum_cy;
  }
};

#endif
