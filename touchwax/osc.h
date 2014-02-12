#ifndef OSC_H
#define OSC_H

#include "interface.h"

struct osc {
  struct twinterface *twinterface;
};

int osc_init(struct twinterface *twinterface);
void osc_free();
//int osc_send_pos(const float pos);
//int osc_send_track_load();
//int osc_send_ppm_block(int d, struct track *tr);
int osc_send_pitch(const float pitch);
int osc_send_position(const float position);

//void osc_start_updater_thread();
//void osc_start_updater();

#endif
