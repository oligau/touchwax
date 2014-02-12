#ifdef _MSC_VER
#include <math.h>
#include <stdio.h>
#else
#include <unistd.h>
#endif

#ifdef _WINDOWS
#include <windows.h>
#else
#include <unistd.h>
#define Sleep(x) usleep((x)*1000)
#endif

#include "closeup.h"
#include "osc.h"

#ifdef __ANDROID__
#include <android/log.h>
#endif

static SDL_Color elapsed_col = {0, 32, 255, 255},  // xwax original
                  prev_col = {0, 255, 32, 255}, 
                  next_col = {255, 32, 0, 255}, // night vision mode
                 needle_col = {255, 255, 255, 255};

struct closeup *closeup_init(int x, int y, int w, int h, struct track *tr, SDL_Renderer *renderer)
{
  // Clamp width to minimum 0 pixel
  if(w < 0)
    w = 0;

  struct closeup *closeup;
  closeup = (struct closeup *) malloc(sizeof(struct closeup));
  closeup->rect.x = x;
  closeup->rect.y = y;
  closeup->rect.w = w;
  closeup->rect.h = h;
  closeup->clicked = 0;
  closeup->renderer = renderer;
  closeup->tr = tr;
  
  closeup->thread_tile_updater_done = 0;
  closeup->rendered_tile = 0;
  closeup->tile = 0;
  closeup->modified = 0;
  
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
  
  /* Crop heigth to renderer's maxiumum texture heigth */
  SDL_RendererInfo renderer_info;
  SDL_GetRendererInfo(closeup->renderer, &renderer_info);
  closeup->padded_h = renderer_info.max_texture_height;
     
  /* Build tiles set*/
  closeup->nb_tile = 0;
  if(closeup->tr->length)
    closeup->nb_tile = (int) ceilf((closeup->tr->length >> closeup->tr->scale) / (float) closeup->padded_h);    
  closeup->tile_prev = (struct tile *) malloc(sizeof(struct tile));  
  closeup->tile_current = (struct tile *) malloc(sizeof(struct tile));
  closeup->tile_next = (struct tile *) malloc(sizeof(struct tile));
  
  printf("nb_tile: %i\n", closeup->nb_tile);
#ifdef __ANDROID__
                __android_log_print(ANDROID_LOG_DEBUG, "closeup.c", 
                    "nb_tile: %i\n", closeup->nb_tile);
#endif  
  
  /* Create surfaces to draw on */  
  //closeup->tile_prev->surface = SDL_CreateRGBSurface(0, 512, closeup->padded_h, 32,
                                   //rmask, gmask, bmask, amask); 
  closeup->tile_current->surface = SDL_CreateRGBSurface(0, 512, closeup->padded_h, 32,
                                   rmask, gmask, bmask, amask);   
  closeup->tile_next->surface = SDL_CreateRGBSurface(0, 512, closeup->padded_h, 32,
                                   rmask, gmask, bmask, amask);   

  /* create textures that links the surface to gpu */
  //closeup->tile_prev->texture = SDL_CreateTexture(closeup->renderer, SDL_PIXELFORMAT_ARGB8888, 
                    //SDL_TEXTUREACCESS_STREAMING, 512, closeup->padded_h);
  closeup->tile_current->texture = SDL_CreateTexture(closeup->renderer, SDL_PIXELFORMAT_ARGB8888, 
                    SDL_TEXTUREACCESS_STREAMING, 512, closeup->padded_h);
  closeup->tile_next->texture = SDL_CreateTexture(closeup->renderer, SDL_PIXELFORMAT_ARGB8888, 
                    SDL_TEXTUREACCESS_STREAMING, 512, closeup->padded_h);                    
  
  /* Find texture width and heigth */
  closeup->texture_w = 0;
  closeup->texture_h = 0;
  if(closeup->tile_current->texture)
    SDL_QueryTexture(closeup->tile_current->texture, NULL, NULL, &closeup->texture_w, &closeup->texture_h);
  
  if(closeup->nb_tile) {
    printf("closeup->surface:\nw:%i h:%i\ncloseup->texture:\nw:%i h:%i\n", 
      closeup->tile_current->surface->w, closeup->tile_current->surface->h,
      closeup->texture_w, closeup->texture_h);
#ifdef __ANDROID__
    __android_log_print(ANDROID_LOG_DEBUG, "closeup.c", 
                    "closeup->surface:\nw:%i h:%i\ncloseup->texture:\nw:%i h:%i\n", 
                    closeup->tile_current->surface->w, closeup->tile_current->surface->h,
                    closeup->texture_w, closeup->texture_h);
#endif       
  }


  closeup_update_init(closeup);
  closeup_start_tile_updater_thread(closeup);

  return closeup;
}

/*
 * Draw the close-up meter, which can be zoomed to a level set by
 * 'scale'
 * Inspired from xwax, www.xwax.co.uk Copyright Mark Hills
 */
void closeup_draw_waveform(struct closeup *closeup, SDL_Surface *surface, int offset, SDL_Color col)
{
    int x, y, w, h, r, position, scale;
    size_t bytes_per_pixel, pitch;
    Uint8 *pixels;

    x = closeup->rect.x;
    y = closeup->rect.y;
    w = 512;//closeup->rect.w;
    //h = closeup->rect.h;
    //h = closeup->tr->length;
    h = closeup->padded_h;
    position = (int) (closeup->tr->position * closeup->tr->rate);
    scale = closeup->tr->scale;
  
    pixels = (Uint8 *) surface->pixels;
    bytes_per_pixel = surface->format->BytesPerPixel;
    pitch = surface->pitch;

    /* Draw in rows */

    for (r = 0; r < h; r++) {
        int c, width, fade;
		unsigned int sp;
        Uint8 *p;
        //SDL_Color col;

        /* Work out the meter width in pixels for this row */
        //sp = position - (position % (1 << scale))
            //+ ((r - h / 2) << scale);
        sp = (r * 64) + (offset * 64);

        if (sp < closeup->tr->length && sp > 0)
            //width = track_get_ppm(closeup->tr, sp) * (w / 2) / 256;
            width = track_get_ppm(closeup->tr, sp) * (w / 2) / 256;
            //width = ((float)track_get_ppm(closeup->tr, sp) / (float)256) * (float) w;
        else
            width = 0;
            
        //printf("r: %i sp:%i length:%i width:%i\n", r, sp, tracks[0].length, width);
            
        
        //printf("w: %i, pitch: %i, width: %i\n", w, pitch, width);

        ///* Select the appropriate colour */
        //if (r == closeup->padded_h / 2) {
            //col = needle_col;
            //fade = 1;
        //} else {
            //col = elapsed_col;
            fade = 3;
        //}

        /* Left waveform */
        /* Get a pointer to the beginning of the row, and increment it
         * for each column */
        //p = pixels + (y + r) * pitch + x * bytes_per_pixel;
        p = pixels + r * pitch;

        c = w/2 + 1;
        while (c > width) {
            //printf("c: %i, width: %i\n", c, width);
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
        
        //* Right waveform */
        //* Get a pointer to the beginning of the row, and increment it
        //* for each column */
        ////p = pixels + (y + r) * pitch + (x + w) * bytes_per_pixel;
        p = pixels + r * pitch + w * bytes_per_pixel;
        
        
        c = w/2 + 1;
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
       
        //printf("drawn sample:%i on surface: %p\n", sp, surface);

    } 
    printf("drawn %i samples on surface: %p\n", r, surface);
    
}


void closeup_update_init(struct closeup *closeup)
{
  int pos = (int)(closeup->tr->position * closeup->tr->rate) / 64;
  closeup->tile_current->offset = pos;
  closeup->tile_next->offset = closeup->tile_current->offset + closeup->padded_h;  
  
  closeup_draw_waveform(closeup, closeup->tile_current->surface, closeup->tile_current->offset, elapsed_col); 
  closeup_draw_waveform(closeup, closeup->tile_next->surface, closeup->tile_next->offset, next_col);

  /* Notify render thread that we have a new tile to render */
  //++closeup->tile;  
}

void closeup_start_tile_updater_thread(struct closeup *closeup)
{
    pthread_create(&closeup->thread_tile_updater, NULL, closeup_tile_updater, (void *) closeup);
    printf("tile updater thread started... %p\n", &closeup->thread_tile_updater);
}

void *closeup_tile_updater(void *param)
{    
    struct closeup *closeup = (struct closeup *) param;
        
    while(!closeup->thread_tile_updater_done) { 
      if(closeup->tile != closeup->rendered_tile) {  
          int pos = (int)(closeup->tr->position * closeup->tr->rate) / 64;
          //closeup->tile_current->offset = pos;
          closeup->tile_next->offset = pos + closeup->padded_h; 
        
        //closeup_draw_waveform(closeup, closeup->tile_prev->surface, closeup->tile_prev->offset, prev_col);
        //closeup_draw_waveform(closeup, closeup->tile_current->surface, closeup->tile_current->offset, elapsed_col);
        closeup_draw_waveform(closeup, closeup->tile_next->surface, closeup->tile_next->offset, next_col);

        /* Clear event, no tile to render for the moment*/
        closeup->rendered_tile = closeup->tile;
        
        /* Tells main thread to upload surface to texture */
        closeup->modified = 1;
        
      }
      Sleep(1);
    }
    
    return 0;
}

void closeup_update(struct closeup *closeup)
{
  if(closeup->nb_tile) {
    
    /* Tile pointer swapping */
    struct tile *tile_tmp = closeup->tile_current;
    closeup->tile_current = closeup->tile_next;
    closeup->tile_next = tile_tmp;  
             
    printf("swapped tiles\n"); 
    fflush(stdout);   
#ifdef __ANDROID__
    __android_log_print(ANDROID_LOG_DEBUG, "closeup.c", 
                    "swapped tiles\n");
#endif       
    
    ++closeup->tile; // Inform render thread that we have a new tile to render
  }
}

void closeup_show(struct closeup *closeup)
{
  
  if(closeup->tr->length != closeup->last_length) {
    closeup->nb_tile = (int) ceilf((closeup->tr->length >> closeup->tr->scale) / (float) closeup->padded_h);
    closeup->last_length = closeup->tr->length;
    printf("updated nb_tile:%i\n", closeup->nb_tile);
#ifdef __ANDROID__
    __android_log_print(ANDROID_LOG_DEBUG, "closeup.c", 
                    "updated nb_tile:%i\n", closeup->nb_tile);
#endif       
  }
  
  int pos = ((int)(closeup->tr->position * closeup->tr->rate) / 64);
  int prev_heigth, current_heigth, next_heigth;
  int prev_y, current_y, next_y;
    
  prev_heigth = closeup->padded_h;
  current_heigth = closeup->padded_h;
  next_heigth = closeup->padded_h;
  
  prev_y = (pos % closeup->padded_h) + closeup->padded_h;
  current_y = pos % closeup->padded_h;
  next_y = (pos % closeup->padded_h) - closeup->padded_h;
  
  SDL_Rect prev_source;
  prev_source.x = closeup->rect.x;
  prev_source.y = closeup->rect.y;
  prev_source.w = closeup->rect.w;
  prev_source.h = closeup->rect.h;
  SDL_Rect prev_dest;
  prev_dest.x = closeup->rect.x;
  prev_dest.y = -prev_y /*+ (closeup->rect.h / 2)*/;
  prev_dest.w = closeup->rect.w;
  prev_dest.h = prev_heigth;
  
  SDL_Rect current_source;
  current_source.x = closeup->rect.x;
  current_source.y = closeup->rect.y;
  current_source.w = 512; //closeup->rect.w;
  current_source.h = current_heigth;
  SDL_Rect current_dest;
  current_dest.x = closeup->rect.x;
  current_dest.y = -current_y /*+ (closeup->rect.h / 2)*/;
  current_dest.w = closeup->rect.w;
  current_dest.h = current_heigth;

  SDL_Rect next_source;
  next_source.x = closeup->rect.x;
  next_source.y = closeup->rect.y;
  next_source.w = 512; //closeup->rect.w;
  next_source.h = next_heigth;
  SDL_Rect next_dest;
  next_dest.x = closeup->rect.x;
  next_dest.y = -next_y /*+ (closeup->rect.h / 2)*/;
  next_dest.w = closeup->rect.w;
  next_dest.h = next_heigth;

  if(closeup->nb_tile) {
    
    /* Ugly hack alert *siren sound*
     * SDL_UpdateTexture should be called only once by redraw of surface
     * inside tile_updater thread. Apparently SDL doesn't support 
     * uploading textures from outside main thread. 
     * Here we use a simple flag to communicate with reandering thread, 
     * meaning you should upload to texture as surface has been modified.
    */
    
    if(closeup->modified) {
      //SDL_UpdateTexture(closeup->tile_current->texture, NULL, closeup->tile_current->surface->pixels, closeup->tile_current->surface->pitch);      
      SDL_UpdateTexture(closeup->tile_next->texture, NULL, closeup->tile_next->surface->pixels, closeup->tile_next->surface->pitch);
      closeup->modified = 0;
    }

    SDL_RenderCopy(closeup->renderer, closeup->tile_current->texture, &current_source, &current_dest);
    SDL_RenderCopy(closeup->renderer, closeup->tile_next->texture, &next_source, &next_dest);

    //printf("RenderCopy. pos: %i boundary_front:%i boundary_rear:%i surface: %p\n", pos, boundary_front, boundary_rear, closeup->tile_current->surface);    


    /* If we get outside our current tiles, redraw next in background*/
    int boundary_front = closeup->tile_current->offset;
    int boundary_rear = closeup->tile_current->offset + current_heigth;
    int is_inside_tile = pos >= boundary_front && pos <= boundary_rear;    
    if(!is_inside_tile){
      printf("pos: %i boundary_front:%i boundary_rear:%i surface: %p\n", pos, boundary_front, boundary_rear, closeup->tile_current->surface);                
      closeup_update(closeup);
    }
  
    //printf("pos:%i current_heigth:%i current_y:%i next_heigth:%i next_y:%i\n", pos, current_heigth, current_y, next_heigth, next_y);

  }    

}

void closeup_handle_events(struct closeup *closeup, SDL_Event event)
{ 
    
    //The mouse offsets
    float /*x = 0.0,*/ y = 0.0;

    //If a mouse button was pressed
    if( event.type == SDL_MOUSEMOTION )
    {
             //Get the mouse offsets
            //x = (float) event.motion.xrel;
            y = (float) event.motion.yrel;

            ////If the mouse is over the button
            //if( ( x > btn->box.x ) && ( x < btn->box.x + btn->box.w ) && ( y > btn->box.y ) && ( y < btn->box.y + btn->box.h ) )
            //{

            //}
            
            if(closeup->clicked) {
                //tracks[0].position = tracks[0].position + x;
                osc_send_position(closeup->tr->position - y/100);
                //printf("x:%f y:%f\n", x, y);
            }
            
            
    } else if( event.type == SDL_MOUSEBUTTONDOWN ) { 
        if( event.button.button == SDL_BUTTON_LEFT ) {
           closeup->clicked = 1;
           //if(tracks[0].play)
            //osc_send_pitch(0);
        }
        
    } else if( event.type == SDL_MOUSEBUTTONUP ) { 
        if( event.button.button == SDL_BUTTON_LEFT ) {
           closeup->clicked = 0;
           //if(tracks[0].play)
            //osc_send_pitch(tracks[0].pitch);
        }
        
    }
    
}

void closeup_free(struct closeup *closeup)
{
  
  /* Wait that tile renderer thread idles before killing closeup */
  while(closeup->tile != closeup->rendered_tile) {
    Sleep(1);
  }
  
  closeup->thread_tile_updater_done = 1;
  pthread_join(closeup->thread_tile_updater, NULL);
  
  SDL_FreeSurface(closeup->tile_current->surface);
  SDL_DestroyTexture(closeup->tile_current->texture); 
  SDL_FreeSurface(closeup->tile_next->surface);
  SDL_DestroyTexture(closeup->tile_next->texture);   

  free(closeup->tile_prev);
  free(closeup->tile_current);
  free(closeup->tile_next);  
  free(closeup);
}
