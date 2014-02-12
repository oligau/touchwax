#ifndef CLOSEUP_H
#define CLOSEUP_H

#include <pthread.h>
#include "SDL.h"

#include "track.h"

struct tile {
  int offset;
  SDL_Surface *surface;
  SDL_Texture *texture; 
};

struct closeup {
  SDL_Rect rect;
  int padded_h;
  int texture_w;
  int texture_h;
  int nb_tile;
  int last_length;
  int clicked;
  SDL_Renderer *renderer;
  struct track *tr;
  struct tile *tile_prev;
  struct tile *tile_current;
  struct tile *tile_next;
  
  pthread_t thread_tile_updater;
  int thread_tile_updater_done;
  int rendered_tile;
  int tile;
  int modified;
};

struct closeup *closeup_init(int x, int y, int w, int h, struct track *tr, SDL_Renderer *renderer);
void closeup_update_init(struct closeup *closeup);

void closeup_start_tile_updater_thread(struct closeup *closeup);
void *closeup_tile_updater(void *param);


void closeup_update(struct closeup *closeup);
void closeup_handle_events(struct closeup *closeup, SDL_Event event);
void closeup_draw_waveform(struct closeup *closeup, SDL_Surface *surface, int offset, SDL_Color col);
void closeup_show(struct closeup *closeup);
void closeup_free(struct closeup *closeup);

#endif
