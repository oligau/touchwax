#ifndef INTERFACE_H
#define INTERFACE_H

#include "SDL.h"

#include "track.h"

struct interface {
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

void interface_closeup_init(struct interface *interface);
void interface_widgets_init(struct interface *interface);
struct interface *interface_init();
void interface_resize(struct interface *interface, int w, int h);
void interface_loop(struct interface *interface);
void interface_update_closeup(struct interface *interface);
void interface_free(struct interface *interface);

/* Timer related functions */
Uint32 ticker(Uint32 interval, void *p);
void push_event(int t);

Uint32 interface_palette(SDL_Surface *sf, SDL_Color *col);




#endif
