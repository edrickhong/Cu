#pragma once 

#include "mode.h"
#include "ttype.h"

u32 Convert_SLE16_TO_F32(void* dst,void* src,u32 sample_count);

u32 Convert_F32_TO_SLE16(void* dst,void* src,u32 sample_count);

u32 Convert_NONE_SLE16(void* dst,void* src,u32 sample_count);

u32 Convert_NONE_F32(void* dst, void* src, u32 sample_count);