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