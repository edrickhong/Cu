#include "aaudio.h"
#include "libload.h"

#define _test(call) if(call < 0){_kill("",1);}


_persist void* pcm_avail_update_fptr  = 0;
_persist void* pcm_writei_fptr = 0;
_persist void* pcm_recover_fptr = 0;




#define								\
snd_pcm_avail_update						\
((snd_pcm_sframes_t (*)(snd_pcm_t*))pcm_avail_update_fptr)

#define									\
snd_pcm_writei							\
((snd_pcm_sframes_t (*)(snd_pcm_t*,const void*,snd_pcm_uframes_t))pcm_writei_fptr)

#define							\
snd_pcm_recover					\
((s32 (*)(snd_pcm_t*,s32,s32))pcm_recover_fptr)

_persist LibHandle audiolib = 0;


void _ainline InternalLoadAudioLib(){
    
    if(audiolib){
        return;
    }
    
    const s8* audio_libs[] = {
        "libasound.so.2",
        "libasound.so",
    };
    
    LibHandle lib = 0;
    
    for(u32 i = 0; i < _arraycount(audio_libs); i++){
        lib = LLoadLibrary(audio_libs[i]);
        if(lib){
            break;
        }
    }
    
    _kill("can't load audio lib\n",!lib);
    
    {
        pcm_avail_update_fptr =
            LGetLibFunction(lib,"snd_pcm_avail_update");
        
        pcm_writei_fptr =
            LGetLibFunction(lib,"snd_pcm_writei");
        
        pcm_recover_fptr =
            LGetLibFunction(lib,"snd_pcm_recover");  
    }
    
    
    audiolib = lib;
}


AAudioContext ACreateAudioDevice(const s8* device_string,u32 frequency,u32 channels,
                                 u32 format){
    
    /*
      Now it only supports 48kHz. We should calculate the period length and period count according
      to the frequency passed to us
    */
    
    InternalLoadAudioLib();
    
    auto snd_pcm_open_fptr = (s32 (*)(snd_pcm_t**,const s8*,snd_pcm_stream_t,s32 mode))
        LGetLibFunction(audiolib,"snd_pcm_open");
    
    auto snd_pcm_hw_params_malloc_fptr  = (s32 (*)(snd_pcm_hw_params_t**))
        LGetLibFunction(audiolib,"snd_pcm_hw_params_malloc");
    
    auto snd_pcm_hw_params_any_fptr  = (s32 (*)(snd_pcm_t *,snd_pcm_hw_params_t *))
        LGetLibFunction(audiolib,"snd_pcm_hw_params_any");
    
    
    auto snd_pcm_hw_params_set_access_fptr =
        (s32 (*)(snd_pcm_t*,snd_pcm_hw_params_t*,snd_pcm_access_t))
        LGetLibFunction(audiolib,"snd_pcm_hw_params_set_access");
    
    
    auto snd_pcm_hw_params_set_format_fptr = (s32 (*)(snd_pcm_t*,snd_pcm_hw_params_t*,snd_pcm_format_t))LGetLibFunction(audiolib,"snd_pcm_hw_params_set_format");
    
    
    auto snd_pcm_hw_params_set_channels_fptr =
        (s32 (*)(snd_pcm_t*,snd_pcm_hw_params_t*,u32))
        LGetLibFunction(audiolib,"snd_pcm_hw_params_set_channels");
    
    auto snd_pcm_hw_params_set_rate_near_fptr = (s32 (*)(snd_pcm_t*,snd_pcm_hw_params_t*,u32*,s32*))LGetLibFunction(audiolib,"snd_pcm_hw_params_set_rate_near");
    
    auto snd_pcm_hw_params_set_periods_near_fptr = (s32 (*)(snd_pcm_t*,snd_pcm_hw_params_t*,u32*,s32*))LGetLibFunction(audiolib,"snd_pcm_hw_params_set_periods_near");
    
    auto snd_pcm_hw_params_set_buffer_size_near_fptr = (s32 (*)(snd_pcm_t*,snd_pcm_hw_params_t*,snd_pcm_uframes_t*))
        LGetLibFunction(audiolib,"snd_pcm_hw_params_set_buffer_size_near");
    
    auto snd_pcm_hw_params_fptr  = (s32 (*)(snd_pcm_t*,snd_pcm_hw_params_t*))
        LGetLibFunction(audiolib,"snd_pcm_hw_params");
    
    auto snd_pcm_hw_params_set_period_size_near_fptr = (s32 (*)(snd_pcm_t*,snd_pcm_hw_params_t*,snd_pcm_uframes_t*,s32*))
        LGetLibFunction(audiolib,"snd_pcm_hw_params_set_period_size_near");
    
    
    auto snd_pcm_sw_params_malloc_fptr  = (s32 (*)(snd_pcm_sw_params_t**))
        LGetLibFunction(audiolib,"snd_pcm_sw_params_malloc");
    
    auto snd_pcm_sw_params_current_fptr =
        (s32 (*)(snd_pcm_t*,snd_pcm_sw_params_t*))
        LGetLibFunction(audiolib,"snd_pcm_sw_params_current");
    
    auto snd_pcm_sw_params_set_avail_min_fptr = (s32 (*)(snd_pcm_t*,snd_pcm_sw_params_t*,snd_pcm_uframes_t))LGetLibFunction(audiolib,"snd_pcm_sw_params_set_avail_min");
    
    
    auto snd_pcm_sw_params_set_start_threshold_fptr = (s32 (*)(snd_pcm_t*,snd_pcm_sw_params_t*,snd_pcm_uframes_t))LGetLibFunction(audiolib,"snd_pcm_sw_params_set_start_threshold");
    
    auto snd_pcm_sw_params_fptr = (s32 (*)(snd_pcm_t*,snd_pcm_sw_params_t*))
        LGetLibFunction(audiolib,"snd_pcm_sw_params");
    
    auto snd_pcm_sw_params_free_fptr = (void (*)(snd_pcm_sw_params_t*))
        LGetLibFunction(audiolib,"snd_pcm_sw_params_free");
    
    auto snd_pcm_nonblock_fptr = (s32 (*)(snd_pcm_t*,s32))
        LGetLibFunction(audiolib,"snd_pcm_nonblock");
    
    auto snd_pcm_hw_params_free_fptr = (void (*)(snd_pcm_hw_params_t *))
        LGetLibFunction(audiolib,"snd_pcm_hw_params_free");
    
#ifdef DEBUG
    
    auto snd_pcm_hw_params_get_buffer_size_fptr = (s32 (*)(const snd_pcm_hw_params_t*,snd_pcm_uframes_t*))LGetLibFunction(audiolib,"snd_pcm_hw_params_get_buffer_size");
    
    auto snd_pcm_hw_params_get_period_size_fptr = (s32 (*)(const snd_pcm_hw_params_t*,snd_pcm_uframes_t*,s32*))LGetLibFunction(audiolib,"snd_pcm_hw_params_get_period_size");
    
    auto snd_pcm_hw_params_get_periods_fptr = (s32 (*)(const snd_pcm_hw_params_t*,u32*,s32*))
        LGetLibFunction(audiolib,"snd_pcm_hw_params_get_periods");
    
#endif
    
    AAudioContext context = {};
    
    snd_pcm_t* handle;
    
    snd_pcm_hw_params_t* hwparams;
    snd_pcm_sw_params_t* swparams;
    
    _test(snd_pcm_open_fptr(&handle,device_string,SND_PCM_STREAM_PLAYBACK,
                            SND_PCM_NONBLOCK));
    
    snd_pcm_hw_params_malloc_fptr(&hwparams);
    
    //hw params
    {
        
        
        _test(snd_pcm_hw_params_any_fptr(handle, hwparams));
        
        _test(snd_pcm_hw_params_set_access_fptr(handle, hwparams,
                                                SND_PCM_ACCESS_RW_INTERLEAVED));
        
        snd_pcm_hw_params_set_format_fptr(handle,hwparams,
                                          (snd_pcm_format_t)format);
        
        //MARK: Figure this out programatically for other values
        context.sample_size = sizeof(s16);
        
        _test(snd_pcm_hw_params_set_channels_fptr(handle, hwparams,channels));
        
        context.channels = channels;
        
        
        _test(snd_pcm_hw_params_set_rate_near_fptr(handle, hwparams,&frequency, 0));
    }
    
    snd_pcm_uframes_t period_frames = _48ms2frames(4.0f);
    
    //internal buffer configuration
    {
        
#define _periodcount 22
        
        _test(snd_pcm_hw_params_set_period_size_near_fptr(handle,hwparams,
                                                          (snd_pcm_uframes_t *)&period_frames,0));
        
        u32 periods = _periodcount;
        
        _test(snd_pcm_hw_params_set_periods_near_fptr(handle, hwparams, &periods, 0));
        
        
        snd_pcm_uframes_t bufferframes = period_frames * _periodcount;
        
        _test(snd_pcm_hw_params_set_buffer_size_near_fptr(handle, hwparams, &bufferframes));
        
        /* "set" the hardware with the desired parameters */
        snd_pcm_hw_params_fptr(handle, hwparams);
        
        
        
#ifdef DEBUG
        
        snd_pcm_uframes_t bufsize;
        
        /* Get samples for the actual buffer size */
        snd_pcm_hw_params_get_buffer_size_fptr(hwparams, &bufsize);
        
        snd_pcm_uframes_t persize = 0;
        periods = 0;
        
        snd_pcm_hw_params_get_period_size_fptr(hwparams, &persize, NULL);
        snd_pcm_hw_params_get_periods_fptr(hwparams, &periods, NULL);
        
        fprintf(stderr,
                "ALSA: period size = %ld, periods = %u, buffer size = %lu\n",
                persize, periods, bufsize);//convert these to miliseconds
        
#endif
        
    }
    
    //sw params
    {
        snd_pcm_sw_params_malloc_fptr(&swparams);
        
        
        _test(snd_pcm_sw_params_current_fptr(handle, swparams));
        
        _test(snd_pcm_sw_params_set_avail_min_fptr(handle, swparams,period_frames));
        
        _test(snd_pcm_sw_params_set_start_threshold_fptr(handle,swparams,1));
        
        _test(snd_pcm_sw_params_fptr(handle, swparams));
        
        snd_pcm_sw_params_free_fptr(swparams);
    }
    
    snd_pcm_nonblock_fptr(handle,1);
    
    snd_pcm_hw_params_free_fptr(hwparams);
    
    context.handle = handle;
    
    return context;
}


u32 AAudioDeviceWriteAvailable(AAudioContext audiocontext){
    return snd_pcm_avail_update(audiocontext.handle);
}

void APlayAudioDevice(AAudioContext audiocontext,void* data,u32 write_frames){
    
    s32 err = snd_pcm_writei(audiocontext.handle,data,write_frames);
    
    if(err == -EPIPE){
        
        printf("failed consistent write\n");
        
#ifdef DEBUG
        snd_pcm_recover(audiocontext.handle,err,0);//set to 1 to not print errors
#else
        snd_pcm_recover(audiocontext.handle,err,1);//set to 1 to not print errors
#endif
        
        
    }
    
}


void APlayAudioDevice(AAudioContext audiocontext,void* data,u32 write_frames,
                      AudioOperation op,void* args){
    
    u32 avail_frames =  snd_pcm_avail_update(audiocontext.handle);
    
    if(avail_frames >= write_frames){
        
        if(op){
            op(args);
        }
        
        s32 err = snd_pcm_writei(audiocontext.handle,data,write_frames);
        
        if(err == -EPIPE){
            
            printf("failed consistent write\n");
            // exit(1);
            //underrun occured
            
#ifdef DEBUG
            snd_pcm_recover(audiocontext.handle,err,0);//set to 1 to not print errors
#else
            snd_pcm_recover(audiocontext.handle,err,1);//set to 1 to not print errors
#endif
            
            
        }
        
    }
    
}

