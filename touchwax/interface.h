#ifndef INTERFACE_H
#define INTERFACE_H

#include "SDL.h"

#include "track.h"

struct twinterface{
  int redraw;
  int volumeup_pressed;
  int renderedFrames;  
  SDL_Rect viewport;
  SDL_Window *window;
  SDL_Surface *surface;
  SDL_Renderer *renderer;
  SDL_Texture *texture;
  SDL_TimerID timer; 
  struct closeup *closeup;  
  struct overview *overview;  
  struct button *btn;
  struct fader *fader;
  int last_track_length;
  
};

void interface_closeup_init(struct twinterface *twinterface);
void interface_widgets_init(struct twinterface *twinterface);
struct twinterface*interface_init();
void interface_resize(struct twinterface *twinterface, int w, int h);
void interface_loop(struct twinterface *twinterface);
void interface_update_closeup(struct twinterface *twinterface);
void interface_free(struct twinterface *twinterface);

/* Timer related functions */
Uint32 ticker(Uint32 interval, void *p);
void push_event(int t);

Uint32 interface_palette(SDL_Surface *sf, SDL_Color *col);




#endif
