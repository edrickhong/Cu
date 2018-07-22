#include "audio_util.h"

#define _max_s16 (1 << 15)

u32 Convert_SLE16_TO_F32(void* dst,void* src,u32 sample_count) {
    
    auto sample_array = (s16*)src;
    auto out_sample_array = (f32*)dst;
    
    for(u32 i = 0; i < sample_count; i++){
        out_sample_array[i] = (f32)(sample_array[i])/((f32)_max_s16);
    }
    
    return sizeof(f32);
}

u32 Convert_F32_TO_SLE16(void* dst,void* src,u32 sample_count) {
    
    auto sample_array = (f32*)src;
    auto out_sample_array = (s16*)dst;
    
    for(u32 i = 0; i < sample_count; i++){
        out_sample_array[i] = (s16)(sample_array[i] * ((f32)_max_s16));
    }
    
    return sizeof(s16);
}

u32 Convert_NONE_SLE16(void* dst,void* src,u32 sample_count) {
    memcpy(dst, src, sample_count * sizeof(s16));
    return sizeof(s16);
}

u32 Convert_NONE_F32(void* dst, void* src, u32 sample_count) {
    memcpy(dst, src, sample_count * sizeof(f32));
    return sizeof(f32);
} 