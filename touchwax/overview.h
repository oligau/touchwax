#ifndef OVERVIEW_H
#define OVERVIEW_H

#include "SDL.h"

#include "track.h"
#include "interface.h"

struct overview {
  SDL_Rect rect;
  int clicked;
  SDL_Surface *surface;
  SDL_Texture *texture;
  SDL_Surface *playhead_surface;
  SDL_Texture *playhead_texture;
  SDL_Renderer *renderer;
  struct track *tr;  
  struct twinterface *twinterface;
};

struct overview *overview_init(int x, int y, int w, int h, struct track *tr, SDL_Renderer *renderer, struct twinterface *twinterface);
int overview_handle_events(struct overview *overview, SDL_Event event);
void overview_jump(struct overview *overview, int y);

void overview_show(struct overview *overview);
void overview_draw(struct overview *overview);
void overview_free(struct overview *overview);

Uint32 overview_palette(SDL_Surface *sf, SDL_Color *col);

#endif
