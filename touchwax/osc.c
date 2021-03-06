#include <stdio.h>
#include <stdlib.h>

#ifndef _MSC_VER
#include <unistd.h>
#endif

#ifdef __ANDROID__
#include <android/log.h>
#endif

#include "osc.h"
#include "track.h"
#include "interface.h"

#include "lo/lo.h"

#ifndef max
	#define max( a, b ) ( ((a) > (b)) ? (a) : (b) )
#endif

struct osc *osc;
lo_server_thread st = 0;

int done = 0;

#ifdef __ANDROID__
const char ADDRESS[] = "10.10.10.1";
#else
const char ADDRESS[] = "127.0.0.1";
#endif                

int osc_init(struct twinterface *twinterface)
{
    osc = (struct osc *) malloc(sizeof(struct osc));
    osc->twinterface = twinterface;
    osc->ndeck = 0;
    
    /* start a new server on port 7770 */
    st = lo_server_thread_new("7771", error);

    /* add method that will match any path and args */
    lo_server_thread_add_method(st, "/xwax/ppm", "ibi", ppm_handler, NULL);
    
    lo_server_thread_add_method(st, "/xwax/ppm_end", "i", ppm_end_handler, NULL);

    /* add method that will match any path and args */
    lo_server_thread_add_method(st, "/xwax/track_load", "iissi", track_load_handler, NULL);

    /* add method that will match any path and args */
    lo_server_thread_add_method(st, "/xwax/position", "iff", pos_handler, NULL);

    /* add method that will match any path and args */
    lo_server_thread_add_method(st, "/xwax/scale", "i", scale_handler, NULL);    

    /* add method that will match any path and args */
    //lo_server_thread_add_method(st, NULL, NULL, generic_handler, NULL);
    
    lo_server_thread_start(st);
    
    /* Notify osc server that where at this address */
    lo_address t = lo_address_new(ADDRESS, "7770");
    lo_send(t, "/xwax/connect", "");
    printf("Sent connect message\n");

    return 0;
}

void osc_free()
{
    lo_server_thread_free(st);
    free(osc);

}

void error(int num, const char *msg, const char *path)
{
    //printf("liblo server error %d in path %s: %s\n", num, path, msg);
#ifdef __ANDROID__
    __android_log_print(ANDROID_LOG_DEBUG, "osc.c", "liblo server error %d in path %s: %s\n", num, path, msg);
#endif

    //fflush(stdout);
}

/* catch any incoming messages and display them. returning 1 means that the
 * message has not been fully handled and the server should try other methods */
int generic_handler(const char *path, const char *types, lo_arg ** argv,
                    int argc, void *data, void *user_data)
{
    int i;

    //printf("path: <%s>\n", path);
#ifdef __ANDROID__
    __android_log_print(ANDROID_LOG_DEBUG, "osc.c", "path: <%s>\n", path);
#endif

    for (i = 0; i < argc; i++) {
#ifdef __ANDROID__       
        __android_log_print(ANDROID_LOG_DEBUG, "osc.c", "arg %d '%c' ", i, types[i]);
#endif
        lo_arg_pp((lo_type)types[i], argv[i]);
        //printf("\n");
    }
    //printf("\n");
    //fflush(stdout);

    return 1;
}

/* catch any incoming messages and display them. returning 1 means that the
 * message has not been fully handled and the server should try other methods */
int track_load_handler(const char *path, const char *types, lo_arg ** argv,
                    int argc, void *data, void *user_data)
{
    int i, d;

    printf("track_load_handler: path: <%s>\n", path);
#ifdef __ANDROID__
    __android_log_print(ANDROID_LOG_DEBUG, "osc.c", "path: <%s>\n", path);
#endif

    for (i = 0; i < argc; i++) {
#ifdef __ANDROID__
        __android_log_print(ANDROID_LOG_DEBUG, "osc.c", "arg %d '%c' ", i, types[i]);
#endif
        //lo_arg_pp((lo_type)types[i], argv[i]);
        //printf("\n");
    }
    
    /* Shut closeup updater thread before swapping track */
    interface_closeup_free(osc->twinterface);
    
    d = argv[0]->i;
    
    osc->ndeck = max(osc->ndeck, d+1);
    
    fprintf(stderr, "updated ndeck: %i\n", osc->ndeck);
    
    track_init(d);
    
    tracks[d].id = argv[1]->i;
    tracks[d].artist = (char *) argv[2];
    tracks[d].title = (char *) argv[3];
    tracks[d].rate = argv[4]->i;
    
    //interface_update_overview(osc->twinterface);    
    interface_update_closeup(osc->twinterface);

#ifdef __ANDROID__
    __android_log_print(ANDROID_LOG_DEBUG, "osc.c", "artist:%s\ntitle:%s", tracks[0].artist, tracks[0].title);
#endif

    
    //printf("\n");
    //fflush(stdout);

    return 1;
}

int ppm_handler(const char *path, const char *types, lo_arg ** argv,
                    int argc, void *data, void *user_data)
{

    printf("ppm_handler: path: <%s>\n", path);
#ifdef __ANDROID__
    __android_log_print(ANDROID_LOG_DEBUG, "osc.c", "path: <%s>\n", path);
#endif
            
    int track_id = argv[0]->i;
    lo_blob blob = argv[1];
    int size = lo_blob_datasize(blob);
    unsigned char *bdata = (unsigned char *) lo_blob_dataptr(blob);
    
    tracks[track_get_deck(track_id)].length = (unsigned int) argv[2]->i;
    //printf("track.length: %i\n", tracks[0].length);
    
    track_add_ppm_block(track_id, bdata, size);
    
#ifdef __ANDROID__
     __android_log_print(ANDROID_LOG_DEBUG, "osc.c", "arg %d '%c' [%d byte blob]\nlength:%d", 0, types[0], size, tracks[track_get_deck(track_id)].length);
#endif

    //printf("\n");
    //fflush(stdout);

    return 1;
}


int ppm_end_handler(const char *path, const char *types, lo_arg ** argv,
                    int argc, void *data, void *user_data)
{

    printf("ppm_handler: path: <%s>\n", path);
#ifdef __ANDROID__
    __android_log_print(ANDROID_LOG_DEBUG, "osc.c", "path: <%s>\n", path);
#endif
    
    //interface_update_closeup(osc->twinterface);
    
    //printf("\n");
    //fflush(stdout);

    return 1;
}

int pos_handler(const char *path, const char *types, lo_arg ** argv,
                    int argc, void *data, void *user_data)
{

    //printf("path: <%s>\n", path);
    //__android_log_print(ANDROID_LOG_DEBUG, "osc.c", "path: <%s>\n", path);
    
    tracks[argv[0]->i].position = argv[1]->f;
    tracks[argv[0]->i].pitch = argv[2]->f;
    
     //__android_log_print(ANDROID_LOG_DEBUG, "osc.c", "arg %d '%c : %f", 0, types[0], argv[0]->f);

    //printf("\n");
    //fflush(stdout);

    return 1;
}

int scale_handler(const char *path, const char *types, lo_arg ** argv,
                    int argc, void *data, void *user_data)
{

    //printf("path: <%s>\n", path);
    //__android_log_print(ANDROID_LOG_DEBUG, "osc.c", "path: <%s>\n", path);
    
    tracks[0].scale = argv[0]->i;
    
     //__android_log_print(ANDROID_LOG_DEBUG, "osc.c", "arg %d '%c : %f", 0, types[0], argv[0]->f);

    //printf("\n");
    //fflush(stdout);

    return 1;
}

int osc_send_pitch(int d, const float pitch)
{
    lo_address t = lo_address_new(ADDRESS, "7770");
    
    if (lo_send(t, "/xwax/pitch", "if", d, pitch) == -1) {
#ifdef __ANDROID__
        __android_log_print(ANDROID_LOG_DEBUG, "osc.c", "OSC error %d: %s\n", lo_address_errno(t),
            lo_address_errstr(t));
#endif
    }

    
    return 1;
}

int osc_send_position(int d, const float position)
{
    lo_address t = lo_address_new(ADDRESS, "7770");
        
    if (lo_send(t, "/xwax/position", "if", d, position) == -1) {
#ifdef __ANDROID__
    __android_log_print(ANDROID_LOG_DEBUG, "osc.c", "OSC error %d: %s\n", lo_address_errno(t),
        lo_address_errstr(t));
#endif
    }  
    
    return 1;
}
