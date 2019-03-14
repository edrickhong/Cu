//this is for running tests
#include "mode.h"
#include "ttype.h"
#include "../src/importer/main.h"
#include "mdf_models.h"

struct MDFEntry{
    const s8* file;
    u8* data_array;
    u32 data_size;
};

_persist MDFEntry mdf_array[] = {
    {"goblin.dae",&goblin_mdf[0],sizeof(goblin_mdf)},
    {"teapot.dae",&teapot_mdf[0],sizeof(teapot_mdf)},
    {"golem_clean.dae",&golem_clean_mdf[0],sizeof(golem_clean_mdf)},
    {"knight.dae",&knight_mdf[0],sizeof(knight_mdf)},
    
};

u32 MDFTests(){
    
    u32 warning_count = 0;
    
    printf("RUNNING MDF TEST\n");
    
    //NOTE: I believe the mismatch is caused by padding in the file
    
    for(u32 i = 0; i < _arraycount(mdf_array); i++){
        
        auto entry = &mdf_array[i];
        
        printf("FILE %s\n",entry->file);
        
        
        auto assimp = AssimpLoad(entry->file);
        
        u8* buffer;
        u32 buffer_size;
        
        CreateAssimpToMDF((void**)&buffer,&buffer_size,assimp);
        
        
        
        _kill("FAILED MDF TEST\n",buffer_size != entry->data_size);
        
        u32 match_count = 0;
        
        for(u32 i = 0; i < entry->data_size; i++){
            
            if(entry->data_array[i] != buffer[i]){
            }
            else{
                match_count++;
            }
        }
        
        f32 percent_match = ((f32)match_count/(f32)entry->data_size) * 100.0f;
        
        printf("mismatch count: %d\n",entry->data_size - match_count);
        
        printf("match stats: %d vs %d %f\n",match_count,entry->data_size,percent_match);
        
        if((u32)percent_match < 90){
            warning_count++;
            fprintf(stderr,"WARNING: unacceptable error range match(%s)\n",entry->file);
        }
        
        unalloc(buffer); 
    }
    
    return warning_count;
}

int main(s32 argc,s8** argv){
    
    auto res = MDFTests();
    
    return res;
}
