#ifndef OSC_H
#define OSC_H

#include "lo/lo.h"

#include "interface.h"

struct osc {
  struct twinterface *twinterface;
  int ndeck;
};

int osc_init(struct twinterface *twinterface);
void osc_free();

int osc_send_pitch(int d, const float pitch);
int osc_send_position(int d, const float position);

void error(int num, const char *m, const char *path);
int generic_handler(const char *path, const char *types, lo_arg ** argv,
                    int argc, void *data, void *user_data);
int ppm_end_handler(const char *path, const char *types, lo_arg ** argv,
                    int argc, void *data, void *user_data);
int ppm_handler(const char *path, const char *types, lo_arg ** argv,
                    int argc, void *data, void *user_data);
int track_load_handler(const char *path, const char *types, lo_arg ** argv,
                    int argc, void *data, void *user_data);                 
int pos_handler(const char *path, const char *types, lo_arg ** argv,
                    int argc, void *data, void *user_data);   
int scale_handler(const char *path, const char *types, lo_arg ** argv,
                    int argc, void *data, void *user_data);   


#endif
