

//Virtual Texture table

#define _tpage_side 128

_persist u32 phys_w = 0;
_persist u32 phys_h = 0;


#define _fetch_dim_scale_w 8
#define _fetch_dim_scale_h 8

_persist VTextureContext global_texturecache = {};



_persist u16* vt_freepages_array = 0;
_persist u32 vt_freepages_count = 0;

_persist TextureAssetHandle texturehandle_array[_texturehandle_max] = {};
_persist u32 texturehandle_count = 0;


union VTReadbackPixelFormat{
    
    struct{
        u8 texture_id;
        u8 mip;
        u8 x;
        u8 y;  
    };
    
    u32 value;
    
};

struct VTReadbackImageContext : VImageContext{
    u16 w;
    u16 h;
};

struct FreepageList{
    
    struct PixelPageFormat{
        u8 x;
        u8 y;
        u8 validflag;
        u8 pad;
    };
    
    union{
        u32 array[_fetch_list_count];
        PixelPageFormat format_array[_fetch_list_count];
    };
    u32 count;
};

//MARK:nvidia is ok with linear if it is not a storage image
_persist VTReadbackImageContext vt_readbackbuffer = {}; //device writes to this
_persist VImageMemoryContext vt_targetreadbackbuffer = {}; // we copy to this for reading 
_persist VTReadbackPixelFormat* vt_readbackpixels = 0;
_persist VTReadbackPixelFormat* threadtexturefetch_array = 0;


_persist VkCommandPool fetch_pool[2];
_persist VkCommandBuffer fetchcmdbuffer_array[2];
_persist u32 fetchcmdbuffer_count = 0;




_persist TextureAssetHandle* evict_texture_handle_array[_texturehandle_max * 2] = {};

_persist u32 evict_texture_handle_count = 0;