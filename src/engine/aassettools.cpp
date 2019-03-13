#include "aassettools.h"
#include <assimp/Importer.hpp> 
#include <assimp/scene.h>     
#include <assimp/postprocess.h>
#include <assimp/cimport.h>
#include "pparse.h"

/*
  Formats store entire scenes. We should take this into account
*/



struct MDFChunk{
    u32 tag;
    u32 size;
};

void MDFPrintSkeleton(ALinearBone* bone){
    printf("count %d\n",bone->children_count);
    
    for(u32 i = 0; i < bone->children_count;i++){
        MDFPrintSkeleton(bone->children_array[i]);
    }
}
void DestroyVertexMDFData(MDFData* data){
    
    unalloc(data->vertex_data);
    unalloc(data->index_data);
    
    data->vertex_data = 0;
    data->index_data = 0;  
}

void DestroyMDFVertexData(MDFData* data){
    
    unalloc(data->vertex_data);
    unalloc(data->index_data);
    
    data->vertex_data = 0;
    data->index_data = 0;  
}

void DestroyMDFAnimBoneData(MDFData* data){
    unalloc(data->animationset_array);
}

void DestroyMDF(MDFData* data){
    DestroyMDFAnimBoneData(data);
    DestroyMDFVertexData(data);
}

void FileReadAnimation(FileHandle file,void* data,u32* count,u32 animcount){
    FRead(file,data,animcount * sizeof(AAnimationSet));
    (*count) += animcount * sizeof(AAnimationSet);
}

void FileReadAnimBoneLinear(FileHandle file,void* data,u32* count,u32 bonecount,
                            u32 animsetcount,ALinearBone** root_linearskeleton,
                            u32* bone_count){
    
#if MATRIX_ROW_MAJOR
    
    _kill("asset matrix layout is incompatible (Asset is COLUMN major)\n",
          (bonecount & (1 << 31)));
    
#else
    
    _kill("asset matrix layout is incompatible (Asset is ROW major)\n",!(bonecount & (1 << 31)));
    
    bonecount = _removesignedbit(bonecount);
    
#endif
    
    auto data_count = *count;
    auto data_ptr = (s8*)data;
    
    u32 childrenindex_array[300];
    u32 childrenindex_count = 0;
    
    ALinearBone* tree[_max_bones];
    u32 tree_count = 0;
    
    for(u32 i = 0; i < bonecount;i++){
        
        data_count = _align16(data_count);
        
        auto bone = (ALinearBone*)(data_ptr + data_count);
        tree[tree_count] = bone;
        tree_count++;
        
        MDFLinearBoneData bonedata;
        FRead(file,&bonedata,sizeof(MDFLinearBoneData));
        
        bone->offset = bonedata.matrix;
        bone->children_count = bonedata.count;
        data_count += sizeof(ALinearBone);
        
        
        
        FRead(file,(childrenindex_array + childrenindex_count),sizeof(u32) * bone->children_count);
        childrenindex_count+= bone->children_count;
        
        u32 animcount = animsetcount;
        
        bone->animationdata_array = (AAnimationData*)(data_ptr + data_count);
        data_count += animcount * sizeof(AAnimationData);
        
        for(u32 j = 0; j < animcount; j++){
            
            MDFKeyCount keydata;
            FRead(file,&keydata,sizeof(MDFKeyCount));
            
            bone->animationdata_array[j].positionkey_count = keydata.pos;
            bone->animationdata_array[j].rotationkey_count = keydata.rot;
            bone->animationdata_array[j].scalekey_count = keydata.scale;
        }
        for(u32 j = 0; j < animcount; j++){
            
            auto anim = &bone->animationdata_array[j];
            
            if(anim->positionkey_count){
                
                data_count = _align16((ptrsize)(data_ptr + data_count)) - (ptrsize)data_ptr;
                
                anim->positionkey_array = (AAnimationKey*)(data_ptr + data_count);
                FRead(file,anim->positionkey_array,anim->positionkey_count * sizeof(AAnimationKey));
                data_count += anim->positionkey_count * sizeof(AAnimationKey);
                
                
                anim->rotationkey_array = (AAnimationKey*)(data_ptr + data_count);
                FRead(file,anim->rotationkey_array,anim->rotationkey_count * sizeof(AAnimationKey));
                data_count += anim->rotationkey_count * sizeof(AAnimationKey);
                
                anim->scalekey_array = (AAnimationKey*)(data_ptr + data_count);
                FRead(file,anim->scalekey_array,anim->scalekey_count * sizeof(AAnimationKey));
                data_count += anim->scalekey_count * sizeof(AAnimationKey);
                
            }
            
            else{
                data_count -= animcount * sizeof(AAnimationData);
                bone->animationdata_array = 0;
                break;
            }
            
        }
        
        bone->children_array = (ALinearBone**)(data_ptr + data_count);
        data_count += bone->children_count * sizeof(ALinearBone**);
    }
    
    u32 index = 0;
    
    //construct skeleton
    for(u32 i = 0; i < bonecount;i++){
        
        ALinearBone* bone = tree[i];
        
        for(u32 j = 0; j < bone->children_count; j++){
            bone->children_array[j] = tree[childrenindex_array[index]];
            index++;
            _kill("array overflow\n",index >= 500)
        }
        
    }
    
    //NOTE: first node in the list is always the root node
    *root_linearskeleton = tree[0];
    *bone_count = bonecount;
    
    (*count) = data_count;
}



MDFData LoadMDF(const s8* filepath,void* vertindex,void* animbone,u32* vertindex_size,
                u32* animbone_size){
    
    MDFData data_mdf;
    
    auto file = FOpenFile(filepath,F_FLAG_READONLY);
    
    ptrsize end = FGetFileSize(file);
    
    s8 tbuffer[100];
    
    FRead(file,tbuffer,sizeof(u32));
    
    _kill("not an mdf file\n",*((u32*)&tbuffer[0]) != TAG_MDF);
    
    FRead(file,tbuffer,sizeof(u32));
    
    _kill("Not Vertex Model 0\n",*((u32*)&tbuffer[0]) != 0);
    
    FRead(file,&data_mdf.vertex_component,sizeof(data_mdf.vertex_component));
    
    if(!(vertindex)){
        FRead(file,vertindex_size,sizeof(u32));
        FRead(file,animbone_size,sizeof(u32));
        
        FCloseFile(file);
        return {};
    }
    
    u32 t;
    
    FRead(file,&t,sizeof(u32));
    FRead(file,&t,sizeof(u32));
    
    auto data_ptr = (s8*)animbone;
    u32 data_count = 0;
    
    while(FCurFilePosition(file) < end){
        
        auto chunk = (MDFChunk*)tbuffer;
        
        FRead(file,chunk,sizeof(MDFChunk));
        
        switch(chunk->tag){
            
            case TAG_VERTEX:{
                
                data_mdf.vertexdata_offset = FCurFilePosition(file);
                data_mdf.vertex_size = chunk->size;
                data_mdf.vertex_data = vertindex;
                FRead(file,data_mdf.vertex_data,chunk->size);
                
                
            }
            break;
            
            case TAG_INDEX:{
                
                data_mdf.indexdata_offset = FCurFilePosition(file);
                data_mdf.index_size = chunk->size;
                data_mdf.index_data = (u32*)(((s8*)(data_mdf.vertex_data)) + data_mdf.vertex_size);
                
                FRead(file,data_mdf.index_data,chunk->size);
            }
            break;
            
            case TAG_ANIM:{
                
                data_mdf.animdata_offset = FCurFilePosition(file);
                data_mdf.animationset_array = (AAnimationSet*)data_ptr;
                data_mdf.animationset_count = chunk->size/sizeof(AAnimationSet);
                
                FileReadAnimation(file,data_mdf.animationset_array,&data_count,
                                  data_mdf.animationset_count);
            }
            break;
            
            case TAG_BLEND_LINEAR:{
                
                data_mdf.bonedata_offset = FCurFilePosition(file);
                
                FileReadAnimBoneLinear(file,data_ptr,&data_count,chunk->size,
                                       data_mdf.animationset_count,&data_mdf.root_linearskeleton,
                                       &data_mdf.bone_count);
                
                // printf("wrote %d\n",data_count);
            }
            break;
            
            //TODO: Need read and write for dq
            case TAG_BLEND_DQ:{
                goto skipall;
            }
            break;
            
            default:{
                goto skipall;
            }
            
        }
        
    }
    
    skipall:
    
    FCloseFile(file);
    
    return data_mdf;
}


void ADFGetInfo(const s8* filepath,u16* compression_type,u32* data_size){
    
    auto file = FOpenFile(filepath,F_FLAG_READONLY);
    
    u32 read_int;
    
    FRead(file,&read_int,sizeof(read_int));
    
    _kill("Not an ADF file\n",read_int != TAG_ADF);
    
    FRead(file,compression_type,sizeof(*compression_type));
    
    auto curpos = FCurFilePosition(file);
    
    *data_size = (u32)(FSeekFile(file,0,F_METHOD_END) - curpos);
    
    FCloseFile(file);
}


void ADFGetData(const s8* filepath,void* data,u32* offset,u32 size){
    
    //TODO: Why not just keep the file open
    
#ifndef IS_IMPORTER
    
    TIMEBLOCK(GhostWhite);
    
#endif
    
    FileHandle file;
    {
#ifndef IS_IMPORTER
        TIMEBLOCKTAGGED("FOPEN",Crimson);
#endif
        file = FOpenFile(filepath,F_FLAG_READONLY); 
    }
    
    u32 offset_t = *offset;
    
    offset_t += 8;
    
    {
#ifndef IS_IMPORTER
        TIMEBLOCKTAGGED("FSEEK",Yellow);
#endif
        FSeekFile(file,offset_t,F_METHOD_START);
    }
    
    {
#ifndef IS_IMPORTER
        TIMEBLOCKTAGGED("FREAD",Salmon);
#endif
        FRead(file,data,size);
    }
    
    *offset = offset_t - 8 + size;
    
    FCloseFile(file);
}




TDFHeader GetHeaderInfoTDF(const s8* filepath){
    
    TDFHeader header = {};
    
    auto file = FOpenFile(filepath,F_FLAG_READONLY);
    
    FRead(file,&header,sizeof(TDFHeader));
    
    FCloseFile(file);
    
    return header;
}

void InternalGetTileData(FileHandle file,u32 offset,f32 bpp,void* data){
    
    u32 pos = offset + sizeof(TDFHeader);
    
    FSeekFile(file,pos,F_METHOD_START);
    
    FRead(file,data,128 * 128 * bpp);
}


void GetTileDataTDF(FileHandle file,u32 w,u32 h,u32 t_x,u32 t_y,u32 mip,f32 bpp,void* data){
    
    u32 t_w = w >> 7;
    u32 t_h = h >> 7;
    
    u32 pos = 0;
    
    for(u32 i = 0; i < mip; i++){
        pos += t_w * t_h;
        
        t_h >>= 1;
        t_w >>= 1;
    }
    
    pos += (t_y * t_w) + t_x;
    pos *= 128 * 128 * bpp;
    
    InternalGetTileData(file,pos,bpp,data);
}

/* MARK:These are from glslparser::main.cpp::GenerateShaderTable */
enum LayoutType : u32{
    LayoutType_IN = PHashString("in"),
    LayoutType_DESC = PHashString("set"),
    LayoutType_PUSHCONST = PHashString("push_constant"),
    LayoutType_SPV = (u32)-1,
};

SPXData LoadSPX(const s8* filepath){
    
    SPXData data = {};
    
    auto spx = FOpenFile(filepath,F_FLAG_READONLY);
    auto spx_size = FGetFileSize(spx);
    
    u32 tag;
    
    FRead(spx,&tag,sizeof(tag));
    
    _kill("This is not an SPX file\n",tag != _encode('S','P','X',' '));
    
    FRead(spx,&data.type,sizeof(data.type));
    
    u32 vertparsed_count = 0;// this is a hack. I am lazy to make a new tag for inst vertices
    
    while(FCurFilePosition(spx) < spx_size){
        
        LayoutType ltype;
        FRead(spx,&ltype,sizeof(ltype));
        
        switch(ltype){
            
            case LayoutType_IN:{
                
                if(!vertparsed_count){
                    FRead(spx,&data.vlayout.size,sizeof(data.vlayout.size));
                    
                    FRead(spx,&data.vlayout.entry_count,sizeof(data.vlayout.entry_count));
                    
                    for(u32 i = 0; i < data.vlayout.entry_count; i++){
                        FRead(spx,&data.vlayout.entry_array[i],sizeof(data.vlayout.entry_array[0]));	
                    }
                    
                    vertparsed_count++;
                    
                }
                
                else{
                    FRead(spx,&data.instlayout.size,sizeof(data.instlayout.size));
                    
                    FRead(spx,&data.instlayout.entry_count,sizeof(data.instlayout.entry_count));
                    
                    for(u32 i = 0; i < data.instlayout.entry_count; i++){
                        FRead(spx,&data.instlayout.entry_array[i],sizeof(data.instlayout.entry_array[0]));	
                    }
                    
                    vertparsed_count++;	
                }
                
            }break;
            
            case LayoutType_DESC:{
                
                FRead(spx,&data.dlayout.entry_count,sizeof(data.dlayout.entry_count));
                
                for(u32 i = 0; i < data.dlayout.entry_count; i++){
                    FRead(spx,&data.dlayout.entry_array[i],sizeof(data.dlayout.entry_array[0]));	
                }
                
            }break;
            
            case LayoutType_PUSHCONST:{
                
                FRead(spx,&data.playout.size,sizeof(data.playout.size));
                
                FRead(spx,&data.playout.entry_count,sizeof(data.playout.entry_count));
                
                for(u32 i = 0; i < data.playout.entry_count; i++){
                    FRead(spx,&data.playout.entry_array[i],sizeof(data.playout.entry_array[0]));	
                }
                
            }break;
            
            case LayoutType_SPV:{
                
                ptrsize spvsize;
                FRead(spx,&spvsize,sizeof(spvsize));
                
                data.spv_size = (u32)spvsize;
                
                data.spv = TAlloc(s8,spvsize);
                
                
                FRead(spx,data.spv,spvsize);
                
                goto exit_loop;
            }break;
            
        }
        
    }
    
    exit_loop:
    
    FCloseFile(spx);
    
    return data;
}