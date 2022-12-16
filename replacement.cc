#include "replacement.hh"
#include "field.hh"

using namespace std;

void Replacement::compute_err(Field *f) {
  err = 0;

  BlobId a = f->cell_blob[c];
  if (a == b)
    return;

  if (a >= 0) {
    Blob *blob_a = f->blobs + a;
    Blob ta = *blob_a;
    ta.del_cell(c);
    ta.compute_err(f);
    err += ta.err - blob_a->err;
  }

  if (b >= 0) {
    Blob *blob_b = f->blobs + b;
    Blob tb = *blob_b;
    tb.add_cell(c);
    tb.compute_err(f);
    err += tb.err - blob_b->err;
  }
}

void Replacement::apply(Field *f) {
  double err0 = err;

  BlobId a = f->cell_blob[c];

  set<Cell> cns;
  f->cc_neighbors(c, &cns);

  f->delete_cell_ops(c);
  foreach (cni, cns)
    f->delete_cell_ops(*cni);

  if (a >= 0) {
    f->blobs[a].del_cell(c);
    if (f->blobs[a].n_cells == 0)
      throw "wtf2";
  }

  f->cell_blob[c] = b;

  if (b >= 0) {
    f->blobs[b].add_cell(c);
  }

  if (a >= 0)
    f->blobs[a].compute_err(f);
  if (b >= 0)
    f->blobs[b].compute_err(f);
  
  f->create_cell_ops(c);
  foreach (cni, cns)
    f->create_cell_ops(*cni);

  if (a >= 0) {
//    pthread_mutex_lock(&events_mutex);
//    events.insert(make_pair(0, a));
//    pthread_mutex_unlock(&events_mutex);
     f->recompute_blob_ops(a);
  }
  if (b >= 0) {
//    pthread_mutex_lock(&events_mutex);
//    events.insert(make_pair(0, b));
//    pthread_mutex_unlock(&events_mutex);
     f->recompute_blob_ops(b);
  }
}

