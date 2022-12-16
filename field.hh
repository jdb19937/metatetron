#ifndef __FIELD_HH__
#define __FIELD_HH__ 1

#include "useful.hh"
#include "point.hh"
#include "cell.hh"
#include "blob.hh"
#include "replacement.hh"
#include "view.hh"

struct Field {
  uint32_t w, h, scale;
  uint32_t ts;

  double Wd, Wa, Wc, Ta, sTa;

  Field(uint32_t _w = 80, uint32_t _h = 100, uint32_t _scale = 3,
    double _Wd = 8, double _Wa = 1, double _Wc = 0.0,
    double _Ta = 64
  ) : w(_w), h(_h), scale(_scale), Wd(_Wd), Wa(_Wa), Wc(_Wc), Ta(_Ta), cell_blob(w, h) {
    ts = 0;
    sTa = sqrt(Ta);
    _create();
  }

  void _create();

  Blob *blobs;
  uint32_t n_blobs;

  CellMap<BlobId> cell_blob;

  std::multimap<Cell, Rotation*> cell_op;
  std::multimap<BlobId, Rotation*> blob_op;

  std::set<std::pair<double, Rotation*> > opq;
  pthread_mutex_t opq_mutex;
  pthread_cond_t opq_cond;
  pthread_t opq_thread;
  void start_opq_thread();

  void delete_op(Rotation *);
  void create_cell_ops(const Cell &);
  void delete_cell_ops(const Cell &);
  void recompute_blob_ops(BlobId b);
  void recompute_cell_ops(const Cell &);

  void cc_neighbors(const Cell &, std::set<Cell> *) const;
  void cb_neighbors(const Cell &, std::set<BlobId> *) const;
  uint32_t count_blob_neighbors(const Cell &, BlobId) const;

  void evolve();

  Point _rotate(const Point &p, double dth) {
    Point q(p.x - w/2, p.y - h/2);
    double r = sqrt(q.x * q.x + q.y * q.y);
    double th = atan2(q.y, q.x) + dth;
    return Point(r * cos(th) + w/2, r * sin(th) + h/2);
  }

  void lock_blobs(BlobId a, BlobId b) {
    if (a > b) {
      BlobId c = b;
      b = a;
      a = c;
    }
    if (a >= 0)
      blobs[a].lock();
    if (b >= 0)
      blobs[b].lock();
  }

  void unlock_blobs(BlobId a, BlobId b) {
    if (a > b) {
      BlobId c = b;
      b = a;
      a = c;
    }
    if (b >= 0)
      blobs[b].unlock();
    if (a >= 0)
      blobs[a].unlock();
  }

  pthread_mutex_t events_mutex;
  std::multimap<double, BlobId> events;

  pthread_t evo_thread;
  void start_evo_thread();

  pthread_t view_thread;
  void start_view_thread();

  int run();
  int wait();
};

#endif
