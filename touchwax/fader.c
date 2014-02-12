#include "fader.h"

/* Colors used */
static SDL_Color fader_col = {255, 0, 0, 255};

struct fader *fader_init(int x, int y, int w, int h, int heigth, SDL_Renderer *renderer)
{
  struct fader *fader;
  fader = (struct fader *) malloc(sizeof(struct fader));
  fader->rect.x = x;
  fader->rect.y = y;
  fader->rect.w = w;
  fader->rect.h = h;
  fader->clicked = 0;
  fader->heigth = heigth;
  fader->col = &fader_col;
  fader->renderer = renderer;
  
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

  fader->surface = SDL_CreateRGBSurface(0, w, h, 32,
                                   rmask, gmask, bmask, amask);   

  /* create texture that links the surface to gpu */
  fader->texture = SDL_CreateTexture(fader->renderer, SDL_PIXELFORMAT_ARGB8888, 
                    SDL_TEXTUREACCESS_STREAMING, w, h);

  fader_update_texture(fader);

  return fader;
}

int fader_handle_events(struct fader *fader, SDL_Event event, int heigth)
{
     
    fader->heigth = heigth;
    
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
            if( ( x > fader->rect.x ) && ( x < fader->rect.x + fader->rect.w ) 
                && ( y > fader->rect.y ) && ( y < fader->rect.y + fader->rect.h ) )
            {
                fader->clicked = 1;
                return 1;
            }
        }
    } else if( event.type == SDL_MOUSEBUTTONUP )
    {
        //If the left mouse button was pressed
        if( event.button.button == SDL_BUTTON_LEFT )
        {
            fader->clicked = 0;
        }
        
    } else if( event.type == SDL_MOUSEMOTION )
    {
            if(fader->clicked) {
                fader->rect.y = event.motion.y - fader->rect.h/2;
                
                //Send pitch information
                fader_pitch(fader);
                if(tracks[0].play)
                    osc_send_pitch(tracks[0].pitch);
                
                return 1;
            }
    }
    
    return 0;
}

void fader_update_texture(struct fader *fader)
{
    SDL_Rect fill_rect;
    fill_rect.x = 0; fill_rect.y = 0;
    fill_rect.w = fader->rect.w; fill_rect.h = fader->rect.h;
    SDL_FillRect(fader->surface, &fill_rect, fader_palette(fader->surface, fader->col));                
    SDL_UpdateTexture(fader->texture, NULL, fader->surface->pixels, fader->surface->pitch);
}

void fader_show(struct fader *fader)
{
    SDL_RenderCopy(fader->renderer, fader->texture, NULL, &fader->rect);
}

void fader_free(struct fader *fader)
{
  SDL_FreeSurface(fader->surface);
  SDL_DestroyTexture(fader->texture);
  free(fader);
}


Uint32 fader_palette(SDL_Surface *sf, SDL_Color *col)
{
    return SDL_MapRGB(sf->format, col->r, col->g, col->b);
}

void fader_pitch(struct fader *fader)
{
    float y = ((float) fader->rect.y )/ fader->heigth;
    float min = 0.86f;
    float max = 1.16f;
    float pitch = y * (max-min) + min;
    tracks[0].pitch = pitch;
}
