#ifndef OVERVIEW_H
#define OVERVIEW_H

#include "SDL.h"

#include "track.h"

struct overview {
  SDL_Rect rect;
  int clicked;
  SDL_Surface *surface;
  SDL_Texture *texture;
  SDL_Renderer *renderer;
  struct track *tr;  
};

struct overview *overview_init(int x, int y, int w, int h, struct track *tr, SDL_Renderer *renderer, struct twinterface *twinterface);
int overview_handle_events(struct overview *overview, SDL_Event event);
void overview_show(struct overview *overview);
void overview_draw(struct overview *overview);
void overview_free(struct overview *overview);

#endif
