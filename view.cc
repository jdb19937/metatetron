#include "view.hh"
#include "world.hh"

void View::poll() {
  SDL_Event event;
  static int x = 0, y = 0;

  while (SDL_PollEvent(&event)) {
    switch (event.type) {
    case SDL_MOUSEMOTION: {
      x = event.motion.x >> scale;
      y = event.motion.y >> scale;
      break;
    }
    case SDL_QUIT:
    case SDL_KEYDOWN:
      throw 0;
    }
  }

}

void View::copy() {
  uint32_t pb = screen->pitch / 4;
  
  if (SDL_MUSTLOCK(screen) && SDL_LockSurface(screen) < 0)
    throw 1;

  uint32_t *p = (uint32_t *)screen->pixels;
  uint32_t w = screen->w, h = screen->h;
  for (uint32_t x = 0; x < w; ++x) {
    for (uint32_t yw = 0, y = 0; y < h; yw += pb, ++y) {
      BlobId b = world->cell_blob[world->cell_id(x >> scale, y >> scale)];
      p[yw + x] = b < 0 ? 0 : world->blobs[b]->color;
    }
  }

  if (SDL_MUSTLOCK(screen))
    SDL_UnlockSurface(screen);
  SDL_Flip(screen); 
}
