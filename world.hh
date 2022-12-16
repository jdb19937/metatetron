#ifndef __WORLD_HH__
#define __WORLD_HH__ 1

#include "useful.hh"
#include "blob.hh"

struct World {
  uint32_t w, h;
  uint32_t wb, wb8, w18, hb, w1, h1;

  class Blob **blobs;
  uint32_t n_blobs;
  uint32_t max_blobs;

  int32_t *cell_blob;
  uint32_t n_cells;

  int64_t *op_error;
  uint32_t n_ops;
  std::set<std::pair<int64_t, uint32_t> > error_op;

  World() {
    wb = 9;
    wb8 = wb - 8;
    w = (1 << wb);
    w1 = w - 1;
    w18 = (1 << wb8) - 1;
    hb = 8;
    h = (1 << hb);
    h1 = h - 1;

    n_cells = w * h;
    cell_blob = new int32_t[n_cells];

    n_ops = n_cells * 4;
    op_error = new int64_t[n_ops];

    blobs = new Blob *[1 + (max_blobs = 1024)];
    n_blobs = 1;
    blobs[0] = new Blob; // empty blob
    blobs[0]->solid = false;
    blobs[0]->color = 0;
  }

  ~World() {
    for (Blob **b = blobs + n_blobs - 1; b >= blobs; --b)
      delete *b;
    delete[] blobs;
    delete[] op_error;
    delete[] cell_blob;
  }

  uint32_t add_blob(Blob *b) {
    if (n_blobs >= max_blobs) {
      Blob **new_blobs = new Blob *[1 + (max_blobs *= 2)];
      memcpy(new_blobs, blobs, n_blobs * sizeof(Blob *));
      delete[] blobs;
      blobs = new_blobs;
    }
    uint32_t i = n_blobs;
    ++n_blobs;
    blobs[i] = b;
    return i;
  }

  void set_cell_blob(BlobId b, int32_t x, int32_t y) {
    uint32_t c = cell_id(x, y);

    uint32_t ob = cell_blob[c];
    if (ob == b)
      return;
    cell_blob[c] = b;

    Blob *aa = blobs[ob];
    Blob *bb = blobs[b];

    aa->del_cell(x, y);
    if (aa->solid) {
      if (x == aa->bb_x0)
        while (aa->bb_x0 != aa->bb_x1 && !bb_contains_min(aa->bb_x0, aa->bb_y0, aa->bb_x0, aa->bb_y1, ob))
          ++aa->bb_x0;
      else if (x == aa->bb_x1)
        while (aa->bb_x1 != aa->bb_x0 && !bb_contains_min(aa->bb_x1, aa->bb_y0, aa->bb_x1, aa->bb_y1, ob))
          --aa->bb_x1;
      if (y == aa->bb_y0)
        while (aa->bb_y0 != aa->bb_y1 && !bb_contains_min(aa->bb_x0, aa->bb_y0, aa->bb_x1, aa->bb_y0, ob))
          ++aa->bb_y0;
      else if (y == aa->bb_y1)
        while (aa->bb_y1 != aa->bb_y0 && !bb_contains_min(aa->bb_x0, aa->bb_y1, aa->bb_x1, aa->bb_y1, ob))
          --aa->bb_y1;
    }

    bb->add_cell(x, y);
    if (bb->solid) {
      if (bb->n_cells == 1) {
        bb->bb_x0 = x;
        bb->bb_x1 = x;
        bb->bb_y0 = y;
        bb->bb_y1 = y;
      } else {
        if (x < bb->bb_x0) {
          bb->bb_x0 = x;
        } else if (x > bb->bb_x1) {
          bb->bb_x1 = x;
        }
  
        if (y < bb->bb_y0) {
          bb->bb_y0 = y;
        } else if (y > bb->bb_x1) {
          bb->bb_y1 = y;
        }
      }
    }

    static uint32_t opbuf[28];
    uint32_t *opp = opbuf, *opq;
    for (opq = gather_cell_opdeps(opp, x, y); opp < opq; ++opp)
      reindex_op(*opp);
  }

  uint32_t cell_id(uint32_t x, uint32_t y) {
#if 1
    return x + (y << wb);
#else
    return (
      (x & 0xFF) | ((y & 0xFF) << 8) |
      ((x & 0xFF00) << 8) |
      ((y & 0xFF00) << wb)
    );
#endif
  }

  uint16_t cell_x(uint32_t c) {
#if 1
    return (c & w1);
#else
    uint32_t x = (c & 0xFF);
    c >>= 16;
    x |= ((c & w18) << 8);
    return x;
#endif
  }

  uint16_t cell_y(uint32_t c) {
#if 1
    return (c >> wb);
#else
    uint32_t y = ((c & 0xFF00) >> 8);
    c >>= (8 + wb);
    y |= ((c & 0xFF) << 8);
    return y;
#endif
  }

  void cell_xy(uint32_t c, uint16_t *xp, uint16_t *yp) {
    *xp = cell_x(c);
    *yp = cell_y(c);
  }

  uint32_t *gather_cell_neighbors(uint32_t c, uint32_t *np) {
    uint16_t x, y;
    cell_xy(c, &x, &y);
    if (x > 0)
      *np++ = cell_id(x - 1, y);
    if (x < w1)
      *np++ = cell_id(x + 1, y);
    if (y > 0)
      *np++ = cell_id(x, y - 1);
    if (y < h1)
      *np++ = cell_id(x, y + 1);
    return np;
  }


  uint32_t op_id(int32_t x, int32_t y, int32_t dir) {
    return ((cell_id(x, y) << 2) | (dir & 3));
  }

  void op_xyd(int32_t op, int32_t *xp, int32_t *yp, int32_t *dp) {
    uint16_t x, y;
    cell_xy(op >> 2, &x, &y);
    *xp = x;
    *yp = y;
    *dp = (op & 3);
  }

  void op_cxyd(int32_t op, int32_t *cp, int32_t *xp, int32_t *yp, int32_t *dp) {
    uint16_t x, y;
    *cp = op >> 2;
    cell_xy(*cp, &x, &y);
    *xp = x;
    *yp = y;
    *dp = (op & 3);
  }

  uint32_t *gather_ops(uint32_t *ops, int32_t x, int32_t y, int32_t dir) {
    switch (dir) {
    case 0: // right
      if (x >= 0 && x < w1 && y >= 0 && y < h)
        *ops++ = op_id(x, y, 0);
      break;
    case 1: // down
      if (x >= 0 && x < w && y >= 0 && y < h1)
        *ops++ = op_id(x, y, 1);
      break;
    case 2: // right-down
      if (x >= 0 && x < w1 && y >= 0 && y < h1)
        *ops++ = op_id(x, y, 2);
      break;
    case 3: // right-up
      if (x >= 0 && x < w1 && y > 0 && y < h)
        *ops++ = op_id(x, y, 3);
      break;
    }
    return ops;
  }

  uint32_t *gather_ops(uint32_t *ops, int32_t x, int32_t y) {
    if (x < 0 || y < 0)
      return ops;

    if (x < w1 && y < h1) {
      int op = op_id(x, y, 0);
      *ops++ = op; // right swap
      *ops++ = op | 1; // down swap
      *ops++ = op | 2; // right-down swap
      if (y > 0)
        *ops++ = op | 3; // right-up swap
    } else if (x < w1 && y == h1) { // bottom edge
      int op = op_id(x, y, 0);
      *ops++ = op; // right swap
      *ops++ = op | 3; // right-up swap
    } else if (y < h1 && x == w1) { // right edge
      *ops++ = op_id(x, y, 1); // down swap
    }

    return ops;
  }

  uint32_t *gather_cell_opdeps(uint32_t *ops, int32_t x, int32_t y) {
    ops = gather_ops(ops, x, y);
    ops = gather_ops(ops, x - 1, y, 0); // right swap
    ops = gather_ops(ops, x, y - 1, 1); // down swap
    ops = gather_ops(ops, x - 1, y - 1, 2); // right-down swap
    ops = gather_ops(ops, x - 1, y + 1, 3); // right-up swap
    return ops;
  }

  uint32_t *gather_bb_opdeps(uint32_t *ops, int32_t x0, int32_t y0, int32_t x1, int32_t y1) {
    int x, y;

    for (x = x0; x <= x1; ++x)
      for (y = y0; y <= y1; ++y)
        ops = gather_ops(ops, x, y);

    for (x = x0; x <= x1; ++x)
      ops = gather_ops(ops, x, y0 - 1, 1);
    for (y = y0; y <= y1; ++y)
      ops = gather_ops(ops, x0 - 1, y, 0);

    for (y = y0; y <= y1; ++y) {
      ops = gather_ops(ops, x0 - 1, y - 1, 2);
      ops = gather_ops(ops, x0 - 1, y + 1, 3);
    }

    for (x = x0; x < x1; ++x) {
      ops = gather_ops(ops, x, y0 - 1, 2);
      ops = gather_ops(ops, x, y1 + 1, 3);
    }

    return ops;
  }

  uint32_t *gather_op_opdeps(uint32_t *ops, uint32_t op) {
    int32_t x, y, dir;
    op_xyd(op, &x, &y, &dir);

    ops = gather_cell_opdeps(ops, x, y);

    switch (dir) {
    case 0: // right swap
      ops = gather_ops(ops, x + 1, y);
      ops = gather_ops(ops, x + 1, y - 1, 1); // down swap
      ops = gather_ops(ops, x, y - 1, 2); // right-down swap
      ops = gather_ops(ops, x, y + 1, 3); // right-up swap
      break;
    case 1: // down swap
      ops = gather_ops(ops, x, y + 1);
      ops = gather_ops(ops, x - 1, y + 1, 0); // right swap
      ops = gather_ops(ops, x - 1, y, 2); // right-down swap
      ops = gather_ops(ops, x - 1, y + 2, 3); // right-up swap
      break;
    case 2: // right-down swap
      ops = gather_ops(ops, x + 1, y + 1);
      ops = gather_ops(ops, x, y + 1, 0); // right swap
      ops = gather_ops(ops, x + 1, y, 1); // down swap
      ops = gather_ops(ops, x, y + 2, 3); // right-up swap
      break;
    case 3: // right-up swap
      ops = gather_ops(ops, x + 1, y - 1);
      ops = gather_ops(ops, x, y - 1, 0); // right swap
      ops = gather_ops(ops, x + 1, y - 2, 1); // down swap
      ops = gather_ops(ops, x, y - 2, 2); // right-down swap
      break;
    }

    return ops;
  }

  bool op_affects(uint32_t op, BlobId b) {
    int32_t c, x, y, dir;
    op_cxyd(op, &c, &x, &y, &dir);

    if (cell_blob[c] == b)
      return true;

    switch (dir) {
    case 0:
      return (cell_blob[cell_id(x + 1, y)] == b);
    case 1:
      return (cell_blob[cell_id(x, y + 1)] == b);
    case 2:
      return (cell_blob[cell_id(x + 1, y + 1)] == b);
    case 3:
      return (cell_blob[cell_id(x + 1, y - 1)] == b);
    }

    return false;
  }

  void apply_op(uint32_t op) {
    int32_t c, x, y, dir;
    op_cxyd(op, &c, &x, &y, &dir);

    switch (dir) {
    case 0:
      apply_swap(c, cell_id(x + 1, y));
      break;
    case 1:
      apply_swap(c, cell_id(x, y + 1));
      break;
    case 2:
      apply_swap(c, cell_id(x + 1, y + 1));
      break;
    case 3:
      apply_swap(c, cell_id(x + 1, y - 1));
      break;
    }

    static uint32_t opbuf[32];
    uint32_t *opp = opbuf, *opq;
    opq = gather_op_opdeps(opbuf, op);
    for (opp = opbuf; opp < opq; ++opp)
      reindex_op(*opp);
  }

  int64_t cost_op(uint32_t op) {
    int32_t c, x, y, dir;
    op_cxyd(op, &c, &x, &y, &dir);

    int64_t err = 0;

    switch (dir) {
    case 0:
      err += cost_swap(c, cell_id(x + 1, y));
      break;
    case 1:
      err += cost_swap(c, cell_id(x, y + 1));
      break;
    case 2:
      err += cost_swap(c, cell_id(x + 1, y + 1));
      break;
    case 3:
      err += cost_swap(c, cell_id(x + 1, y - 1));
      break;
    }

    return err;
  }

  void reindex_op(uint32_t op) {
    int64_t err = cost_op(op);
    int64_t old_err = op_error[op];

    if (old_err == err)
      return;

    op_error[op] = err;

    if (old_err < 0)
      error_op.erase(std::make_pair(old_err, op));
    if (err < 0)
      error_op.insert(std::make_pair(err, op));
  }

  void improve() {
double t0 = now();
int i = 0;
    while (i < 500000) {
      typeof(error_op.begin()) eoi = error_op.begin();
      if (eoi == error_op.end())
        break;
      uint32_t op = eoi->second;
      if (eoi->first >= 0)
        break;

//fprintf(stderr, "executing op err=%lf op=%d\n", eoi->first, eoi->second);

      apply_op(op);
      ++i;
    }
double t1 = now(), dt = t1 - t0;
fprintf(stderr, "improve %lf seconds, %u ops, %lf ops/second\n", dt, i, i / dt);
  }

  void evolve() {
//    std::set<uint32_t> ops;


double t0 = now();
long i = 0;

#if 0
for (int x = 0; x < w1; ++x) {
for (int y = 0; y < h1; ++y) {
  uint32_t c = cell_id(x, y);
  reindex_op((c << 2) | 1);
//  reindex_op((c << 2) | 2);
  reindex_op((c << 2) | 3);
  i += 3;
}
}
#endif

#if 1
    for (int b = 0; b < n_blobs; ++b) {
      Blob *bl = blobs[b];

      if (!bl->solid)
        continue;
      if (!bl->dirty)
        continue;

      static uint32_t opbuf[65536];
      uint32_t *opp = opbuf, *opq;
      for (opq = gather_bb_opdeps(opp, bl->bb_x0, bl->bb_y0, bl->bb_x1, bl->bb_y1); opp < opq; ++opp) {
        int32_t op = *opp;

        if (op_affects(op, b))
          reindex_op(op);
      }
    }

//    foreach (opi, ops)
//      reindex_op(*opi);
#endif

double t1 = now(), dt = t1 - t0;
fprintf(stderr, "evolve %lf seconds, %u blobs, %lf blobs/second\n", dt, n_blobs, n_blobs / dt);
  }

  void apply_swap(uint32_t a, uint32_t b) {
    BlobId z = cell_blob[a];
    execute_move(b, a);
    execute_move(a, b, z);

    apply_move(b, a, cell_blob[a]);
    apply_move(a, b, z);
  }

  int64_t cost_swap(uint32_t a, uint32_t b) {
    uint32_t xa = cell_blob[a], xb = cell_blob[b];
    if (xa == xb)
      return 0;
    return cost_move(b, a) + cost_move(a, b);
  }



  void apply_rotate(uint32_t a, uint32_t b, uint32_t c, uint32_t d) {
    uint32_t xa = cell_blob[a], xb = cell_blob[b];
    if (xa == xb) {
      apply_rotate(b, c, d);
      return;
    }
    uint32_t xc = cell_blob[c];
    if (xb == xc) {
      apply_rotate(a, c, d);
      return;
    }
    uint32_t xd = cell_blob[d];
    if (xc == xd) {
      apply_rotate(a, b, d);
      return;
    }
    if (xa == xd) {
      apply_rotate(a, b, c);
      return;
    }
  
    BlobId z = cell_blob[a];
    execute_move(b, a);
    execute_move(c, b);
    execute_move(d, c);
    execute_move(a, d, z);

    apply_move(b, a, cell_blob[a]);
    apply_move(c, b, cell_blob[b]);
    apply_move(d, c, cell_blob[c]);
    apply_move(a, d, z);
  }

  int64_t cost_rotate(uint32_t a, uint32_t b, uint32_t c, uint32_t d) {
    uint32_t xa = cell_blob[a], xb = cell_blob[b];
    if (xa == xb)
      return cost_rotate(b, c, d);
    uint32_t xc = cell_blob[c];
    if (xb == xc)
      return cost_rotate(a, c, d);
    uint32_t xd = cell_blob[d];
    if (xc == xd)
      return cost_rotate(a, b, d);
    if (xa == xd)
      return cost_rotate(a, b, c);
  
    return cost_move(b, a) + cost_move(c, b) + cost_move(d, c) + cost_move(a, d);
  }

  void apply_rotate(uint32_t a, uint32_t b, uint32_t c) {
    uint32_t xa = cell_blob[a], xb = cell_blob[b];
    if (xa == xb) {
      apply_swap(b, c);
      return;
    }
    uint32_t xc = cell_blob[c];
    if (xb == xc) {
      apply_swap(a, c);
      return;
    }
    if (xa == xc) {
      apply_swap(a, b);
      return;
    }

    BlobId z = cell_blob[a];
    execute_move(b, a);
    execute_move(c, b);
    execute_move(a, c, z);

    apply_move(b, a, cell_blob[a]);
    apply_move(c, b, cell_blob[b]);
    apply_move(a, c, z);
  }

  int64_t cost_rotate(uint32_t a, uint32_t b, uint32_t c) {
    uint32_t xa = cell_blob[a], xb = cell_blob[b];
    if (xa == xb)
      return cost_swap(b, c);
    uint32_t xc = cell_blob[c];
    if (xb == xc)
      return cost_swap(a, c);
    if (xa == xc)
      return cost_swap(a, b);

    return cost_move(b, a) + cost_move(c, b) + cost_move(a, c);
  }

  void apply_move(uint32_t ca, uint32_t cb, uint32_t z) {
    int32_t ax = cell_x(ca), ay = cell_y(ca);
    int32_t bx = cell_x(cb), by = cell_y(cb);

    Blob *b = blobs[z];
    b->apply_move(ax, ay, bx, by);

    if (!b->solid)
      return;

    switch (b->n_cells) {
    case 0:
      break;
    case 1:
      b->bb_x0 = b->sum_cx;
      b->bb_x1 = b->sum_cx;
      b->bb_y0 = b->sum_cy;
      b->bb_y1 = b->sum_cy;
      break;
    default:
      {
        if (bx < b->bb_x0)
          b->bb_x0 = bx;
        else if (bx > b->bb_x1)
          b->bb_x1 = bx;

        if (by < b->bb_y0)
          b->bb_y0 = by;
        else if (by > b->bb_y1)
          b->bb_y1 = by;

        if (ax == b->bb_x0 && bx > ax)
          while (b->bb_x0 != b->bb_x1 && !bb_contains_min(b->bb_x0, b->bb_y0, b->bb_x0, b->bb_y1, z))
            ++b->bb_x0;
        else if (ax == b->bb_x1 && bx < ax)
          while (b->bb_x1 != b->bb_x0 && !bb_contains_min(b->bb_x1, b->bb_y0, b->bb_x1, b->bb_y1, z))
            --b->bb_x1;
        if (ay == b->bb_y0 && by > ay)
          while (b->bb_y0 != b->bb_y1 && !bb_contains_min(b->bb_x0, b->bb_y0, b->bb_x1, b->bb_y0, z))
            ++b->bb_y0;
        else if (ay == b->bb_y1 && by < ay)
          while (b->bb_y1 != b->bb_y0 && !bb_contains_min(b->bb_x0, b->bb_y1, b->bb_x1, b->bb_y1, z))
            --b->bb_y1;
      }
      break;
    }
  }

  void execute_move(uint32_t a, uint32_t b) {
    execute_move(a, b, cell_blob[a]);
  }

  void execute_move(uint32_t a, uint32_t b, BlobId z) {
    cell_blob[b] = z;
  }

  int64_t cost_move(uint32_t a, uint32_t b) {
    int32_t ax = cell_x(a), ay = cell_y(a);
    int32_t bx = cell_x(b), by = cell_y(b);
    return blobs[cell_blob[a]]->cost_move(ax, ay, bx, by);
  }

  bool bb_contains_min(uint32_t x0, uint32_t y0, uint32_t x1, uint32_t y1, BlobId b, uint32_t n = 1) {
    uint k = 0;

    for (int y = y0; y <= y1; ++y) {
      for (int x = x0; x <= x1; ++x) {
        uint32_t c = cell_id(x, y);
        if (cell_blob[c] == b) {
          ++k;
          if (k >= n)
            return true;
        }
      }
    }

    return false;
  }
};

#endif
