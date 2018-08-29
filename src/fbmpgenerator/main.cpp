#include "stdio.h"
#include "stdlib.h"


#include "mode.h"
#include "ttype.h"

#include "gui_draw.h"
#include "ffileio.h"
#include "pparse.h"

void PrintHelp(){
    
    printf("Format: fbmp infile size outfile\n");
    
#ifdef _WIN32
    
    
    printf("Sys File Dir: C:\\Windows\\Fonts\\\n");
    
#else
    
    printf("Sys File Dir: /usr/share/fonts/\n");
    
#endif
}

s32 main(s32 argc,s8** argv){
    
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