#include "cell.hh"

using namespace std;

double Cell::d2(const Point &p) const {
  double dx = p.x - x;
  double dy = p.y - y;
  return (dx * dx + dy * dy);
}

