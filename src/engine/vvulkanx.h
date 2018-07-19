#pragma once
#include "vvulkan.h"
#include "aassettools.h"


#define VX_INDEXBUFFER_GETINDEX(value) _removesignedbit(value)
#define VX_INDEXBUFFER_GETINDEXTYPE(value) ((VkIndexType)(value >> 31))


/*These encompass extended functions that are part of the framework. 
  The base vvulkan implementation will be as independent from the rest of the other tools as 
  possible*/

void VSetComputePipelineSpecShaderX(VComputePipelineSpec* spec,const s8* filepath,
                                    VkSpecializationInfo specialization = {});



VkDescriptorPool VCreateDescriptorPoolX(VDeviceContext* _in_ vdevice,
                                        VDescriptorPoolSpec poolspec,u32 flags = 0);

VShaderObj VMakeShaderObjSPX(SPXData* spx_array,u32 spx_count,VkSpecializationInfo* spec_array = 0,u32 spec_count = 0,u32 vert_binding_no = 0,u32 inst_binding_no = 1);

VkDescriptorSetLayout VCreateDescriptorSetLayout(const  VDeviceContext* vdevice,VShaderObj* obj,u32 descset_no);

VkPipelineLayout VCreatePipelineLayout(const  VDeviceContext* _restrict vdevice,
                                       VkDescriptorSetLayout* descriptorset_array,
                                       u32 descriptorset_count,
                                       VShaderObj* obj);

void VDescPushBackPoolSpec(VDescriptorPoolSpec* spec,VShaderObj* obj,u32 descset_count = 1,u32 desc_set = (u32)-1);


/*
NOTE: VCreateStaticIndexBufferX is the same as their counterparts
the difference being that this function assumes an optimal index format.
If the data size/sizeof(u16) is <= 65535, then indices are expected to be in a 16 bit format. Otherwise, the indices are expected to be in u32 format.

If u32 , the top bit in ind_count will be set
*/

VBufferContext VCreateStaticIndexBufferX(const  VDeviceContext* _restrict vdevice,
                                         VkCommandBuffer commandbuffer,
                                         VkDeviceMemory memory,
                                         VkDeviceSize offset,
                                         VBufferContext src,VkDeviceSize src_offset,void* data,
                                         ptrsize data_size);

VBufferContext VCreateStaticIndexBufferX(const  VDeviceContext* _restrict vdevice,
                                         ptrsize data_size,logic isdevice_local = true,VMappedBufferProperties prop = VMAPPED_COHERENT);