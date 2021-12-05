#pragma once
#include "ffileio.h"
#include "ttype.h"
#include "mode.h"
#include "mmath.h"
#include "aanimation.h"
#include "aallocator.h"

#include "spx_common.h"

#include "image.h"

#define _encode(a,b,c,d) (u32)  (((u32)(a << 0)) | ((u32)(b << 8)) | ((u32)(c << 16)) | ((u32)(d << 24)))

/*NOTE: All loading functions are considered obsolete and will not be used in the main 
  framework. Loading will be implicitly handled by the asset memory allocator*/

//actual vector3 (not vec4 acting as vec3)
union AVec3{

	float ar[3];

	struct{
		f32 x;
		f32 y;
		f32 z;  
	};

};

struct AssimpBoneNode{
	u32 name_hash;
	Mat4 offset;

	u32 children_count;
	u32 childrenindex_array[10];

#if 1
	s8* name;
	AssimpBoneNode* children_array;
	Mat4 res = {0};
#endif
};

//this is for mapping vertices to bones
struct VertexBoneData{
	u32 bone_index[4];
	f32 bone_weight[4];
};

//We might not even use these
enum AnimationBehaviour{
	ANIMATION_DEFAULT = 0,
	ANIMATION_CONSTANT = 1,
	ANIMATION_LINEAR = 2,
	ANIMATION_REPEAT = 3,
	ANIMATION_FORCE32BIT = 0x8fffffff,
};

struct AnimationKey{
	f32 time;

	//this is perfect to SOA
	Vec4 key;
};


//we will only support skeletal animation
struct AssimpAnimationData{
	s8* name;
	u32 name_hash;
	u32 positionkey_count;
	u32 rotationkey_count;
	u32 scalekey_count;
	AnimationKey* positionkey_array;
	AnimationKey* rotationkey_array;
	AnimationKey* scalekey_array;
	AnimationBehaviour prestate;
	AnimationBehaviour poststate;
};

struct AssimpAnimation{
	//main data
	AssimpAnimationData* data;//this should be contiguous

	//header data
	u32 data_count;
	f32 duration;
	f32 tps;//ticks per second
	s8* name;
};



struct AssimpData{
	Vec4* vertex_array;
	u32 vertex_count;
	Vec2* texcoord_array;
	u32 texcoord_count;
	Vec4* normal_array;
	u32 normal_count;

	void* index_array;
	u32 index_count;

	//skinning info
	VertexBoneData* vertexbonedata_array;
	u32 vertexbonedata_count;

	//boneinfo - bone nodes rest position
	AssimpBoneNode* bone_array;
	u32 bone_count;

	//animation data
	AssimpAnimation* animation_array;
	u32 animation_count;
};


AssimpData AssimpLoad(const s8* filepath);

enum AnimationBlendType{
	BLEND_LINEAR = 0,
	BLEND_DQ,
	BLEND_NONE
};

struct MDFData{
	u16 vertex_component;//TODO: replace this with a vertext hash. 
	void* vertex_data;
	u32 vertex_size;

	u32* index_data;
	u32 index_size;

	AAnimationSet* animationset_array;
	u32 animationset_count;

	ALinearBone* root_linearskeleton;
	ADQBone* root_dqskeleton;
	u32 bone_count;

	//additional data for the memory allocator
	u32 vertexdata_offset;
	u32 indexdata_offset;
	u32 animdata_offset;
	u32 bonedata_offset;
};

//#define VERTEX_POSITION is implied
#define VERTEX_NORMAL 1
#define VERTEX_TEXCOORD 2
#define VERTEX_BONEID_WEIGHT 4
#define VERTEX_MATERIAL 8

void DestroyMDFVertexData(MDFData* data);

void DestroyMDFAnimBoneData(MDFData* data);

void DestroyMDF(MDFData* data);

MDFData LoadMDF(const s8* filepath,void* vertindex,void* animbone,u32* vertindex_size,
		u32* animbone_size);

void FileReadAnimation(FileHandle file,void* data,u32* count,u32 animcount);

void FileReadAnimBoneLinear(FileHandle file,void* data,u32* count,u32 bonecount,
		u32 animsetcount,ALinearBone** root_linearskeleton,
		u32* bone_count);


//NOTE: More of a conceptual structure than an actual structure. We usually
//do not want the whole file in data anyway

/* struct ADFData{ */
/* ADFTags compression_type; */
//u32 file_offset; //NOTE: We don't need this. our offset will always be 8 bytes (ADF header + comp type)
/* }; */


void ADFGetInfo(const s8* file,u16* compression_type,u32* data_size);

void ADFGetData(const s8* file,void* data,u32* offset,u32 size);

struct TDFHeader{
	u32 tag;
	u16 w;
	u16 h;
	f32 bpp; //bytes per pixel
	u8 mips;
	u8 format;
};

#include "vulkan/vulkan.h"
//TODO: support srgb as well
enum TexFormat{
	Format_RGBA = VK_FORMAT_R8G8B8A8_UNORM,
	Format_BC1 = VK_FORMAT_BC1_RGB_UNORM_BLOCK,
	Format_BC1a = VK_FORMAT_BC1_RGBA_UNORM_BLOCK,
	Format_BC2 = VK_FORMAT_BC2_UNORM_BLOCK,
	Format_BC3 = VK_FORMAT_BC3_UNORM_BLOCK,
	Format_BC3n = VK_FORMAT_BC3_UNORM_BLOCK,
	Format_BC4 = VK_FORMAT_BC4_UNORM_BLOCK,
	Format_BC5 = VK_FORMAT_BC5_UNORM_BLOCK,
};

//TODO: support srgb as well
f32 _ainline GetBPP(TexFormat format){

	f32 bpp = 0.0f;

	switch(format){

		case Format_RGBA:{
					 bpp = 4.0f;
				 }break;

		case Format_BC1:{
					bpp = 0.5f;
				}break;

		case Format_BC1a:{
					 bpp = 0.5f;
				 }break;

		case Format_BC2:{
					bpp = 1.0f;
				}break;

		case Format_BC3:{
					bpp = 1.0f;
				}break;

		case Format_BC4:{
					bpp = 0.5f;
				}break;

		case Format_BC5:{
					bpp = 1.0f;
				}break;

	}

	return bpp;
}


void WriteTDF(void* data,u32 w,u32 h,u32 size,u32 mips,TexFormat format,
		const s8* writefilepath);

TDFHeader GetHeaderInfoTDF(const s8* filepath);

void GetTileDataTDF(FileHandle file,u32 w,u32 h,u32 t_x,u32 t_y,u32 mip,f32 bpp,void* data);

/* MARK:These are from glslparser::main.cpp::GenerateShaderTable */

struct SPXData{
	//meta data
	VkShaderStageFlagBits type;
	VertexLayout vlayout;
	VertexLayout instlayout;
	DescLayout dlayout;
	PushConstLayout playout;

	//actual shader data
	void* spv;
	u32 spv_size;
};


SPXData LoadSPX(const s8* filepath);

enum MDFTags{
	TAG_MDF = _encode('M','D','F',' '),
	TAG_VERTEX = _encode('V','E','R','T'),
	TAG_INDEX = _encode('I','N','D','X'),

	//TODO:these two should not be here because they describe operation
	TAG_BLEND_LINEAR = _encode('B','L','I','N'),
	TAG_BLEND_DQ = _encode('B','D','Q',' '),

	TAG_ANIM = _encode('A','N','I','M'),
	TAG_SKEL = _encode('S','K','E','L'),
};

struct MDFLinearBoneData{
	u32 hash;
	u32 count;
	Mat4 matrix;
};

struct MDFKeyCount{
	u32 pos,rot,scale;
};

enum ADFTags{
	TAG_ADF = _encode('A','D','F',' '),
	TAG_UNCOMPRESSED = _encode('U','N','C','S'),
	TAG_VORBIS = _encode('V','O','R','B'),
};

/* We should do some quick basic compression. our files are huge AF

   Audio Dump Format specification
   u32 - "ADF" string encoding
   u32 - Version
   u32 -audio size
   audio dump

   Version -1 48kHz, s16_LE
   Version 0 48kHz, float32_LE

   Model Dump Format specification
   u32 - "MDF" string encoding
   u32 - Static Vertex model version
   u16 - Vertex component bitfield

   u32 index + vertex size
   u32 bones + anim size

   Vertex buffer dump
   Index data dump
   Skeleton dump
   animation dump
   material dump


   -(Version 0 Position,Normal,Texcoord,BoneIDWeight)

   Vertex Data Components
   Vertex data is written in the order of the corresponding version number
   if a component as stated by the vertex version number is not available, the corresponding bitfield
   is marked zero (NOTE: Position is implied so the first bit is from the 2nd component onwards. EG:
   Texcoord in Version 0) and that component is skipped. The user is expected to follow the Vertex 
   structure provided by the file instead of traditionally constructing the vertex structure from 
   separate vertex data components. Thus, in MDF the user only has to copy all the vertices as is.

   Skeleton and Keys
   The animation keys are stored together with its corresponding animation node. This makes data 
   traversal more linear and should allow for better performance(Maybe. Based on what I know abt
   cache lines)

   a dump consists of
   u32 tag
   u32 size
   data of 'size' bytes
   */
