#pragma once

#include "mode.h" // include first
#include "ttype.h"
#include "ffileio.h"
#include "ccontainer.h"
#include "aallocator.h"
#include "pparse.h"

enum AssetType : u32 
{
	
    //these are raw asset data
    ASSET_AUDIO = 0,
	ASSET_TEXTURE = 1,
	ASSET_MODEL = 2,
	ASSET_SHADER = 3,
    
    //these are meta assets that reference raw asset data
    ASSET_MAT = 255,
    
	ASSET_UNKNOWN = (u32)-1,
};

struct AssetTableEntry 
{
	AssetType type;
	s8 file_location[256] = {};
	u64 file_location_hash = 0;
    
    //this is the filenode of the original file
	union 
	{
		FileNode original_file_node;
		s8 opaque_file_node_padding[32];
	};
    s8 original_file_location[256] = {};
    
	u32 size;
	u32 offset;
};

void LoadAssetFile(const s8* filepath,AssetTableEntry* array,u32* count){
    
    auto file = FOpenFile(filepath,F_FLAG_READONLY);
    
    u32 c = FGetFileSize(file)/sizeof(AssetTableEntry);
    
    if(array){
        
        for(u32 i = 0; i < c; i++){
            auto entry = &array[i];
            FRead(file,entry,sizeof(AssetTableEntry));
        }
        
        qsort(array,c,sizeof(AssetTableEntry),
              [](const void * a, const void* b)->s32 {
              
              auto ast_a = (AssetTableEntry*)a;
              auto ast_b = (AssetTableEntry*)b;
              
              return ast_a->type - ast_b->type;
              });
        
    }
    
    if(count){
        *count = c;
    }
    
    FCloseFile(file);
}
