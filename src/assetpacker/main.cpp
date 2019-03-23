#include "mode.h" // include first
#include "ttype.h"
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

Pack File:
TABLE
DATA
When adding additional data, append to the end.
When removing data, recalculate all the data behind the removed data and shift it forward, recalculating offsets.
Arrange data at bake time (by type, etc)

TODO:
Optimisation:
Reorder table to back end
Create a separate table for invalidating assets
separate 2 files : bake to executable and storing data in asset file
store data in asset file
executable: order asset by type

API:
load up the table
function: pull individual asset
return buffer with data for half an item
load whole item
RetrievePartialData
RetrieveFullData

Function:
Write file data to pack file when adding assets
cmp function to validate data by checking file contents with data stored in pack file

don't use new
dont use malloc
alloc and unalloc and ralloc(malloc ualloc realloc sub)
strcpy or memcpy
asset_table.h stores the data
include 'asset_table.h'

*/

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

	union 
	{
		FileNode file_node;
		s8 opaque_file_node_padding[32];
	};

	u32 size;
	u32 offset;
};

//AssetTable Declaration
_declare_list(AssetTable, AssetTableEntry);
bool HashAndCompareString(const s8* lhs, const s8* rhs);

/// Read resource file and store it as an AssetTable
void LoadFileToMemory(FileHandle file, AssetTable* outTable) 
{

	if (FGetFileSize(file) <= 0)
	{
		return;
	}

	ptrsize list_count;

	auto list = (AssetTableEntry*)FReadFileToBuffer(file, &list_count);
	
	//store the list count
	list_count /= sizeof(AssetTableEntry);

	//printf("initial list count %d\n", (u32)list_count);

	//populate the table
	for (u32 i = 0; i < list_count; i++)
	{
		auto entry = list[i];
		outTable->PushBack(entry);
	}

	unalloc(list);
}

///Initializes the pack
void PackInit(FileHandle fh, AssetTable* outTable) 
{
	u32 last_offset = 0;

	//find the offset
	for (u32 i = 0; i < outTable->count; i++) 
	{
		auto entry = &outTable->container[i];
		if (entry->offset >= last_offset) 
		{
			last_offset = entry->offset + entry->size;
		}
	}

	//strcat((s8*)last_offset,"\n");
	//strcat(dt, "\n");
	s8 o = 'o';
	
	FWrite(fh, &last_offset, sizeof(last_offset));
	FWrite(fh, &o, sizeof("o"));

	return;
}

//
//void PackWriteData(const s8* packfile, AssetTable* outTable, const s8* asset) {
//
//
//}

///Adds asset from file path to table
void AddAssetToTable(const s8* assetFileName, AssetTable* outTable) 
{
	//always adds asset behind the last offset
	u32 last_offset = 0;
	//check for duplicates and also store entry with largest offset
	for (u32 i = 0; i < outTable->count; i++) 
	{
		auto entry = &outTable->container[i];
		if (entry->offset >= last_offset) 
		{
			last_offset = entry->offset + entry->size;
		}
		if (PHashString(assetFileName) == entry->file_location_hash) {
			printf("Warning! %s file exists in the table! Resource %s was not added.\n", assetFileName, assetFileName);
			return;
			//file already exists
		}
	}

	AssetTableEntry entry = {ASSET_UNKNOWN};

	auto file = FOpenFile(assetFileName, F_FLAG_READWRITE);

	_kill("File not found!", file == F_FILE_INVALID);
	
	auto file_size = FGetFileSize(file);
	FCloseFile(file);
	entry.size = file_size;

	//calculate offset
	if (outTable->count > 0) 
	{
		//search for entry with largest offset(last item for baking) and put this behind it
		entry.offset = last_offset;
	}
	else {
		entry.offset = 0;
	}

	u32 len = strlen(assetFileName);


	//label file type
	auto file_extension = assetFileName[len - 3] + assetFileName[len - 2] + assetFileName[len - 1];
	if (file_extension == ('m' + 'd' + 'f')) 
	{
		entry.type = ASSET_MODEL;
	}
	else if (file_extension == ('a' + 'd' + 'f')) 
	{
		entry.type = ASSET_AUDIO;
	}
	else if (file_extension == ('t' + 'd' + 'f'))
	{
		entry.type = ASSET_TEXTURE;
	}
	else if (file_extension == ('s' + 'p' + 'x')) 
	{
		entry.type = ASSET_SHADER;
	}
	else 
	{
		printf("unknown asset type. exiting.");
		_kill("unknown asset type\n", 1);
	}

	// location
	memcpy(&entry.file_location, &assetFileName[0], strlen(assetFileName));
	// hash
	entry.file_location_hash = PHashString(assetFileName);
	memset(&entry.opaque_file_node_padding[0], 0, sizeof(entry.opaque_file_node_padding));


	entry.file_node = FGetFileNode(assetFileName);

	outTable->PushBack(entry);

}

///Removes asset from table in memory
void RemoveAssetFromTable(const s8* assetFileName, AssetTable* outTable) 
{
	for (u32 i = 0; i < outTable->count; i++) 
	{
		auto entry = &outTable->container[i];
		//printf("entryhash:%llu, toremove:%s, r3.spx:%llu\n", entry->file_location_hash, asset, PHashString("r3.spx"));
		if (entry->file_location_hash == PHashString(assetFileName)) {
			outTable->Remove(entry);
			return;
		}

	}
	printf("WARNING: %s doesn't exist\n", assetFileName);
}

///Removes asset from table in memory
void RemoveAssetFromTable(u32 index, AssetTable* outTable) 
{
	if (index >= outTable->count) 
	{
		printf("WARNING: Index %d doesn't exist\n", index);
		return;
	}
	outTable->Remove(index);
}

///Sorts the table by asset type
///secondary sort by file path length
void SortTableByType(AssetTable* outTable) 
{
	printf("Table Count: %d\n\n", (u32)outTable->count);

	//sort by type
	qsort(outTable->container, outTable->count,
		sizeof(AssetTableEntry),
		[](const void * a, const void* b)->s32
	{

		auto block_a = (AssetTableEntry*)a;
		auto block_b = (AssetTableEntry*)b;

		if (block_a == block_b)
		{
			return strlen(block_a->file_location) > strlen(block_b->file_location);
		}
		return block_a->type > block_b->type;
	}
	);

	//reset the offsets
	u32 offset = (u32)0;
	for (u32 i = 0; i < outTable->count; i++) 
	{
		auto entry = &outTable->container[i];

		entry->offset = offset;
		offset += entry->size;
	}
}

///Write table to file
void WriteTableToFile(const s8* packFilePath, AssetTable* outTable) 
{
	auto file = FOpenFile(packFilePath, F_FLAG_WRITEONLY | F_FLAG_TRUNCATE);

	FWrite(file, outTable->container, outTable->count * sizeof(AssetTableEntry));

	FCloseFile(file);
}

///Write table to file
void WriteTableToFile(FileHandle fh, AssetTable* outTable) 
{
	FWrite(fh, outTable->container, outTable->count * sizeof(AssetTableEntry));
}

///check for duplicates, also verify the data is loaded correctly by visiting each pointer
///or just scrap the old data and populate new one
//void ValidateList(const s8* assetlist, AssetTable* outTable) 
//{
//	printf("table count %d\n", (u32)outTable->count);
//
//}
//

///Prints Asset List to Console
void PrintTableToConsole(AssetTable* table) 
{

	u32 t = (u32)-1;
	printf("\n\nTABLESTART\n\n");
	for (u32 i = 0; i < table->count; i++) 
	{

		auto entry = &table->container[i];

		if (t != entry->type) 
		{
			printf("-----------------------\n");
			t = entry->type;
		}

		printf("SN%d TYPE%d SIZE%d OFFSET%d %s %llu\n", i, entry->type, entry->size, entry->offset, entry->file_location, entry->file_location_hash);

	}
	printf("\n\nTABLEEND\n\n");
}

/*
TODO: in general engine work, we should output to a separate file. this is mainly for windows
*/

void PrintHelp() 
{

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
		-tbake : bakes by type
        -cmp: byte compares the data in the list to that in the file
        -sort: sorts the table by type
        
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

///Write one assetTableEntry to file
void WriteDataToFile(FileHandle fileToWrite, const AssetTableEntry* asset)
{
	auto entry_file = FOpenFile(asset->file_location, F_FLAG_READWRITE);
	ptrsize entry_size = 0;
	auto entry_data = FReadFileToBuffer(entry_file, &entry_size);
	_kill("data written and entry size does not match", (u32)entry_size != (u32)asset->size);
	FWrite(fileToWrite, &entry_data[0], entry_size);
	FCloseFile(entry_file);
	unalloc(entry_data);
}

///Write one asset to file with file name fileName
void WriteDataToFile(FileHandle fileToWrite, AssetTable* assetTable, const s8* fileName) 
{	
	for (u32 i = 0; i < assetTable->count; i++)
	{
		const AssetTableEntry* entry = &assetTable->container[i];
		if (entry->file_location_hash == PHashString(fileName)) 
		{
			WriteDataToFile(fileToWrite, entry);
			return;
		}
	}
}

///Write whole table to file with file name fileName
void WriteDataToFile(FileHandle fileToWrite, AssetTable* assetTable)
{
	printf("\n\nBAKING DATA\n\n");
	for (u32 i = 0; i < assetTable->count; i++) 
	{
		const AssetTableEntry* entry = &assetTable->container[i];
		WriteDataToFile(fileToWrite, entry);
	}
	printf("\n\nDONE BAKING DATA\n\n");
}

void WriteDataToBuffer(AssetTable* asset_table, s8* buffer)
{
	for (u32 i = 0; i < asset_table->count; i++) {
		auto entry = asset_table->container[i];
		auto entry_file = FOpenFile(entry.file_location, F_FLAG_READWRITE);
		ptrsize entry_size;
		auto entry_data = FReadFileToBuffer(entry_file, &entry_size);
		_kill("data read and size do not match", (u32)entry_size != (u32)entry.size);
		strcat(buffer, entry_data);
		FCloseFile(entry_file);
		unalloc(entry_data);
	}
}

bool HashAndCompareString(const s8* lhs, const s8* rhs)
{
	return PHashString(lhs) == PHashString(rhs);
}

s32 main(s32 argc, s8** argv) 
{
	for (s32 i = 0; i < argc; i++) 
	{
		printf("%s\n", argv[i]);
	}

	if (argc < 3) 
	{
		printf("not enough arguments\n");
		PrintHelp();
		getchar();
		return -1;
	}

	AssetTable assetTable;
	assetTable.Init();
	//pack_file is the string name of the table file
	s8* pack_file = argv[1];
	s8* data_file = (s8*)alloc(sizeof(s8*));
	strcpy(data_file, pack_file);
	strcat(data_file, ".data");

	auto pFile = FOpenFile(pack_file, F_FLAG_READWRITE | F_FLAG_CREATE);
	auto dFile = FOpenFile(data_file, F_FLAG_READWRITE | F_FLAG_CREATE);

	if (FGetFileSize(pFile) == 0)
	{
		PackInit(pFile, &assetTable);
		//WriteDataToFile(&pfile, &asset_table);
	}
	LoadFileToMemory(pFile, &assetTable);

	FCloseFile(pFile);
	FCloseFile(dFile);

	WriteTableToFile(pack_file, &assetTable);
	/*
	if (PHashString(argv[2]) == PHashString("-up")) 
	{
		auto dFile = FOpenFile(data_file, F_FLAG_READWRITE|F_FLAG_CREATE);
		u32 table_size = 0;
		FSeekFile(dFile, 0, F_METHOD_START);
		FRead(dFile, &table_size, sizeof(u32));
		if (table_size > 0) 
		{
			auto fileTableBuffer = (AssetTableEntry*)alloc(table_size * sizeof(AssetTableEntry));
			FSeekFile(dFile, sizeof(u32),F_METHOD_START);
			FRead(dFile, fileTableBuffer, table_size * sizeof(AssetTableEntry));
			//auto ftB = strdup((s8*)fileTableBuffer);
			AssetTable assetsToRemove;// = (AssetTable*)alloc(sizeof(u32) * sizeof(AssetTableEntry*));
			assetsToRemove.Init();
			AssetTable toBeCreated;// = (AssetTable*)alloc(sizeof(asset_table));;
			toBeCreated.Init();

		//	for (u32 i = 0; i < table_size; i++) 
		//	{
		//		auto entry = fileTableBuffer[i];
		//		assetsToRemove.PushBack(entry);
		//	}

			PrintTableToConsole(&assetsToRemove);
			PrintTableToConsole(&assetTable);

			//go through every element in localFile
			//detect for changes in the datafile
			for (u32 localTableIndex = 0; localTableIndex < assetTable.count; localTableIndex++)
			{
				bool assetExists = false;
				//if we find a entry with same name, check if it remains the same. If it does, mark for nothing.
				//																	else mark for deletion, mark for creation
				//  if entry with same name does not exist, mark for creation
				//  if there is excess unwanted data in fileTable, mark all for deletion
				//

				for (u32 fileTableIndex = 0; fileTableIndex < assetsToRemove.count; fileTableIndex++)
				{
					//check each element of the same name with one another
					//printf("%llu %llu %llu", assetsToRemove[fileTableIndex].file_location_hash, asset_table[localTableIndex].file_location_hash, PHashString(assetsToRemove[fileTableIndex].file_location));
					//printf("%llu %llu %d %d %d %s", assetsToRemove[0].file_location_hash, assetsToRemove[1].file_location_hash, assetsToRemove[0].offset, assetsToRemove[0].size, assetsToRemove[0].type, assetsToRemove[0].file_location);
					if (assetsToRemove[fileTableIndex].file_location_hash == assetTable[localTableIndex].file_location_hash)
					{
						//check for changes in the file itself
						if (assetsToRemove[fileTableIndex].size != assetTable[localTableIndex].size)
						{
							//if data has changed, update data/mark data for editing
							auto assetToCreate = assetTable[localTableIndex];
							toBeCreated.PushBack(assetToCreate);
						}
						else 
						{
							//if data is unchanged, mark data as safe
							assetsToRemove.Remove(fileTableIndex);
						}
						assetExists = true;
						//fileTableIndex -= 1;
						break;
					}

				}
				if (!assetExists)
				{
					toBeCreated.PushBack(assetTable[localTableIndex]);
				}

			}
		unalloc(fileTableBuffer);
		}
		//dFile = FOpenFile(data_file, F_FLAG_WRITEONLY | F_FLAG_TRUNCATE);
		FSeekFile(dFile, 0, F_METHOD_START);
		FWrite(dFile,&assetTable.count,sizeof(u32));
		WriteTableToFile(dFile,&assetTable);
		FCloseFile(dFile);
		//WriteDataToFile(dFile, &asset_table);
	}*/
	
	//Read the -commands
	//add to asset list
	if (HashAndCompareString(argv[2],"-add")) 
	{
		for (s32 i = 3; i < argc; i++) 
		{
			AddAssetToTable(argv[i], &assetTable);
		}
		WriteTableToFile(argv[1], &assetTable);
	}
	else if (HashAndCompareString(argv[2],"-remove"))
	{
		if (PHashString(argv[3]) == PHashString("-all")) 
		{
			for (; assetTable.count > 0;) 
			{
				RemoveAssetFromTable(assetTable.container[0].file_location, &assetTable);
				printf("Removed %s\n", assetTable.container[0].file_location);
				//PrintAssetList(&asset_table);
			}
		}
		else 
		{
			for (s32 i = 3; i < argc; i++) 
			{
				if (PIsStringInt(argv[i]))
				{
					RemoveAssetFromTable(atoi(argv[i]), &assetTable);
				}
				else
				{
					RemoveAssetFromTable(argv[i], &assetTable);
				}
			}
		}

		WriteTableToFile(argv[1], &assetTable);
	}
	else if (HashAndCompareString(argv[2], "-list"))  
	{
		PrintTableToConsole(&assetTable);
	}
	else if (HashAndCompareString(argv[2], "-sort")) 
	{
		PrintTableToConsole(&assetTable);
		SortTableByType(&assetTable);
		PrintTableToConsole(&assetTable);
		WriteTableToFile(argv[1], &assetTable);
	}
	//MARK: Where do we fetch the data?
	else if (HashAndCompareString(argv[2],"-cmp"))
	{

	}
	else if (PHashString(argv[2]) == PHashString("-bake") ||
		      PHashString(argv[2]) == PHashString("-dbake")) 
	{
		u8 flag = 0;
		if (PHashString(argv[2]) == PHashString("-dbake"))
		{
			flag = 1;
		}
		//TODO: we should check if file is an executable

		//executable data

		ptrsize exec_size;
		s8* exec_buffer;

		auto exec_string = argv[3];

		_kill("no file entered", strlen(exec_string) == (u32)0);
		auto exec_file = FOpenFile(exec_string, F_FLAG_READWRITE);

		exec_buffer = FReadFileToBuffer(exec_file, &exec_size);
		printf("exec size %d\n", (u32)exec_size);

		auto offset = PFindStringInString("patchthisvalueinatassetpacktime", exec_buffer);
		_kill("couldn't find patch string", offset == (u32)-1);
		exec_buffer[offset] = '!';

		auto n_exec_file = FOpenFile("a.exe", F_FLAG_CREATE | F_FLAG_READWRITE);

		auto size_ptr = (u32*)&exec_buffer[offset + 1];

		*size_ptr = exec_size;

		FSeekFile(n_exec_file, 0, F_METHOD_START);

		FWrite(n_exec_file, &exec_buffer[0], exec_size);

		//sort by type before baking??
		//SortListByType(&asset_table);
		if (flag == 1) 
		{
			printf("Data sorting by type before baking.");
			SortTableByType(&assetTable);
			WriteTableToFile(argv[1], &assetTable);

		}
		printf("Baking according to table as shown: ");
		PrintTableToConsole(&assetTable);

		WriteDataToFile(n_exec_file, &assetTable);

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
		FWrite(n_exec_file, &len, sizeof(len));
		FWrite(n_exec_file, (void*)&poem[0], len);

		FCloseFile(n_exec_file);
		unalloc(exec_buffer);
	}
	else
	{
		printf("unrecognized command %s\n", argv[2]);
		PrintHelp();
	}

	return 0;
}
