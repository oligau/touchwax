#ifndef LABEL_H
#define LABEL_H

#include <SDL.h>

#include <SDL_ttf.h>

#define LABEL_MAX_BUFFER 1024

struct label{
  SDL_Rect rect;
  char text[LABEL_MAX_BUFFER];
  SDL_Renderer *renderer;
  SDL_Surface *surface;
  SDL_Texture *texture;
  TTF_Font *font;

};

struct label *label_init(int x, int y, int w, int h, const char *text, SDL_Renderer *renderer);
//int label_handle_events(struct button *btn, SDL_Event event, struct twinterface *twinterface);
void label_set_text(struct label *label, const char *text);
int label_draw_text(SDL_Surface *sf, SDL_Rect *rect,
                     const char *buf, TTF_Font *font,
                     SDL_Color fg, SDL_Color bg);

TTF_Font* label_open_font(const char *name, int size);
int label_load_fonts(struct label *label);
void label_update_texture(struct label *label);
void label_show(struct label *label);
void label_free(struct label *label);

Uint32 label_palette(SDL_Surface *sf, SDL_Color *col);



#endif
