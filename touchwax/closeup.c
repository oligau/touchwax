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

#define CLOSEUP_WAVEFORM_WIDTH 512

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
  closeup->modified[0] = 0;
  closeup->modified[1] = 0;
  closeup->modified[2] = 0;
  closeup->modified[3] = 0; 
  closeup->modified[4] = 0;  
  closeup->last_pos = ((int)(closeup->tr->position * closeup->tr->rate) / 64);
  
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
    closeup->nb_tile = (int) ceilf((closeup->tr->length / 64) / (float) closeup->padded_h);    
    
  closeup->tiles[0] = (struct tile *) malloc(sizeof(struct tile));  
  closeup->tiles[1] = (struct tile *) malloc(sizeof(struct tile));  
  closeup->tiles[2] = (struct tile *) malloc(sizeof(struct tile));
  closeup->tiles[3] = (struct tile *) malloc(sizeof(struct tile));
  closeup->tiles[4] = (struct tile *) malloc(sizeof(struct tile));  
  
  /* Initiate tiles destination rects */
  closeup->tiles[0]->rect.x = closeup->rect.x;
  closeup->tiles[0]->rect.y = closeup->padded_h * -2;
  closeup->tiles[0]->rect.w = closeup->rect.w;
  closeup->tiles[0]->rect.h = closeup->padded_h;
  closeup->tiles[1]->rect.x = closeup->rect.x;
  closeup->tiles[1]->rect.y = closeup->padded_h * -1;
  closeup->tiles[1]->rect.w = closeup->rect.w;
  closeup->tiles[1]->rect.h = closeup->padded_h;
  closeup->tiles[2]->rect.x = closeup->rect.x;
  closeup->tiles[2]->rect.y = closeup->padded_h * 0;
  closeup->tiles[2]->rect.w = closeup->rect.w;
  closeup->tiles[2]->rect.h = closeup->padded_h;
  closeup->tiles[3]->rect.x = closeup->rect.x;
  closeup->tiles[3]->rect.y = closeup->padded_h * 1;
  closeup->tiles[3]->rect.w = closeup->rect.w;
  closeup->tiles[3]->rect.h = closeup->padded_h;
  closeup->tiles[4]->rect.x = closeup->rect.x;
  closeup->tiles[4]->rect.y = closeup->padded_h * 2;
  closeup->tiles[4]->rect.w = closeup->rect.w;
  closeup->tiles[4]->rect.h = closeup->padded_h;
  
  printf("nb_tile: %i\n", closeup->nb_tile);
#ifdef __ANDROID__
                __android_log_print(ANDROID_LOG_DEBUG, "closeup.c", 
                    "nb_tile: %i\n", closeup->nb_tile);
#endif  
  
  /* Create surfaces to draw on */ 
  closeup->tiles[0]->surface = SDL_CreateRGBSurface(0, CLOSEUP_WAVEFORM_WIDTH, closeup->padded_h, 32,
                                   rmask, gmask, bmask, amask);   
  closeup->tiles[1]->surface = SDL_CreateRGBSurface(0, CLOSEUP_WAVEFORM_WIDTH, closeup->padded_h, 32,
                                   rmask, gmask, bmask, amask); 
  closeup->tiles[2]->surface = SDL_CreateRGBSurface(0, CLOSEUP_WAVEFORM_WIDTH, closeup->padded_h, 32,
                                   rmask, gmask, bmask, amask);   
  closeup->tiles[3]->surface = SDL_CreateRGBSurface(0, CLOSEUP_WAVEFORM_WIDTH, closeup->padded_h, 32,
                                   rmask, gmask, bmask, amask);
  closeup->tiles[4]->surface = SDL_CreateRGBSurface(0, CLOSEUP_WAVEFORM_WIDTH, closeup->padded_h, 32,
                                   rmask, gmask, bmask, amask);                                    

  /* create textures that links the surface to gpu */
  closeup->tiles[0]->texture = SDL_CreateTexture(closeup->renderer, SDL_PIXELFORMAT_ARGB8888, 
                    SDL_TEXTUREACCESS_STREAMING, CLOSEUP_WAVEFORM_WIDTH, closeup->padded_h);  
  closeup->tiles[1]->texture = SDL_CreateTexture(closeup->renderer, SDL_PIXELFORMAT_ARGB8888, 
                    SDL_TEXTUREACCESS_STREAMING, CLOSEUP_WAVEFORM_WIDTH, closeup->padded_h);
  closeup->tiles[2]->texture = SDL_CreateTexture(closeup->renderer, SDL_PIXELFORMAT_ARGB8888, 
                    SDL_TEXTUREACCESS_STREAMING, CLOSEUP_WAVEFORM_WIDTH, closeup->padded_h);
  closeup->tiles[3]->texture = SDL_CreateTexture(closeup->renderer, SDL_PIXELFORMAT_ARGB8888, 
                    SDL_TEXTUREACCESS_STREAMING, CLOSEUP_WAVEFORM_WIDTH, closeup->padded_h);                
  closeup->tiles[4]->texture = SDL_CreateTexture(closeup->renderer, SDL_PIXELFORMAT_ARGB8888, 
                    SDL_TEXTUREACCESS_STREAMING, CLOSEUP_WAVEFORM_WIDTH, closeup->padded_h);


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
    w = CLOSEUP_WAVEFORM_WIDTH;
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
        
        sp = ((r * 64) + (offset *64));
        
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
    printf("drawn %i samples at offset:%i on surface: %p\n", r, offset, surface);
    
}


void closeup_update_init(struct closeup *closeup)
{
  int pos = (int)(closeup->tr->position * closeup->tr->rate) / 64;
  int floor_pos = floorf((float)pos / (float)closeup->padded_h);
  
  closeup->tiles[0]->offset = floor_pos + (closeup->padded_h * -2); 
  closeup->tiles[1]->offset = floor_pos + (closeup->padded_h * -1);
  closeup->tiles[2]->offset = floor_pos;
  closeup->tiles[3]->offset = floor_pos + (closeup->padded_h * 1);
  closeup->tiles[4]->offset = floor_pos + (closeup->padded_h * 2);  
  
  closeup_draw_waveform(closeup, closeup->tiles[0]->surface, closeup->tiles[0]->offset, next_col); 
  closeup_draw_waveform(closeup, closeup->tiles[1]->surface, closeup->tiles[1]->offset, elapsed_col);  
  closeup_draw_waveform(closeup, closeup->tiles[2]->surface, closeup->tiles[2]->offset, next_col); 
  closeup_draw_waveform(closeup, closeup->tiles[3]->surface, closeup->tiles[3]->offset, elapsed_col);
  closeup_draw_waveform(closeup, closeup->tiles[4]->surface, closeup->tiles[4]->offset, next_col);  

  SDL_UpdateTexture(closeup->tiles[0]->texture, NULL, closeup->tiles[0]->surface->pixels, closeup->tiles[0]->surface->pitch);
  SDL_UpdateTexture(closeup->tiles[1]->texture, NULL, closeup->tiles[1]->surface->pixels, closeup->tiles[1]->surface->pitch);
  SDL_UpdateTexture(closeup->tiles[2]->texture, NULL, closeup->tiles[2]->surface->pixels, closeup->tiles[2]->surface->pitch);
  SDL_UpdateTexture(closeup->tiles[3]->texture, NULL, closeup->tiles[3]->surface->pixels, closeup->tiles[3]->surface->pitch);
  SDL_UpdateTexture(closeup->tiles[4]->texture, NULL, closeup->tiles[4]->surface->pixels, closeup->tiles[4]->surface->pitch);
  
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
      closeup_update(closeup);
      Sleep(1);
    }
    
    return 0;
}

void closeup_update(struct closeup *closeup)
{
  if(closeup->nb_tile) {
    int pos = ((int)(closeup->tr->position * closeup->tr->rate) / 64);
    if(closeup->forward) {
      //if(closeup->last_pos - pos > closeup->padded_h) {
          closeup_draw_waveform(closeup, closeup->tiles[4]->surface, closeup->tiles[4]->offset, next_col);
          ++closeup->modified[4]; // Inform render thread that we have a new tile to copy to gpu
      //}
    } else {
      //if(pos - closeup->last_pos > closeup->padded_h) {
          closeup_draw_waveform(closeup, closeup->tiles[0]->surface, closeup->tiles[0]->offset, next_col);
          ++closeup->modified[0]; // Inform render thread that we have a new tile to copy to gpu
      //}      
    }
  }
}

void closeup_show(struct closeup *closeup)
{
  
  if(closeup->tr->length != closeup->last_length) {
    
    /* Keep track of track changes */
    closeup->last_length = closeup->tr->length;
    
    
        
    closeup->nb_tile = (int) ceilf((closeup->tr->length / 64) / (float) closeup->padded_h);
        
    printf("updated nb_tile:%i\n", closeup->nb_tile);
#ifdef __ANDROID__
    __android_log_print(ANDROID_LOG_DEBUG, "closeup.c", 
                    "updated nb_tile:%i\n", closeup->nb_tile);
#endif       
  }
  
  int pos = (int)(closeup->tr->position * closeup->tr->rate) / 64;
  
  /* Determine direction */
  closeup->forward = (pos - closeup->last_pos) >= 0;
  closeup->last_pos = pos;
      
  /* Source texture is always complete tile */    
  SDL_Rect source;
  source.x = closeup->rect.x;
  source.y = closeup->rect.y;
  source.w = CLOSEUP_WAVEFORM_WIDTH;
  source.h = closeup->padded_h;
  
  /* Destination for texture is calculated after position */
  closeup->tiles[0]->rect.y = (-pos % closeup->padded_h) + (closeup->padded_h * -2);
  closeup->tiles[1]->rect.y = (-pos % closeup->padded_h) + (closeup->padded_h * -1);
  closeup->tiles[2]->rect.y = (-pos % closeup->padded_h) + (closeup->padded_h * 0);
  closeup->tiles[3]->rect.y = (-pos % closeup->padded_h) + (closeup->padded_h * 1);
  closeup->tiles[4]->rect.y = (-pos % closeup->padded_h) + (closeup->padded_h * 2);

  if(closeup->nb_tile) {
    
    /* Ugly hack alert *siren sound*
     * SDL_UpdateTexture should be called only once by redraw of surface
     * inside tile_updater thread. Apparently SDL doesn't support 
     * uploading textures from outside main thread. 
     * Here we use a simple flag to communicate with reandering thread, 
     * meaning you should upload to texture as surface has been modified.
    */
    
    if(closeup->modified[0]) {
      SDL_UpdateTexture(closeup->tiles[0]->texture, NULL, closeup->tiles[0]->surface->pixels, closeup->tiles[0]->surface->pitch);    
      closeup->modified[0] = 0;
    } else if(closeup->modified[4]) {
      SDL_UpdateTexture(closeup->tiles[0]->texture, NULL, closeup->tiles[0]->surface->pixels, closeup->tiles[0]->surface->pitch);    
      closeup->modified[4] = 0;      
    }
    
    int current_tile = (pos / 4096);
    printf("current_tile: %i\n", current_tile);
        
    int tile_index[5];
    tile_index[0] = ((current_tile % 5) + 0) % 5;
    tile_index[1] = ((current_tile % 5) + 1) % 5;
    tile_index[2] = ((current_tile % 5) + 2) % 5;
    tile_index[3] = ((current_tile % 5) + 3) % 5;
    tile_index[4] = ((current_tile % 5) + 4) % 5;
    
    if(tile_index[0] >= 0)
      SDL_RenderCopy(closeup->renderer, closeup->tiles[tile_index[0]]->texture, &source, &closeup->tiles[0]->rect);
    if(tile_index[1] >= 0)
      SDL_RenderCopy(closeup->renderer, closeup->tiles[tile_index[1]]->texture, &source, &closeup->tiles[1]->rect);
    if(tile_index[2] >= 0)    
      SDL_RenderCopy(closeup->renderer, closeup->tiles[tile_index[2]]->texture, &source, &closeup->tiles[2]->rect);
    if(tile_index[3] >= 0)    
      SDL_RenderCopy(closeup->renderer, closeup->tiles[tile_index[3]]->texture, &source, &closeup->tiles[3]->rect);
    if(tile_index[4] >= 0)    
      SDL_RenderCopy(closeup->renderer, closeup->tiles[tile_index[4]]->texture, &source, &closeup->tiles[4]->rect);
    
    //printf(" tile0: %i\n tile1: %i\n tile2: %i\n tile3: %i\n tile4: %i\n",
    //tile_index[0],
    //tile_index[1],
    //tile_index[2],
    //tile_index[3],
    //tile_index[4]);
    
    //printf("RenderCopy. pos: %i boundary_front:%i boundary_rear:%i surface: %p\n", pos, boundary_front, boundary_rear, closeup->tile_current->surface);    

    //printf("direction:%i\n", closeup->forward);
    //fflush(stdout);

    /* If we get outside current tile, prefetch next in background*/
    //int is_inside_current = pos < closeup->tile_current->offset + current_heigth && pos > closeup->tile_current->offset;
    //int is_inside_prefetch = pos < closeup->tile_next->offset + current_heigth && pos > closeup->tile_prev->offset;
    //if(!is_inside_current){
      //printf("pos: %i surface: %p\n", pos, closeup->tile_current->surface);           
      //if(!is_inside_prefetch)
        //closeup_update_init(closeup);
      //else
        //closeup_update(closeup);
    //}
  
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
  //while(closeup->tile != closeup->rendered_tile) {
    //Sleep(1);
  //}
  
  closeup->thread_tile_updater_done = 1;
  //pthread_join(closeup->thread_tile_updater, NULL);
  
  int i;
  for(i = 0; i < 5; ++i) {
    SDL_FreeSurface(closeup->tiles[i]->surface);
    SDL_DestroyTexture(closeup->tiles[i]->texture);
    free(closeup->tiles[i]);
    
  }
 
  free(closeup);
}
