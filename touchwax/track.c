#include "track.h"

#ifdef __ANDROID__
#include <android/log.h>
#endif

struct track tracks[2]; // extern from track.h

void track_init(unsigned int index)
{
  tracks[index].id = 0;
  tracks[index].ppm_pos = 0;
  tracks[index].length = 0;
  tracks[index].rate = 0;
  tracks[index].position = 0;
  tracks[index].scale = 8;
  tracks[index].pitch = 1;
  tracks[index].play = 0;
}

int track_get_deck(int track_id)
{
  int i;
  for(i = 0; i < TRACK_NDECK; ++i) {
    if(tracks[i].id == track_id)
      return i;
  }
  return -1;
}

void track_add_ppm_block(int track_id, unsigned char *ppm_block, int size)
{
  int d = track_get_deck(track_id);
  
  /* TODO change for memcpy implementation */
  int i;
  for(i = 0; i < size; ++i) {
    unsigned char c = ppm_block[i];
    tracks[d].ppm[i+tracks[d].ppm_pos] = c;    
    //tracks[0].ppm[i] = c;    

  }
  tracks[d].ppm_pos += size;

  fprintf(stderr, "track_add_ppm_block: deck: %i, ppm_pos: %i\n", d, tracks[d].ppm_pos);
  
}

unsigned char track_get_ppm(struct track *tr, unsigned int sp)
{
  return tr->ppm[sp / TRACK_PPM_RES];
}

void track_toggle_play(int d, float pitch)
{
  if(tracks[d].play) {
    osc_send_pitch(d, 0); //stop
    tracks[d].play = 0;
  }
  else {
    osc_send_pitch(d, pitch); //play
    tracks[d].play = 1;    
  }
  
}

void track_reverse_play(int d)
{
  tracks[d].pitch = -tracks[0].pitch;
  
  if(tracks[d].play) {
    osc_send_pitch(d, tracks[d].pitch);
  }
  fprintf(stderr, "reverse deck %i pitch %f\n", d, tracks[d].pitch);
  
}

void track_reset(int d)
{
  osc_send_position(d, 0);
  
  fprintf(stderr, "reset deck %i\n", d);
}
