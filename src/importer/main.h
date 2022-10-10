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

#include "pprint.h"

#include "aanimation.h"

//FIXME: I think the write is wrong somewhere. The allocator is failing because it is trying to allocated a 4GB block


struct WriteSizeBlock{
	void** ptr = 0;
	s8* start = 0;
	s8* name = 0;
	s8* file = 0;
	s8* function = 0;
	u32 line = 0;
	WriteSizeBlock(void** ptr,const s8* name,const s8* file,const s8* function,u32 line){
		this->ptr = ptr;
		start = (s8*)(*ptr);
		this->name = (s8*)name;
		this->function = (s8*)function;
		this->line = line;
		this->file = (s8*)file;
	}
	~WriteSizeBlock(){
		auto end = (s8*)(*ptr);
		u32 size = end - start;
		auto name = this->name ? this->name : "";
		printf("%s %s %d (%s): Written %d bytes\n",file,function,line,name,size);
	}
};


#define WRITEBLOCK(ptr) WriteSizeBlock t_##__LINE__((void**)ptr,0,__FILE__,__FUNCTION__,__LINE__)

#define WRITEBLOCKTAGGED(ptr,name) WriteSizeBlock t_##__LINE__((void**)ptr,name,__FILE__,__FUNCTION__,__LINE__)

#define _hash(a,b,c) (a * 1) + (b * 2) + (c * 3)
#define _encode(a,b,c,d) (u32)  (((u32)(a << 0)) | ((u32)(b << 8)) | ((u32)(c << 16)) | ((u32)(d << 24)))


//TODO: we should have a notion of a string pool to allocation string from and to delete all strings

//TODO:this will used to offset and access each of the channels
//should this be part of anim???
struct KeyCount{
	u16 offset;
	u16 pos_count;
	u16 rot_count;
	u16 scale_count;
};

_ainline s8* PNewStringCopy(s8* string){

	u32 len =  PStrLen(string) + 1;

	if(len == 1){
		return 0;
	}

	s8* nstring = (s8*)alloc(len);

	memcpy(nstring,string,len);

	return nstring;
}



//when writing, we will seprate the counts from the data
struct TAnimChannel{
	u32 positionkey_count;
	u32 rotationkey_count;
	u32 scalekey_count;
	AAnimationKey* positionkey_array;
	AAnimationKey* rotationkey_array;
	AAnimationKey* scalekey_array;

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
};

struct TSkin{
	u32 index[4];
	f32 weight[4];
};

_declare_list(TBoneList,TBone);
_declare_list(TAnimList,TAnim);
_declare_list(TSkinList,TSkin);


//TODO: remove these

union AVec3{

	float ar[3];

	struct{
		f32 x;
		f32 y;
		f32 z;  
	};

};


//We might not even use these
enum AnimationBehaviour{
	ANIMATION_DEFAULT = 0,
	ANIMATION_CONSTANT = 1,
	ANIMATION_LINEAR = 2,
	ANIMATION_REPEAT = 3,
	ANIMATION_FORCE32BIT = 0x8fffffff,
};



enum AnimationBlendType{
	BLEND_LINEAR = 0,
	BLEND_DQ,
	BLEND_NONE
};
struct InterMDF{
	Vec4* vertex_array;
	u32 vertex_count;
	Vec2* texcoord_array;
	u32 texcoord_count;
	Vec4* normal_array;
	u32 normal_count;

	void* index_array;
	u32 index_count;


	TBone* bone_array;
	u32 bone_count;

	TAnim* anim_array;
	u32 anim_count;

	TSkin* skin_array;
	u32 skin_count;
};


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
		(_hash(a,b,c) == _hash('3','d','s')) || (_hash(a,b,c) == _hash('f','b','x')) || 
		(_hash(a,b,c) == _hash('l','t','f'))
		;
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

void PrintNodeHeirarchy(aiNode* node,aiNode* root){

	printf("%s %d\n",node->mName.data,node->mNumChildren);

	for(u32 i = 0; i < node->mNumChildren;i++){

		PrintNodeHeirarchy(node->mChildren[i],root);
	}

}



void DestroyInterMDF(InterMDF data){

	unalloc(data.vertex_array);
	unalloc(data.texcoord_array);
	unalloc(data.normal_array);
	unalloc(data.index_array);
	unalloc(data.bone_array);

	//FIXME: this needs to be redone!
#if 0
	for(u32 i = 0; i < data.animation_count;i++){

		for(u32 j = 0; j < data.animation_array[i].data_count;j++){
			unalloc(data.animation_array[i].data[j].positionkey_array);
		}

		unalloc(data.animation_array[i].data);

	}

	unalloc(data.animation_array);
#else
	_breakpoint();
#endif

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

		//printf("WARNING: Vertes has more than 4 weights!\n");
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
				(AAnimationKey*)alloc(sizeof(AAnimationKey) *
						(node->mNumPositionKeys + node->mNumRotationKeys +
						 node->mNumScalingKeys));

			channel-> rotationkey_array = channel-> positionkey_array + node->mNumPositionKeys;
			channel-> scalekey_array = channel-> rotationkey_array + node->mNumRotationKeys;

			for(u32 k = 0; k < node->mNumPositionKeys;k++){

				aiVectorKey vec = node->mPositionKeys[k];
				AAnimationKey key = {(f32)vec.mTime,{vec.mValue.x,vec.mValue.y,vec.mValue.z,1.0f}};

				memcpy(channel->positionkey_array + k,&key,sizeof(AAnimationKey));
			}

			for(u32 k = 0; k < node->mNumRotationKeys;k++){

				aiQuatKey q = node->mRotationKeys[k];
				AAnimationKey key =
				{(f32)q.mTime,{q.mValue.x,q.mValue.y,q.mValue.z,q.mValue.w}};

				memcpy(channel->rotationkey_array + k,&key,sizeof(AAnimationKey));
			}

			for(u32 k = 0; k < node->mNumScalingKeys;k++){
				aiVectorKey vec = node->mScalingKeys[k];
				AAnimationKey key = {(f32)vec.mTime,{vec.mValue.x,vec.mValue.y,vec.mValue.z,1.0f}};

				memcpy(channel->scalekey_array + k,&key,sizeof(AAnimationKey));
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
	auto current = key_array[frameindex];


	auto next =
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

	auto current = key_array[frameindex];
	auto next =
		key_array[(frameindex + 1) % key_count];


	f32 step = (animationtime - current.time)/(next.time - current.time);

	return NLerpQuat(Vec4ToQuat(current.value),Vec4ToQuat(next.value),step);

}


void TLinearBlend(u32 index,f32 time_ms,TBoneList skel,TAnimList anims,
		Mat4* _restrict res){

	u32 res_count = 0; 

	auto anim = anims[index];
	auto tps = anim.tps != 0.0f ? anim.tps : 25.0f;
	auto ticks = tps * _ms2s(time_ms);

	auto anim_time = fmodf(ticks,anim.duration);

	auto channels = anim.channels;

	auto add_res = [](Mat4* _restrict res,u32* _restrict count,Mat4 node)-> void{
		res[*count] =  node;
		(*count)++;
	};

	add_res(res,&res_count, IdentityMat4());

	for(u32 i = 0; i < skel.count; i++){
		auto bone = skel[i];
		auto channel = channels[i];

		_kill("Not matching nodes!\n",bone.name_hash != channel.name_hash &&
				channel.name);

		Mat4 matrix = {};
		auto transform = IdentityVectorTransform();

		auto total_keys = channel.positionkey_count + 
			channel.rotationkey_count + channel.scalekey_count;

		if(total_keys){
			transform.translation = 
				ALerpAnimation(channel.positionkey_array,
						channel.positionkey_count,anim_time);

			transform.scale =
				ALerpAnimation(channel.scalekey_array,
						channel.scalekey_count,anim_time);

			transform.rotation = 
				ALerpAnimationQuat(channel.rotationkey_array,
						channel.rotationkey_count,anim_time);
		}


		matrix =
			WorldMat4Q(Vec4ToVec3(transform.translation),
					transform.rotation,
					Vec4ToVec3(transform.scale));

		auto parent_matrix = res[i];

		matrix = parent_matrix * matrix;
		res[i] = 
#if _row_major

			TransposeMat4(matrix * bone.offset);

#else

		matrix * bone.offset;

#endif

		for(u32 j = 0; j < bone.children_count; j++){
			add_res(res,&res_count,matrix);
		}
	}
}

#endif


InterMDF AssimpLoad(const s8* filepath){

	InterMDF data = {};

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




	TBoneList bones;
	TAnimList anims;
	TSkinList skins;

	bones.Init();
	anims.Init();
	skins.Init(mesh->mNumVertices);
	memset(skins.container,0,sizeof(TSkin) * mesh->mNumVertices);

	if(mesh->mNumBones){
		BuildTSkel(scene->mRootNode,&bones);
		LoadTAnim(scene,&anims,bones);
		LoadTSkin(mesh,&skins,bones);
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
		data.bone_array = bones.container;
		data.bone_count = bones.count;

		data.anim_array = anims.container;
		data.anim_count = anims.count;

		data.skin_array = skins.container;
		data.skin_count = skins.count;
	}

	return data;
}

void _ainline PtrCopy(s8** ptr,void* data,u32 size){
	memcpy(*ptr,data,size);
	*ptr += size;
}

#define _string_block 16
void GetAnimBoneSize(InterMDF data,u32* animsize,u32* skelsize,u32* channelssize,b32 use_names = true){
	u32 string_block = use_names? _string_block : 0;

	auto bone_count = data.bone_count;
	auto anim_count = data.anim_count;

	auto skel_size = bone_count * 
		(
		 sizeof(TBone::offset) + sizeof(TBone::children_count)
		 + string_block
		);

	auto anim_size = anim_count * 
		(sizeof(TAnim::tps) + sizeof(TAnim::duration) + string_block);

	auto channels_size = anim_count * bone_count * sizeof(KeyCount);

	for(u32 i = 0; i < anim_count; i++){
		auto a = data.anim_array[i];

		for(u32 j = 0; j < a.channels.count; j++){
			auto c = a.channels[j];
			channels_size += ((c.positionkey_count + c.rotationkey_count + 
						c.scalekey_count) * sizeof(AAnimationKey));		}

	}

	*animsize = anim_size;
	*skelsize = skel_size;
	*channelssize = channels_size;
}



#define _print_log 1

_intern u32 ProcessIndices(InterMDF data){
	u32 size = sizeof(u32);
	if(data.index_count <= 65535){

		printf("using 16 bit indices\n");

		size = sizeof(u16);

		auto u16_ptr = (u16*)data.index_array;
		auto u32_ptr = (u32*)data.index_array;

		//collapse indices
		for(u32 i = 0; i < data.index_count; i++){
			u16_ptr[i] = (u16)u32_ptr[i];
		}
	}

	else{
		printf("using 32 bit indices\n");
	}

	return size;
}


void CreateMDFContent(void** out_buffer,u32* out_buffer_size,InterMDF data,
		AnimationBlendType blendtype = BLEND_LINEAR,b32 use_names = true){

	s8* buffer = (s8*)alloc(_megabytes(30));
	s8* ptr = buffer;
	u32 index_size = ProcessIndices(data);

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

			if(data.skin_array){
				vertex_component |= VERTEX_BONEID_WEIGHT;
			}

#if _print_log
			printf("vertex component: %d\n",vertex_component);
#endif

			PtrCopy(&ptr,&vertex_component,sizeof(u16));

			u32 vert_size = (data.vertex_count * sizeof(AVec3)) +
				(data.texcoord_count * sizeof(Vec2)) +
				(data.normal_count * sizeof(AVec3)) +
				(data.skin_count * sizeof(TSkin));
			vert_size = _devicealign(vert_size);

			u32 ind_size = data.index_count * index_size;

			u32 anim_size = 0;
			u32 skel_size = 0;
			u32 channels_size = 0;

			GetAnimBoneSize(data,&anim_size,&skel_size,&channels_size,use_names);


#if _print_log

			printf("animbone size %d\n",(anim_size + skel_size + channels_size));
			printf("vertindex size %d\n",(vert_size + ind_size));

#endif

			PtrCopy(&ptr,&vert_size,sizeof(vert_size));
			PtrCopy(&ptr,&ind_size,sizeof(ind_size));

			PtrCopy(&ptr,&anim_size,sizeof(anim_size));
			PtrCopy(&ptr,&skel_size,sizeof(skel_size));
			PtrCopy(&ptr,&channels_size,sizeof(channels_size));

		}

		u32 header = TAG_VERTEX;
		u32 datasize =
			(data.vertex_count * sizeof(AVec3)) + //positions
			(data.texcoord_count * sizeof(Vec2)) + //texcoords
			(data.normal_count * sizeof(AVec3)) + //normals
			(data.skin_count * sizeof(TSkin)); //boneid and weight

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

			if(data.skin_count){
				PtrCopy(&ptr,&data.skin_array[i],sizeof(TSkin));
			}

		}
	}

	//write indices
	{
		u32 header = TAG_INDEX;
		u32 datasize = data.index_count * index_size;

		PtrCopy(&ptr,&header,sizeof(header));
		PtrCopy(&ptr,&datasize,sizeof(u32));
		PtrCopy(&ptr,data.index_array,datasize);

#if _print_log
		printf("Index size %d\n",datasize);
#endif
	}


	//write skeleton and animation data if any
	//MARK: rewrite
	if(data.bone_count){



		//u32* bones_ch;
		//Mat4 bones_offsets;
		//u32 bones_count;

		u32 header = TAG_SKEL;
		u32 datasize = data.bone_count;

		PtrCopy(&ptr,&header,sizeof(header));
		PtrCopy(&ptr,&datasize,sizeof(u32));

		{
			WRITEBLOCKTAGGED(&ptr,"SKEL");

			// children count first. this is essentially the tree structure
			for(u32 i = 0; i < data.bone_count; i++){
				auto c = data.bone_array[i].children_count;
				PtrCopy(&ptr,&c,sizeof(c));
			}

			// offsets
			for(u32 i = 0; i < data.bone_count; i++){
				auto off = data.bone_array[i].offset;
				PtrCopy(&ptr,&off,sizeof(off));
			}

			//name of the nodes. limit these to 16 char buffs
			for(u32 i = 0; i < data.bone_count; i++){
				auto s = data.bone_array[i].name;
				s8 buffer[_string_block] = {};

				u32 cpy_size = PStrLen(s) > (_string_block - 1) ? (_string_block - 1) : PStrLen(s);
				memcpy(buffer,s,cpy_size);

				PtrCopy(&ptr,buffer,sizeof(buffer));
			}
		}

		//Anim* anim_array;
		//u32 anim_count;

		//AnimChannel* channels; //[anim_count][bones_count]
		//s8** bones_names;
		//u32* bones_namehash;
	}

	if(data.anim_count){
		u32 header = TAG_ANIM;
		u32 datasize = data.anim_count;

		PtrCopy(&ptr,&header,sizeof(header));
		PtrCopy(&ptr,&datasize,sizeof(u32));

		{
			WRITEBLOCKTAGGED(&ptr,"ANIM");
			for(u32 i = 0; i < data.anim_count; i++){
				struct {
					f32 tps;
					f32 duration;
				} an;
				auto a = data.anim_array[i];
				an.tps = a.tps;
				an.duration = a.duration;

				PtrCopy(&ptr,&an,sizeof(an));
			}

			for(u32 i = 0; i < data.anim_count; i++){
				auto s = data.anim_array[i].name;
				s8 buffer[_string_block] = {};

				u32 cpy_size = PStrLen(s) > (_string_block - 1) ? (_string_block - 1) : PStrLen(s);
				memcpy(buffer,s,cpy_size);

				PtrCopy(&ptr,buffer,sizeof(buffer));
			}
		}





		//write the channel sets
		header = TAG_CHANNELS;
		u32 anim_count = data.anim_count; // this is the number of channel sets
		u32 bone_count = data.bone_count; // this is the number of channels

		PtrCopy(&ptr,&header,sizeof(header));
		PtrCopy(&ptr,&anim_count,sizeof(u32));
		PtrCopy(&ptr,&bone_count,sizeof(u32));

		{
			WRITEBLOCKTAGGED(&ptr,"CHANNELS");
			u32 offset = 0;

			// NOTE: channel keys
			for(u32 i = 0; i < anim_count; i++){

				auto a = data.anim_array[i];
				auto ch = a.channels;

				for(u32 j = 0; j < bone_count; j++){
					auto c = ch[j];
					auto p = c.positionkey_count;
					auto r = c.rotationkey_count;
					auto s = c.scalekey_count;

					//NOTE: we are assuming given any of the members, the members either can be 0
					//or if it is non-zero, it would have the same value as the other non-zero fields

					_kill("Cannot narrow\n", p > 65535);
					_kill("Cannot narrow\n", r > 65535);
					_kill("Cannot narrow\n", s > 65535);

					KeyCount k = {
						(u16)offset,
						(u16)p,
						(u16)r,
						(u16)s
					};
					PtrCopy(&ptr,&k,sizeof(k));

					offset += p + r + s;
				}
			}


			//NOTE: channel data
			u32 data_size = offset; // NOTE: this would be the total keys
			PtrCopy(&ptr,&data_size,sizeof(u32));


			for(u32 i = 0; i < anim_count; i++){

				auto a = data.anim_array[i];
				auto ch = a.channels;

				for(u32 j = 0; j < bone_count; j++){
					auto c = ch[j];
					auto p = c.positionkey_count;
					auto r = c.rotationkey_count;
					auto s = c.scalekey_count;

					auto m = Max(Max(p,s),r);

					for(u32 k = 0; k < m; k++){
						if(k < p){
							PtrCopy(&ptr,&c.positionkey_array[k],sizeof(AAnimationKey));
						}
						if(k < r){
							PtrCopy(&ptr,&c.rotationkey_array[k],sizeof(AAnimationKey));
						}
						if(k < s){
							PtrCopy(&ptr,&c.scalekey_array[k],sizeof(AAnimationKey));
						}
					}

				}
			}
		}





	}


	//write material data if any

	//write texture data if any

	*out_buffer = buffer;
	*out_buffer_size = (((s8*)ptr) - ((s8*)buffer));
}

//smaller footprint
void AssimpWriteMDF(InterMDF data,const s8* filepath,
		AnimationBlendType blendtype){

	s8* buffer;
	u32 buffer_size;

	CreateMDFContent((void**)&buffer,&buffer_size,data,blendtype);

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
		u32 len = PStrLen(string);

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



struct MDF{
};

void InternalLoadMDF(const s8* path,u16* vertcomp, void* vertind,void* animbone,
		u32* vertind_size, u32* animbone_size){

	MDF mdf = {};

	auto file = FOpenFile(path, F_FLAG_READONLY);
	auto end = FGetFileSize(file);

	u32 header = 0;
	u32 version = 0;

	FRead(file,&header,sizeof(header));
	FRead(file,&version,sizeof(version));

	u16 comp = 0;

	u32 vert_size = 0;
	u32 ind_size = 0;
	u32 anim_size = 0;
	u32 skel_size = 0;
	u32 channels_size = 0;

	s8* vert = 0;
	s8* ind = 0;
	s8* bone = 0;
	s8* anim = 0;
	s8* channels = 0;

	if(header != TAG_MDF){
		goto __fin;
	}

	FRead(file,&comp,sizeof(comp));
	FRead(file,&vert_size,sizeof(vert_size));
	FRead(file,&ind_size,sizeof(ind_size));
	FRead(file,&anim_size,sizeof(anim_size));
	FRead(file,&skel_size,sizeof(skel_size));
	FRead(file,&channels_size,sizeof(channels_size));

	if(vertcomp){
		*vertcomp = comp;
	}

	if(vertind_size){
		*vertind_size = vert_size + ind_size;
	}

	if(animbone_size){
		*animbone_size = anim_size + skel_size + channels_size;
	}


	if(vertind){
		vert = (s8*)vertind;
		ind = (s8*)(vertind) + vert_size;
	}

	else{
		goto __fin;
	}

	if(animbone){
		//TODO: actually set this
		bone = (s8*)animbone;
		anim = (s8*)animbone;
		channels = (s8*)animbone;
	}

	while(FCurFilePosition(file) < end){
		u32 tag = 0;
		FRead(file,&tag,sizeof(tag));

		switch(tag){
			case TAG_VERTEX: {
						 u32 size = 0;
						 FRead(file,&size,sizeof(size));
						 FRead(file,vert,size);
					 }break;

			case TAG_INDEX: {
						u32 size = 0;
						FRead(file,&size,sizeof(size));
						FRead(file,ind,size);
					}break;

			case TAG_SKEL: {
					       u32 count = 0;
					       FRead(file,&count,sizeof(count));
					       u32 * children = 0;
					       Mat4* offset = 0;
					       s8* names = 0;

					       FRead(file,children,sizeof(TBone::children_count) * count);
					       FRead(file,offset,sizeof(TBone::offset) * count);
					       FRead(file,names,_string_block * count);

				       }break;
			case TAG_ANIM: {
					       u32 count = 0;
					       FRead(file,&count,sizeof(count));

					       //TODO: make this a proper struct
					       struct An{
						       f32 t,d;
					       };

					       An* anim = 0;
					       s8* names = 0;

					       FRead(file,anim,sizeof(An) * count);
					       FRead(file,names,_string_block * count);

				       }break;

			case TAG_CHANNELS:
				       {
					       u32 anim_count = 0, bone_count = 0;
					       FRead(file,&anim_count,sizeof(anim_count));
					       FRead(file,&bone_count,sizeof(bone_count));

					       KeyCount* keycounts = 0;
					       FRead(file,keycounts,sizeof(KeyCount) * anim_count * bone_count);

					       u32 key_count = 0;
					       FRead(file,&key_count,sizeof(key_count));

					       AAnimationKey* keys = 0;
					       FRead(file,keys,sizeof(AAnimationKey) * key_count);
				       }break;

			default:{
					_kill("",1);
				}break;
		}
	}

__fin:

	FCloseFile(file);
}
