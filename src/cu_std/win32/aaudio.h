#pragma once

#include "mode.h"
#include "ttype.h"
#include "xaudio2.h"

#include "mmreg.h"

/*
  NOTE: We will only support signed 16 for now
*/

#define _48ms2frames(ms) (((f32)(ms) * 48.0f) + 0.5f)
#define _48frames2ms(frames) (((f32)frames)/48.0f)

#define _441ms2frames(ms) (((f32)(ms) * 44.1f) + 0.5f)
#define _441frames2ms(frames) (((f32)frames)/44.1ff)

#define A_FORMAT_S16LE WAVE_FORMAT_PCM

#define A_DEVICE_DEFAULT 0

struct AAudioBuffer{
  void* data;
  u32 size_frames;
  u32 size;
  u32 cur_pos;
  u32 curpos_frames;
};

struct AAudioContext
{
  IXAudio2MasteringVoice* handle;//we do not really need to keep this
  IXAudio2SourceVoice* source_voice;//this is more like the handle
  u32 channels;

  u32 buffer_size;
  u32* buffer_offset;
  s8* buffer;
};

typedef void (AudioOperation(void* args));
	
AAudioContext ACreateAudioDevice(const s8* device_name,u32 frequency,u32 channels,
				 u32 format);

void ADestroyAudioDevice(AAudioContext audio_context);

void APlayAudioDevice(AAudioContext audio_context,void* data,u32 write_frames);

u32 AAudioDeviceWriteAvailable(AAudioContext audiocontext);
