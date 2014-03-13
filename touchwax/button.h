#ifndef BUTTON_H
#define BUTTON_H

#include "SDL.h"

#include "track.h"

struct button {
  SDL_Rect rect;
  //SDL_Rect *clip;
  //SDL_Rect clips[4];
  SDL_Surface *buttonSheet;
  SDL_Color *col;
  SDL_Renderer *renderer;
  SDL_Surface *surface;
  SDL_Texture *texture;
  void* (*callback)(struct twinterface *twinterface);
};

struct button *button_init(int x, int y, int w, int h, const char *filename, SDL_Renderer *renderer, void *callback);
int button_handle_events(struct button *btn, SDL_Event event, struct twinterface *twinterface);
void button_update_texture(struct button *btn);
void button_show(struct button *btn);
void button_free(struct button *btn);

SDL_Surface *button_load_image(const char *filename);
void button_apply_surface(int x, int y, SDL_Surface* source, SDL_Surface* destination, SDL_Rect* clip);
void button_set_clips();

Uint32 button_palette(SDL_Surface *sf, SDL_Color *col);



#endif
