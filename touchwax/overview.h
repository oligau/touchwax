#ifndef OVERVIEW_H
#define OVERVIEW_H

#include "SDL.h"

#include "track.h"

struct overview {
  SDL_Rect rect;
  int clicked;
};

struct overview *overview_init(int x, int y, int w, int h);
int overview_handle_events(struct overview *overview, SDL_Event event);
void overview_show(struct overview *overview, SDL_Surface *surface, 
                    struct track *tr);
void overview_free(struct overview *overview);

#endif
