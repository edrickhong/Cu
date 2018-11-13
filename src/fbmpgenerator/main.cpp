#include "stdio.h"
#include "stdlib.h"


#include "mode.h"
#include "ttype.h"

#include "gui_draw.h"
#include "ffileio.h"
#include "pparse.h"


//TODO: rename this
#include "image.h"

void PrintHelp(){
    
    printf("Format: fbmp infile size outfile\n");
    
#ifdef _WIN32
    
    
    printf("Sys File Dir: C:\\Windows\\Fonts\\\n");
    
#else
    
    printf("Sys File Dir: /usr/share/fonts/\n");
    
#endif
}

s32 main(s32 argc,s8** argv){
    
    
    
    // fbmp -bmp *.fbmp
    if(PHashString(argv[1]) == PHashString("-bmp")){
        
        auto len = strlen(argv[2]);
        
        
        if(PHashString(&argv[2][len - 4]) == PHashString("fbmp")){
            
            
            
            auto file = FOpenFile(argv[2],F_FLAG_READONLY);
            
            u32 width,height;
            
            FRead(file,(s8*)&width,sizeof(width));
            FRead(file,(s8*)&height,sizeof(height));
            
            auto image_data = (s8*)alloc(sizeof(u32) * width * height);
            
            FRead(file,image_data,sizeof(u32) * width * height);
            
            
            {
                argv[2][len - 4] = 'b';
                argv[2][len - 3] = 'm';
                argv[2][len - 2] = 'p';
                argv[2][len - 1] = 0;
            }
            
            
            WriteBMP(image_data,width,height,argv[2]);
            
            FCloseFile(file);
            unalloc(image_data);
            
            
        }
        
        
        return 0;
    }
    
    
    if(argc != 4){
        PrintHelp();
        return 0;
    }
    
    if(!PIsStringFloat(argv[2])){
        
        PrintHelp();
        return 0;
    }
    
    auto infile = argv[1];
    f32 size = atof(argv[2]);
    auto outfile = argv[3];
    
    GUIGenFontFile(infile,outfile,size);
    
    return 0;
}