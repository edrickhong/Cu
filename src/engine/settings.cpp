#pragma once


//TODO: seems like a perfect candidate for reflection tbh

struct Settings{
    u32 backend = W_CREATE_FORCE_XLIB;
    u32 window_width = 1280;
    u32 window_height = 720;
    u32 window_x = 0;
    u32 window_y = 0;
    u32 swapchain_depth = 3;
    u32 vsync_mode = (u32)-1;
    u32 frame_alloc_size = 32;
    u32 host_alloc_size = 1024;
    u32 gpu_alloc_size = 22;
    u32 vt_width = 128;
    u32 vt_height = 64;
    u32 audio_format = A_FORMAT_S16LE;
    u32 audio_channels = 2;
    u32 audio_frequency = 48000;
    u32 playbuffer_size_ms = 24;
    f32 master_volume = 1.0f;
    u32 launch_threads = (u32)-1;
    u32 gpu_device_hash = (u32)-1;
    
};


//TODO: cparser cannot handle enum REFL SETTING_VALUE : u32
enum REFL SETTING_VALUE{
    
    SETTING_VALUE_CHOOSE_BEST = (s32)-2,
    
    
    SETTING_VALUE_DIRECT = 0, //doesn't do anything now
    SETTING_VALUE_XLIB = W_CREATE_FORCE_XLIB,
    SETTING_VALUE_WAYLAND = W_CREATE_FORCE_WAYLAND,
    
    SETTING_VALUE_CENTER = (s32)-1,
    
    SETTING_VALUE_OFF = VSYNC_NONE,
    SETTING_VALUE_NORMAL = VSYNC_NORMAL,
    SETTING_VALUE_FAST = VSYNC_FAST,
    SETTING_VALUE_LAZY = VSYNC_LAZY,
    
    
    SETTING_VALUE_S16 = A_FORMAT_S16LE,
    SETTING_VALUE_S24 = (s32)-3,
    SETTING_VALUE_S32 = (s32)-4,
    
    SETTING_VALUE_F32 = (s32)-5,
    SETTING_VALUE_F64 = (s32)-6,
    
    SETTING_VALUE_MONO = 1,
    SETTING_VALUE_STEREO = 2,
    SETTING_VALUE_5_1 = 6,
    SETTING_VALUE_7_1 = 8,
    
    SETTING_VALUE_44100 = 44100,
    SETTING_VALUE_48000 = 48000,
    
    SETTING_VALUE_ALL = (s32)-1,
    SETTING_VALUE_DEFAULT = (s32)-1,
};

struct REFL Settings2{
    
    u32 WINDOW_BACKEND = SETTING_VALUE_XLIB;
    u32 WINDOW_WIDTH = 1280;
    u32 WINDOW_HEIGHT = 720;
    u32 WINDOW_POSX = 0;
    u32 WINDOW_POSY = 0;
    u32 SWAPCHAIN_DEPTH = 3;
    u32 VSYNC_MODE = SETTING_VALUE_CHOOSE_BEST;
    u32 FRAME_ALLOC_SIZE = 32;
    u32 HOST_ALLOC_SIZE = 1024;
    u32 GPU_ALLOC_SIZE = 22;
    u32 VT_PHYS_WIDTH = 128;
    u32 VT_PHYS_HEIGHT = 64;
    u32 AUDIO_FORMAT = SETTING_VALUE_S16;
    u32 AUDIO_CHANNELS = SETTING_VALUE_STEREO;
    u32 AUDIO_FREQUENCY = SETTING_VALUE_48000;
    u32 PLAYBUFFER_SIZE_MS = 24;
    f32 MASTER_VOLUME_FACTOR = 1.0f;
    u32 LAUNCH_THREADS = SETTING_VALUE_ALL;
    u32 GPU_DEVICE = SETTING_VALUE_DEFAULT;
};

void GenerateSettingsString2(Settings2* settings,s8* src_buffer){
    
    auto settings_meta = MetaGetStructByName("Settings2");
    
    auto enum_meta = MetaGetEnumByName("SETTING_VALUE");
    
    for(u32 i = 0; i < settings_meta->member_count; i++){
        
        auto member = &settings_meta->member_array[i];
        
        s8 buffer[1024] = {};
        
        m32 value = {};
        
        MetaGetValueByNameHash(settings,0,&value,member->name_hash,settings_meta->member_array,
                               settings_meta->member_count);
        
        s8* enum_names_array[16] = {};
        u32 enum_names_count = 0;
        
        MetaGetEnumNamesByValue((s64)value.i,enum_meta->entry_array,enum_meta->entry_count,(const s8**)&enum_names_array,&enum_names_count);
        
        if(enum_names_count){
            
            printf("%s : %s\n",member->name_string,&enum_names_array[0][14]);
            
        }
        
        else{
            
            if(IsIntType(member->type_hash)){
                printf("%s : %d\n",member->name_string,value.u);
            }
            
            else{
                printf("%s : %f\n",member->name_string,value.f);
            }
            
            
            
        }
        
    }
    
}

void Test(){
    
    Settings2 settings;
    
    s8 buffer[2048] = {};
    
    GenerateSettingsString2(&settings,buffer);
    
    printf("%s\n",buffer);
    
    exit(0);
}

//TODO: use Settings2. it should condense to less code

void WindowBackendFlagToString(u32 flag,s8* dst_string){
    
#ifdef _WIN32
    
    auto string = "CHOOSE_BEST";
    memcpy(dst_string,string,strlen(string));
    
#else
    
    switch(flag){
        
        case W_CREATE_FORCE_WAYLAND:{
            auto string = "WAYLAND";
            memcpy(dst_string,string,strlen(string));
        }break;
        
        case W_CREATE_FORCE_XLIB:{
            auto string = "XLIB";
            memcpy(dst_string,string,strlen(string));
        }break;
        
        case 0:{
            auto string = "CHOOSE_BEST";
            memcpy(dst_string,string,strlen(string));
        }break;
        
        //TODO: support direct to display
        
        default:{
            _kill("not supported right now\n",1);
        }break;
    }
    
#endif
    
}

void VsyncFlagToString(u32 flag,s8* dst_string){
    
    //OFF/NORMAL/FAST/LAZY/CHOOSE_BEST
    
    switch(flag){
        
        case VSYNC_NONE:{
            auto string = "OFF";
            memcpy(dst_string,string,strlen(string));
        }break;
        
        
        case VSYNC_NORMAL:{
            auto string = "NORMAL";
            memcpy(dst_string,string,strlen(string));
        }break;
        
        case VSYNC_FAST:{
            auto string = "FAST";
            memcpy(dst_string,string,strlen(string));
        }break;
        
        
        case VSYNC_LAZY:{
            auto string = "LAZY";
            memcpy(dst_string,string,strlen(string));
        }break;
        
        
        case (u32)-1:{
            auto string = "CHOOSE_BEST";
            memcpy(dst_string,string,strlen(string));
        }break;
        
        
        default:{
            _kill("invalid value\n",1);
        }break;
        
    }
    
}



void AudioFormatFlagToString(u32 flag,u32 channels,s8* dst_string){
    
    if(flag == A_FORMAT_S16LE && channels == 2){
        auto string = "S16_STEREO";
        memcpy(dst_string,string,strlen(string));
    }
    
    else{
        _kill("not supported right now\n",1);
    }
    
}

void LaunchThreadsNoToString(u32 no,s8* dst_string){
    
    if(no == (u32)-1){
        auto string = "ALL";
        memcpy(dst_string,string,strlen(string));
        return;
    }
    
    sprintf(dst_string,"%d",no);
}

u32 LaunchThreadStringToNo(s8* string){
    
    auto hash = PHashString(string);
    
    if(hash == PHashString("ALL")){
        return (u32)-1;
    }
    
    return atoi(string);
    
}


u32 WindowBackendStringToFlag(s8* string){
    
    auto hash = PHashString(string);
    
#ifndef _WIN32
    
    switch(hash){
        
        case PHashString("WAYLAND"):{
            return W_CREATE_FORCE_WAYLAND;
        }break;
        
        
        case PHashString("XLIB"):{
            return W_CREATE_FORCE_XLIB;
        }break;
        
        
        
        case PHashString("CHOOSE_BEST"):{
            return 0;
        }break;
    }
    
#endif
    
    return 0;
}

u32 VsyncStringToFlag(s8* string){
    
    auto hash = PHashString(string);
    
    switch(hash){
        
        case PHashString("OFF"):{
            return VSYNC_NONE;
        }break;
        
        
        case PHashString("NORMAL"):{
            return VSYNC_NORMAL;
        }break;
        
        case PHashString("FAST"):{
            return VSYNC_FAST;
        }break;
        
        
        case PHashString("LAZY"):{
            return VSYNC_LAZY;
        }break;
        
        case PHashString("CHOOSE_BEST"):{
            return (u32)-1;
        }break;
    }
    
    
    return (u32)-1;
    
}

void AudioFormatStringToFlag(s8* string,u32* format,u32* channels){
    
    auto hash = PHashString(string);
    
    switch(hash){
        
        case PHashString("S16_STEREO"):{
            
            *format = A_FORMAT_S16LE;
            *channels = 2;
            
            return;
        }break;
    }
}

void GenerateSettingsString(Settings* settings,s8* buffer){
    
    
    s8 backend_string[256] = {};
    s8 vsync_string[256] = {};
    s8 audioformat_string[256] = {};
    s8 threads_string[256] = {};
    s8 gpudevice_string[256] = {};
    
    if(settings->gpu_device_hash == (u32)-1){
        auto string = "DEFAULT";
        memcpy(gpudevice_string,string,strlen(string));
    }
    
    else{
        //get gpu hash
    }
    
    WindowBackendFlagToString(settings->backend,backend_string);
    
    VsyncFlagToString(settings->vsync_mode,vsync_string);
    
    AudioFormatFlagToString(settings->audio_format,settings->audio_channels,audioformat_string);
    
    LaunchThreadsNoToString(settings->launch_threads,threads_string);
    
    sprintf(buffer,
            
            "\n"
            "#choose between DIRECT/XLIB/WAYLAND/CHOOSE_BEST\n"
            "WINDOW_BACKEND : %s\n"
            "\n"
            "WINDOW_WIDTH : %d\n"
            "WINDOW_HEIGHT : %d\n"
            "\n"
            "#choose CENTER to centre window\n"
            "WINDOW_POSX : %d\n"
            "WINDOW_POSY : %d\n"
            "\n"
            "SWAPCHAIN_DEPTH : %d\n"
            "\n"
            "#choose between OFF/NORMAL/FAST/LAZY/CHOOSE_BEST\n"
            "VSYNC_MODE : %s\n"
            "\n"
            "\n"
            "#in megabytes\n"
            "FRAME_ALLOC_SIZE : %d\n"
            "HOST_ALLOC_SIZE : %d\n"
            "GPU_ALLOC_SIZE : %d\n"
            "\n"
            "#in tiles (a tile is 128 x 128 pixels large)\n"
            "VT_PHYS_WIDTH : %d\n"
            "VT_PHYS_HEIGHT : %d\n"
            "\n"
            "# [DATA_FORMAT]_[CHANNEL_FORMAT]\n"
            "# DATA_FORMAT S16/S24/S32/F32/F64\n"
            "# CHANNEL_FORMAT STEREO/5.1/7.1\n"
            "AUDIO_OUTPUT : %s\n"
            "\n"
            "#cboose between 44100/48000\n"
            "AUDIO_OUTPUT_FREQUENCY : %d\n"
            "\n"
            "PLAYBUFFER_SIZE_MS : %d\n"
            "\n"
            "# 0 to 1\n"
            "MASTER_VOLUME_FACTOR : %f\n"
            "\n"
            "# choose ALL to launch all\n"
            "# this exlcudes the main thread\n"
            "LAUNCH_THREADS : %s\n"
            "\n"
            "GPU_DEVICE : %s\n"
            "\n",
            
            backend_string,
            settings->window_width,
            settings->window_height,
            settings->window_x,
            settings->window_y,
            settings->swapchain_depth,
            vsync_string,
            settings->frame_alloc_size,
            settings->host_alloc_size,
            settings->gpu_alloc_size,
            settings->vt_width,
            settings->vt_height,
            audioformat_string,
            settings->audio_frequency,
            settings->playbuffer_size_ms,
            settings->master_volume,
            threads_string,
            gpudevice_string
            );
    
}

void WriteSettingsValue(Settings* settings,u64 name_hash,s8* value_string){
    
    switch(name_hash){
        
        case PHashString("WINDOW_BACKEND"):{
            settings->backend = WindowBackendStringToFlag(value_string);
        }break;
        
        case PHashString("WINDOW_WIDTH"):{
            
            settings->window_width = atoi(value_string);
            
            if(!settings->window_width){
                settings->window_width = 1280;
            }
            
        }break;
        
        case PHashString("WINDOW_HEIGHT"):{
            
            settings->window_height= atoi(value_string);
            
            if(!settings->window_height){
                settings->window_height = 720;
            }
            
        }break;
        
        case PHashString("WINDOW_POSX"):{
            settings->window_x = atoi(value_string);
        }break;
        
        case PHashString("WINDOW_POSY"):{
            settings->window_y = atoi(value_string);
        }break;
        
        case PHashString("SWAPCHAIN_DEPTH"):{
            
            settings->swapchain_depth = atoi(value_string);
            
            if(!settings->swapchain_depth){
                settings->swapchain_depth = 3;
            }
            
        }break;
        
        case PHashString("VSYNC_MODE"):{
            settings->vsync_mode = VsyncStringToFlag(value_string);
        }break;
        
        case PHashString("FRAME_ALLOC_SIZE"):{
            
            settings->frame_alloc_size = atoi(value_string);
            
            if(!settings->frame_alloc_size){
                settings->frame_alloc_size = 32;
            }
            
        }break;
        
        case PHashString("HOST_ALLOC_SIZE"):{
            
            settings->host_alloc_size = atoi(value_string);
            
            if(!settings->host_alloc_size){
                settings->host_alloc_size = 1024;
            }
            
        }break;
        
        case PHashString("GPU_ALLOC_SIZE"):{
            
            settings->gpu_alloc_size = atoi(value_string);
            
            if(!settings->gpu_alloc_size){
                settings->gpu_alloc_size = 22;
            }
            
        }break;
        
        case PHashString("VT_PHYS_WIDTH"):{
            
            settings->vt_width = atoi(value_string);
            
            if(!settings->vt_width){
                settings->vt_width = 16384;
            }
            
        }break;
        
        case PHashString("VT_PHYS_HEIGHT"):{
            
            settings->vt_height = atoi(value_string);
            
            if(!settings->vt_height){
                settings->vt_height = 8192;
            }
            
        }break;
        
        case PHashString("AUDIO_OUTPUT"):{
            
            AudioFormatStringToFlag(value_string,&settings->audio_format,&settings->audio_channels);
        }break;
        
        
        case PHashString("AUDIO_OUTPUT_FREQUENCY"):{
            
            settings->audio_frequency = atof(value_string);
            
            logic is_valid = settings->audio_frequency == 48000 || settings->audio_frequency == 44100;
            
            if(!is_valid){
                settings->audio_frequency = 48000;
            }
            
        }break;
        
        case PHashString("PLAYBUFFER_SIZE_MS"):{
            
            settings->playbuffer_size_ms = atoi(value_string);
            
            if(!settings->playbuffer_size_ms){
                settings->playbuffer_size_ms = 24;
            }
            
        }break;
        
        case PHashString("MASTER_VOLUME_FACTOR"):{
            settings->master_volume = _clamp(atof(value_string),0.0f,1.0f);
        }break;
        
        case PHashString("LAUNCH_THREADS"):{
            settings->master_volume = LaunchThreadStringToNo(value_string);
        }break;
        
        case PHashString("GPU_DEVICE"):{
            
            settings->gpu_device_hash = PHashString(value_string);
            
            if(settings->gpu_device_hash == PHashString("DEFAULT")){
                settings->gpu_device_hash = (u32)-1;
            }
        }break;
    }
    
}

void DebugPrintSettings(Settings* settings){
    
    s8 buffer[2048] = {};
    
    GenerateSettingsString(settings,buffer);
    
    printf("%s\n",buffer);
}

Settings ParseSettings(){
    
    
    
    Settings settings;
    
    
    s8 default_settings_string[2048] = {};
    
    GenerateSettingsString(&settings,default_settings_string);
    
    
    auto file = FOpenFile("SETTINGS.txt",F_FLAG_CREATE | F_FLAG_READWRITE);
    
    auto size = FGetFileSize(file);
    
    if(!size){
        
        FWrite(file,(s8*)default_settings_string,strlen(default_settings_string));
    }
    
    else{
        
        ptrsize size = 0;
        auto buffer = FReadFileToBuffer(file,&size);
        
        ptrsize cur = 0;
        
        while(cur < size){
            
            PSkipWhiteSpace(buffer,&cur);
            
            s8 line_buffer[256] = {};
            u32 line_len = 0;
            
            PGetLine(line_buffer,buffer,&cur,&line_len);
            
            if(line_buffer[0] == '#'){
                continue;
            }
            
            if(line_len){
                
                s8 name_buffer[256] = {};
                u32 name_count = 0;
                s8 value_buffer[256] = {};
                u32 value_count = 0;
                
                logic is_value = false;
                
                for(ptrsize i = 0; i < line_len; i++){
                    
                    PSkipWhiteSpace(line_buffer,&i);
                    
                    auto c = line_buffer[i];
                    
                    if(c == ':'){
                        is_value = true;
                        continue;
                    }
                    
                    if(is_value){
                        value_buffer[value_count] = c;
                        value_count++;
                    }
                    
                    else{
                        name_buffer[name_count] = c;
                        name_count++;
                    }
                    
                }
                
                WriteSettingsValue(&settings,PHashString(name_buffer),value_buffer);
                
            }
        }
        
        unalloc(buffer);
    }
    
    FCloseFile(file);
    
    
    return settings;
}