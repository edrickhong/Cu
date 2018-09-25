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
    u32 vt_width = 16384;
    u32 vt_height = 8192;
    u32 audio_format = A_FORMAT_S16LE;
    u32 audio_frequency = 48000;
    u32 playbuffer_size_ms = 24;
    f32 master_volume = 1.0f;
    u32 launch_threads = (u32)-1;
    u32 gpu_device_hash = (u32)-1;
    
};

void WindowBackendFlagToString(u32 flag,s8* dst_string){
    
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



void AudioFormatFlagToString(u32 flag,s8* dst_string){
    
    switch(flag){
        
        case A_FORMAT_S16LE:{
            auto string = "S16_STEREO";
            memcpy(dst_string,string,strlen(string));
        }break;
        
        default:{
            
            _kill("not supported right now\n",1);
            
        }break;
        
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


u32 StringToWindowBackendFlag(s8* string){
    
    auto hash = PHashString(string);
    
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
    
    return 0;
}

u32 StringToVsyncFlag(s8* string){
    
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

u32 AudioFormatStringToFlag(s8* string){
    
    auto hash = PHashString(string);
    
    switch(hash){
        
        case PHashString("S16_STEREO"):{
            return A_FORMAT_S16LE;
        }break;
    }
    
    return A_FORMAT_S16LE;
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
    
    AudioFormatFlagToString(settings->audio_format,audioformat_string);
    
    LaunchThreadsNoToString(settings->launch_threads,threads_string);
    
    sprintf(buffer,
            R"FOO(
            
            #choose between DIRECT/XLIB/WAYLAND/CHOOSE_BEST
            WINDOW_BACKEND : %s
            
            WINDOW_WIDTH : %d
            WINDOW_HEIGHT : %d
            
            #choose CENTER to centre window
            WINDOW_POSX : %d
            WINDOW_POSY : %d
            
            SWAPCHAIN_DEPTH : %d
            
            #choose between OFF/NORMAL/FAST/LAZY/CHOOSE_BEST
            VSYNC_MODE : %s
            
            
            #in megabytes
            FRAME_ALLOC_SIZE : %d
            HOST_ALLOC_SIZE : %d
            GPU_ALLOC_SIZE : %d
            
            #in tiles (a tile is 128 x 128 pixels large)
            VT_PHYS_WIDTH : %d
            VT_HEIGHT_WIDTH : %d
            
            # [DATA_FORMAT]_[CHANNEL_FORMAT]
            # DATA_FORMAT S16/S24/S32/F32/F64
            # CHANNEL_FORMAT STEREO/5.1/7.1
            AUDIO_OUTPUT : %s
            
            #cboose between 44100/48000
            AUDIO_OUTPUT_FREQUENCY : %d
            
            PLAYBUFFER_SIZE_MS : %d
            
            # 0 to 1
            MASTER_VOLUME_FACTOR : %f
            
            # choose ALL to launch all
            # this exlcudes the main thread
            LAUNCH_THREADS : %s
            
            GPU_DEVICE : %s
            
            )FOO",backend_string,
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
                
                
                auto hash = PHashString(name_buffer);
                
                //TODO: match hash to values
                
                
            }
        }
        
        unalloc(buffer);
    }
    
    FCloseFile(file);
    
    
    
    return settings;
}