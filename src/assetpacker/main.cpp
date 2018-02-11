#include "stdio.h"
#include "stdlib.h"

#include "ttype.h"
#include "mode.h"
#include "main.h"
#include "ffileio.h"
#include "ccontainer.h"
#include "pparse.h"

#include "aallocator.h"

typedef s8 AssetFilePathEntry[256];

enum AssetType : u32{
  ASSET_AUDIO = 0,
    ASSET_TEXTURE = 1,
    ASSET_MODEL = 2,
    ASSET_SHADER = 3,
    ASSET_UNKNOWN,
    };

struct AssetTableEntry{
  AssetType type;
  AssetFilePathEntry file_location = {};
  u64 file_location_hash;
};

struct AssetMap{
  u64 filehash;
  u64 offset;
};

_declare_list(AssetTable,AssetTableEntry);

void InternalParseAssetList(const s8* assetlist,AssetTable* out_table){
  
  auto file = FOpenFile(assetlist,F_FLAG_READWRITE);
  ptrsize list_count;

  auto list = (AssetTableEntry*)FReadFileToBuffer(file,&list_count);
  
  list_count /= sizeof(AssetTableEntry);

  printf("list count %d\n",(u32)list_count);

  for(u32 i = 0; i < list_count; i++){
    auto entry = list[i];
    out_table->PushBack(entry);
  }

  FCloseFile(file);
  unalloc(list);
}

void InternalWriteAssetList(const s8* assetlist,AssetTable* out_table){

  printf("table count %d\n",(u32)out_table->count);

  qsort(out_table->container,out_table->count,
	sizeof(AssetTableEntry),
	[](const void * a, const void* b)->s32 {
          
          auto block_a = (AssetTableEntry*)a;
          auto block_b = (AssetTableEntry*)b;
          
          return block_a->type - block_b->type;
	});
  
  auto file = FOpenFile(assetlist,F_FLAG_WRITEONLY | F_FLAG_TRUNCATE);

  FWrite(file,out_table->container,out_table->count * sizeof(AssetTableEntry));
  
  FCloseFile(file);
}

void InternalAddAssetList(const s8* asset,AssetTable* out_table){

  for(u32 i = 0; i < out_table->count; i++){
    
    auto entry = &out_table->container[i];
    
    if(entry->file_location_hash == PHashString(asset)){
      printf("WARNING: %s already exists\n",asset);
      return;
    }
    
  }
  
  u32 len = strlen(asset);

  auto k = asset[len - 3] + asset[len - 2] + asset[len - 1];

  AssetTableEntry entry = {ASSET_UNKNOWN};

  memcpy(&entry.file_location,&asset[0],strlen(asset));
  entry.file_location_hash = PHashString(asset);

  if(k == ('m' + 'd' + 'f') ){
    entry.type = ASSET_MODEL;
  }

  else if(k == ('a' + 'd' + 'f') ){
    entry.type = ASSET_AUDIO;
  }

  else if(k == ('t' + 'd' + 'f') ){
    entry.type = ASSET_TEXTURE;
  }

  else if(k == ('s' + 'p' + 'x') ){
    entry.type = ASSET_SHADER;
  }

  else{
    _kill("unknown asset type\n",1);
  }

  out_table->PushBack(entry);
}

void InternalRemoveAssetList(const s8* asset,AssetTable* out_table){
  
  for(u32 i = 0; i < out_table->count; i++){
    
    auto entry = &out_table->container[i];
    
    if(entry->file_location_hash == PHashString(asset)){
      out_table->Remove(entry);
      return;
    }
    
  }

  printf("WARNING: %s doesn't exist\n",asset);
}

void PrintAssetList(AssetTable* table){

  u32 t = (u32)-1;
  
  for(u32 i = 0; i < table->count; i++){
    
    auto entry = &table->container[i];

    if(t != entry->type){
      printf("-----------------------\n");
      t = entry->type;
    }
    
    printf("%d %s %llu\n",i,entry->file_location,entry->file_location_hash);
    
  }
  
}

/*
  TODO: in general engine work, we should output to a separate file. this is mainly for windows
*/

void PrintHelp(){
  
  printf(
	 R"FOO(
Asset packer is a tool to keep manage assets. A list of all the asset files are kept in a file [assetlist] 
which specifies where the asset files are. When specified, this tool will bake these assets into the 
target executable or a separate file. If specified, a header will also be generated 
that contains a table of asset names to offset locations.

The expected syntax is:
assetpacker [assetlist] command args1 args2 ...

commands:
-list : prints the asset file list
-add : adds asset(s) to the asset list
-remove : removes asset(s) to the asset list
-bake : bakes asset data either into an executable or a file

The syntax for bake is a little different:

assetpacker [assetlist] -bake [executable] [optional separate file to bake in] [optional header file]

The executable field is mandatory because this packer will attempt to patch in information 
about the baked data executable itself. To allow patching, declare a global string with the value
"patchthisvalueinatassetpacktime" [patch string]. When patched, the first character in the string 
will be replaced by an '!' and the following 4 character bytes after will contain the offset into the 
baked file. The following bytes is the name of the file the asset data is baked into ([executable] or 
[optional separate file to bake in]). The name of the baked file must not exceed the capacity  of
the [patch string].

)FOO"
	 );
  
}

s32 main(s32 argc,s8** argv){

  if(argc < 3){
    printf("not enough arguments\n");
    PrintHelp();
    return -1;
  }

  AssetTable assetlist;
  assetlist.Init();

  InternalParseAssetList(argv[1],&assetlist);

  //add to asset list
  if(PHashString(argv[2]) == PHashString("-add")){
    
    for(s32 i = 3; i < argc; i++){
      
      InternalAddAssetList(argv[i],&assetlist);
      
    }

    InternalWriteAssetList(argv[1],&assetlist);
  }

  //remove asset list
  else if(PHashString(argv[2]) == PHashString("-remove")){
    
    for(s32 i = 3; i < argc; i++){
      InternalRemoveAssetList(argv[i],&assetlist);
    }
    
    InternalWriteAssetList(argv[1],&assetlist);
  }

  else if(PHashString(argv[2]) == PHashString("-list")){
    PrintAssetList(&assetlist);
  }

  else if(PHashString(argv[2]) == PHashString("-bake")){

    //TODO: we should check if file is an executable
    
    //executable data
    ptrsize exec_size;
    s8* exec_buffer;
  
    auto exec_string = argv[3];
    
    auto exec_file = FOpenFile(exec_string,F_FLAG_READWRITE);
    
    exec_buffer = FReadFileToBuffer(exec_file,&exec_size);

    printf("exec size %d\n",(u32)exec_size);

    auto offset = PFindStringInString("patchthisvalueinatassetpacktime",exec_buffer);

    _kill("couldn't find patch string",offset == (u32)-1);

    exec_buffer[offset] = '!';

    auto size_ptr = (u32*)&exec_buffer[offset + 1];

    *size_ptr = exec_size;

    FSeekFile(exec_file,0,F_METHOD_START);

    FWrite(exec_file,&exec_buffer[0],exec_size);

    auto poem = R"FOO(
I'm nobody! Who are you?
Are you nobody, too?
Then there's a pair of us -- don't tell!
They'd advertise -- you know!

How dreary to be somebody!
How public like a frog
To tell one's name the livelong day
To an admiring bog!
)FOO";

    u32 len = strlen(poem) + 1;

    FWrite(exec_file,&len,sizeof(len));

    FWrite(exec_file,(void*)&poem[0],len);

    FCloseFile(exec_file);
    unalloc(exec_buffer); 
  }

  else{
    printf("unrecognized command %s\n",argv[2]);
    PrintHelp();
  }

  return 0;
}
