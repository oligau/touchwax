#include "track.h"

#ifdef __ANDROID__
#include <android/log.h>
#endif


void track_reset(unsigned int index)
{
  tracks[index].ppm_pos = 0;
  tracks[index].length = 0;
  tracks[index].rate = 0;
  tracks[index].position = 0;
  tracks[index].scale = 8;
  tracks[index].pitch = 0;
  tracks[index].play = 0;
}

void track_init(unsigned int index)
{
  track_reset(index);
}

void track_add_ppm_block(unsigned char *ppm_block, int size)
{
  
  /* TODO change for memcpy implementation */
  int i;
  for(i = 0; i < size; ++i) {
    unsigned char c = ppm_block[i];
    tracks[0].ppm[i+tracks[0].ppm_pos] = c;    
    //tracks[0].ppm[i] = c;    

  }
  tracks[0].ppm_pos += size;

#ifdef __ANDROID__
  __android_log_print(ANDROID_LOG_DEBUG, "track.c", "ppm_pos: %d\n", tracks[0].ppm_pos);
#endif

}

unsigned char track_get_ppm(struct track *tr, unsigned int sp)
{
  return tr->ppm[sp/TRACK_PPM_RES];
}

void track_toggle_play(int d)
{
  if(tracks[0].play) {
    osc_send_pitch(0); //stop
    tracks[0].play = 0;
  }
  else {
    osc_send_pitch(tracks[0].pitch); //play
    tracks[0].play = 1;    
  }
  
}
