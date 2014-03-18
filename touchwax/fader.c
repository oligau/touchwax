#include "fader.h"

/* Colors used */
static SDL_Color fader_col = {255, 0, 0, 255};

struct fader *fader_init(int x, int y, int w, int h, int heigth, SDL_Renderer *renderer, struct twinterface *twinterface)
{
  struct fader *fader;
  fader = (struct fader *) malloc(sizeof(struct fader));
  fader->rect.x = x;
  fader->rect.y = y;
  fader->rect.w = w;
  fader->rect.h = h;
  fader->clicked = 0;
  fader->heigth = heigth;
  fader->pitch = 1.0f;
  fader->col = &fader_col;
  fader->renderer = renderer;
  fader->twinterface = twinterface;
  
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

  SDL_SetTextureBlendMode(fader->texture, SDL_BLENDMODE_BLEND);
  SDL_SetTextureAlphaMod(fader->texture, 128);

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
                SDL_SetTextureAlphaMod(fader->texture, 255);
                return 1;
            }
        }
    } else if( event.type == SDL_MOUSEBUTTONUP )
    {
        //If the left mouse button was pressed
        if( event.button.button == SDL_BUTTON_LEFT )
        {
            fader->clicked = 0;
            SDL_SetTextureAlphaMod(fader->texture, 128);
        }
        
    } else if( event.type == SDL_MOUSEMOTION )
    {
            if(fader->clicked) {
                fader->rect.y = event.motion.y;

                
                //Send pitch information
                fader_pitch(fader);
                if(tracks[fader->twinterface->current_deck].play)
                    osc_send_pitch(fader->twinterface->current_deck, tracks[fader->twinterface->current_deck].pitch);
                
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
    // Move fader to current pitch position
    float first = 0.84f;
    float last = 1.16f;
    float from_0_to_1 = (fader->pitch - first) / (last - first);
    float from_0_to_2 = from_0_to_1 * 2;
    float from_minus1_to_plus1 = from_0_to_2 - 1;
    
    float min = 0.0f;
    float max = (float) fader->twinterface->viewport.h/2;
    fader->rect.y = from_minus1_to_plus1 * (max-min) + min;
    fader->rect.y += (float) fader->twinterface->viewport.h/2;
    fader->rect.y -= fader->rect.h/2;
    
    // Tell GPU to draw texture at rect destination
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
    return SDL_MapRGBA(sf->format, col->r, col->g, col->b, col->a);
}

void fader_pitch(struct fader *fader)
{
    float y = ((float) fader->rect.y )/ fader->heigth;
    float min = 0.84f;
    float max = 1.16f;
    fader->pitch = y * (max-min) + min;
    
    if(tracks[fader->twinterface->current_deck].pitch < 0){
      tracks[fader->twinterface->current_deck].pitch = -fader->pitch;
    } else {
      tracks[fader->twinterface->current_deck].pitch = fader->pitch;
    }
    
    fprintf(stderr, "fader: new pitch %f\n", fader->pitch);
    
}
