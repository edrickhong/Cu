#pragma once

#include "vvulkan.h"

struct VertexEntry{
    VkFormat format;
    u32 size;
};

struct DescEntry{
    VkDescriptorType type;
    u32 set;
    u32 bind;
    u32 total_count;//TODO: change this to total count
};


struct PushConstEntry{
    VkFormat format;
    
    
#ifdef _log_string
    
#if _log_string
    
    s8 string[128];
    
#endif
    
#endif
    
    
};


struct VertexLayout{
    VertexEntry entry_array[16];
    u16 entry_count = 0;
    u16 size;
};


struct DescLayout{
    DescEntry entry_array[16];
    u32 entry_count = 0;
};

struct PushConstLayout{
    PushConstEntry entry_array[16];
    u16 entry_count = 0;
    u16 size;
};