#include "main.h"
#include "stdio.h"
#include "stdlib.h"
#include "ttype.h"
#include "mode.h"
#include "main.h"
#include "ffileio.h"
#include "ccontainer.h"
#include "aallocator.h"
#include "pparse.h"

/*
NOTES:
press ctrl+f5 to run console
Program creates blank resource file if you try to add a nonexistent one
resource files and shizz are taken from directory Jupiter\Make\

************
Flow:
assetlist file

Globals:
AssetTable
Buffer to hold the data


on startup:
filetolist

list
printlist

add
listtofile

remove
listtofile






PROGRESS:

AddToList (T)(note: assign FileNode)
RemoveFromList (T)
Parse (T)
WriteTableToFile (T)
FileToTable (T)
SortByType (T)
BakeToExecutable (P)


TODO:
VerifyData[by comparing each byte]


Additional:
Additional Console Commands
Validate


P=Inprogress
D=written but not tested
T=tested and working but not optimised
O=optimised,working


?:
Can I run functions in functions? eg in Sort(), Print().
Validating: What makes the data valid? checking the whole table for all attributes?
Checking for duplicates: qsort, std::set, or comparing each element with nested for loop?
*************
*/

enum AssetType{
	ASSET_AUDIO = 0,
    ASSET_TEXTURE = 1,
    ASSET_MODEL = 2,
    ASSET_SHADER = 3,
    ASSET_UNKNOWN,
    };



struct AssetTableEntry {
	AssetType type;
	s8 file_location[256] = {};
	u64 file_location_hash = 0;
	FileNode file_node;
	u32 size;
	u32 offset;
};

//AssetTable Declaration
_declare_list(AssetTable,AssetTableEntry);



// run on startup
// read resource file and store it as an AssetTable, run on startup if resource file exists
void FileToTable(const s8* asset_list, AssetTable* out_table) {
	//open asset list file and store as asset table
	auto file = FOpenFile(asset_list, F_FLAG_READWRITE); //FileHandle
	ptrsize list_count;
	auto list = (AssetTableEntry*)FReadFileToBuffer(file, &list_count);
	list_count /= sizeof(AssetTableEntry);
	//store the list count

	printf("initial list count %d\n", (u32)list_count);

	//populate the table
	for (u32 i = 0; i < list_count; i++) {
		auto entry = list[i];
		out_table->PushBack(entry);
	}

	FCloseFile(file);
	unalloc(list);
}

void AddAssetToList(const s8* asset, AssetTable* out_table) {
	//check for duplicates 
	for (u32 i = 0; i < out_table->count; i++) {
		auto entry = &out_table->container[i]; 
		if (PHashString(asset) == entry->file_location_hash) {
			printf("Warning! %s file exists in the table! Resouce %s was not added.\n",asset, asset);
			return;
		//file already exists
		}
	}

	AssetTableEntry entry = { ASSET_UNKNOWN };

	auto file = FOpenFile(asset, F_FLAG_READWRITE);
	auto file_size = FGetFileSize(file);
	entry.size = file_size;

	//calculate offset
	if (out_table->count > 0) {
		entry.offset = out_table->container[out_table->count-1].offset + out_table->container[out_table->count - 1].size;
	}
	else {
		entry.offset = 0;
	}

	u32 len = strlen(asset);


	//label file type
	auto file_extension = asset[len - 3] + asset[len - 2] + asset[len - 1];
	if (file_extension == ('m' + 'd' + 'f')) {
		entry.type = ASSET_MODEL;
	}

	else if (file_extension == ('a' + 'd' + 'f')) {
		entry.type = ASSET_AUDIO;
	}

	else if (file_extension == ('t' + 'd' + 'f')) {
		entry.type = ASSET_TEXTURE;
	}

	else if (file_extension == ('s' + 'p' + 'x')) {
		entry.type = ASSET_SHADER;
	}

	else {
		printf("unknown asset type. exiting.");
		_kill("unknown asset type\n", 1);
	}
	// location
	memcpy(&entry.file_location, &asset[0], strlen(asset));
	// hash
	entry.file_location_hash = PHashString(asset);
	//assign file node?
	entry.file_node.filehandle = 0;
	

	out_table->PushBack(entry);
}

void RemoveAssetFromList(const s8* asset, AssetTable* out_table) {
	for (u32 i = 0; i < out_table->count; i++) {

		auto entry = &out_table->container[i];
		//printf("entryhash:%llu, toremove:%s, r3.spx:%llu\n", entry->file_location_hash, asset, PHashString("r3.spx"));
		if (entry->file_location_hash == PHashString(asset)) {
			out_table->Remove(entry);
			return;
		}

	}
	printf("WARNING: %s doesn't exist\n", asset);
}


	
void SortListByType(AssetTable* out_table) {
	
	printf("Table Count: %d\n\n", (u32)out_table->count);

	//sort by type
	qsort(out_table->container, out_table->count,
		sizeof(AssetTableEntry),
		[](const void * a, const void* b)->s32{

			auto block_a = (AssetTableEntry*)a;
			auto block_b = (AssetTableEntry*)b;

			return block_a->type - block_b->type;
		}
	);

	//reset the offsets
	u32 offset = (u32)0;
	for (u32 i = 0; i < out_table->count; i++) {
		auto entry = &out_table->container[i];

		entry->offset = offset;
		offset += entry->size;
	}

	return;
}


void WriteToFile(const s8* assetlist, AssetTable* out_table) {
	auto file = FOpenFile(assetlist, F_FLAG_WRITEONLY | F_FLAG_TRUNCATE);

	FWrite(file, out_table->container, out_table->count * sizeof(AssetTableEntry));

	FCloseFile(file);
}

//check for duplicates, also verify the data is loaded correctly by visiting each pointer
//or just scrap the old data and populate new one
void ValidateList(const s8* assetlist, AssetTable* out_table) {
	printf("table count %d\n", (u32)out_table->count);

}


///Prints Asset List to Console
void PrintAssetList(AssetTable* table){

  u32 t = (u32)-1;
  printf("\n\nTABLESTART\n\n");
  for(u32 i = 0; i < table->count; i++){
    
    auto entry = &table->container[i];

    if(t != entry->type){
      printf("-----------------------\n");
      t = entry->type;
    }
    
    printf("SN%d TYPE%d SIZE%d OFFSET%d %s %llu\n",i,entry->type,entry->size,entry->offset,entry->file_location,entry->file_location_hash);
    
  }
  printf("\n\nTABLEEND\n\n");
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

void BakeExecutable(AssetTable* asset_table, FileHandle exec_file) {
	u32 t = (u32)-1;
	printf("\n\nBAKING DATA\n\n");
	for (u32 i = 0; i < asset_table->count; i++) {
		auto entry = asset_table->container[i];
		printf("WRITING %d of %s\n\n", entry.size, entry.file_location);
		auto entry_file = FOpenFile(entry.file_location, F_FLAG_READWRITE);
		ptrsize entry_size;
		auto entry_data = FReadFileToBuffer(entry_file, &entry_size);
		printf("WROTE %d of %s\n\n", entry_size, entry.file_location);

		if (t != entry.type) {
			printf("----BAKING OF %s TYPE %d-------------------\n", entry.file_location, entry.type);
			t = entry.type;
		}

		FWrite(exec_file, &entry_data[0], entry.size);

	}
	printf("\n\nDONE BAKING DATA\n\n");


}

s32 main(s32 argc,s8** argv){


	for (s32 i = 0; i < argc; i++) {
		printf("%s\n", argv[i]);

	}

	//printf("%llu,%llu,%llu,%llu", PHashString("-list"), PHashString("-sort"), PHashString("-sirt"), PHashString("-lort"));


  if(argc < 3){
    printf("not enough arguments\n");
    PrintHelp();
	int b;
	scanf("%d",&b);
    return -1;
  }

  AssetTable asset_table;
  asset_table.Init();
  s8* asset_file = argv[1];
  FileToTable(asset_file,&asset_table);

  //Read the -commands
  //add to asset list
  if(PHashString(argv[2]) == PHashString("-add")){
    
    for(s32 i = 3; i < argc; i++){
      
      AddAssetToList(argv[i],&asset_table);
      
    }

    WriteToFile(argv[1],&asset_table);
  }

  //remove asset list
  else if(PHashString(argv[2]) == PHashString("-remove")){

	 if (PHashString(argv[3]) == PHashString("-all")) {
		  for (; asset_table.count > 0;) {
			  RemoveAssetFromList(asset_table.container[0].file_location, &asset_table);
			  printf("Removed %s\n", asset_table.container[0].file_location);
			  //PrintAssetList(&asset_table);
		  }
	 }
	 else {
		 for (s32 i = 3; i < argc; i++) {
			 RemoveAssetFromList(argv[i], &asset_table);
		 }
	 }

    WriteToFile(argv[1],&asset_table);
  }

  else if(PHashString(argv[2]) == PHashString("-list")){
	  PrintAssetList(&asset_table);
  }
  else if(PHashString(argv[2]) == PHashString("-arrange")) {
	  PrintAssetList(&asset_table);
	  SortListByType(&asset_table);
	  PrintAssetList(&asset_table);
	  WriteToFile(argv[1], &asset_table);
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

	auto offset = PFindStringInString("patchthisvalueinatassetpacktime", exec_buffer);
    _kill("couldn't find patch string",offset == (u32)-1);
    exec_buffer[offset] = '!';

    auto size_ptr = (u32*)&exec_buffer[offset + 1];

    *size_ptr = exec_size;

    FSeekFile(exec_file,0,F_METHOD_START);

    FWrite(exec_file,&exec_buffer[0],exec_size);


	BakeExecutable(&asset_table, exec_file);
	/*
	u32 t = (u32)-1;
	printf("\n\nBAKING DATA\n\n");
	for (u32 i = 0; i < asset_table.count; i++) {
		auto entry = asset_table.container[i];
		printf("WRITING %d of %s\n\n", entry.size, entry.file_location);
		auto entry_file = FOpenFile(entry.file_location, F_FLAG_READWRITE);
		ptrsize entry_size;
		auto entry_data = FReadFileToBuffer(entry_file, &entry_size);
		printf("WROTE %d of %s\n\n", entry_size, entry.file_location);

		if (t != entry.type) {
			printf("----BAKING OF %s TYPE %d-------------------\n", entry.file_location, entry.type);
			t = entry.type;
		}

		FWrite(exec_file, &entry_data[0], entry.size);

	}
	printf("\n\nDONE BAKING DATA\n\n");

	*/


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
