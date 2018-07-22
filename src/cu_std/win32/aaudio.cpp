
#include "aaudio.h"
#include "aallocator.h"

/*
  TODO: replace this with WASAPI instead
  https://msdn.microsoft.com/en-us/library/windows/desktop/dd316756(v=vs.85).aspx
*/

_persist bool audio_initialized;

#define COBJMACROS
#include "mmdeviceapi.h"
#include "audioclient.h"

#define _buffersize 880000

//signed 16 bit range -32,768 to 32,767

#include "audio_util.h"


static IMMDeviceEnumerator* device_enum = 0;

//WASAPI allows us to change the sample rate of a stream but not the stream format
AAudioContext ACreateAudioDevice(const s8* device_name,u32 frequency,u32 channels,
                                 u32 format){
    
    AAudioContext context = {};
    
    HRESULT res = 0;
    
    if (!audio_initialized) {
        
        res = CoInitializeEx(0,COINIT_MULTITHREADED);
        _kill("", res != S_OK);
        
        CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
        IID IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);
        
        res = CoCreateInstance(
            CLSID_MMDeviceEnumerator, 0,
            CLSCTX_INPROC_SERVER, IID_IMMDeviceEnumerator,
            (void**)&device_enum);
        
        _kill("", res != S_OK);
        
        
        audio_initialized = true;
    }
    
    if (device_name) {
        _kill("we do not suppor this case yet\n", 1);
    }
    
    else {
        res = device_enum->GetDefaultAudioEndpoint(eRender, eMultimedia, &context.device);
    }
    
    _kill("", res != S_OK);
    
    res = context.device->Activate(__uuidof(IAudioClient), CLSCTX_ALL, 0, (void**)&context.audioclient);
    
    _kill("", res != S_OK);
    
    WAVEFORMATEX* wv_format = 0;
    context.audioclient->GetMixFormat(&wv_format);
    
    
    {
        _kill("do not support this format\n", (((WAVEFORMATEXTENSIBLE*)wv_format)->SubFormat != KSDATAFORMAT_SUBTYPE_PCM) && ((WAVEFORMATEXTENSIBLE*)wv_format)->SubFormat != KSDATAFORMAT_SUBTYPE_IEEE_FLOAT);
        
        if (format == A_FORMAT_S16LE && ((WAVEFORMATEXTENSIBLE*)wv_format)->SubFormat == KSDATAFORMAT_SUBTYPE_PCM) {
            context.conversion_function = Convert_NONE_SLE16;
        }
        
        if (format == A_FORMAT_S16LE && ((WAVEFORMATEXTENSIBLE*)wv_format)->SubFormat == KSDATAFORMAT_SUBTYPE_IEEE_FLOAT) {
            context.conversion_function = Convert_SLE16_TO_F32;
        }
        
        _kill("for now\n", wv_format->nChannels != channels);
        
        context.channels = wv_format->nChannels;
    }
    
    _kill("", res != S_OK);
    
    //AUDCLNT_STREAMFLAGS_RATEADJUST  must be in shared mode only. lets you set the sample rate
    //AUDCLNT_SHAREMODE_EXCLUSIVE Windows only
    res = context.audioclient->Initialize(AUDCLNT_SHAREMODE_SHARED, AUDCLNT_STREAMFLAGS_RATEADJUST,
                                          _buffersize,
                                          0,//period size in - 100 nanoseconds. cannot be 0 in exclusive mode
                                          wv_format, 0);
    
    _kill("", res != S_OK);
    
    if (wv_format->nSamplesPerSec != frequency) {
        IAudioClockAdjustment* clockadj = 0;
        IID IID_IAudioClockAdjustment = __uuidof(IAudioClockAdjustment);
        
        res = context.audioclient->GetService(IID_IAudioClockAdjustment, (void**)&clockadj);
        _kill("", res != S_OK);
        
        res = clockadj->SetSampleRate(frequency);
        _kill("", res != S_OK);
    }
    
    IID IID_IAudioRenderClient = __uuidof(IAudioRenderClient);
    
    res = context.audioclient->GetService(IID_IAudioRenderClient, (void**)&context.renderclient);
    _kill("", res != S_OK);
    
    
    //maybe we should start only after we do our first submission
    res = context.audioclient->Start();
    _kill("", res != S_OK);
    
    return context;
}

u32 AAudioDeviceWriteAvailable(AAudioContext context){
    
    u32 buffer_size_frames = 0;
    u32 frames_locked = 0;
    
    auto res = context.audioclient->GetBufferSize(&buffer_size_frames);
    _kill("", res != S_OK);
    
    
    res = context.audioclient->GetCurrentPadding(&frames_locked);
    _kill("", res != S_OK);
    
    
    return buffer_size_frames - frames_locked;
}

void ConvertAndWrite(AAudioContext* context, void* data, u32 frame_count, void* dst_buffer){
    
#define _reserved_frames (u32)_48ms2frames(36)
    
    auto sample_count = frame_count * context->channels;
    
    u32 conversion_buffer[sizeof(u32) * _reserved_frames] = {};
    _kill("exceeded conversion reserved conversion buffer\n", frame_count > _reserved_frames);
    
    auto samplesize = context->conversion_function(conversion_buffer,data,sample_count);
    
    memcpy(dst_buffer, conversion_buffer, samplesize * sample_count);
}

void APlayAudioDevice(AAudioContext context,void* data,u32 write_frames){
    
    s8* dst_buffer = 0;
    context.renderclient->GetBuffer(write_frames, (BYTE**)&dst_buffer);
    
    ConvertAndWrite(&context,data,write_frames,dst_buffer);
    
    context.renderclient->ReleaseBuffer(write_frames, 0);
}
