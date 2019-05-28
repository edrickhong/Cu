#include "main.h"
#include "../importer/main.h"



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
    
	ptrsize list_count = 0;
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
	AssetTableEntry entry ={ ASSET_UNKNOWN };
    
	//always adds asset behind the last offset
	u32 lastOffset = 0;
	//check for duplicates and also find the entry with largest offset
	for (u32 i = 0; i < outTable->count; i++) 
	{
		auto entry = &outTable->container[i];
		if (entry->offset >= lastOffset) 
		{
			lastOffset = entry->offset + entry->size;
		}
		if (PHashString(assetFileName) == entry->file_location_hash) 
		{
			printf("Warning! %s file exists in the table! Resource %s was not added.\n", assetFileName, assetFileName);
			return;
			//file already exists
		}
	}
	entry.offset = lastOffset;
    
	auto file = FOpenFile(assetFileName, F_FLAG_READWRITE);
	_kill("File not found!", file == F_FILE_INVALID);
	auto fileSize = FGetFileSize(file);
	FCloseFile(file);
    
	entry.size = fileSize;
    
	u32 len = strlen(assetFileName);
	//label file type
	auto file_extension = assetFileName[len - 3] + assetFileName[len - 2] + assetFileName[len - 1];
    
    auto a = assetFileName[len - 3];
    auto b = assetFileName[len - 2];
    auto c = assetFileName[len - 1];
    
    b32 need_import = false;
    
	if (isModel(a,b,c) || _hash(a,b,c) == _hash('m','d','f')){
		entry.type = ASSET_MODEL;
        
        if(_hash('m','d','f')){
            need_import = true;
        }
	}
	else if (isAudio(a,b,c) || _hash(a,b,c) == _hash('a','d','f')){
		entry.type = ASSET_AUDIO;
        
        if(_hash('a','d','f')){
            need_import = true;
        }
	}
	else if (isImage(a,b,c) || _hash(a,b,c) == _hash('t','d','f')){
		entry.type = ASSET_TEXTURE;
        
        if(_hash('t','d','f')){
            need_import = true;
        }
	}
	else if (_hash(a,b,c) == _hash('s','p','x')){
		entry.type = ASSET_SHADER;
	}
    else if (_hash(a,b,c) == _hash('m','a','t')){
		entry.type = ASSET_MAT;
	}
	else{
		printf("unknown asset type. exiting.");
		_kill("unknown asset type\n", 1);
	}
    
    // location
    if(need_import){
        
        Import((s8**)&assetFileName,1);
        
        auto len = strlen(assetFileName);
        
        entry.original_file_node = {};
        
        memcpy(entry.original_file_location,assetFileName,len);
        entry.original_file_node = FGetFileNode(assetFileName);
        
        s8 buffer[256] = {};
        memcpy(buffer,assetFileName,len);
        
        switch(entry.type){
            
            case ASSET_AUDIO:{
                buffer[len - 3] = 'a';
                buffer[len - 2] = 'd';
                buffer[len - 1] = 'f';
            }break;
            
            case ASSET_TEXTURE:{
                buffer[len - 3] = 't';
                buffer[len - 2] = 'd';
                buffer[len - 1] = 'f';
            }break;
            
            case ASSET_MODEL:{
                buffer[len - 3] = 'm';
                buffer[len - 2] = 'd';
                buffer[len - 1] = 'f';
            }break;
        }
        
        memcpy(&entry.file_location,&buffer[0],len);
        // hash
        entry.file_location_hash = PHashString(buffer);
        
    }
    
    else{
        memcpy(&entry.file_location,&assetFileName[0],strlen(assetFileName));
        // hash
        entry.file_location_hash = PHashString(assetFileName);
        memset(&entry.opaque_file_node_padding[0],0,sizeof(entry.opaque_file_node_padding));
        entry.original_file_node = {};
        memset(entry.original_file_location,0,sizeof(entry.original_file_location));
    }
    
#if 1
	
	printf("location:%s\n",entry.file_location);
    printf("original:%s\n",entry.original_file_location);
    
#endif
    
	outTable->PushBack(entry);
    
}

///Removes asset from table in memory using file path
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

///Removes asset from table in memory using index
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

///Write table to file using file path
void WriteTableToFile(const s8* packFilePath, AssetTable* outTable) 
{
	auto file = FOpenFile(packFilePath, F_FLAG_WRITEONLY | F_FLAG_TRUNCATE);
    
	FWrite(file, outTable->container, outTable->count * sizeof(AssetTableEntry));
    
	FCloseFile(file);
}

///Write table to file using opened file handle
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
        
		printf("SN:%d TYPE:%d SIZE:%d OFFSET:%d LOC:%s HASH:%llu ORG:%s\n", i, entry->type, entry->size, entry->offset, entry->file_location, entry->file_location_hash,entry->original_file_location);
        
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
        -bake : bakes asset data either into an executable
		-tbake : bakes after sorting by type
        -bcmp: byte compares the data in the table to the baked data in the executable
        -cmp:  compares the data size in the table to the size of bytes written in the executable
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

///Byte compare the table and the data written
void VerifyFileData(const s8* dataBuffer, const AssetTable* assetTable)
{
	for (u32 i = 0; i < assetTable->count; i++)
	{
		const AssetTableEntry* entry = &assetTable->container[i];
		auto entry_file = FOpenFile(entry->file_location, F_FLAG_READWRITE);
		ptrsize entry_size = 0;
		auto entry_data = FReadFileToBuffer(entry_file, &entry_size);
		FCloseFile(entry_file);
        
		auto i_EntryData = entry_data;
		_kill("data written and entry size does not match", (u32)entry_size != (u32)entry->size);
		//byte compare
		for (ptrsize i_EntrySize = 0; i_EntrySize < entry_size; ++i_EntrySize)
		{
			if (*dataBuffer++ != *i_EntryData++)
			{
				printf("%c does not match %c", *dataBuffer, *i_EntryData);
				//_kill("data is wrong", 1);
			}
		}
		unalloc(entry_data);
	}
	printf("File data matches table entries!");
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

///Write whole table to file
void WriteDataToFile(FileHandle fileToWrite, AssetTable* assetTable)
{
	printf("\nBAKING DATA...\n");
	for (u32 i = 0; i < assetTable->count; i++) 
	{
		const AssetTableEntry* entry = &assetTable->container[i];
		WriteDataToFile(fileToWrite, entry);
	}
	printf("\nDONE BAKING DATA!\n");
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
		return -1;
	}
    
	AssetTable assetTable;
	assetTable.Init();
	//pack_file is the string name of the table file
	s8* packFilePath = argv[1];
	s8  dataFilePath[100];
	strcpy(dataFilePath, packFilePath);
	strcat(dataFilePath, ".data");
    
	auto pFile = FOpenFile(packFilePath, F_FLAG_READWRITE | F_FLAG_CREATE);
	auto dFile = FOpenFile(dataFilePath, F_FLAG_READWRITE | F_FLAG_CREATE);
    
	if (FGetFileSize(pFile) == 0)
	{
		PackInit(pFile, &assetTable);
		//WriteDataToFile(&pfile, &asset_table);
	}
	LoadFileToMemory(pFile, &assetTable);
    
	FCloseFile(pFile);
	FCloseFile(dFile);
    
	WriteTableToFile(packFilePath, &assetTable);
    
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
	else if (HashAndCompareString(argv[2], "-bcmp"))
	{
		ptrsize execSize;
        
		auto execString = argv[3];
		_kill("no file entered", strlen(execString) == (u32)0);
        
		auto fileExtension = execString + (strlen(execString) - 3);
		_kill("not an exe", !HashAndCompareString(fileExtension, "exe"));
        
		auto execFileHandle = FOpenFile(execString, F_FLAG_READWRITE);
		s8* exeBuffer = FReadFileToBuffer(execFileHandle, &execSize);
		FCloseFile(execFileHandle);
        
		printf("exec size %d\n", (u32)execSize);
        
		//Locate the 4 bytes that store the offset value
		auto bufferOffset = PFindStringInBuffer("thisvalueinatassetpacktime",
                                                exeBuffer, execSize);
        
		bufferOffset -= 5;
		char* exclaim = (char*)&exeBuffer[bufferOffset];
		//Get the start of data buffer
		++bufferOffset;
        
		//ptrsize dataOffset = strtoull(&exeBuffer[bufferOffset], &(exeBuffer), 10);
		u32* dataOffset = (u32*)&exeBuffer[bufferOffset];
        
		VerifyFileData(&exeBuffer[*dataOffset], &assetTable);
        
		////Locate the end of the data buffer
		//ptrsize endOffset = PFindStringInBuffer("!dataend!", exeBuffer,
		//	execSize);
        
		//AssetTableEntry lastEntry = assetTable[assetTable.count - 1];
		//ptrsize correctSize = lastEntry.offset + lastEntry.size;
        
		//if (endOffset - *dataOffset == correctSize)
		//{
		//	printf("Size of written data matches table.");
		//}
		//else
		//{
		//	printf("Size of written data does not match table.");
		//}
        
	}
	else if (HashAndCompareString(argv[2], "-cmp"))
	{
		ptrsize execSize;
        
		auto execString = argv[3];
		_kill("no file entered", strlen(execString) == (u32)0);
        
		auto fileExtension = execString + (strlen(execString) - 3);
		_kill("not an exe", !HashAndCompareString(fileExtension, "exe"));
        
		auto execFileHandle = FOpenFile(execString, F_FLAG_READWRITE);
		s8* exeBuffer = FReadFileToBuffer(execFileHandle, &execSize);
		FCloseFile(execFileHandle);
        
		printf("exec size %d\n", (u32)execSize);
        
		//Locate the 4 bytes that store the offset value
		auto bufferOffset = PFindStringInBuffer("thisvalueinatassetpacktime",
                                                exeBuffer, execSize);
		
		bufferOffset -= 5;
		char* exclaim = (char*)&exeBuffer[bufferOffset];
		//Get the start of data buffer
		++bufferOffset;
        
		//ptrsize dataOffset = strtoull(&exeBuffer[bufferOffset], &(exeBuffer), 10);
		u32* dataOffset = (u32*)&exeBuffer[bufferOffset];
        
		//Locate the end of the data buffer
		ptrsize endOffset = PFindStringInBuffer("!dataend!", exeBuffer,
                                                execSize);
        
		AssetTableEntry lastEntry = assetTable[assetTable.count - 1];
		ptrsize correctSize = lastEntry.offset + lastEntry.size;
        
		if (endOffset - *dataOffset == correctSize)
		{
			printf("Size of written data matches table.");
		}
		else
		{
			printf("Size of written data does not match table.");
		}
        
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
	else if (PHashString(argv[2]) == PHashString("-bake") ||
             PHashString(argv[2]) == PHashString("-tbake")) 
	{
		u8 flag = 0;
		if (PHashString(argv[2]) == PHashString("-tbake"))
		{
			flag = 1;
		}
        
		ptrsize execSize;
		s8* exec_buffer;
        
		auto execString = argv[3];
		_kill("no file entered", strlen(execString) == (u32)0);
        
		auto file_extension = execString + (strlen(execString) - 3);
		_kill("not an exe", !HashAndCompareString(file_extension, "exe"));
        
		auto execFileHandle = FOpenFile(execString, F_FLAG_READWRITE);
        
		exec_buffer = FReadFileToBuffer(execFileHandle, &execSize);
		printf("exec size %d\n", (u32)execSize);
		
		auto offset = PFindStringInBuffer("patchthisvalueinatassetpacktime", 
                                          exec_buffer, execSize);
        
		_kill("couldn't find patch string", offset == (u32)-1);
		exec_buffer[offset ] = '!';
        
		auto n_exec_file = FOpenFile("a.exe", F_FLAG_CREATE | F_FLAG_READWRITE);
        
		auto size_ptr = (u32*)&exec_buffer[offset + 1];
        
		*size_ptr = execSize;
        
		FSeekFile(n_exec_file, 0, F_METHOD_START);
        
		FWrite(n_exec_file, &exec_buffer[0], execSize);
        
		if (flag == 1) 
		{
			printf("Data sorting by type before baking.");
			SortTableByType(&assetTable);
			WriteTableToFile(argv[1], &assetTable);
            
		}
		printf("Baking according to table as shown: ");
		PrintTableToConsole(&assetTable);
        
		WriteDataToFile(n_exec_file, &assetTable);
		auto dataEnd = R"(!dataend!)";
		FWrite(n_exec_file, (void*)dataEnd, strlen(dataEnd) + 1);
        
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
		FWrite(n_exec_file, (void*)poem, len);
        
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

