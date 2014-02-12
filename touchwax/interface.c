#include <stdio.h>

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

void interface_update_closeup(struct interface *interface)
{
    //if(interface->closeup)
      //closeup_free(interface->closeup);
    //interface->closeup = closeup_init(100,
                           //0, 
                           //interface->viewport.w - 100 - 100,
                           //interface->viewport.h, 
                           //&tracks[0],
                           //interface->renderer);
    printf("Updated closeup.\n");
}

void interface_closeup_init(struct interface *interface)
{        
    if(interface->closeup)
        closeup_free(interface->closeup);
    interface->closeup = closeup_init(0,
                           0, 
                           interface->viewport.w,
                           interface->viewport.h, 
                           &tracks[0],
                           interface->renderer);
}

void interface_widgets_init(struct interface *interface)
{
    interface_closeup_init(interface);
    if(interface->btn)
        button_free(interface->btn);
    interface->btn = button_init(interface->viewport.w-100,
                      interface->viewport.h-100, 
                      100, 
                      100, 
                      "button.bmp",
                      interface->renderer);
    if(interface->fader)
        fader_free(interface->fader);                      
    interface->fader = fader_init(interface->viewport.w-100, 
                       interface->viewport.h/2-50, 
                       100, 
                       100, 
                       interface->viewport.h,
                       interface->renderer);
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

    
struct interface *interface_init()
{
    struct interface *interface;
    interface = malloc(sizeof(struct interface));
    interface->closeup = 0;
    interface->btn = 0;
    interface->fader = 0;
    interface->redraw = 0;
    interface->volumeup_pressed = 0;
    interface->renderedFrames = 0;
    interface->last_track_length = tracks[0].length;  
          
    /* Enable standard application logging */
    SDL_LogSetPriority(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_INFO);

    /* Initialize SDL */
    if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0)
    {
      SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDL_Init fail : %s\n", SDL_GetError());
      return interface;
    }

    /* Create window and renderer for given surface */
    interface->window = SDL_CreateWindow(TITLE, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 480, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    if(!interface->window)
    {
      SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Window creation fail : %s\n",SDL_GetError());
      return interface;
    }	
    
    interface->renderer = SDL_CreateRenderer(interface->window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
//    interface->renderer = SDL_CreateRenderer(interface->window, -1, 0);
    if(!interface->renderer)
    {
      SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Render creation for surface fail : %s\n",SDL_GetError());
      return interface;
    }    
    
    /* Get the Size of drawing surface */
    SDL_RenderGetViewport(interface->renderer, &interface->viewport);
    
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

    interface->surface = SDL_CreateRGBSurface(0, interface->viewport.w, interface->viewport.h, 32,
                                   rmask, gmask, bmask, amask);                                   
    if(!interface->surface)
    {
      SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Surface creation fail : %s\n",SDL_GetError());
      return interface;
    }                 
    
    interface->texture = SDL_CreateTexture(interface->renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, interface->viewport.w, interface->viewport.h);
    
    /* Clear the rendering surface with the specified color */
    SDL_SetRenderDrawColor(interface->renderer, 0x0, 0x0, 0x0, 0);
    SDL_RenderClear(interface->renderer);

    /* Start timer to post ticker events */ 
    interface->timer = SDL_AddTimer(REFRESH, ticker, NULL);
    
    /* Initialize widgets */
    interface_widgets_init(interface);

    return interface;
}

void interface_resize(struct interface *interface, int w, int h)
{
#ifdef __ANDROID__
    __android_log_print(ANDROID_LOG_DEBUG, "interface.c", 
                    "Asked new interface size %dx%d.\n", w, h);
#endif

    SDL_SetWindowSize(interface->window, w, h);
    SDL_RenderGetViewport(interface->renderer, &interface->viewport);
    SDL_DestroyTexture(interface->texture);
    SDL_FreeSurface(interface->surface);
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
    interface->surface = SDL_CreateRGBSurface(0, interface->viewport.w, interface->viewport.h, 32,
                                                rmask, gmask, bmask, amask);
    interface->texture = SDL_CreateTexture(interface->renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, interface->viewport.w, interface->viewport.h);
                
    interface_widgets_init(interface);

                          
    printf("New interface size is %dx%d.\n", interface->viewport.w, interface->viewport.h);
#ifdef __ANDROID__
    __android_log_print(ANDROID_LOG_DEBUG, "interface.c", 
                    "New interface size is %dx%d.\n", interface->viewport.w, interface->viewport.h);
#endif
    
}

void interface_loop(struct interface *interface)
{
    /* Main loop, process events until we quit */
    while(1)
    {
      SDL_Event e;
      if (SDL_PollEvent(&e)) {
               
        /* Handle system events */
        if (e.type == SDL_QUIT) {
          printf("SDL_Quit event have been recevied\n");
          break;
        }
        
        /* Pressing volume up+down close the application */
        if(e.type == SDL_KEYDOWN) {
            if(e.key.keysym.sym == SDLK_VOLUMEUP) {
                interface->volumeup_pressed = 1;
                printf("Volume up key have been pressed\n");
#ifdef __ANDROID__
                __android_log_print(ANDROID_LOG_DEBUG, "interface.c", 
                    "Volume up key have been pressed\n");
#endif                
            }else if(e.key.keysym.sym == SDLK_VOLUMEDOWN) {
                if(interface->volumeup_pressed) {                    
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
                interface->volumeup_pressed = 0;
                printf("Volume up key have been released\n");
#ifdef __ANDROID__
                __android_log_print(ANDROID_LOG_DEBUG, "interface.c", 
                    "Volume up key have been released\n");
#endif                
            }
        }
            
        if(e.type == SDL_WINDOWEVENT) {
            if(e.window.event == SDL_WINDOWEVENT_RESIZED) {
                interface_resize(interface, e.window.data1, e.window.data2);
            }
        }
        
        /* Handle timer events */
        if(e.type == EVENT_TICKER)
            interface->redraw = 1;
        
      }
      
      /* Handle widgets events */        
      fader_pitch(interface->fader); // calculate pitch value from fader position
      if(!fader_handle_events(interface->fader, e, interface->viewport.h) &&
         !button_handle_events(interface->btn, e) /* &&
         !overview_handle_events(overview, e)*/)
          closeup_handle_events(interface->closeup, e);
      
      /* draw each widgets to surface if timer said so */
      if(interface->redraw) {
          SDL_RenderClear(interface->renderer);
          
          /* Clear surface with background color*/
          SDL_FillRect(interface->surface, NULL, interface_palette(interface->surface, &background_col));
          SDL_UpdateTexture(interface->texture, NULL, interface->surface->pixels, interface->surface->pitch);
          SDL_RenderCopy(interface->renderer, interface->texture, NULL, NULL);                
                 
          /* Listen for track change, if change update closeup */       
          if(interface->last_track_length != tracks[0].length) {
            interface_closeup_init(interface);
            interface->last_track_length = tracks[0].length;
          }
          
          /* Render widgets on surface */
          closeup_show(interface->closeup);  
          //overview_show(overview, surface, &tracks[0]);
          button_show(interface->btn);
          fader_show(interface->fader);
               
          /* Got everything on rendering surface,
             now Update the drawing image on window screen */
          SDL_RenderPresent(interface->renderer);
                    
          interface->renderedFrames++;
          
          //printf("FPS:%f\n", (renderedFrames / (float) SDL_GetTicks())*1000);
          
          interface->redraw = 0;
      }
    }

#ifdef __ANDROID__
    __android_log_print(ANDROID_LOG_DEBUG, "interface.c", 
                    "interface_loop exit\n");
#endif       
}


void interface_free(struct interface *interface)
{
    button_free(interface->btn);
    fader_free(interface->fader);
    //overview_free(interface->overview);
    closeup_free(interface->closeup);
    
    SDL_FreeSurface(interface->surface);
    SDL_DestroyTexture(interface->texture);
    SDL_DestroyRenderer(interface->renderer);
    SDL_DestroyWindow(interface->window);
    SDL_RemoveTimer(interface->timer);
    
    free(interface);
    
    SDL_Quit();    
}

Uint32 interface_palette(SDL_Surface *sf, SDL_Color *col)
{
    return SDL_MapRGB(sf->format, col->r, col->g, col->b);
}
