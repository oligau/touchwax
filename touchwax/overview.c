#include "overview.h"
#include "osc.h"

static SDL_Color elapsed_col = {0, 32, 255, 255},
                 needle_col = {255, 255, 255, 255};
                 
struct overview *overview_init(int x, int y, int w, int h, struct track *tr, SDL_Renderer *renderer, struct twinterface *twinterface)
{
  struct overview *overview;
  overview = (struct overview *) malloc(sizeof(struct overview));
  overview->rect.x = x;
  overview->rect.y = y;
  overview->rect.w = w;
  overview->rect.h = h;
  overview->clicked = 0;
  overview->tr = &tracks[twinterface->current_deck];  
  overview->renderer = renderer;
  overview->twinterface = twinterface;
  
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
  overview->surface = SDL_CreateRGBSurface(0, w, h, 32,
                                   rmask, gmask, bmask, amask);
  overview->texture = SDL_CreateTexture(overview->renderer, SDL_PIXELFORMAT_ARGB8888, 
                    SDL_TEXTUREACCESS_STREAMING, w, h); 

  overview->playhead_surface = SDL_CreateRGBSurface(0, w, 1, 32,
                                   rmask, gmask, bmask, amask);
  overview->playhead_texture = SDL_CreateTexture(overview->renderer, SDL_PIXELFORMAT_ARGB8888, 
                    SDL_TEXTUREACCESS_STREAMING, w, 1);
                    
  SDL_FillRect(overview->playhead_surface, NULL, overview_palette(overview->playhead_surface, &needle_col));
  SDL_UpdateTexture(overview->playhead_texture, NULL, overview->playhead_surface->pixels, overview->playhead_surface->pitch);   

  //SDL_SetTextureBlendMode(overview->texture, SDL_BLENDMODE_BLEND);
  //SDL_SetTextureAlphaMod(overview->texture, 255);
                    
  overview_draw(overview);
  SDL_UpdateTexture(overview->texture, NULL, overview->surface->pixels, overview->surface->pitch);    
  
  printf("overview inited.\n");
  return overview;
}

int overview_handle_events(struct overview *overview, SDL_Event event)
{
    
    //The mouse offsets
    int x = 0.0, y = 0.0;

    //If a mouse button was pressed
    if( event.type == SDL_MOUSEMOTION )
    {
             //Get the mouse offsets
            //x = (float) event.motion.xrel;
            y = event.motion.y;

            ////If the mouse is over the button
            //if( ( x > btn->box.x ) && ( x < btn->box.x + btn->box.w ) && ( y > btn->box.y ) && ( y < btn->box.y + btn->box.h ) )
            //{

            //}
            
            if(overview->clicked) {
                overview_jump(overview, y);
                return 1;
            }
    } else if( event.type == SDL_MOUSEBUTTONDOWN ) { 
        if( event.button.button == SDL_BUTTON_LEFT) {
            //Get the mouse offsets
            x = event.button.x;
            y = event.button.y;
            
            if( ( x > overview->rect.x ) && ( x < overview->rect.x + overview->rect.w ) 
                && ( y > overview->rect.y ) && ( y < overview->rect.y + overview->rect.h ) ) 
            {
                printf("overview clicked\n");
                overview->clicked = 1;
                overview_jump(overview, y);
                return 1;              
            }
        }
    } else if( event.type == SDL_MOUSEBUTTONUP ) { 
        if( event.button.button == SDL_BUTTON_LEFT && overview->clicked ) {
           printf("overview unclicked\n");
           overview->clicked = 0;
           return 1;
        }
    }
    
    return 0;
}

void overview_jump(struct overview *overview, int y)
{
    float pourcent = (float) y / (float) overview->rect.h;
    float length = (float) tracks[overview->twinterface->current_deck].length / (float) tracks[overview->twinterface->current_deck].rate;
    tracks[overview->twinterface->current_deck].position =  length * pourcent;

    osc_send_position(overview->twinterface->current_deck, tracks[overview->twinterface->current_deck].position);
    //printf("deck %i jumped to position %f\n", overview->twinterface->current_deck, tracks[overview->twinterface->current_deck].position);
    //printf("y:%i\n", y);
}
  
  
void overview_show(struct overview *overview)
{
    SDL_RenderCopy(overview->renderer, overview->texture, NULL, &overview->rect);  
    //SDL_RenderCopy(overview->renderer, overview->playhead_texture, NULL, &overview->rect);      


}
  
/*
 * Draw the track overview
 */
void overview_draw(struct overview *overview)
{
    int x, y, w, h, r, position, current_position, scale;
    size_t bytes_per_pixel, pitch;
    Uint8 *pixels;

    x = overview->rect.x;
    y = overview->rect.y;
    w = overview->rect.w;
    h = overview->rect.h;    
    position = (int) (overview->tr->position * overview->tr->rate);
    scale = overview->tr->scale;
    

    pixels = (Uint8 *) overview->surface->pixels;
    bytes_per_pixel = overview->surface->format->BytesPerPixel;
    pitch = overview->surface->pitch;
    
    if (overview->tr->length)
        current_position = (long long)position * h / overview->tr->length;
    else
        current_position = 0;    

    /* Draw in columns. This may seem like a performance hit,
     * but oprofile shows it makes no difference */
    
    /* Draw in rows */

    for (r = 0; r < h; r++) {
        int c, sp, width, fade;
        Uint8 *p;
        SDL_Color col;

        //sp = position - (position % (1 << scale))
        //    + ((r - h / 2) << scale);
        sp = r * (overview->tr->length / h);
        //sp = 100;
        
        //printf("overview sp: %i length: %i\n", sp, overview->tr->length);
        
        if (sp < overview->tr->length && sp > 0) {
            width = track_get_ppm(overview->tr, sp) * w / 256;
            //printf("width: %i\n", width);
        } else {
            width = 0;
        }

        /* Select the appropriate colour */
        //int sp_pos = tracks[0].position*tracks[0].rate;
        //printf("sp:%i sp_pos:%i\n", sp, sp_pos);
        //if (r == current_position) {
            //col = needle_col;
            //fade = 1;
        //} else {
            col = elapsed_col;
            fade = 3;
        //}

        /* Left waveform */
        /* Get a pointer to the beginning of the row, and increment it
         * for each column */
        p = pixels + (y + r) * pitch + (x + w) * bytes_per_pixel;

        c = w;
        while (c > width) {
            p[0] = col.b >> fade;
            p[1] = col.g >> fade;
            p[2] = col.r >> fade;
            p -= bytes_per_pixel;
            c--;
        }
        while (c) {
            p[0] = col.b;
            p[1] = col.g;
            p[2] = col.r;
            p -= bytes_per_pixel;            
            c--;
        }
        
        /* Right waveform */
        /* Get a pointer to the beginning of the row, and increment it
         * for each column */
        //p = pixels + (y + r) * pitch + x * bytes_per_pixel;        
        
        //c = w;
        //while (c > width) {
            //p[0] = col.b >> fade;
            //p[1] = col.g >> fade;
            //p[2] = col.r >> fade;
            //p -= bytes_per_pixel;
            //c--;
        //}
        //while (c) {
            //p[0] = col.b;
            //p[1] = col.g;
            //p[2] = col.r;
            //p -= bytes_per_pixel;            
            //c--;
        //}
    }
}

void overview_free(struct overview *overview)
{
  SDL_FreeSurface(overview->surface);
  SDL_DestroyTexture(overview->texture);  
  free(overview);
}

Uint32 overview_palette(SDL_Surface *sf, SDL_Color *col)
{
    return SDL_MapRGB(sf->format, col->r, col->g, col->b);
}
