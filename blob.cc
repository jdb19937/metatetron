#include "blob.hh"
#include "world.hh"

using namespace std;

void Blob::add_cell(int32_t cx, int32_t cy) {
  if (!solid)
    return;

  int64_t cx2 = cx * cx;
  int64_t cy2 = cy * cy;

  sum_cx += cx;
  sum_cy += cy;
  sum_cx2 += cx2;
  sum_cy2 += cy2;

  sum_d2 += cx2 + px * (px - 2 * cx);
  sum_d2 += cy2 + py * (py - 2 * cy);

  ++n_cells;
}

void Blob::del_cell(int32_t cx, int32_t cy) {
  if (!solid)
    return;

  int64_t cx2 = cx * cx;
  int64_t cy2 = cy * cy;

  sum_cx -= cx;
  sum_cy -= cy;
  sum_cx2 -= cx * cx;
  sum_cy2 -= cy * cy;
  sum_d2 -= cx2 + px * (px - 2 * cx);
  sum_d2 -= cy2 + py * (py - 2 * cy);

  --n_cells;
}
