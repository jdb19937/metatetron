#ifndef __VIEW_HH__
#define __VIEW_HH__ 1

#include "useful.hh"
#include "world.hh"

#include <SDL/SDL.h>

struct View {
  class World *world;

  uint32_t w, h;
  int scale;
  SDL_Surface *screen;
  SDL_Event event;

  int32_t bb_x0, bb_x1;
  int32_t bb_y0, bb_y1;

  View(World *_world, int _scale) {
    world = _world;
    scale = _scale;
    w = (world->w << scale);
    h = (world->h << scale);

    bb_x0 = (world->w / 2) - (w / 2);
    bb_x1 = bb_x0 + w - 1;
    bb_y0 = (world->h / 2) - (h / 2);
    bb_y1 = bb_y0 + h - 1;

    if (SDL_Init(SDL_INIT_VIDEO) < 0)
      throw;
    if (!(screen = SDL_SetVideoMode(w, h, 32, SDL_HWSURFACE)))
      throw 1;
  }

  ~View() {
    SDL_Quit();
  }

  void copy();
  void poll();

  void update(int32_t x, int32_t y, BlobId b);
};

#endif
