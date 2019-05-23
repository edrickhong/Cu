#pragma once

#include "mode.h" // include first
#include "ttype.h"
#include "ffileio.h"
#include "ccontainer.h"
#include "aallocator.h"
#include "pparse.h"

enum AssetType : u32 
{
	ASSET_AUDIO = 0,
	ASSET_TEXTURE = 1,
	ASSET_MODEL = 2,
	ASSET_SHADER = 3,
	ASSET_UNKNOWN,
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
        
    }
    
    if(count){
        *count = c;
    }
    
    FCloseFile(file);
}
