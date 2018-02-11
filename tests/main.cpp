//this is for running tests
#include "mode.h"
#include "ttype.h"
#include "../src/importer/main.h"
#include "mdf_models.h"
#include "pparse.h"

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
    "shark.dae",
    "teapot.dae",
    "monkey.obj",
    "skel.dae"
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

  auto res = MDFTests();

  return res;
}
