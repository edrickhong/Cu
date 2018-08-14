
#include "main.h"

/*
FIXME:

NOTE: These assets are used internally only.
Problem files:
LOW_barbarian_rig_051.dae too many bones 65
new_thin_zombie.dae file cannot load
Spider.dae  file cannot load

*/

s32 main(s32 argc,s8** argv){
    
    if(argc == 1){
        printf("please provide files\n");  
    }
    
    printf("Only mdf and adf(WAV) files are supported now\n");
    
    AnimationBlendType blendtype = BLEND_LINEAR;
    
    for(s32 i = 1; i < argc; i++){
        
        s8* string = argv[i];
        
        u32 len = strlen(string);
        
        auto a = string[len - 3];
        auto b = string[len - 2];
        auto c = string[len - 1];
        
        
        // if(string[0] == '-'){
        //   blendtype = BLEND_DQ;
        //   continue;
        // }
        
        if(isAudio(a,b,c)){
            
            auto writepath = (s8*)alloc(len + 1);
            
            memcpy(writepath,string,len + 1);
            
            writepath[len - 3] = 'a';
            writepath[len - 2] = 'd';
            writepath[len - 1] = 'f';
            
            WavWriteADF(string,writepath);
            
            unalloc(writepath);
        }
        
        if(isModel(a,b,c)){
            
#if MATRIX_ROW_MAJOR
            
            printf("operating in matrix ROW major!\n");
            
#else
            
            printf("operating in matrix COLUMN major!\n");
            
#endif
            
            auto assimp = AssimpLoad(string);
            
            string[len - 3] = 'm';
            string[len - 2] = 'd';
            string[len - 1] = 'f';
            
            AssimpWriteMDF(assimp,string,blendtype);
            
        }
        
        if(isImage(a,b,c)){
            
            s8 buffer[1024] = {};
            
            memcpy(buffer,string,len);
            
            buffer[len - 3] = 't';
            buffer[len - 2] = 'd';
            buffer[len - 1] = 'f';
            
            //Format_RGBA
            CreateTextureAssetTDF(string,buffer,Format_BC1,true,true);
        }
        
    }
    
    
    
    return 0;
}


// enum Format
//   {
//     // No block-compression (linear).
//     Format_RGB,
//     Format_RGBA = Format_RGB,

//     // DX9 formats.
//     Format_DXT1,
//     Format_DXT1a,   // DXT1 with binary alpha.
//     Format_DXT3,
//     Format_DXT5,
//     Format_DXT5n,   // Compressed HILO: R=1, G=y, B=0, A=x

//     // DX10 formats.
//     Format_BC1 = Format_DXT1,
//     Format_BC1a = Format_DXT1a,
//     Format_BC2 = Format_DXT3,
//     Format_BC3 = Format_DXT5,
//     Format_BC3n = Format_DXT5n,
//     Format_BC4,     // ATI1
//     Format_BC5,     // 3DC, ATI2

//     Format_DXT1n,   // Not supported.
//     Format_CTX1,    // Not supported.

//     Format_BC6,
//     Format_BC7,

//     Format_BC3_RGBM,    // 

//     Format_Count
//   };
