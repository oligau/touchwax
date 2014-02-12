#ifndef FADER_H
#define FADER_H

#include "SDL.h"

#include "track.h"
#include "osc.h"

struct fader {
  SDL_Rect rect;
  int clicked;
  int heigth;
  SDL_Color *col;
  SDL_Renderer *renderer;
  SDL_Surface *surface;
  SDL_Texture *texture;  
};

struct fader *fader_init(int x, int y, int w, int h, int heigth, SDL_Renderer *renderer);
int fader_handle_events(struct fader *fader, SDL_Event event, int heigth);
void fader_update_texture(struct fader *fader);
void fader_show(struct fader *fader);
void fader_free(struct fader *fader);
void fader_pitch(struct fader *fader);

Uint32 fader_palette(SDL_Surface *sf, SDL_Color *col);



#endif
