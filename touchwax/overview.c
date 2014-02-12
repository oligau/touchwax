#include "overview.h"
#include "osc.h"

static SDL_Color elapsed_col = {0, 32, 255, 255},
                 needle_col = {255, 255, 255, 255};
                 
struct overview *overview_init(int x, int y, int w, int h)
{
  struct overview *overview;
  overview = (struct overview *) malloc(sizeof(struct overview));
  overview->rect.x = x;
  overview->rect.y = y;
  overview->rect.w = w;
  overview->rect.h = h;
  overview->clicked = 0;

  return overview;
}

int overview_handle_events(struct overview *overview, SDL_Event event)
{
    
    //The mouse offsets
    float /*x = 0.0,*/ y = 0.0;

    //If a mouse button was pressed
    if( event.type == SDL_MOUSEMOTION )
    {
             //Get the mouse offsets
            //x = (float) event.motion.xrel;
            y = (float) event.motion.y;

            ////If the mouse is over the button
            //if( ( x > btn->box.x ) && ( x < btn->box.x + btn->box.w ) && ( y > btn->box.y ) && ( y < btn->box.y + btn->box.h ) )
            //{

            //}
            
            if(overview->clicked) {
                float pourcent = y / overview->rect.h;
                tracks[0].position = tracks[0].length * pourcent;

                osc_send_position(tracks[0].position);
                //printf("x:%f y:%f\n", x, y);
            }
            
            return 1;
    } else if( event.type == SDL_MOUSEBUTTONDOWN ) { 
        if( event.button.button == SDL_BUTTON_LEFT) {
           if( ( event.button.x > overview->rect.x ) && 
                        ( event.button.x < overview->rect.x + overview->rect.w ) && 
                        ( event.button.y > overview->rect.y ) && 
                        ( event.button.y < overview->rect.y + overview->rect.h ) ) {
                overview->clicked = 1;
                return 1;              
            }
        }
    } else if( event.type == SDL_MOUSEBUTTONUP ) { 
        if( event.button.button == SDL_BUTTON_LEFT ) {
           overview->clicked = 0;
           return 1;
        }
    }
    
    return 0;
}
  
/*
 * Draw the track overview
 */
void overview_show(struct overview *overview, SDL_Surface *surface, 
                    struct track *tr)
{
    int x, y, w, h, r, position, current_position, scale;
    size_t bytes_per_pixel, pitch;
    Uint8 *pixels;

    x = overview->rect.x;
    y = overview->rect.y;
    w = overview->rect.w;
    h = overview->rect.h;    
    position = (int) (tr->position * tr->rate);
    scale = tr->scale;
    

    pixels = (Uint8 *) surface->pixels;
    bytes_per_pixel = surface->format->BytesPerPixel;
    pitch = surface->pitch;
    
    if (tr->length)
        current_position = (long long)position * h / tracks[0].length;
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
        sp = tracks[0].length * r / h;

        if (sp < tr->length && sp > 0)
            width = track_get_ppm(tr, sp) * w / 256;
        else
            width = 0;

        /* Select the appropriate colour */
        //int sp_pos = tracks[0].position*tracks[0].rate;
        //printf("sp:%i sp_pos:%i\n", sp, sp_pos);
        if (r == current_position) {
            col = needle_col;
            fade = 1;
        } else {
            col = elapsed_col;
            fade = 3;
        }

        /* Left waveform */
        /* Get a pointer to the beginning of the row, and increment it
         * for each column */
        p = pixels + (y + r) * pitch + x * bytes_per_pixel;

        c = w/2;
        while (c > width) {
            p[0] = col.b >> fade;
            p[1] = col.g >> fade;
            p[2] = col.r >> fade;
            p += bytes_per_pixel;
            c--;
        }
        while (c) {
            p[0] = col.b;
            p[1] = col.g;
            p[2] = col.r;
            p += bytes_per_pixel;            
            c--;
        }
        
        /* Right waveform */
        /* Get a pointer to the beginning of the row, and increment it
         * for each column */
        p = pixels + (y + r) * pitch + x * bytes_per_pixel;        
        
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
    }
}

void overview_free(struct overview *overview)
{
  free(overview);
}
