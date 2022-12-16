#include "field.hh"

using namespace std;

void Field::_create() {
  pthread_mutex_init(&opq_mutex, NULL);
  pthread_cond_init(&opq_cond, NULL);

  pthread_mutex_init(&events_mutex, NULL);

  for (uint32_t x = 0; x < w; ++x) {
    for (uint32_t y = 0; y < h; ++y) {
      Cell c = Cell(x, y);
      cell_blob[c] = -1;
    }
  }

  const static double f = 0.3;
  double x0 = f * w, x1 = (1-f) * w;
  double y0 = f * h, y1 = (1-f) * h;

  double skip = 1;

  n_blobs = 0;
  for (double x = x0; x < x1; x += sTa + skip) {
    for (double y = y0; y < y1; y += sTa + skip) {
      ++n_blobs;
    }
  }
  blobs = new Blob[n_blobs];

  uint32_t bi = 0;
  for (double x = x0; x < x1; x += sTa + skip) {
    for (double y = y0; y < y1; y += sTa + skip) {
      Blob *b = blobs + bi;
      b->target = Point(x, y); //_rotate(Point(x,y), 1); //Point(w/2, h/2); //_rotate(Point(x, y), 0);
#if 1
      double th = rand();
     double rr = 1.0;
     b->velocity = Point(rr * cos(th), rr * sin(th));
#endif

      int ix0 = (int)x, ix1 = (int)(x + sTa);
      int iy0 = (int)y, iy1 = (int)(y + sTa);
      b->color = (rand() & 0x7F7F7F) + (rand() & 0x3F3F3F) + 0x303030;
if (bi < 8)
b->color &= 0xFFFF00;
else
b->color &= 0xFFFF;

      for (int ix = ix0; ix < ix1; ++ix) {
        for (int iy = iy0; iy < iy1; ++iy) {
          Cell c(ix, iy);
          cell_blob[c] = bi;
          b->add_cell(c);
        }
      }

      b->compute_err(this);
      ++bi;
    }
  }

  for (uint32_t x = 0; x < w; ++x) {
    for (uint32_t y = 0; y < h; ++y) {
      Cell c = Cell(x, y);
      create_cell_ops(c);
    }
  }

  for (int bi = 0; bi < n_blobs; ++bi) {
    events.insert(make_pair(0, bi));
  }
}

void Field::cb_neighbors(const Cell &c, set<BlobId> *m) const {
  if (c.x > 0)
    m->insert(cell_blob[Cell(c.x - 1, c.y)]);
  if (c.x < w - 1)
    m->insert(cell_blob[Cell(c.x + 1, c.y)]);
  if (c.y > 0)
    m->insert(cell_blob[Cell(c.x, c.y - 1)]);
  if (c.y < h - 1)
    m->insert(cell_blob[Cell(c.x, c.y + 1)]);
}

uint32_t Field::count_blob_neighbors(const Cell &c, BlobId b) const {
  uint32_t count = 0;
  set<Cell> ns;
  cc_neighbors(c, &ns);
  foreach (nsi, ns)
    if (cell_blob[*nsi] == b)
      ++count;
  return count;
}

void Field::create_cell_ops(const Cell &c) {
  BlobId a = cell_blob[c];
  set<BlobId> bc;
  cb_neighbors(c, &bc);

  if (c.x == w - 1 || c.x == 0 || c.y == h - 1 || c.y == 0)
    bc.insert(-1);

  pthread_mutex_lock(&opq_mutex);

  foreach (bci, bc) {
    BlobId b = *bci;

    if (a == b)
      continue;
    if (a >= 0 && blobs[a].n_cells <= 1)
      continue;

    Replacement *r = new Replacement(c, b);
    r->compute_err(this);

    cell_op.insert(make_pair(c, r));
    blob_op.insert(make_pair(a, r));
    blob_op.insert(make_pair(b, r));
    if (r->err < 0) {
      pthread_cond_signal(&opq_cond);
      opq.insert(make_pair(r->err, r));
    }
  }

  pthread_mutex_unlock(&opq_mutex);
}

void Field::recompute_blob_ops(BlobId x) {
  pthread_mutex_lock(&opq_mutex);

  pair<typeof(blob_op.begin()), typeof(blob_op.begin())> ber = blob_op.equal_range(x);

  list<Replacement*> rs;

  for (typeof(blob_op.begin()) bi = ber.first; bi != ber.second; ++bi) {
    Replacement *r = bi->second;

    BlobId a = cell_blob[r->c];
    if (a < 0 || blobs[a].n_cells > 1) {
      if (r->err < 0)
        opq.erase(make_pair(r->err, r));
      r->compute_err(this);
      if (r->err < 0) {
        pthread_cond_signal(&opq_cond);
        opq.insert(make_pair(r->err, r));
      }
    } else {
      rs.push_back(r);
    }
  }

  pthread_mutex_unlock(&opq_mutex);

  foreach (ri, rs)
    delete_op(*ri);
}


void Field::recompute_cell_ops(const Cell &c) {
  delete_cell_ops(c);
  create_cell_ops(c);
}

void Field::delete_cell_ops(const Cell &c) {
  pthread_mutex_lock(&opq_mutex);

  pair<typeof(cell_op.begin()), typeof(cell_op.begin())> cer = cell_op.equal_range(c);
  for (typeof(cell_op.begin()) ci = cer.first; ci != cer.second; ++ci) {
    Replacement *r = ci->second;
    BlobId a = cell_blob[c], b = r->b;
    mm_erase_pair(blob_op, make_pair(b, r));
    mm_erase_pair(blob_op, make_pair(a, r));
    if (r->err < 0)
      opq.erase(make_pair(r->err, r));
    delete r;
  }
  cell_op.erase(cer.first, cer.second);

  pthread_mutex_unlock(&opq_mutex);
}

void Field::delete_op(Replacement *r) {
  const Cell &c = r->c;
  BlobId a = cell_blob[c], b = r->b;

  pthread_mutex_lock(&opq_mutex);

  mm_erase_pair(cell_op, make_pair(c, r));
  mm_erase_pair(blob_op, make_pair(a, r));
  mm_erase_pair(blob_op, make_pair(b, r));
  if (r->err < 0)
    opq.erase(make_pair(r->err, r));
  delete r;

  pthread_mutex_unlock(&opq_mutex);
}

void Field::cc_neighbors(const Cell &c, set<Cell> *m) const {
  if (c.x > 0)
    m->insert(Cell(c.x - 1, c.y));
  if (c.x < w - 1)
    m->insert(Cell(c.x + 1, c.y));
  if (c.y > 0)
    m->insert(Cell(c.x, c.y - 1));
  if (c.y < h - 1)
    m->insert(Cell(c.x, c.y + 1));
}

static void *_opq_thread(void *_f) {
  Field *f = (Field *)_f;

  try {
    while (1) {
      if (pthread_mutex_lock(&f->opq_mutex))
        throw -1;
  
      while (f->opq.empty())
        if (pthread_cond_wait(&f->opq_cond, &f->opq_mutex))
          throw -1;
  
      typeof(f->opq.begin()) opqi = f->opq.begin();
      if (opqi == f->opq.end())
        throw "huh";

      Replacement r = *opqi->second;
  
      if (pthread_mutex_unlock(&f->opq_mutex))
        throw -1;

      r.apply(f);
    }
  } catch (const char *x) {
    fprintf(stderr, "error: %s\n", x);
  } catch (const std::string &x) {
    fprintf(stderr, "error: %s\n", x.c_str());
  }

  return NULL;
}

void Field::start_opq_thread() {
  if (pthread_create(&opq_thread, NULL, _opq_thread, this))
    throw -1;
}

static void *_evo_thread(void *_f) {
  Field *f = (Field *)_f;

  try {
    while (1) {
      f->evolve();
//usleep(20000);
    }
  } catch (const char *x) {
    fprintf(stderr, "error: %s\n", x);
  } catch (const std::string &x) {
    fprintf(stderr, "error: %s\n", x.c_str());
  }

  return NULL;
}
  
void Field::start_evo_thread() {
  if (pthread_create(&evo_thread, NULL, _evo_thread, this))
    throw -1;
}

static void *_view_thread(void *_f) {
  Field *f = (Field *)_f;
  View *v = new View(f->w << f->scale, f->h << f->scale);
  long ret = -1;

  try {
    while (1) {
      v->copy(f);
      v->poll(f);
    }
  } catch (const char *x) {
    fprintf(stderr, "error: %s\n", x);
  } catch (const std::string &x) {
    fprintf(stderr, "error: %s\n", x.c_str());
  } catch (int x) {
    if (x == 0)
      ret = 0;
  }

  return ((void *)ret);
}

void Field::start_view_thread() {
  if (pthread_create(&view_thread, NULL, _view_thread, this))
    throw -1;
}

void Field::evolve() {

//Ta = 10.0 * sin(now() / (1 * 1e6)) + 30;
//sTa = sqrt(Ta);
  pthread_mutex_lock(&events_mutex);
  typeof(events.begin()) ei = events.begin();
  double t = ei->first;
  BlobId bi = ei->second;
  events.erase(ei);
  pthread_mutex_unlock(&events_mutex);
  
  double tt=  now();
//  if (t > tt)
//    usleep((t - tt) * 1000000.0);
//  tt = now();

  double dt = (tt - blobs[bi].lt);

Point force;
#if 1
    double g = 30.0;
    double k = 0.1;
    force = Point(-blobs[bi].sum_dx() / blobs[bi].n_cells / k, g + -blobs[bi].sum_dy() / blobs[bi].n_cells / k);
    double sl = 0.95;
    force *= dt;
    Point oldv = blobs[bi].velocity;
    blobs[bi].velocity += force;
    blobs[bi].velocity *= pow(sl, dt);
    force = blobs[bi].velocity - oldv;
#endif
#if 0
      double th = rand();
     double rr = 0.9;
     blobs[bi].velocity += Point(rr * cos(th), rr * sin(th));
#endif

  Point ot = blobs[bi].target;
    blobs[bi].set_target(blobs[bi].target + (oldv + force * 0.5) * dt);
blobs[bi].lt= tt;
//    blobs[bi].target += blobs[bi].velocity;

    blobs[bi].compute_err(this);
    recompute_blob_ops(bi);

//double nxt = 10/(1 + blobs[bi].velocity.length2() + force.length2()/dt);
double nxt = 0.01;
//fprintf(stderr, "%d nxt=%lf\n", bi, nxt);
  pthread_mutex_lock(&events_mutex);
  events.insert(make_pair(nxt + tt, bi));
  pthread_mutex_unlock(&events_mutex);
//  events.insert(make_pair(0.001 + now(), bi));
}

int Field::wait() {
  void *ret = (void *)-1, *eret;
  pthread_join(view_thread, &ret);
#if 0
  pthread_kill(evo_thread, 2);
  pthread_join(evo_thread, &eret);
  pthread_kill(opq_thread, 2);
  pthread_join(opq_thread, &eret);
#endif
  long rret = (long)ret;
  int r = (int)rret;
  return r;
}

int Field::run() {
  start_opq_thread();
  start_evo_thread();
  start_view_thread();
  return wait();
}
  
