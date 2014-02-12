/*
   Copyright (C) 1997-2013 Sam Lantinga <slouken@libsdl.org>

   This software is provided 'as-is', without any express or implied
   warranty.  In no event will the authors be held liable for any damages
   arising from the use of this software.

   Permission is granted to anyone to use this software for any purpose,
   including commercial applications, and to alter it and redistribute it
   freely.

   This file is created by : Nitin Jain (nitin.j4@samsung.com)
*/

/* Sample program:  Draw a Chess Board  by using SDL_CreateSoftwareRenderer API */

#include <stdlib.h>
#include <stdio.h>

#include "osc.h"
#include "track.h"
#include "interface.h"

int
main(int argc, char *argv[])
{
        struct interface *interface;
        
        track_init(0);
        interface = interface_init();
        if(!interface) {
                printf("Interface creation exited with error\n");
                return -1;
        }
        osc_init(interface); /* start OSC server to receive network messages */
        
        interface_loop(interface);
        
        interface_free(interface);
        osc_free();
        
        printf("Exiting cleanly...\n");
        
        return 0;
}

