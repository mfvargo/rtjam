#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <alsa/asoundlib.h>
#include </usr/include/alsa/pcm.h>

const snd_pcm_format_t FORMAT = SND_PCM_FORMAT_FLOAT ;
#define SAMPLING_RATE 48000
#define CHANNELS 2
#define FRAMES_PER_PERIOD 64

#endif
