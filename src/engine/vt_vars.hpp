

//Virtual Texture table

#define _tpage_side 128

_global u32 phys_w = 0;
_global u32 phys_h = 0;


#define _fetch_dim_scale_w 8
#define _fetch_dim_scale_h 8

_global VTextureContext global_texturecache = {};



_global u16* vt_freepages_array = 0;
_global u32 vt_freepages_count = 0;

_global TextureAssetHandle texturehandle_array[_texturehandle_max] = {};
_global u32 texturehandle_count = 0;


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

struct EvictTextureList{
    
    TextureAssetHandle* array[_texturehandle_max];
    u32 count = 0;
};

//MARK:nvidia is ok with linear if it is not a storage image
_global VTReadbackImageContext vt_readbackbuffer = {}; //device writes to this
_global VImageMemoryContext vt_targetreadbackbuffer = {}; // we copy to this for reading 
_global VTReadbackPixelFormat* vt_readbackpixels = 0;
_global VTReadbackPixelFormat* threadtexturefetch_array = 0;


_global VkCommandPool fetch_pool[2] = {};
_global VkCommandBuffer fetchcmdbuffer_array[2] = {};
_global u32 fetchcmdbuffer_count = 0;




_global TextureAssetHandle* evict_texture_handle_array[_texturehandle_max * 2] = {};

_global u32 evict_texture_handle_count = 0;