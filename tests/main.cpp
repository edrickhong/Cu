//this is for running tests
#include "mode.h"
#include "ttype.h"
#include "../src/importer/main.h"
#include "mdf_models.h"
#include "pparse.h"k

#include "hash_array.h"

struct HashEntry{
    u64 hash;
    const s8* string_array[1024 * 8];
    u32 string_count = 0;
};

void AddHash(HashEntry* table,u32* c,u64 hash,const s8* string){
    
    auto count = *c;
    
    
    
    for(u32 i = 0; i < count; i++){
        
        auto k = &table[i];
        
        if(k->hash == hash){
            
            k->string_array[k->string_count] = string;
            k->string_count++;
            
            return;
        }
        
    }
    
    table[count].hash = hash;
    table[count].string_array[0] = string;
    table[count].string_count = 1;
    count++;
    
    *c = count;
    
}

logic PrintHashDiagnostics(HashEntry* table,u32 count){
    
    u32 collide_count = 0;
    
    for(u32 i = 0; i < count; i++){
        
        auto entry = &table[i];
        
        if(entry->string_count > 1){
            
            collide_count += entry->string_count;
            
            printf("HASH COLLISION(%llu): ",entry->hash);
            
            for(u32 j = 0; j < entry->string_count; j++){
                
                printf("%s ",entry->string_array[j]);
                
            }
            
            printf("\n");
            
        }
        
    }
    
    auto percent = ((f64)collide_count/(f64)(_arraycount(hash_array))) * 100.0f;
    
    
    printf("%d collisions out of %d hashes. (%f percent collision rate)\n",collide_count,(u32)_arraycount(hash_array),percent);
    
    return percent > 20.0;
}

logic HashTest(){
    
    printf("RUNNING PHashString Collision test\n");
    
    auto hash_table = (HashEntry*)alloc(sizeof(HashEntry) * (_arraycount(hash_array)));
    
    u32 hash_count = 0;
    
    for(u32 i = 0; i < _arraycount(hash_array); i++){
        
        auto hash = PHashString(hash_array[i]);
        
        AddHash(hash_table,&hash_count,hash,hash_array[i]);
    }
    
    return PrintHashDiagnostics(hash_table,hash_count);
    
}

struct MDFEntry{
    u32 file_hash;
    u8* data_array;
    u32 data_size;
};

_persist MDFEntry mdf_array[] = {
    {PHashString("goblin.dae"),&goblin_mdf[0],sizeof(goblin_mdf)},
    {PHashString("teapot.dae"),&teapot_mdf[0],sizeof(teapot_mdf)},
};

MDFEntry* GetMDFEntry(const s8* file){
    
    u32 hash = PHashString(file);
    
    for(u32 i = 0; i < _arraycount(mdf_array); i++){
        
        if(hash == mdf_array[i].file_hash){
            printf("FOUND %s\n",file);    
            return &mdf_array[i];
        }
        
    }
    
    printf("%s not found\n",file);
    
    _kill("Entry not found",1);
    
    return 0;
}


u32 MDFTests(){
    
    u32 warning_count = 0;
    
    printf("RUNNING MDF TEST\n");
    
    //NOTE: I believe the mismatch is caused by padding in the file
    
    const s8* test_files[] = {
        "goblin.dae",
        "teapot.dae",
    };
    
    for(u32 i = 0; i < _arraycount(test_files); i++){
        
        printf("FILE %s\n",test_files[i]);
        
        
        auto assimp = AssimpLoad(test_files[i]);
        
        u8* buffer;
        u32 buffer_size;
        
        CreateAssimpToMDF((void**)&buffer,&buffer_size,assimp);
        
        auto entry = GetMDFEntry(test_files[i]);
        
        
        
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
            fprintf(stderr,"WARNING: unacceptable error range match(%s)\n",test_files[i]);
        }
        
        unalloc(buffer); 
    }
    
    return warning_count;
}

int main(s32 argc,s8** argv){
    
    auto res = HashTest();
    
    res |= MDFTests();
    
    return res;
}
