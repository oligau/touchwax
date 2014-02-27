#include <stdio.h>
#include <stdint.h>

#include "interface.h"
#include "closeup.h"
#include "overview.h"
#include "button.h"
#include "fader.h"
#ifdef __ANDROID__
#include <android/log.h>
#endif

#define TITLE "touchwax Copyright 2014 Olivier Gauthier <oligau@oscille.ca>"

/* Screen refresh time in milliseconds */

#define REFRESH 10

/* Types of SDL_USEREVENT */

#define EVENT_TICKER (SDL_USEREVENT)

/* Colors used */

//static SDL_Color background_col = {31, 4, 0, 255};
static SDL_Color background_col = {31, 4, 255, 255};

void interface_update_closeup(struct twinterface *twinterface)
{
    //if(twinterface->closeup)
      //closeup_free(twinterface->closeup);
    //twinterface->closeup = closeup_init(100,
                           //0, 
                           //twinterface->viewport.w - 100 - 100,
                           //twinterface->viewport.h, 
                           //&tracks[0],
                           //twinterface->renderer);
    printf("Updated closeup.\n");
}

void interface_closeup_init(struct twinterface *twinterface)
{     
    if(twinterface->closeup)
        closeup_free(twinterface->closeup);
    twinterface->closeup = closeup_init(0,
                           0, 
                           twinterface->viewport.w,
                           twinterface->viewport.h, 
                           &tracks[0],
                           twinterface->renderer);
}

void interface_widgets_init(struct twinterface *twinterface)
{
    interface_closeup_init(twinterface);
    if(twinterface->btn)
        button_free(twinterface->btn);
    twinterface->btn = button_init(twinterface->viewport.w-100,
                      twinterface->viewport.h-100, 
                      100, 
                      100, 
                      "button.bmp",
                      twinterface->renderer);
    if(twinterface->fader)
        fader_free(twinterface->fader);                      
    twinterface->fader = fader_init(twinterface->viewport.w-100, 
                       twinterface->viewport.h/2-50, 
                       100, 
                       100, 
                       twinterface->viewport.h,
                       twinterface->renderer);
}


void push_event(int t)
{
    SDL_Event e;

    if (!SDL_PeepEvents(&e, 1, SDL_PEEKEVENT, t, t)) {
        e.type = t;
        if (SDL_PushEvent(&e) == -1)
            abort();
    }
}

/*
 * Timer which posts a screen redraw event
 */

Uint32 ticker(Uint32 interval, void *p)
{
    push_event(EVENT_TICKER);
    return interval;
}

    
struct twinterface*interface_init()
{
    struct twinterface *twinterface;
    twinterface = (struct twinterface*) malloc(sizeof(struct twinterface));
    twinterface->closeup = 0;
    twinterface->btn = 0;
    twinterface->fader = 0;
    twinterface->redraw = 0;
    twinterface->volumeup_pressed = 0;
    twinterface->renderedFrames = 0;
    twinterface->last_track_length = tracks[0].length;  
          
    /* Enable standard application logging */
    SDL_LogSetPriority(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_INFO);

    /* Initialize SDL */
    if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0)
    {
      SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDL_Init fail : %s\n", SDL_GetError());
      return twinterface;
    }

    /* Create window and renderer for given surface */
    twinterface->window = SDL_CreateWindow(TITLE, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 480, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    if(!twinterface->window)
    {
      SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Window creation fail : %s\n",SDL_GetError());
      return twinterface;
    }	
    
    twinterface->renderer = SDL_CreateRenderer(twinterface->window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
//    interface->renderer = SDL_CreateRenderer(interface->window, -1, 0);
    if(!twinterface->renderer)
    {
      SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Render creation for surface fail : %s\n",SDL_GetError());
      return twinterface;
    }    
    
    /* Get the Size of drawing surface */
    SDL_RenderGetViewport(twinterface->renderer, &twinterface->viewport);
    
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

    twinterface->surface = SDL_CreateRGBSurface(0, twinterface->viewport.w, twinterface->viewport.h, 32,
                                   rmask, gmask, bmask, amask);                                   
    if(!twinterface->surface)
    {
      SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Surface creation fail : %s\n",SDL_GetError());
      return twinterface;
    }                 
    
    twinterface->texture = SDL_CreateTexture(twinterface->renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, twinterface->viewport.w, twinterface->viewport.h);
    
    /* Clear the rendering surface with the specified color */
    SDL_SetRenderDrawColor(twinterface->renderer, 0x0, 0x0, 0x0, 0);
    SDL_RenderClear(twinterface->renderer);

    /* Start timer to post ticker events */ 
    twinterface->timer = SDL_AddTimer(REFRESH, ticker, NULL);
    
    /* Initialize widgets */
    interface_widgets_init(twinterface);

    return twinterface;
}

void interface_resize(struct twinterface *twinterface, int w, int h)
{
#ifdef __ANDROID__
    __android_log_print(ANDROID_LOG_DEBUG, "interface.c", 
                    "Asked new twinterfacesize %dx%d.\n", w, h);
#endif

    SDL_SetWindowSize(twinterface->window, w, h);
    SDL_RenderGetViewport(twinterface->renderer, &twinterface->viewport);
    SDL_DestroyTexture(twinterface->texture);
    SDL_FreeSurface(twinterface->surface);
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
    twinterface->surface = SDL_CreateRGBSurface(0, twinterface->viewport.w, twinterface->viewport.h, 32,
                                                rmask, gmask, bmask, amask);
    twinterface->texture = SDL_CreateTexture(twinterface->renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, twinterface->viewport.w, twinterface->viewport.h);
                
    interface_widgets_init(twinterface);

                          
    printf("New twinterfacesize is %dx%d.\n", twinterface->viewport.w, twinterface->viewport.h);
#ifdef __ANDROID__
    __android_log_print(ANDROID_LOG_DEBUG, "interface.c", 
                    "New twinterfacesize is %dx%d.\n", twinterface->viewport.w, twinterface->viewport.h);
#endif
    
}

void interface_loop(struct twinterface *twinterface)
{
    /* Main loop, process events until we quit */
    while(1)
    {
      SDL_Event e;
      if (SDL_PollEvent(&e)) {
               
        /* Handle system events */
        if (e.type == SDL_QUIT) {
          printf("SDL_Quit event have been received\n");
          break;
        }
        
        /* Pressing volume up+down close the application */
        if(e.type == SDL_KEYDOWN) {
            if(e.key.keysym.sym == SDLK_VOLUMEUP) {
                twinterface->volumeup_pressed = 1;
                printf("Volume up key have been pressed\n");
#ifdef __ANDROID__
                __android_log_print(ANDROID_LOG_DEBUG, "interface.c", 
                    "Volume up key have been pressed\n");
#endif                
            }else if(e.key.keysym.sym == SDLK_VOLUMEDOWN) {
                if(twinterface->volumeup_pressed) {                    
                    printf("Volume up & down keys have been pressed\n");
#ifdef __ANDROID__
                __android_log_print(ANDROID_LOG_DEBUG, "interface.c", 
                    "Volume up & down keys have been pressed\n");
#endif              
                    break;
                }                
            }
        } else if(e.type == SDL_KEYUP) {
            if(e.key.keysym.sym == SDLK_VOLUMEUP) {
                twinterface->volumeup_pressed = 0;
                printf("Volume up key have been released\n");
#ifdef __ANDROID__
                __android_log_print(ANDROID_LOG_DEBUG, "interface.c", 
                    "Volume up key have been released\n");
#endif                
            }
        }
            
        if(e.type == SDL_WINDOWEVENT) {
            if(e.window.event == SDL_WINDOWEVENT_RESIZED) {
                interface_resize(twinterface, e.window.data1, e.window.data2);
            }
        }
        
        /* Handle timer events */
        if(e.type == EVENT_TICKER)
            twinterface->redraw = 1;
        
      }
      
      /* Handle widgets events */        
      fader_pitch(twinterface->fader); // calculate pitch value from fader position
      if(!fader_handle_events(twinterface->fader, e, twinterface->viewport.h) &&
         !button_handle_events(twinterface->btn, e) /* &&
         !overview_handle_events(overview, e)*/)
          closeup_handle_events(twinterface->closeup, e);
      
      /* draw each widgets to surface if timer said so */
      if(twinterface->redraw) {
          SDL_RenderClear(twinterface->renderer);
          
          /* Clear surface with background color*/
          SDL_FillRect(twinterface->surface, NULL, interface_palette(twinterface->surface, &background_col));
          SDL_UpdateTexture(twinterface->texture, NULL, twinterface->surface->pixels, twinterface->surface->pitch);
          SDL_RenderCopy(twinterface->renderer, twinterface->texture, NULL, NULL);                
                 
          /* Listen for track change, if change update closeup */       
          if(twinterface->last_track_length != tracks[0].length) {
            interface_closeup_init(twinterface);
            twinterface->last_track_length = tracks[0].length;
          }
          
          /* Render widgets on surface */
          closeup_show(twinterface->closeup);  
          //overview_show(overview, surface, &tracks[0]);
          button_show(twinterface->btn);
          fader_show(twinterface->fader);
               
          /* Got everything on rendering surface,
             now Update the drawing image on window screen */
          SDL_RenderPresent(twinterface->renderer);
                    
          twinterface->renderedFrames++;
          
          //printf("FPS:%f\n", (twinterface->renderedFrames / (float) SDL_GetTicks())*1000);
          
          twinterface->redraw = 0;
      }
    }

#ifdef __ANDROID__
    __android_log_print(ANDROID_LOG_DEBUG, "interface.c", 
                    "interface_loop exit\n");
#endif       
}


void interface_free(struct twinterface *twinterface)
{
    button_free(twinterface->btn);
    fader_free(twinterface->fader);
    //overview_free(twinterface->overview);
    closeup_free(twinterface->closeup);
    
    SDL_FreeSurface(twinterface->surface);
    SDL_DestroyTexture(twinterface->texture);
    SDL_DestroyRenderer(twinterface->renderer);
    SDL_DestroyWindow(twinterface->window);
    SDL_RemoveTimer(twinterface->timer);
    
    free(twinterface);
    
    SDL_Quit();    
}

Uint32 interface_palette(SDL_Surface *sf, SDL_Color *col)
{
    return SDL_MapRGB(sf->format, col->r, col->g, col->b);
}
