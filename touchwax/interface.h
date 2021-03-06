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
  struct deck *deck[2];
  struct closeup *closeup;  
  struct overview *overview;  
  struct button *btn_play;
  struct button *btn_reset;
  struct button *btn_reverse; 
  struct button *btn_deck;
  struct button *btn_touch_mode;
  struct label *label_pitch;
  struct fader *fader;
  int last_track_length;
  int current_deck;
  
};

void interface_button_reset_callback(struct twinterface *twinterface);
int interface_button_play_color_callback(struct twinterface *twinterface, int depressed);
void interface_button_reverse_callback(struct twinterface *twinterface);
int interface_button_reverse_color_callback(struct twinterface *twinterface, int depressed);
void interface_button_play_callback(struct twinterface *twinterface);
int interface_button_play_color_callback(struct twinterface *twinterface, int depressed);
void interface_button_deck_callback(struct twinterface *twinterface);
int interface_button_play_deck_callback(struct twinterface *twinterface, int depressed);
void interface_button_touch_mode_callback(struct twinterface *twinterface);
int interface_button_touch_mode_color_callback(struct twinterface *twinterface, int depressed);

void interface_update_overview(struct twinterface *twinterface);
void interface_closeup_init(struct twinterface *twinterface);
void interface_closeup_free(struct twinterface *twinterface);
void interface_overview_init(struct twinterface *twinterface);
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
