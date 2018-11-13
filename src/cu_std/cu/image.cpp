#include "image.h"

void WriteBMP(void* data,u32 w,u32 h,const s8* outputfile){
    
    struct BMPFileHeader{
        u16 type;
        u32 size;
        u16 r1;
        u16 r2;
        u32 offset;
    } _packed;
    
    struct BMPImageHeader{
        u32 this_size;
        u32 width;
        u32 height;
        u16 plane;
        u16 bpp;
        u32 comp_type;
        u32 image_size;
        u32 pixelspermeter_x;
        u32 pixelspermeter_y;
        u32 colormap_entries;
        u32 sigcolors;
    } _packed;
    
    u32 tsize = sizeof(BMPFileHeader) + sizeof(BMPImageHeader) * w * h * 4;
    auto ptr = (s8*)alloc(tsize);
    
    auto header = (BMPFileHeader*)ptr;
    
    auto imageheader = (BMPImageHeader*)(ptr + sizeof(BMPFileHeader));
    
    auto imagedata = ptr + sizeof(BMPFileHeader) + sizeof(BMPImageHeader);
    
    header->type = 'B' | ('M' << 8);
    header->size = tsize;
    header->r1 = 0;
    header->r2 = 0;
    header->offset = sizeof(BMPFileHeader) + sizeof(BMPImageHeader);
    
    
    imageheader->this_size = sizeof(BMPImageHeader);
    imageheader->width = w;
    imageheader->height = h;
    imageheader->plane = 1;
    imageheader->bpp = 32;
    imageheader->comp_type = 0;
    imageheader->image_size = 0;
    imageheader->pixelspermeter_x = 0;
    imageheader->pixelspermeter_y = 0;
    imageheader->colormap_entries = 0;
    imageheader->sigcolors = 0;
    
    struct Pixel{
        u8 r;
        u8 g;
        u8 b;
        u8 a;
    };
    
    u32 at = 0;
    
    for(s32 y = (h - 1); y > -1; y--){
        
        for(u32 x = 0; x < w; x++){
            
            u32 i = (y * w) + x;
            
            auto dst_pix = (u32*)imagedata;
            auto src_pix = (u32*)data;
            
            auto pix = *((Pixel*)&src_pix[i]);
            
            
            dst_pix[at] = pix.r << 16 | pix.g << 8 | pix.b;
            
            at++;
        }
        
    }
    
    auto file = FOpenFile(outputfile,F_FLAG_WRITEONLY | F_FLAG_CREATE |
                          F_FLAG_TRUNCATE);
    
    FWrite(file,ptr,tsize);
    
    FCloseFile(file);
    
    unalloc(ptr);
}