
#include "aaudio.h"
#include "aallocator.h"

/*
  TODO: replace this with WASAPI instead
  https://msdn.microsoft.com/en-us/library/windows/desktop/dd316756(v=vs.85).aspx
*/

class VoiceCallback;

// Global Flags
_persist bool audio_initialized;
_persist IXAudio2* xaudio2;
_persist VoiceCallback* voice_callback;


#define COBJMACROS
#include "mmdeviceapi.h"
#include "audioclient.h"

#define _buffersize 880000

void TestWASAPI(){

  auto res = CoInitialize(0);

  _kill("", res != S_OK);

  CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
  IID IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);
  IMMDeviceEnumerator* device_enum = 0;

  res = CoCreateInstance(
			 CLSID_MMDeviceEnumerator,0,
			 CLSCTX_INPROC_SERVER, IID_IMMDeviceEnumerator,
			 (void**)&device_enum);

  _kill("", res != S_OK);

  IMMDevice* device = 0;

  res = device_enum->GetDefaultAudioEndpoint(eRender, eMultimedia, &device);

  _kill("", res != S_OK);

  IAudioClient* audioclient = 0;

  res = device->Activate(__uuidof(IAudioClient), CLSCTX_ALL, 0, (void**)&audioclient);

  _kill("", res != S_OK);

  WAVEFORMATEX* format = 0;

  res = audioclient->GetMixFormat(&format);

  _kill("", res != S_OK);

  //AUDCLNT_STREAMFLAGS_RATEADJUST  must be in shared mode only. lets you set the sample rate
  //AUDCLNT_SHAREMODE_EXCLUSIVE Windows only
  res =audioclient->Initialize(AUDCLNT_SHAREMODE_SHARED,0, _buffersize,
			       0,//period size in - 100 nanoseconds. cannot be 0 in exclusive mode
			       format,0);

  _kill("", res != S_OK);

  //it looks like wasapi requires us to do our own format conversion
  _kill("", 1);
}

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
		      void* data,
		      u32 write_frames){

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

