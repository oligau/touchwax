#include "button.h"
#include "osc.h"

/* Colors used */
static SDL_Color play_col = {0, 255, 0, 255},
                 stop_col = {0, 0, 255, 255};

/* The button states in the sprite sheet */
const int CLIP_MOUSEOVER = 0;
const int CLIP_MOUSEOUT = 1;
const int CLIP_MOUSEDOWN = 2;
const int CLIP_MOUSEUP = 3;

struct button *button_init(int x, int y, int w, int h, const char *filename, SDL_Renderer *renderer, void *callback)
{
  struct button *btn;
  btn = (struct button *) malloc(sizeof(struct button));
  btn->rect.x = x;
  btn->rect.y = y;
  btn->rect.w = w;
  btn->rect.h = h;
  //button_set_clips(btn);
  //btn->buttonSheet = button_load_image(filename);
  btn->buttonSheet = 0;
  btn->clip = &(btn->clips[ CLIP_MOUSEOUT ]);
  btn->col = &stop_col;
  btn->renderer = renderer;
  btn->callback = callback;
  
  /* create surface that holds the button */
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

  btn->surface = SDL_CreateRGBSurface(0, w, h, 32,
                                   rmask, gmask, bmask, amask);   

  /* create texture that links the surface to gpu */
  btn->texture = SDL_CreateTexture(btn->renderer, SDL_PIXELFORMAT_ARGB8888, 
                    SDL_TEXTUREACCESS_STREAMING, w, h);

  button_update_texture(btn);
  return btn;
}

int button_handle_events(struct button *btn, SDL_Event event, struct twinterface *twinterface)
{
     
     //The mouse offsets
    int x = 0, y = 0;

    //If a mouse button was pressed
    if( event.type == SDL_MOUSEBUTTONDOWN )
    {
        //If the left mouse button was pressed
        if( event.button.button == SDL_BUTTON_LEFT )
        {
            //Get the mouse offsets
            x = event.button.x;
            y = event.button.y;

            //If the mouse is over the button
            if( ( x > btn->rect.x ) && ( x < btn->rect.x + btn->rect.w ) && ( y > btn->rect.y ) && ( y < btn->rect.y + btn->rect.h ) )
            {
                //Set the button sprite
                btn->clip = &btn->clips[ CLIP_MOUSEDOWN ];
                
                // Toggle play
                // track_toggle_play(0);
                btn->callback(twinterface);
                
                // Swap color
                if(btn->col == &play_col)
                  btn->col = &stop_col;
                else
                  btn->col = &play_col;
                  
                // Update gpu texture with current color
                button_update_texture(btn);
                
                return 1;  
            }
        }
    } else if( event.type == SDL_MOUSEBUTTONUP )
    {
      if( ( x > btn->rect.x ) && ( x < btn->rect.x + btn->rect.w ) && ( y > btn->rect.y ) && ( y < btn->rect.y + btn->rect.h ) )
        return 1;
    }
    
    return 0;
}

void button_update_texture(struct button *btn)
{
    SDL_Rect fill_rect;
    fill_rect.x = 0; fill_rect.y = 0;
    fill_rect.w = btn->rect.w; fill_rect.h = btn->rect.h;
    SDL_FillRect(btn->surface, &fill_rect, button_palette(btn->surface, btn->col));                
    SDL_UpdateTexture(btn->texture, NULL, btn->surface->pixels, btn->surface->pitch);
}
  
void button_show(struct button *btn)
{

    //button_apply_surface(btn->box.x, btn->box.y, btn->buttonSheet, surface, btn->clip );
    
    SDL_RenderCopy(btn->renderer, btn->texture, NULL, &btn->rect);
}

void button_free(struct button *btn)
{
  if(btn->buttonSheet != NULL)
    SDL_FreeSurface(btn->buttonSheet);
  SDL_FreeSurface(btn->surface);
  SDL_DestroyTexture(btn->texture);
  free(btn);
}

SDL_Surface *button_load_image(const char *filename)
{
    //The image that's loaded
    SDL_Surface* loadedImage = NULL;

    //Load the image
    loadedImage = SDL_LoadBMP(filename);

    //Return the optimized surface
    return loadedImage;
}

void button_apply_surface(int x, int y, SDL_Surface* source, SDL_Surface* destination, SDL_Rect* clip)
{
    //Holds offsets
    SDL_Rect offset;

    //Get offsets
    offset.x = x;
    offset.y = y;

    //Blit
    SDL_BlitSurface(source, clip, destination, &offset);
}

void button_set_clips(struct button *btn)
{
    //Clip the sprite sheet
    btn->clips[ CLIP_MOUSEOVER ].x = 0;
    btn->clips[ CLIP_MOUSEOVER ].y = 0;
    btn->clips[ CLIP_MOUSEOVER ].w = 320;
    btn->clips[ CLIP_MOUSEOVER ].h = 240;

    btn->clips[ CLIP_MOUSEOUT ].x = 320;
    btn->clips[ CLIP_MOUSEOUT ].y = 0;
    btn->clips[ CLIP_MOUSEOUT ].w = 320;
    btn->clips[ CLIP_MOUSEOUT ].h = 240;

    btn->clips[ CLIP_MOUSEDOWN ].x = 0;
    btn->clips[ CLIP_MOUSEDOWN ].y = 240;
    btn->clips[ CLIP_MOUSEDOWN ].w = 320;
    btn->clips[ CLIP_MOUSEDOWN ].h = 240;

    btn->clips[ CLIP_MOUSEUP ].x = 320;
    btn->clips[ CLIP_MOUSEUP ].y = 240;
    btn->clips[ CLIP_MOUSEUP ].w = 320;
    btn->clips[ CLIP_MOUSEUP ].h = 240;
}

Uint32 button_palette(SDL_Surface *sf, SDL_Color *col)
{
    return SDL_MapRGB(sf->format, col->r, col->g, col->b);
}
