#pragma once

#include "mode.h"
#include "ttype.h"

#define COBJMACROS
#include "mmdeviceapi.h"
#include "audioclient.h"


struct AAudioContext{
    IMMDevice* device;
    IAudioClient* audioclient;
    IAudioRenderClient* renderclient;
    u32 channels;
    u32 (*conversion_function)(void*, void*, u32);
};

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

typedef void (AudioOperation(void* args));

AAudioContext ACreateAudioDevice(const s8* device_name,u32 frequency,u32 channels,
                                 u32 format);

void ADestroyAudioDevice(AAudioContext audio_context);

void APlayAudioDevice(AAudioContext audio_context,void* data,u32 write_frames);

u32 AAudioDeviceWriteAvailable(AAudioContext audiocontext);
