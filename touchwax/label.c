#include <sys/stat.h>
#include <errno.h>

#include "label.h"

/* Macro functions */
#define MIN(x,y) ((x)<(y)?(x):(y))

#define BIG_FONT "DejaVuSans-Bold.ttf"
#define BIG_FONT_SIZE 14
#define BIG_FONT_SPACE 19

/* List of directories to use as search path for fonts. */

static const char *font_dirs[] = {
    ".",
    "/usr/X11R6/lib/X11/fonts/TTF",
    "/usr/share/fonts/truetype/ttf-dejavu",
    "/usr/share/fonts/ttf-dejavu",
    "/usr/share/fonts/dejavu",
    "/usr/share/fonts/TTF",
    "/usr/share/fonts/truetype/dejavu",
    "/usr/share/fonts/truetype/ttf-dejavu",
    NULL
};


static SDL_Color text_col = {255, 255, 255, 255},
                  background_col = {31, 4, 0, 255};


struct label *label_init(int x, int y, int w, int h, const char *text, SDL_Renderer *renderer)
{
  struct label *label;
  label = (struct label *) malloc(sizeof(struct label));
  label->rect.x = x;
  label->rect.y = y;
  label->rect.w = w;
  label->rect.h = h;
  label->renderer = renderer;
  label->surface = 0;
  label->texture = 0;
  memset(label->text, '\0', sizeof(label->text));

  
  //label->callback = callback;
  //btn->color_callback = color_callback;
  
  /* create surface that holds the label */
  /* SDL interprets each pixel as a 32-bit number, so our masks must depend
    on the endianness (byte order) of the machine */
  Uint32 rmask, gmask, bmask, amask;  
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
  rmask = 0xff000000;
  gmask = 0x00ff0000;
  bmask = 0x0000ff00;
  amask = 0x000000ff;
#else
  rmask = 0x000000ff;
  gmask = 0x0000ff00;
  bmask = 0x00ff0000;
  amask = 0xff000000;
#endif

  label->surface = SDL_CreateRGBSurface(0, w, h, 32,
                                   rmask, gmask, bmask, amask);   

  /* create texture that links the surface to gpu */
  label->texture = SDL_CreateTexture(label->renderer, SDL_PIXELFORMAT_ARGB8888, 
                    SDL_TEXTUREACCESS_STREAMING, w, h);
                    
  //SDL_SetTextureBlendMode(btn->texture, SDL_BLENDMODE_BLEND);
  //SDL_SetTextureAlphaMod(btn->texture, 128);
  
  if (TTF_Init() == -1) {
      fprintf(stderr, "%s\n", TTF_GetError());
      exit(-1);
  }
  
  label_load_fonts(label);
  
  /* Copy string */
  label_set_text(label, text);  

  return label;
}

TTF_Font* label_open_font(const char *name, int size) {
    int r, pt;
    char buf[256];
    const char **dir;
    struct stat st;
    TTF_Font *font;

    pt = size;

    dir = &font_dirs[0];

    while (*dir) {

        sprintf(buf, "%s/%s", *dir, name);

        r = stat(buf, &st);

        if (r != -1) { /* something exists at this path */
            fprintf(stderr, "Loading font '%s', %dpt...\n", buf, pt);

            font = TTF_OpenFont(buf, pt);
            if (!font)
                fprintf(stderr, "Font error: %s\n", TTF_GetError());
            return font; /* or NULL */
        }

        if (errno != ENOENT) {
            perror("stat");
            return NULL;
        }

        dir++;
        continue;
    }

    fprintf(stderr, "Font '%s' cannot be found in", name);

    dir = &font_dirs[0];
    while (*dir) {
        fputc(' ', stderr);
        fputs(*dir, stderr);
        dir++;
    }
    fputc('.', stderr);
    fputc('\n', stderr);

    return NULL;
}

int label_load_fonts(struct label *label)
{
  
  label->font = label_open_font(BIG_FONT, BIG_FONT_SIZE);
  if (!label->font) {
    printf("label_load_fonts: error: %s", TTF_GetError());
    return -1;
  }
  
  //If everything loaded fine 
  return 0; 
}

void label_set_text(struct label *label, const char *text)
{
  if(!(strcmp(label->text, text) == 0)) {
  
    memset(label->text, '\0', sizeof(label->text));
    strcpy(label->text, text);
    
    label_draw_text(label->surface, &label->rect,
                       label->text, label->font,
                       text_col, background_col);
    
    label_update_texture(label);

    //printf("label_set_text: text: %s\n", label->text);
  }
}

int label_draw_text(SDL_Surface *sf, SDL_Rect *rect,
                     const char *buf, TTF_Font *font,
                     SDL_Color fg, SDL_Color bg)
{
    SDL_Surface *rendered;
    SDL_Rect src, fill;

    if (buf == NULL) {
        src.w = 0;
        src.h = 0;

    } else if (buf[0] == '\0') { /* SDL_ttf fails for empty string */
        src.w = 0;
        src.h = 0;

    } else {
        rendered = TTF_RenderText_Shaded(font, buf, fg, bg);
        //fprintf(stderr, "%s\n", buf);

        src.x = 0;
        src.y = 0;
        src.w = MIN(rect->w, rendered->w);
        src.h = MIN(rect->h, rendered->h);
        
        SDL_BlitSurface(rendered, &src, sf, &src);
        SDL_FreeSurface(rendered);
    }

    /* Complete the remaining space with a blank rectangle */

    if (src.w < rect->w) {
        fill.x = rect->x + src.w;
        fill.y = rect->y;
        fill.w = rect->w - src.w;
        fill.h = rect->h;
        SDL_FillRect(sf, &fill, label_palette(sf, &bg));
    }

    if (src.h < rect->h) {
        fill.x = rect->x;
        fill.y = rect->y + src.h;
        fill.w = src.w; /* the x-fill rectangle does the corner */
        fill.h = rect->h - src.h;
        SDL_FillRect(sf, &fill, label_palette(sf, &bg));
    }
    
    return src.w;
}

//int button_handle_events(struct button *btn, SDL_Event event, struct twinterface *twinterface)
//{
     
     ////The mouse offsets
    //int x = 0, y = 0;

    ////If a mouse button was pressed
    //if( event.type == SDL_MOUSEBUTTONDOWN )
    //{
        ////If the left mouse button was pressed
        //if( event.button.button == SDL_BUTTON_LEFT )
        //{
            ////Get the mouse offsets
            //x = event.button.x;
            //y = event.button.y;

            ////If the mouse is over the button
            //if( ( x > btn->rect.x ) && ( x < btn->rect.x + btn->rect.w ) && ( y > btn->rect.y ) && ( y < btn->rect.y + btn->rect.h ) )
            //{
                ////Set the button sprite
                ////btn->clip = &btn->clips[ CLIP_MOUSEDOWN ];
                
                //btn->callback(twinterface);
                
                //// Swap color
                //if(btn->color_callback(twinterface, 1))
                  //btn->col = &stop_col;
                //else
                  //btn->col = &play_col;
                  
                //// Update gpu texture with current color
                //button_update_texture(btn);
                //SDL_SetTextureAlphaMod(btn->texture, 255);
                
                //return 1;  
            //}
        //}
    //} else if( event.type == SDL_MOUSEBUTTONUP )
    //{
        //// Swap color
        //if(btn->color_callback(twinterface, 0))
          //btn->col = &stop_col;
        //else
          //btn->col = &play_col;
          
        //// Update gpu texture with current color
        //button_update_texture(btn);
        
        //SDL_SetTextureAlphaMod(btn->texture, 128);
    //}
    
    //return 0;
//}

void label_update_texture(struct label *label)
{
    SDL_Rect fill_rect;
    fill_rect.x = 0; 
    fill_rect.y = 0;
    fill_rect.w = label->rect.w; 
    fill_rect.h = label->rect.h;
        
    /* Upload text surface to GPU */                
    SDL_UpdateTexture(label->texture, NULL, label->surface->pixels, label->surface->pitch);
}
  
void label_show(struct label *label)
{    
    SDL_RenderCopy(label->renderer, label->texture, NULL, &label->rect);
}

void label_free(struct label *label)
{
  SDL_FreeSurface(label->surface);
  SDL_DestroyTexture(label->texture);
	TTF_CloseFont(label->font); 
	TTF_Quit();
  free(label);
}


Uint32 label_palette(SDL_Surface *sf, SDL_Color *col)
{
    return SDL_MapRGB(sf->format, col->r, col->g, col->b);
}
