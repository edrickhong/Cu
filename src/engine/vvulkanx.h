#pragma once
#include "vvulkan.h"
#include "aassettools.h"


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