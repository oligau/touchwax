#include <stdio.h>
#include <stdlib.h>

#ifndef _MSC_VER
#include <unistd.h>
#endif

#ifdef __ANDROID__
#include <android/log.h>
#endif
#include "track.h"
#include "interface.h"

#include "lo/lo.h"

struct osc *osc;
lo_server_thread st = 0;

int done = 0;

#ifdef __ANDROID__
const char ADDRESS[] = "10.10.10.1";
#else
const char ADDRESS[] = "127.0.0.1";
#endif

void error(int num, const char *m, const char *path);

int generic_handler(const char *path, const char *types, lo_arg ** argv,
                    int argc, void *data, void *user_data);

int ppm_handler(const char *path, const char *types, lo_arg ** argv,
                    int argc, void *data, void *user_data);

int track_load_handler(const char *path, const char *types, lo_arg ** argv,
                    int argc, void *data, void *user_data);
                    
int pos_handler(const char *path, const char *types, lo_arg ** argv,
                    int argc, void *data, void *user_data);   

int scale_handler(const char *path, const char *types, lo_arg ** argv,
                    int argc, void *data, void *user_data);                   

int osc_init(struct twinterface *twinterface)
{
    osc = (struct osc *) malloc(sizeof(struct osc));
    osc->twinterface = twinterface;
    
    /* start a new server on port 7770 */
    st = lo_server_thread_new("7771", error);

    /* add method that will match any path and args */
    lo_server_thread_add_method(st, "/touchwax/ppm", "bi", ppm_handler, NULL);

    /* add method that will match any path and args */
    lo_server_thread_add_method(st, "/touchwax/track_load", "ssi", track_load_handler, NULL);

    /* add method that will match any path and args */
    lo_server_thread_add_method(st, "/touchwax/position", "f", pos_handler, NULL);

    /* add method that will match any path and args */
    lo_server_thread_add_method(st, "/touchwax/scale", "i", scale_handler, NULL);    

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
    int i;

    //printf("path: <%s>\n", path);
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
    
    track_init(0);
    
    tracks[0].artist = (char *) argv[0];
    tracks[0].title = (char *) argv[1];
    tracks[0].rate = argv[2]->i;
    
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

    //printf("path: <%s>\n", path);
#ifdef __ANDROID__
    __android_log_print(ANDROID_LOG_DEBUG, "osc.c", "path: <%s>\n", path);
#endif
            
    lo_blob blob = argv[0];
    int size = lo_blob_datasize(blob);
    unsigned char *bdata = (unsigned char *) lo_blob_dataptr(blob);
    
    tracks[0].length = (unsigned int) argv[1]->i;
    
    track_add_ppm_block(bdata, size);
    
#ifdef __ANDROID__
     __android_log_print(ANDROID_LOG_DEBUG, "osc.c", "arg %d '%c' [%d byte blob]\nlength:%d", 0, types[0], size, tracks[0].length);
#endif

    //printf("\n");
    //fflush(stdout);

    return 1;
}

int pos_handler(const char *path, const char *types, lo_arg ** argv,
                    int argc, void *data, void *user_data)
{

    //printf("path: <%s>\n", path);
    //__android_log_print(ANDROID_LOG_DEBUG, "osc.c", "path: <%s>\n", path);
    
    tracks[0].position = argv[0]->f;
    
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

int osc_send_pitch(const float pitch)
{
    lo_address t = lo_address_new(ADDRESS, "7770");
    
    if (lo_send(t, "/xwax/pitch", "f", pitch) == -1) {
#ifdef __ANDROID__
        __android_log_print(ANDROID_LOG_DEBUG, "osc.c", "OSC error %d: %s\n", lo_address_errno(t),
        lo_address_errstr(t));
#endif
    }
    
    return 1;
}

int osc_send_position(const float position)
{
    lo_address t = lo_address_new(ADDRESS, "7770");
    
    if (lo_send(t, "/xwax/position", "f", position) == -1) {
#ifdef __ANDROID__
        __android_log_print(ANDROID_LOG_DEBUG, "osc.c", "OSC error %d: %s\n", lo_address_errno(t),
        lo_address_errstr(t));
#endif
    }
    
    return 1;
}
