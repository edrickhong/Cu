#pragma once

#include "mode.h"
#include "alsa/asoundlib.h"
#include "ttype.h"

/*
  NOTE: We will only support signed 16 for now
*/


#define _48ms2frames(ms) (((f32)(ms) * 48.0f) + 0.5f)
#define _48frames2ms(frames) (((f32)frames)/48.0f)

#define _441ms2frames(ms) (((f32)(ms) * 44.1f) + 0.5f)
#define _441frames2ms(frames) (((f32)frames)/44.1ff)

#define A_FORMAT_S16LE SND_PCM_FORMAT_S16_LE

#define A_DEVICE_DEFAULT "default"

struct AAudioBuffer{
  void* data;
  u32 size_frames;
  u32 size;
  u32 cur_pos;
  u32 curpos_frames;
};


struct AAudioContext{
  snd_pcm_t* handle;
  u32 channels;
  u32 sample_size;
};

typedef void (AudioOperation(void* args));


AAudioContext ACreateAudioDevice(const s8* devicename,u32 frequency,u32 channels,u32 format);

void ADestroyAudioDevice(AAudioContext audiocontext);

u32 AAudioDeviceWriteAvailable(AAudioContext audiocontext);

void APlayAudioDevice(AAudioContext audiocontext,void* data,u32 write_frames);
