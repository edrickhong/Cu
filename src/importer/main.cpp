
#include "main.h"

/*
FIXME:

NOTE: These assets are used internally only.
Problem files:
LOW_barbarian_rig_051.dae too many bones 65
new_thin_zombie.dae file cannot load
Spider.dae  file cannot load
*/


void TestAnim(){

	//TODO: once mdf is written, load the mdf file and test JUST the animations.
	//Look thru the old archives, load the tree as DFS ready
	//load the asset in the new way BSF read
	//run animation and compare results w error
	
	s8* files[] = {(s8*)"../rsrc/models/goblin.dae"};
	Import(files,_arraycount(files));

	u16 vertcomp = 0;
	u32 vertindex_size = 0, animbone_size = 0;

	InternalLoadMDF("../rsrc/models/goblin.mdf",&vertcomp,0,0,&vertindex_size,&animbone_size);

	void* vertind = alloc(vertindex_size);
	void* animbone = alloc(animbone_size);

	InternalLoadMDF("../rsrc/models/goblin.mdf",&vertcomp,vertind,animbone,&vertindex_size,&animbone_size);

	exit(0);

}

s32 main(s32 argc,s8** argv){

	TestAnim();
    
    if(argc == 1){
        printf("please provide files\n");  
    }
    
    Import(&argv[1],(u32)(argc - 1));

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
