#ifndef TRACK_H
#define TRACK_H

#include "osc.h"

#define TRACK_NDECK 2
#define TRACK_MAX_BLOCKS 64
#define TRACK_BLOCK_SAMPLES (2048 * 1024)
#define TRACK_PPM_RES 64
#define TRACK_OVERVIEW_RES 2048

struct track {
    int id;
    char *artist, *title;
    int rate;
    unsigned int length; /* track length in samples */
    unsigned int ppm_pos;
    float position;
    int scale;
    float pitch;
    int play;
    unsigned char ppm[TRACK_MAX_BLOCKS * TRACK_BLOCK_SAMPLES / TRACK_PPM_RES];
};

extern struct track tracks[2];

void track_init(unsigned int index);
int track_get_deck(int track_id);
void track_add_ppm_block(int track_id, unsigned char *ppm_block, int size);
unsigned char track_get_ppm(struct track *tr, unsigned int sp);
void track_toggle_play(int d);
void track_reverse_play(int d);
void track_reset(int d);

#endif
