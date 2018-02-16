
#include "aaudio.h"
#include "aallocator.h"

/*
  TODO: replace this with WASAPI instead
  https://msdn.microsoft.com/en-us/library/windows/desktop/dd316756(v=vs.85).aspx
*/

_persist bool audio_initialized;

#if _use_xaudio

class VoiceCallback;

// Global Flags

_persist IXAudio2* xaudio2;
_persist VoiceCallback* voice_callback;


AAudioContext ACreateAudioDevice(const s8* device_name, u32 frequency, u32 channels,
				 u32 format){

  // Create AAudioContext
  AAudioContext audio_context = {};
  audio_context.channels = channels;

  HRESULT hr;

  if (!audio_initialized)
    {
      CoInitializeEx(NULL, COINIT_MULTITHREADED);

      hr = XAudio2Create(&xaudio2, 0, XAUDIO2_DEFAULT_PROCESSOR);

      // Create XAudio2 Engine
      _kill("Audio Engine creation failed.",FAILED(hr));

      // Set creation flag
      audio_initialized = true;
    }


  hr	= xaudio2->CreateMasteringVoice(&audio_context.handle,
					channels,
					frequency,
					0,
					(LPCWSTR) device_name,
					NULL,
					AudioCategory_GameEffects);

  // Create mastering voice
  _kill("Mastering Voice creation failed.",
	FAILED(hr));

  // TODO: Avg Bytes Per Sec
  // Create source voice
  WAVEFORMATEX wave_format;
  wave_format.wFormatTag = format;
  wave_format.nChannels = channels;
  wave_format.cbSize = 0;
  wave_format.wBitsPerSample = 16;
  wave_format.nBlockAlign = (wave_format.nChannels * wave_format.wBitsPerSample) / 8;
  wave_format.nSamplesPerSec = frequency;
  wave_format.nAvgBytesPerSec = wave_format.nSamplesPerSec * wave_format.nBlockAlign;

  hr = xaudio2->CreateSourceVoice(&(audio_context.source_voice), &wave_format, 0,
				  XAUDIO2_DEFAULT_FREQ_RATIO,0, NULL, NULL);

  // Create the callback
  _kill("Source Voice creation failed", hr);
  (audio_context.source_voice)->Start(0);

  //FIXME: We should not need this much data abt 192 mb over 30,000 submissions
  audio_context.buffer_size = _48ms2frames(1000000);

  
  audio_context.buffer_offset = (u32*) alloc(audio_context.buffer_size + sizeof(u32));
  *(audio_context.buffer_offset) = 0;

  audio_context.buffer = (s8*) (++audio_context.buffer_offset);

  return audio_context;
}


void APlayAudioDevice(AAudioContext audio_context,
		      void* data,u32 write_frames){

  // Sample Size -> sizeof(s16)
  u32 write_size = write_frames * audio_context.channels * sizeof(s16);
		
  _kill("Write size larger than max buffer bytes", write_size > XAUDIO2_MAX_BUFFER_BYTES);

  // Check if offset is larger than the buffer size
  if (*(audio_context.buffer_offset) + write_size > audio_context.buffer_size)
    {
      *(audio_context.buffer_offset) = 0;
    }

  s8* play_data = audio_context.buffer + *(audio_context.buffer_offset);

  memcpy(play_data, data, write_size);
  *(audio_context.buffer_offset) += write_size;

  XAUDIO2_BUFFER xaudio2_buffer =
    {
      0,
      write_size,
      (const BYTE*) play_data,
      0,
      0,
      0,
      0,
      0,
      play_data
    };

  HRESULT hr;

  hr = (audio_context.source_voice)->SubmitSourceBuffer(&xaudio2_buffer);
  _kill("Unable to submit source buffer", hr);
}


u32 AAudioDeviceWriteAvailable(AAudioContext audiocontext){

  XAUDIO2_VOICE_STATE voice_state;
  audiocontext.source_voice->GetState(&voice_state, 0);
	
  if (voice_state.BuffersQueued < XAUDIO2_MAX_QUEUED_BUFFERS){
    return (u32)-1;  
  }
  return 0;
}

#else

#define COBJMACROS
#include "mmdeviceapi.h"
#include "audioclient.h"

#define _buffersize 880000
//signed 16 bit range -32,768 to 32,767

u32 Convert_SLE16_TO_F32(void* dst,void* src,u32 frame_count) {
  _kill("do actual coversion TODO: need the channel count as well",1);
  return sizeof(f32);
}

u32 Convert_F32_TO_SLE16(void* dst,void* src,u32 frame_count) {
  _kill("do actual coversion TODO: need the channel count as well",1);
  return sizeof(s16);
}

u32 Convert_NONE_SLE16(void* dst,void* src,u32 frame_count) {
  _kill("do actual coversion TODO: need the channel count as well",1);
  memcpy(dst, src, frame_count * sizeof(s16));
  return sizeof(s16);
}

u32 Convert_NONE_F32(void* dst, void* src, u32 frame_count) {
  _kill("do actual coversion TODO: need the channel count as well",1);
  memcpy(dst, src, frame_count * sizeof(f32));
  return sizeof(f32);
} 

static IMMDeviceEnumerator* device_enum = 0;

//WASAPI allows us to change the sample rate of a stream but not the stream format
AAudioContext ACreateAudioDevice(const s8* device_name,u32 frequency,u32 channels,
				 u32 format){

	AAudioContext context = {};

  HRESULT res = 0;

  if (!audio_initialized) {

    res = CoInitialize(0);
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

  u32 conversion_buffer[sizeof(u32) * _reserved_frames] = {};
  _kill("exceeded conversion reserved conversion buffer\n", frame_count > _reserved_frames);

  auto samplesize = context->conversion_function(data, conversion_buffer, frame_count);
  memcpy(dst_buffer, conversion_buffer, (samplesize * context->channels) * frame_count);
}

void APlayAudioDevice(AAudioContext context,void* data,u32 write_frames){

  s8* dst_buffer = 0;
  context.renderclient->GetBuffer(write_frames, (BYTE**)&dst_buffer);

  ConvertAndWrite(&context, data, write_frames, dst_buffer);

  context.renderclient->ReleaseBuffer(write_frames, 0);
}

#endif
