#include "world.hh"
#include "view.hh"

int main(int argc, char **argv) {
  World *w = new World;
  View *v = new View(w, 1);


  while (1) {
for (double t0 = now(), t1 = t0; t1 < t0 + 0.02; t1 = now()) {
    w->improve();
int zz = 0, ww = 0;
//for (int jj = 0; jj < 1; ++jj) {
//if (rand() % 3 == 0) {
while (zz < (1)  && ww < 1000) {
++ww;
int x0 = rand() % (w->w1 - 100) + 50;
int y0 = rand() % (w->h1 - 30) + 15;
int bad = 0;
int rr = 15;
int rrr = 28;
for (int x = x0 - rr; x < x0 + rr; ++x) {
if (bad) break;
for (int y = y0 - rr; y < y0 + rr; ++y) {
if (w->cell_blob[w->cell_id(x, y)]) { bad = 1; break; }
}}

if (bad) {
continue;
}

Blob *b = new Blob;
b->set_target(x0, y0);
b->vx = 0;

BlobId bi = w->add_blob(b);
 
for (int x = x0 - rr; x < x0 + rr; ++x) {
for (int y = y0 - rr; y < y0 + rr; ++y) {
if ((x-x0)*(x-x0) + (y-y0)*(y-y0) < rrr) {
if (b->n_cells >= 63)
break;
  w->set_cell_blob(bi, x, y);
}}}

++zz;
}

for (int i = 1; i < w->n_blobs; ++i) {
Blob *b = w->blobs[i];
double rx =  0; //((rand() % 1000) - 500) / 1500.0;
double ry =  0; //((rand() % 1000) - 500) / 1500.0;
double g = 0.8;
double k = 0.18;
b->vx += -b->sum_dx() * k/b->n_cells + rx;
b->vy += -b->sum_dy() * k/b->n_cells + ry + g;
double f = 0.96; //0.8;
b->vx *= f;
b->vy *= f;

b->dirty = 0;
b->add_target(b->vx, b->vy);

if (b->dirty) {
//  fprintf(stderr, "bvx=%lf, bvy=%lf\n", b->vx, b->vy);
}
}
w->evolve();

}
    v->copy();
    v->poll();
  }


  return 0;
}
