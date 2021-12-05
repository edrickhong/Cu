#ifndef _WIN32
#pragma GCC diagnostic warning "-Wextra-semi"
#pragma GCC diagnostic error "-Wextra-semi"
#pragma GCC diagnostic ignored "-Wextra-semi"

#pragma GCC diagnostic warning "-Wcast-align"
#pragma GCC diagnostic error "-Wcast-align"
#pragma GCC diagnostic ignored "-Wcast-align"

#pragma GCC diagnostic warning "-Wshadow"
#pragma GCC diagnostic error "-Wshadow"
#pragma GCC diagnostic ignored "-Wshadow"


#endif


#include "string.h"
#include "mode.h"
#include "ttype.h"

#include "ffileio.h"
#include "ccontainer.h"

#ifndef _WIN32
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdouble-promotion"
#pragma GCC diagnostic ignored "-Wimplicit-fallthrough"
#pragma GCC diagnostic ignored "-Wfloat-equal"
#pragma GCC diagnostic ignored "-Wpacked"
#endif

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#define STB_DXT_IMPLEMENTATION

#include "stb/stb_image.h"
#include "stb/stb_image_resize.h"
#include "stb/stb_dxt.h"

#include <assimp/Importer.hpp> 
#include <assimp/scene.h>     
#include <assimp/postprocess.h>
#include <assimp/cimport.h>

#ifndef _WIN32
#pragma GCC diagnostic pop
#endif

#include "aassettools.h"

#include "pparse.h"


#define _hash(a,b,c) (a * 1) + (b * 2) + (c * 3)
#define _encode(a,b,c,d) (u32)  (((u32)(a << 0)) | ((u32)(b << 8)) | ((u32)(c << 16)) | ((u32)(d << 24)))


//TODO: we should have a notion of a string pool to allocation string from and to delete all strings

_ainline s8* PNewStringCopy(s8* string){

	u32 len = strlen(string) + 1;

	if(len == 1){
		return 0;
	}

	s8* nstring = (s8*)alloc(len);

	memcpy(nstring,string,len);

	return nstring;
}


struct TAnimChannel{
	u32 positionkey_count;
	u32 rotationkey_count;
	u32 scalekey_count;
	AnimationKey* positionkey_array;
	AnimationKey* rotationkey_array;
	AnimationKey* scalekey_array;

	s8* name;
	u32 name_hash;
};


_declare_list(TAnimChannelList,TAnimChannel);

struct TAnim{
	f32 tps;
	f32 duration;


	s8* name;
	u32 name_hash;


	TAnimChannelList channels;
};



//TODO: these ultimately have to be stored SOA style
struct TBone{
	Mat4 offset;
	u32 children_count;
	s8* name;
	u32 name_hash;

#if 1
	Mat4 res = {};
#endif
};

struct TSkin{
	u32 index[4];
	f32 weight[4];
};

_declare_list(TBoneList,TBone);
_declare_list(TAnimList,TAnim);
_declare_list(TSkinList,TSkin);

//TODO: handle non pow2, non uniform sizes
void NextMipDim(u32* w,u32* h){
    (*w) >>= 1;
    (*h) >>= 1;
}

void GenerateMipMaps(u8* indata,u8* outdata,u32 w,u32 h,u32 maxmip){
    
    u32 offset = w * h * 4;
    
    memcpy(outdata,indata,offset);
    
    u32 nw = w;
    u32 nh = h;
    
    for(u32 i = 0; i < (maxmip - 1); i++){
        
        NextMipDim(&nw,&nh);
        
        stbir_resize_uint8(indata,w,h ,0,(outdata + offset),nw,nh, 0, 4);
        
        offset += nw * nh * 4;
    }
    
    nw = w;
    nh = h;
    offset = 0;
    
#if 0
    
    for(u32 i = 0; i < (maxmip); i++){
        
        s8 string[100] = {};
        sprintf(string,"test_%d.bmp",i);
        WriteBMP((outdata + offset),nw,nh,string);
        
        offset += nw * nh * 4;
        
        NextMipDim(&nw,&nh);
    }
    
#endif
    
}


_intern void ExtractTile(void* indata,void* outdata,u32 w,u32 tx,u32 ty){
    
    auto px = tx << 7;
    auto py = ty << 7;
    
    auto src = (u32*)indata;
    
    auto dst = (u32*)outdata;
    auto cur = dst;
    
    for(u32 y = py; y < (py + 128); y++){
        
        for(u32 x = px; x < (px + 128); x++){
            *cur = src[(y * w) + x];
            cur++;
        }
        
    }
    
}

_intern void ExtractBlock(void* indata,void* outdata,u32 w,u32 b_x,
                          u32 b_y){
    auto px = b_x << 2;
    auto py = b_y << 2;
    
    auto src = (u32*)indata;
    
    auto dst = (u32*)outdata;
    auto cur = dst;
    
    for(u32 y = py; y < (py + 4); y++){
        
        for(u32 x = px; x < (px + 4); x++){
            *cur = src[(y * w) + x];
            cur++;
        }
        
    }
}

void CompressBlockBC1(void* indata,void* outdata,u32 w,u32 h){
    
    auto cur = (u8*)outdata;
    
    u32 b_w = w >> 2;
    u32 b_h = h >> 2;
    
    for(u32 b_y = 0; b_y < b_h;b_y++){
        
        for(u32 b_x = 0; b_x < b_w;b_x++){
            
            u32 tbuffer[16];
            
            ExtractBlock(indata,tbuffer,w,b_x,b_y);
            
            stb_compress_dxt_block(cur,(u8*)tbuffer,0,STB_DXT_HIGHQUAL);
            
            cur += 8;
        }
    }
    
}


void* ArrangeImageToTiles(void* data,u32 w,u32 h,TexFormat format,
                          u32* size){
    
    auto bpp = GetBPP(format);
    
    u32 tile_size = (u32)(128 * 128 * bpp);
    *size = (u32)(w * h * bpp);
    
    auto ret_data = (s8*)alloc(*size);
    auto cur = ret_data;
    
    auto tw = w >> 7;
    auto th = h >> 7;
    
    for(u32 ty = 0; ty < th;ty++){
        
        for(u32 tx = 0; tx < tw;tx++){
            
            u32 tbuffer[128 * 128];
            
            
            ExtractTile(data,tbuffer,w,tx,ty);
            
            if(format == Format_RGBA){
                memcpy(cur,tbuffer,sizeof(tbuffer));
            }
            
            if(format == Format_BC1){
                CompressBlockBC1(tbuffer,cur,128,128);
            }
            
            cur += tile_size;
        }
        
    }
    
    return ret_data;
}

void WriteTDF(void* data,u32 w,u32 h,u32 size,u32 mips,TexFormat format,
              const s8* writefilepath){
    
    u32 tsize = size + sizeof(TDFHeader);
    
    auto buffer = (s8*)alloc(tsize);
    
    auto header = (TDFHeader*)buffer;
    auto tdata = (buffer + sizeof(TDFHeader));
    
    f32 bpp = GetBPP(format);
    
    header->tag = _encode('T','D','F',' ');
    header->w = w;
    header->h = h;
    header->bpp = bpp;
    header->mips = mips;
    header->format = format;
    
    memcpy(tdata,data,size);
    
    auto writefile = FOpenFile(writefilepath,F_FLAG_WRITEONLY | F_FLAG_CREATE |
                               F_FLAG_TRUNCATE);
    
    FWrite(writefile,buffer,tsize);
    
    FCloseFile(writefile);
    
    unalloc(buffer);
}



void CreateTextureAssetTDF(const s8* inputfile,const s8* outputfile,TexFormat format,
                           b32 gen_mips,b32 is_vt = 1){
    
    /*
      writeData is always called to write 128 bytes at the start. Is this redundant?
    */
    
    s32 w,h,comp;
    
    auto imagedata = stbi_load(inputfile,&w,&h,&comp,4);
    
    u32 maxmip = (u32)-1;
    
    struct Dimensions{
        u32 w;
        u32 h;
    };
    
    if(is_vt){
        
        _kill("we do not support non square images for now\n", w != h);
        
        _kill("not in units of 128 pixels\n", (w % 128));
        
        gen_mips = is_vt;
        
        maxmip = 0;
        
        for(u32 dim = w; dim >= 128; dim >>= 1 ){
            maxmip++;
        }
    }
    
    u32 tsize = 0;
    
    {
        u32 tw = w;
        u32 th = h;
        
        for(u32 i = 0; i < maxmip; i++){
            tsize += tw * th * 4;
            NextMipDim(&tw,&th);
        }    
    }
    
    
    
    printf("w %d h %d mips %d\n",w,h,maxmip);
    
    auto mipdata = (u8*)alloc(tsize);
    
    if(gen_mips){
        GenerateMipMaps(imagedata,mipdata,w,h,maxmip);
    }
    
    BufferRegion region;
    
    region.Init();
    
    if(is_vt){
        
        u32 tw = w;
        u32 th = h;
        
        u32 offset = 0;//offset to the next mip map
        
        for(u32 i = 0; i < maxmip; i++){
            
            u32 size;
            
            auto  tdata = 
                ArrangeImageToTiles((mipdata + offset),tw,th,format,&size);
            
            offset += tw * th * 4;
            
            region.Write(tdata,size);
            
            NextMipDim(&tw,&th);
            
            free(tdata);
        }
        
    }
    
    WriteTDF(region.container,w,h,region.count,maxmip,format,outputfile);
    
#if 0
    
    {
        
        u32 ty = w >> 7;
        u32 tx = h >> 7;
        
        auto file = FOpenFile(outputfile,F_FLAG_READONLY);
        
        for(u32 y = 0; y < ty; y++){
            
            for(u32 x = 0; x < tx; x++){
                
                u32 data[128 * 128] = {};
                
                // GetTileDataTDF(file,w,h,x,y,0,4.0f,data);
                TestGetTileData(file,w,h,x,y,0,4.0f,data);
                
                s8 string[100] = {};
                
                sprintf(string,"dst/test_y%d_x%d_%d.bmp",y,x,0);
                WriteBMP(data,128,128,string);
            }
            
        }
        
        FCloseFile(file);
        
        exit(0); 
    }
    
#endif
    
#if 0
    {
        printf("total write %d\n",region.count);
        
        u32 offset = 84 * 128 * 128;
        auto ptr = ((u32*)region.container) + offset;
        
        WriteBMP(ptr,128,128,"tile.bmp");
    }
    
#if 0
    {
        
        u32 buffer[128 * 128];
        
        auto file = FOpenFile(outputfile,F_FLAG_READONLY);
        
        // TestGetTileData(file,128 * 128 * 4,buffer);
        
        TestGetTileData(file,1024,1024,2,5,5,4.0f,buffer);
        
        FCloseFile(file);
        
        WriteBMP(buffer,128,128,"fetch.bmp");
    }
#endif
    
    
#endif
    
    stbi_image_free(imagedata);
    free(mipdata);
    region.Destroy();
}

u32 isModel(s8 a,s8 b,s8 c){
    return (_hash(a,b,c) == _hash('d','a','e')) || (_hash(a,b,c) == _hash('o','b','j')) ||
        (_hash(a,b,c) == _hash('3','d','s')) || (_hash(a,b,c) == _hash('f','b','x'));
}

u32 isAudio(s8 a,s8 b,s8 c){
    return (_hash(a,b,c) == _hash('w','a','v'));
}

u32 isImage(s8 a,s8 b,s8 c){
    return (_hash(a,b,c) == _hash('j','p','g')) || (_hash(a,b,c) == _hash('p','n','g')) ||
        (_hash(a,b,c) == _hash('b','m','p')) || (_hash(a,b,c) == _hash('t','g','a'));
}


#define _riffcode(a,b,c,d)  (u32)  (((u32)(a << 0)) | ((u32)(b << 8)) | ((u32)(c << 16)) | ((u32)(d << 24)))

enum WavChunkID{
    CHUNK_FMT = _riffcode('f','m','t',' '),
    CHUNK_RIFF = _riffcode('R','I','F','F'),
    CHUNK_WAVE = _riffcode('W','A','V','E'),
    CHUNK_DATA = _riffcode('d','a','t','a'),
};

struct WavHeader{
    u32 riff_id;
    u32 size;
    u32 wave_id;
};

struct WavChunk{
    u32 id;
    u32 size;
};


struct WavFormat{
    u16 format_tag;
    u16 channels;
    u32 samplespersecond;
    u32 avgbytespersample;
    u16 blockalign;
    u16 bitspersample;
    u16 size;
    u16 validbitspersample;
    u32 channelmask;
    u8 subformat[16];
};

struct WavIterator{
    u8* at;
    u8* stop;
};



u32 WavGetType(WavIterator iter){
    
    WavChunk* chunk = (WavChunk*)iter.at;
    
    return chunk->id;
}


WavIterator WavNextChunk(WavIterator iter){
    
    WavChunk* chunk = (WavChunk*)iter.at;
    
    u32 size = (chunk->size + 1) & ~1;
    
    iter.at += sizeof(WavChunk) + size;
    
    return iter;
}

void* WavGetChunkData(WavIterator iter){
    
    return (iter.at + sizeof(WavChunk));
}

u32 WavGetChunkDataSize(WavIterator iter){
    
    WavChunk* chunk = (WavChunk*)iter.at;
    
    return (chunk->size);
}

void WavLoadSound(void* filedata,void** data,u32* size){
    
    WavHeader* header = (WavHeader*)filedata;
    
    _kill("",header->riff_id != CHUNK_RIFF);
    _kill("",header->wave_id != CHUNK_WAVE);
    
    for(WavIterator iter = {(u8*)(header + 1),(((u8*)(header + 1)) + (header->size - 4))};
        iter.at < iter.stop; iter = WavNextChunk(iter)){
        
        
        switch(WavGetType(iter)){
            
            case CHUNK_FMT:{
                WavFormat* fmt = (WavFormat*)WavGetChunkData(iter);
                
                
                _kill("",fmt->channels > 2);
                _kill("",fmt->bitspersample != 16);
                _kill("",fmt->samplespersecond != 48000);
                
                //MARK:we should resample this to the appropriate format. Should we though?
            }break;
            
            case CHUNK_DATA:{
                
                *size = WavGetChunkDataSize(iter);
                *data = alloc(*size);
                
                memcpy(*data,WavGetChunkData(iter),*size);
            }break;
            
        }
        
        
    }
    
}


void WavWriteADF(const s8* filepath,const s8* writefilepath){
    
    FileHandle file = FOpenFile(filepath,F_FLAG_READONLY);
    
    void* filedata = FReadFileToBuffer(file,0);
    
    FCloseFile(file);
    
    void* audio_data;
    u32 size;
    
    WavLoadSound(filedata,&audio_data,&size);
    
    FileHandle writefile = FOpenFile(writefilepath,F_FLAG_WRITEONLY | F_FLAG_CREATE |
                                     F_FLAG_TRUNCATE);
    
    /*ADF format. u32 (ADF'') u32(Compression type) (DATA)*/
    
    u32 write_int = TAG_ADF;
    
    FWrite(writefile,&write_int,sizeof(write_int));
    
    write_int = TAG_UNCOMPRESSED;
    
    FWrite(writefile,&write_int,sizeof(write_int));
    
    FWrite(writefile,audio_data,size);
    
    FCloseFile(writefile);
    
    unalloc(filedata);
    
    unalloc(audio_data);
}

_declare_list(VertexBoneDataList,VertexBoneData);

_declare_list(BonenodeList,AssimpBoneNode);
_declare_list(AssimpAnimationList,AssimpAnimation);

void PrintNodeHeirarchy(aiNode* node,aiNode* root){
    
    printf("%s %d\n",node->mName.data,node->mNumChildren);
    
    for(u32 i = 0; i < node->mNumChildren;i++){
        
        PrintNodeHeirarchy(node->mChildren[i],root);
    }
    
}

void PrintNodeHeirarchy(AssimpBoneNode bonenode,BonenodeList bonenodelist){
    
    printf("%s %d\n",bonenode.name,bonenode.children_count);
    
    for(u32 i = 0; i < bonenode.children_count;i++){
        
        u32 index = bonenode.childrenindex_array[i];
        
        AssimpBoneNode next = bonenodelist[index];
        
        PrintNodeHeirarchy(next,bonenodelist);
    }
    
}

void AssimpLoadAnimations(aiScene* scene,AssimpAnimationList* list){
    
    for(u32 i = 0; i < scene->mNumAnimations;i++){
        
        AssimpAnimation animation = {};
        aiAnimation* anim = scene->mAnimations[i];
        
        animation.data_count = anim->mNumChannels;
        animation.duration = anim->mDuration;
        animation.tps = anim->mTicksPerSecond;
        
        // printf("Total animation channels %d\n",anim->mNumChannels);
        
        u32 len = strlen(anim->mName.data);

        
        if(len){
            
            animation.name = (s8*)alloc(len);
            memcpy(animation.name,anim->mName.data,len);
        }
        
        animation.data =
            (AssimpAnimationData*)alloc(anim->mNumChannels * sizeof(AssimpAnimationData));
        
        for(u32 j = 0; j < anim->mNumChannels;j++){
            
            aiNodeAnim* node = anim->mChannels[j];
            
            AssimpAnimationData data;
	    data.name_hash = PHashString(node->mNodeName.data);

	    printf("Anim node:%s\n",node->mNodeName.data);

	    _kill("too large\n",node->mNumPositionKeys >
			    _unsigned_max(AAnimationData::positionkey_count));

	    _kill("too large\n",node->mNumRotationKeys >
			    _unsigned_max(AAnimationData::rotationkey_count));

	    _kill("too large\n",node->mNumScalingKeys >
			    _unsigned_max(AAnimationData::scalekey_count));

	    data.positionkey_count = node->mNumPositionKeys;
	    data.rotationkey_count = node->mNumRotationKeys;
	    data.scalekey_count = node->mNumScalingKeys;

	    data. positionkey_array =
		    (AnimationKey*)alloc(sizeof(AnimationKey) *
				    (node->mNumPositionKeys + node->mNumRotationKeys +
				     node->mNumScalingKeys));

	    data. rotationkey_array = data. positionkey_array + node->mNumPositionKeys;
	    data. scalekey_array = data. rotationkey_array + node->mNumRotationKeys;

	    data.prestate = (AnimationBehaviour)node->mPreState;
	    data.poststate = (AnimationBehaviour)node->mPostState;

	    for(u32 k = 0; k < node->mNumPositionKeys;k++){

		    aiVectorKey vec = node->mPositionKeys[k];
		    AnimationKey key = {(f32)vec.mTime,{vec.mValue.x,vec.mValue.y,vec.mValue.z,1.0f}};

		    memcpy(data.positionkey_array + k,&key,sizeof(AnimationKey));
	    }

	    for(u32 k = 0; k < node->mNumRotationKeys;k++){

		    aiQuatKey q = node->mRotationKeys[k];
		    AnimationKey key =
		    {(f32)q.mTime,{q.mValue.x,q.mValue.y,q.mValue.z,q.mValue.w}};

		    memcpy(data.rotationkey_array + k,&key,sizeof(AnimationKey));
	    }

	    for(u32 k = 0; k < node->mNumScalingKeys;k++){
		    aiVectorKey vec = node->mScalingKeys[k];
		    AnimationKey key = {(f32)vec.mTime,{vec.mValue.x,vec.mValue.y,vec.mValue.z,1.0f}};

		    memcpy(data.scalekey_array + k,&key,sizeof(AnimationKey));
	    }

	    memcpy(animation.data + j,&data,sizeof(AssimpAnimationData));
	}

	list->PushBack(animation);

    }

}

void DestroyAssimpData(AssimpData data){

	unalloc(data.vertex_array);
	unalloc(data.texcoord_array);
	unalloc(data.normal_array);
	unalloc(data.index_array);
	unalloc(data.vertexbonedata_array);
	unalloc(data.bone_array);

	for(u32 i = 0; i < data.animation_count;i++){

		for(u32 j = 0; j < data.animation_array[i].data_count;j++){
			unalloc(data.animation_array[i].data[j].positionkey_array);
		}

		unalloc(data.animation_array[i].data);

	}

	unalloc(data.animation_array);

}

void AssimpAddBoneData(VertexBoneData* bonedata,u32 index,f32 weight){


	for(u32 i = 0; i < 4;i++){

		if(bonedata->bone_weight[i] == 0.0f){
			bonedata->bone_weight[i] = weight;
			bonedata->bone_index[i] = index;
			/* printf("bindex:%d\n",index); */
			return;
		}

	}
#if 0
	printf("Vertice is influenced by more than 4 bones\n");
#endif

	// _kill("Vertice is influenced by more than 4 bones\n",1);
}

_intern u32 FindIndex(const s8* string,BonenodeList bonenodelist){


	for(u32 i = 0; i < bonenodelist.count;i++){

		if(PStringCmp(bonenodelist[i].name,string)){
			return i;
		}
	}

	//_kill("bone not found\n",1);

	return -1;

}


_intern u32 FindIndex(const s8* string,TBoneList bonenodelist){


	for(u32 i = 0; i < bonenodelist.count;i++){

		if(PStringCmp(bonenodelist[i].name,string)){
			return i;
		}
	}

	//_kill("bone not found\n",1);

	return -1;

}

#if 0
_declare_list(VertexBoneDataList,VertexBoneData);

_declare_list(BonenodeList,AssimpBoneNode);
_declare_list(AssimpAnimationList,AssimpAnimation);

#endif



void AssimpLoadBoneVertexData(aiMesh* mesh,VertexBoneDataList* bonedatalist,
		BonenodeList* bonenodelist){

	memset(bonedatalist->container,0,mesh->mNumVertices * sizeof(VertexBoneData));

	for(u32 i = 0; i < mesh->mNumBones;i++){

		aiBone* bone = mesh->mBones[i];

		auto node_index = FindIndex(bone->mName.data,*bonenodelist);
		auto bonenode = &(*bonenodelist)[node_index];




		//NOTE: ok so we actually need to do this replacement
#if 1


		//PrintMat4(bonenode->offset);
		memcpy(&bonenode->offset,&bone->mOffsetMatrix,sizeof(Mat4));
		PrintMat4(bonenode->offset);
		//printf("------------------------------------");

#if !MATRIX_ROW_MAJOR
		bonenode->offset = Transpose(bonenode->offset);
#endif


#endif

		for(u32 j = 0; j < bone->mNumWeights;j++){

			//which vertex it influences
			u32 vertexid = bone->mWeights[j].mVertexId;


			//MARK:
			AssimpAddBoneData(&((*bonedatalist)[vertexid]),
					node_index,bone->mWeights[j].mWeight);
		}
	}
}


_intern void LoadTSkin(aiMesh* mesh,TSkinList* list,TBoneList skel){



	auto get_index = [](const s8* string,TBoneList skel) -> u32 {
		for(u32 i = 0; i < skel.count;i++){

			if(PStringCmp(skel[i].name,string)){
				return i;
			}
		}

		return -1;
	};

	auto addvert = [](TSkin* vert, u32 index, f32 weight)-> void {
		for(u32 i = 0; i < 4; i++){

			if(vert->weight[i] == 0.0f){
				vert->index[i] = index;
				vert->weight[i] = weight;

				return;
			}
		}

		printf("WARNING: Vertes has more than 4 weights!\n");
	};

	for(u32 i = 0; i < mesh->mNumBones; i++){
		auto bone = mesh->mBones[i];
		auto bone_index = get_index(bone->mName.data,skel);

		_kill("Not Found\n", bone_index == (u32)-1);

		auto bonenode = &skel[bone_index];


		memcpy(&bonenode->offset,&bone->mOffsetMatrix,sizeof(Mat4));

#if !MATRIX_ROW_MAJOR
		bonenode->offset = Transpose(bonenode->offset);
#endif

		for(u32 j = 0; j < bone->mNumWeights; j++){
			u32 vertindex = bone->mWeights[j].mVertexId;
			f32 weight = bone->mWeights[j].mWeight;

			addvert(&list->container[vertindex],bone_index,weight);
		}
	}
}



//we are gonna store this soa style too
_intern void LoadTAnim(aiScene* scene,TAnimList* anims,TBoneList skel){

	for(u32 i = 0; i  < scene->mNumAnimations; i++){

		TAnim animation = {};
		auto  anim = scene->mAnimations[i];

		animation.duration = anim->mDuration;
		animation.tps = anim->mTicksPerSecond;
		animation.name = PNewStringCopy(anim->mName.data);
		animation.name_hash = animation.name? PHashString(animation.name) : 0;

		animation.channels.Init(skel.count);
		animation.channels.count = skel.count;
		memset(&animation.channels[0],0,skel.count * sizeof(TAnimChannel));

		auto get_index = [](u32 hash,TBoneList skel) -> u32 {
			for(u32 i = 0; i < skel.count; i++){
				if(hash == skel[i].name_hash){
					return i;
				}
			}


			_kill("Bone not found\n",true);

			return (u32)-1;
		};

		for(u32 j = 0; j < anim->mNumChannels; j++){

			auto node = anim->mChannels[j];
			auto name = node->mNodeName.data;
			auto name_hash = PHashString(name);
			auto channel = &animation.channels[(u32)get_index(name_hash,skel)];

			channel->name = PNewStringCopy(name);
			channel->name_hash = name_hash;

			channel->positionkey_count = node->mNumPositionKeys;
			channel->rotationkey_count = node->mNumRotationKeys;
			channel->scalekey_count = node->mNumScalingKeys;

			//NOTE: we can store these interleaved because cmoves don't count as branches

			channel-> positionkey_array =
				(AnimationKey*)alloc(sizeof(AnimationKey) *
						(node->mNumPositionKeys + node->mNumRotationKeys +
						 node->mNumScalingKeys));

			channel-> rotationkey_array = channel-> positionkey_array + node->mNumPositionKeys;
			channel-> scalekey_array = channel-> rotationkey_array + node->mNumRotationKeys;

			for(u32 k = 0; k < node->mNumPositionKeys;k++){

				aiVectorKey vec = node->mPositionKeys[k];
				AnimationKey key = {(f32)vec.mTime,{vec.mValue.x,vec.mValue.y,vec.mValue.z,1.0f}};

				memcpy(channel->positionkey_array + k,&key,sizeof(AnimationKey));
			}

			for(u32 k = 0; k < node->mNumRotationKeys;k++){

				aiQuatKey q = node->mRotationKeys[k];
				AnimationKey key =
				{(f32)q.mTime,{q.mValue.x,q.mValue.y,q.mValue.z,q.mValue.w}};

				memcpy(channel->rotationkey_array + k,&key,sizeof(AnimationKey));
			}

			for(u32 k = 0; k < node->mNumScalingKeys;k++){
				aiVectorKey vec = node->mScalingKeys[k];
				AnimationKey key = {(f32)vec.mTime,{vec.mValue.x,vec.mValue.y,vec.mValue.z,1.0f}};

				memcpy(channel->scalekey_array + k,&key,sizeof(AnimationKey));
			}
		}

		anims->PushBack(animation);

	}
}



//TODO: Some bones are not bound to the same mesh eg: camera diff mesh etc
_intern void BuildTSkel(aiNode* root,TBoneList* bone_list){

#define _max_nodes 1024

	aiNode* node_array[1024];
	u32 count = 0;

	auto push = [](aiNode** array,u32* count,aiNode* node) -> void{

		_kill("array full\n",*count >= _max_nodes);
		array[*count] = node;
		(*count)++;
	};

	push(node_array,&count,root);

	for(u32 i = 0; i < count; i++){
		auto n = node_array[i];

		TBone bone = {};
		memcpy(&bone.offset,&(n->mTransformation),sizeof(Mat4));

#if !MATRIX_ROW_MAJOR
		bone.offset = Transpose(bone.offset);
#endif


		bone.children_count = n->mNumChildren;
		bone.name = PNewStringCopy(n->mName.data);
		bone.name_hash = PHashString(bone.name);

		bone_list->PushBack(bone);

		for(u32 j = 0; j < n->mNumChildren; j++){
			push(node_array,&count,n->mChildren[j]);
		}
	}

}

_intern void AssimpFillBoneNodeList(aiNode* node,BonenodeList* bonenodelist,aiBone** aibones_array,u32 aibones_count){
    
#if 0
    
    printf("%s %d\n",node->mName.data,node->mNumChildren);
    
    
    
#else
    
#if 0
    
    
    
    for(u32 i = 0; i < bonenodelist->count; i++){
        
        if(PHashString((*bonenodelist)[i].name) == PHashString(node->mName.data)){
            
            printf("%s vs %s\n",(*bonenodelist)[i].name,node->mName.data);
            
            break;
        }
        
    }
    
#endif
    
    AssimpBoneNode bonenode = {};
    
    bonenode.name = node->mName.data;
    bonenode.name_hash = PHashString(node->mName.data);
    bonenode.children_count = node->mNumChildren;
    
    
    //default bone transform. some nodes aren't bound to vertices but are still needed for correct transform
    memcpy(&bonenode.offset,&(node->mTransformation),sizeof(Mat4));
    
#if !MATRIX_ROW_MAJOR
    
    bonenode.offset = Transpose(bonenode.offset);
    
#endif
    
    bonenodelist->PushBack(bonenode);
    
#endif
    
    
    for(u32 i = 0; i < node->mNumChildren;i++){
        
        AssimpFillBoneNodeList(node->mChildren[i],bonenodelist,aibones_array,aibones_count);
    }
    
}


_intern void AssimpBuildSkeleton(aiNode* node,BonenodeList* bonenodelist){
    
    u32 index = FindIndex(node->mName.data,(*bonenodelist));
    
    if(index != (u32)-1){
        
        auto bnode = 
            &(*bonenodelist)[index];
        
        
        for(u32 i = 0; i < bnode->children_count;i++){
            
            bnode->childrenindex_array[i] = FindIndex(node->mChildren[i]->mName.data,*bonenodelist);
        }
        
    }

    else{
	    printf("Not attached node %s\n",node->mName.data);
    }
    
    
    
    for(u32 i = 0; i < node->mNumChildren;i++){
        
        AssimpBuildSkeleton(node->mChildren[i],bonenodelist);
    }
    
    
}

void AssimpBuildSkeleton(aiNode* node,BonenodeList* bonenodelist,aiBone** aibones_array,u32 aibones_count){
    
    AssimpFillBoneNodeList(node,bonenodelist,aibones_array,aibones_count);
    
    AssimpBuildSkeleton(node,bonenodelist);
}



void AssimpRemapBones(AssimpBoneNode* dst,AssimpBoneNode* src,
                      AssimpBoneNode node,u32* skinindexmap,u32* curindex){
    
    /* printf("read index %d\n",*curindex); */
    
    dst[*curindex] = node;
    u32 oldindex;
    
    for(u32 i = 0;;i++){
        if(node.name_hash == src[i].name_hash){
            oldindex = i;
            break;
        }
    }
    
    skinindexmap[oldindex] = *curindex;
    *curindex = (*curindex) + 1;
    
    /* printf("added index %d\n",skinindexmap[oldindex]); */
    
    for(u32 i = 0; i < node.children_count;i++){
        
        AssimpBoneNode child = src[node.childrenindex_array[i]];
        AssimpRemapBones(dst,src,child,skinindexmap,curindex);
        
    }
    
}


//MARK: Test animations


#define _ms2s(ms)  ((f32)ms/1000.0f)


struct VectorTransform{
    Vec4 translation;
    Quat rotation;
    Vec4 scale;
};


VectorTransform _ainline IdentityVectorTransform(){
    VectorTransform transform;
    transform.translation = Vec4{};
    transform.rotation = MQuatIdentity();
    transform.scale = Vec4{1.0f,1.0f,1.0f,1.0f};
    
    return transform;
}


#if 1
Vec4 _ainline ALerpAnimation(AAnimationKey* key_array,u32 key_count,f32 animationtime){
    
    if(key_count ==1){
        return key_array[0].value;
    }
    
    u32 frameindex = 0;
    
    for(u32 i = 0; i < key_count -1;i++){
        
        if(animationtime < key_array[i + 1].time){
            frameindex = i;
            break;
        }
        
    }
    
    //we need to align the data
    AAnimationKey current = key_array[frameindex];
    
    
    AAnimationKey next =
        key_array[(frameindex + 1) % key_count];
    
    f32 step = (animationtime - current.time)/(next.time - current.time);
    
    return LerpVec4(current.value,next.value,step);
}


Quat _ainline  ALerpAnimationQuat(AAnimationKey* key_array,u32 key_count,
                                                     f32 animationtime){

    if(key_count ==1){
        
        Quat ret = Vec4ToQuat(key_array[0].value);
        return ret;
    }
    
    u32 frameindex = 0;
    
    for(u32 i = 0; i < key_count -1;i++){
        
        if(animationtime < key_array[i + 1].time){
            frameindex = i;
            break;
        }
        
    }
    
    AAnimationKey current = key_array[frameindex];
    AAnimationKey next =
        key_array[(frameindex + 1) % key_count];
    
    
    f32 step = (animationtime - current.time)/(next.time - current.time);

    return NLerpQuat(Vec4ToQuat(current.value),Vec4ToQuat(next.value),step);

}

void AssimpTransformSkel(AssimpAnimation channels,f32 animationtime,
                                    AssimpBoneNode* _restrict node,Mat4 parent_matrix,
				    BonenodeList list){
    
    //hand optimize this. the compiler has no idea wtf it's doing
    
    Mat4 matrix;
    VectorTransform transform = IdentityVectorTransform();

    auto get_channel = [](u32 name_hash,AssimpAnimation* channels) -> TAnimChannel* {

	    for(u32 i = 0; i < channels->data_count; i++){

		    auto c = &channels->container[i];

		    if(c->name_hash == name_hash){
			    return c;
		    }
	    }

	    return 0;
    };

    auto create_children = [](AssimpBoneNode* parent,BonenodeList list) -> AssimpBoneNode** {

	    if(parent->children_count){
		    return 0;
	    }
	    
	    auto arr = (AssimpBoneNode**)alloc(sizeof(AssimpBoneNode*) * parent->children_count);
	    for(u32 i = 0; i < parent->children_count;i++){
		    auto index = parent->childrenindex_array[i];
		    arr[i] = &list[index];
	    }


	    return arr;
    };

    auto channel = get_channel(node->name_hash,&channels);
    
    
    if(channel){
        
        
        transform.translation = 
            ALerpAnimation(channel->positionkey_array,
                                  channel->positionkey_count,animationtime);
        
        transform.scale =
            ALerpAnimation(channel->scalekey_array,
                                  channel->scalekey_count,animationtime);
        
        transform.rotation = 
            ALerpAnimationQuat(channel->rotationkey_array,
                                            channel->rotationkey_count,animationtime);
    }
    
    matrix =
        WorldMat4Q(Vec4ToVec3(transform.translation),
                    transform.rotation,
                    Vec4ToVec3(transform.scale));
    
    matrix = parent_matrix * matrix;
    
    node->res = 
#if _row_major
    
        TransposeMat4(matrix * node->offset);
    
#else
    
    matrix * node->offset;
    
#endif
    
    
    for(u32 i = 0; i < node->children_count;i++){
        
        AssimpBoneNode* child = node->children_array[i];
        
        AssimpTransformSkel(channels,animationtime,child,matrix,list);
    }
    
}


void AssimpBlend(f32 time_ms,u32 animation_index,AssimpAnimationList* animation,
                  AssimpBoneNode* root){
    
    TIMEBLOCK(DarkViolet);

    
    AAnimationSet animation = animation[animation_index];
    
    f32 tps =  _either(animation.tps != 0.0f,animation.tps,25.0f);
    f32 ticks = tps * _ms2s(time_ms);
    
    //returns the remainder of x divided by y
    f32 animationtime = fmodf(ticks,animation.duration);
    
    AssimpTransformSkel(animation->channels,animationtime,root,
                                   IdentityMat4());
}

#endif


AssimpData AssimpLoad(const s8* filepath){
    
    AssimpData data = {};
    
    _declare_list(Vec4List,Vec4);
    _declare_list(Vec2List,Vec2);
    _declare_list(U32List,u32);
    
    Vec4List pos_list;
    Vec4List normal_list;
    Vec2List texcoord_list;
    U32List index_list;
    
    pos_list.Init();
    normal_list.Init();
    texcoord_list.Init();
    index_list.Init();
    
    Assimp::Importer importer;
    
    aiScene* scene =
        (aiScene*)importer.ReadFile(filepath, aiProcess_Triangulate | aiProcess_GenSmoothNormals
                                    | aiProcess_OptimizeMeshes);
    
    //TODO: add in support for this
    _kill("Failed to load file\n",!scene);
    
    _kill("ERROR: Multiple meshes currently not supported\n",scene->mNumMeshes > 1);
    
    aiMesh* mesh = scene->mMeshes[0];
    
    if(scene->mNumCameras || scene->mNumLights){
        
        printf("camera count %d light count %d\n",scene->mNumCameras,scene->mNumLights);
        
        printf("WARNING: No cameras or lights allowed. Only meshes\n");
        
    }
    
#if 1
    
    printf("num meshes %d\n",scene->mNumMeshes);
    
    printf("Total vertices %d\n",mesh->mNumVertices);
    printf("Total indices %d\n",mesh->mNumFaces * 3);
    
    printf("Total actual bones %d\n",mesh->mNumBones);
    printf("Total animations %d\n",scene->mNumAnimations);
    
#endif
    
    
    
    if((mesh->mNumBones && !scene->mNumAnimations) || (!mesh->mNumBones && scene->mNumAnimations)){
        
        printf("WARNING: Skeleton doesn't have corresponding animation - vice versa\n");
    }
    
    for(u32 i = 0; i < mesh->mNumVertices;i++){
        
        aiVector3D pos = mesh->mVertices[i];
        aiVector3D normal = mesh->mNormals[i];
        
        aiVector3D texcoord = {};
        
        if(mesh->HasTextureCoords(0)){
            texcoord = mesh->mTextureCoords[0][i];
        }
        
        pos_list.PushBack(Vec4{pos.x,pos.y,pos.z,1.0f});
        normal_list.PushBack(Vec4{normal.x,normal.y,normal.z,1.0f});
        texcoord_list.PushBack(Vec2{texcoord.x,texcoord.y});
        // printf("%f %f\n",texcoord.x,texcoord.y);
    }
    
    for(u32 i = 0; i < mesh->mNumFaces;i++){
        
        aiFace face = mesh->mFaces[i];
        
        _kill("face is not triangle\n",face.mNumIndices != 3);
        
        index_list.PushBack(face.mIndices[0]);
        index_list.PushBack(face.mIndices[1]);
        index_list.PushBack(face.mIndices[2]);
        
    }
    
    _kill("top bit in index count is reserved\n",index_list.count & (1 << 31));
    
    VertexBoneDataList bonedatalist;
    BonenodeList bonenodelist;
    AssimpAnimationList animationlist;
    
    bonenodelist.Init(mesh->mNumBones + _max_bones);
    bonedatalist.Init(mesh->mNumVertices);
    animationlist.Init();




    if(mesh->mNumBones){

        
        bonedatalist.count = mesh->mNumVertices;
        
        //build skeleton from the bones
        AssimpBuildSkeleton(scene->mRootNode,&bonenodelist,mesh->mBones,mesh->mNumBones);

        
        
#if 0
        
        PrintNodeHeirarchy(scene->mRootNode,scene->mRootNode);
        
        printf("-------------------\n");
        
        PrintNodeHeirarchy(bonenodelist[0],bonenodelist);
        
#endif

        
        //MARK: max bones check
        //this is the effective bone count
        _kill("Erorr: exceeds max bone limit\n",bonenodelist.count > _max_bones);
        
        AssimpLoadBoneVertexData(mesh,&bonedatalist,&bonenodelist); //this loads the weights + indices
        
        //get animation data
        AssimpLoadAnimations(scene,&animationlist);


#if 1
	TBoneList bones;
	TAnimList anims;
	TSkinList skins;

	bones.Init();
	anims.Init();

	skins.Init(mesh->mNumVertices);
	memset(skins.container,0,sizeof(TSkin) * mesh->mNumVertices);

	BuildTSkel(scene->mRootNode,&bones);

#if 0

	printf("---------START----------------\n");
	for(u32 i = 0; i < bones.count; i++){
		printf("%s\n",bones[i].name);
	}

	printf("---------END----------------\n");

#endif

	LoadTAnim(scene,&anims,bones);

	LoadTSkin(mesh,&skins,bones);

	_breakpoint();
	u32 a = 1;
#endif
    }
    
    {
        data.vertex_array = pos_list.container;
        data.vertex_count = pos_list.count;
        
        data.texcoord_array = texcoord_list.container;
        data.texcoord_count = texcoord_list.count;
        data.normal_array = normal_list.container;
        data.normal_count = normal_list.count;
        
        data.index_array = index_list.container;
        data.index_count = index_list.count;
        
        data.vertexbonedata_array = bonedatalist.container;
        data.vertexbonedata_count = bonedatalist.count;
        data.bone_array = bonenodelist.container;
        data.bone_count = bonenodelist.count;
        
        data.animation_array = animationlist.container;
        data.animation_count = animationlist.count;
    }
    
    return data;
}

void _ainline PtrCopy(s8** ptr,void* data,u32 size){
    memcpy(*ptr,data,size);
    *ptr += size;
}

u32 AnimBoneSize(AssimpData data,AssimpBoneNode* bones){
    
    u32 size = data.animation_count * (sizeof(u32) + (sizeof(f32) * 2));
    
    for(u32 i = 0; i < data.bone_count;i++){
        
        size = _align16(size);
        
        auto bone = bones[i];
        
        size += sizeof(ALinearBone);
        size += bone.children_count * sizeof(u32);
        size += sizeof(AAnimationData);
        
        AssimpAnimationData array[100];
        u32 array_count = 0;
        
        for(u32 j = 0; j < data.animation_count; j++){
            AssimpAnimation set = data.animation_array[j];
            
            for(u32 k = 0; k < set.data_count; k++){
                AssimpAnimationData keydata = set.data[k];
                
                if(keydata.bone_hash == bone.bone_hash){
                    array[array_count] = keydata;
                    array_count++;
                    _kill("",array_count > 100);
                    break;
                }
            }
        }
        
        size += sizeof(MDFKeyCount) * data.animation_count;
        
        for(u32 b = 0; b < array_count;b++){
            auto anim = array[b];
            
            size += sizeof(AAnimationKey) * anim.positionkey_count;
            size += sizeof(AAnimationKey) * anim.rotationkey_count;
            size += sizeof(AAnimationKey) * anim.scalekey_count;
        }
    }
    
    return _align16(size);
}



//TODO: Consider compressing the data

#define _print_log 1

//smaller footprint
void CreateAssimpToMDF(void** out_buffer,u32* out_buffer_size,AssimpData data,
                       AnimationBlendType blendtype = BLEND_LINEAR){
    
    
    s8* buffer = (s8*)alloc(_megabytes(30));
    
    s8* ptr = buffer;
    
    AssimpBoneNode* bones = 0;
    
    //write the header and vertex component
    {
        u32 header = TAG_MDF;
        PtrCopy(&ptr,&header,sizeof(header));
        
        //Vertex Model Version
        header = 0;
        PtrCopy(&ptr,&header,sizeof(header));
    }
    
    //write vertex data
    {
        
        _kill("Model has no vertex positions\n",data.vertex_count == 0);
        
        //figure out the vertex components
        {
            u16 vertex_component = 0;
            
            if(data.normal_count){
                vertex_component |= VERTEX_NORMAL;
            }
            
            if(data.texcoord_count){
                vertex_component |= VERTEX_TEXCOORD;
            }
            
            if(data.vertexbonedata_count){
                vertex_component |= VERTEX_BONEID_WEIGHT;
                bones = data.bone_array;
            }
            
#if _print_log
            printf("vertex component: %d\n",vertex_component);
#endif
            
            PtrCopy(&ptr,&vertex_component,sizeof(u16));
            
            u32 indversize = (data.vertex_count * sizeof(AVec3)) +
                (data.texcoord_count * sizeof(Vec2)) +
                (data.normal_count * sizeof(AVec3)) +
                (data.vertexbonedata_count * sizeof(VertexBoneData)) +
                data.index_count * sizeof(u32);
            
            u32 animbonesize = AnimBoneSize(data,bones); //FIXME: this is wrong pretty sure
            
#if _print_log
            
            printf("animbone size %d\n",(animbonesize));//MARK:Not exact but will do
            printf("vertindex size %d\n",(indversize));//MARK:Not exact but will do
            
#endif
            
            PtrCopy(&ptr,&indversize,sizeof(indversize));
            PtrCopy(&ptr,&animbonesize,sizeof(animbonesize));
        }
        
        u32 header = TAG_VERTEX;
        u32 datasize =
            (data.vertex_count * sizeof(AVec3)) + //positions
            (data.texcoord_count * sizeof(Vec2)) + //texcoords
            (data.normal_count * sizeof(AVec3)) + //normals
            (data.vertexbonedata_count * sizeof(VertexBoneData)); //boneid and weight
        
        PtrCopy(&ptr,&header,sizeof(header));
        PtrCopy(&ptr,&datasize,sizeof(u32));
        
#if _print_log
        
        printf("Vertices size %d\n",datasize);
        
#endif
        
        //write vertex data
        for(u32 i = 0; i < data.vertex_count;i++){
            
            PtrCopy(&ptr,&data.vertex_array[i],sizeof(AVec3));
            
            if(data.normal_count){
                PtrCopy(&ptr,&data.normal_array[i],sizeof(AVec3));
            }
            
            if(data.texcoord_count){
                PtrCopy(&ptr,&data.texcoord_array[i],sizeof(Vec2));
                //MARK:
                // printf("texcoord%f %f\n",data.texcoord_array[i].x,data.texcoord_array[i].y);
            }
            
            if(data.vertexbonedata_count){
                PtrCopy(&ptr,&data.vertexbonedata_array[i],sizeof(VertexBoneData));
            }
            
        }
    }
    
    //write indices
    {
        u32 header = TAG_INDEX;
        u32 datasize = data.index_count;
        
        
        //optimization - save as u16 if allowed
        
        if(data.index_count <= 65535){
            
#if _print_log
            
            printf("using 16 bit indices\n");
            
#endif
            
            datasize *= sizeof(u16);
            
            auto u16_ptr = (u16*)data.index_array;
            auto u32_ptr = (u32*)data.index_array;
            
            //collapse indices
            for(u32 i = 0; i < data.index_count; i++){
                
                u16_ptr[i] = (u16)u32_ptr[i];
            }
        }
        
        else{
            
#if _print_log
            
            printf("using 32 bit indices\n");
            
#endif
            
            datasize *= sizeof(u32);
        }
        
        PtrCopy(&ptr,&header,sizeof(header));
        PtrCopy(&ptr,&datasize,sizeof(u32));
        PtrCopy(&ptr,data.index_array,datasize);
        
#if _print_log
        
        printf("Index size %d\n",datasize);
        
#endif
        
    }
    
    //write skeleton and animation data if any
    {
        
        if(bones){
            
            u32 header;
            u32 datasize;
            
            //animation set + keys
            {
                header = TAG_ANIM;
                datasize = data.animation_count * (sizeof(u32) + (sizeof(f32) * 2));
                
#if _print_log
                
                printf("animation size %d\n",datasize);
                
#endif
                
                PtrCopy(&ptr,&header,sizeof(header));
                PtrCopy(&ptr,&datasize,sizeof(u32));
                
                for(u32 i = 0; i < data.animation_count;i++){
                    AssimpAnimation set = data.animation_array[i];
                    
                    PtrCopy(&ptr,&set.data_count,sizeof(u32));
                    PtrCopy(&ptr,&set.duration,sizeof(f32));
                    PtrCopy(&ptr,&set.tps,sizeof(f32));
                }
                
            }

            //skeleton
            if(blendtype == BLEND_LINEAR){
                
                header = TAG_BLEND_LINEAR;
                datasize = data.bone_count;
                
                _kill("too many bones (signed bit is reserved)\n",
                      (datasize & (1 << 31)));
                
                //we top bit indicates column major if it is set
#if !MATRIX_ROW_MAJOR
                
                datasize = _addsignedbit(datasize);
                
#endif
                
#if _print_log
                
                printf("bone count %d\n",datasize);
                
#endif
                
                PtrCopy(&ptr,&header,sizeof(header));
                PtrCopy(&ptr,&datasize,sizeof(datasize));
                
                for(u32 i = 0; i < data.bone_count;i++){
                    
                    AssimpBoneNode bone = bones[i];
                    
                    MDFLinearBoneData lbd = {bone.bone_hash,bone.children_count,bone.offset};
                    PtrCopy(&ptr,&lbd,sizeof(MDFLinearBoneData));
                    
                    PtrCopy(&ptr,bone.childrenindex_array,sizeof(u32) * bone.children_count);
                    
                    AssimpAnimationData array[100] = {};
                    u32 array_count = 0;
                    
                    //write keys
                    for(u32 j = 0; j < data.animation_count; j++){
                        AssimpAnimation set = data.animation_array[j];
                        
                        for(u32 k = 0; k < set.data_count; k++){
                            AssimpAnimationData keydata = set.data[k];
                            
                            if(keydata.bone_hash == bone.bone_hash){
                                array[array_count] = keydata;
                                array_count++;
                                _kill("",array_count > 100);
                                break;
                            }
                            
                        }
                        
                    }
                    
                    if(!array_count){
                        for(u32 b = 0; b < data.animation_count;b++){
                            MDFKeyCount keycount = {};
                            PtrCopy(&ptr,&keycount,sizeof(MDFKeyCount));    
                        }
                        
                    }
                    
                    else{
                        for(u32 b = 0; b < data.animation_count;b++){
                            auto anim = array[b];
                            MDFKeyCount keycount = {anim.positionkey_count,anim.rotationkey_count,
                                anim.scalekey_count};
                            PtrCopy(&ptr,&keycount,sizeof(MDFKeyCount));    
                        }
                        
                        for(u32 b = 0; b < data.animation_count;b++){
                            auto anim = array[b];
                            
                            PtrCopy(&ptr,anim.positionkey_array,sizeof(AAnimationKey) * anim.positionkey_count);
                            PtrCopy(&ptr,anim.rotationkey_array,sizeof(AAnimationKey) * anim.rotationkey_count);
                            PtrCopy(&ptr,anim.scalekey_array,sizeof(AAnimationKey) * anim.scalekey_count);    
                        }
                        
                    }
                    
                }
                
            }
            
            else{
                _kill("not supported yet\n",1)
            }
            
        }
    }
    
    
    
    //write material data if any
    
    //write texture data if any
    
    *out_buffer = buffer;
    *out_buffer_size = (((s8*)ptr) - ((s8*)buffer));
}



//smaller footprint
void AssimpWriteMDF(AssimpData data,const s8* filepath,
                    AnimationBlendType blendtype){
    
    s8* buffer;
    u32 buffer_size;
    
    CreateAssimpToMDF((void**)&buffer,&buffer_size,data,blendtype);
    
    
    FileHandle file = FOpenFile(filepath,F_FLAG_WRITEONLY | F_FLAG_CREATE |
                                F_FLAG_TRUNCATE);
    
    FWrite(file,buffer,buffer_size);
    
    unalloc(buffer);
}


void Import(s8** files,u32 count){
    
    printf("Only mdf and adf(WAV) files are supported now\n");
    
    AnimationBlendType blendtype = BLEND_LINEAR;
    
    for(u32 i = 0; i < count; i++){
        
        s8* string = files[i];
        u32 len = strlen(string);
        
        s8 buffer[2048] = {};
        memcpy(buffer,string,len);
        
        auto a = string[len - 3];
        auto b = string[len - 2];
        auto c = string[len - 1];
        
        
        // if(string[0] == '-'){
        //   blendtype = BLEND_DQ;
        //   continue;
        // }
        
        if(isAudio(a,b,c)){
            
            buffer[len - 3] = 'a';
            buffer[len - 2] = 'd';
            buffer[len - 1] = 'f';
            
            WavWriteADF(string,buffer);
        }
        
        if(isModel(a,b,c)){
            
#if MATRIX_ROW_MAJOR
            
            printf("operating in matrix ROW major!\n");
            
#else
            
            printf("operating in matrix COLUMN major!\n");
            
#endif
            
            auto assimp = AssimpLoad(string);

	    exit(0);
            
            buffer[len - 3] = 'm';
            buffer[len - 2] = 'd';
            buffer[len - 1] = 'f';
            
            AssimpWriteMDF(assimp,buffer,blendtype);
            
        }
        
        if(isImage(a,b,c)){
            
            buffer[len - 3] = 't';
            buffer[len - 2] = 'd';
            buffer[len - 1] = 'f';
            
            //Format_RGBA
            CreateTextureAssetTDF(string,buffer,Format_BC1,true,true);
        }
        
    }
    
}
