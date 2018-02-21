#pragma once
#include "vvulkan.h"
#include "aassettools.h"


/*These encompass extended functions that are part of the framework. 
  The base vvulkan implementation will be as independent from the rest of the other tools as 
  possible*/

void _ainline VSetComputePipelineSpecShader(VComputePipelineSpec* spec,const s8* filepath,
                                            void** shaderdataptr,VkSpecializationInfo specialization = {}){
    
    auto file = FOpenFile(filepath,F_FLAG_READONLY);
    
    ptrsize shadersize;
    void* shaderdata = FReadFileToBuffer(file,&shadersize);
    
    
    VSetComputePipelineSpecShader(spec,shaderdata,shadersize,specialization);
    
    *shaderdataptr = shaderdata;
    
    FCloseFile(file);
}

void VDescPushBackPoolSpecX(VDescriptorPoolSpec* spec,const SPXData* spx_array,
                            u32 spx_count,u32 descset_count = 1);


u32 VPushBackShaderPipelineSpecX(VGraphicsPipelineSpec* spec,const SPXData* spx,
                                 VkSpecializationInfo specialization = {},
                                 u32 vert_bindingno = 0,u32 inst_bindingno = 1);

void VPushBackShaderArrayPipelineSpecX(VGraphicsPipelineSpec* spec,
                                       SPXData* spx_array,u32 spx_count,
                                       u32 vert_bindingno = 0,u32 inst_bindingno = 1);


VkDescriptorSetLayout VCreateDescriptorSetLayoutX(
const  VDeviceContext* _restrict vdevice,
SPXData* spx_array,u32 spx_count,u32 descset_no);


VkPipelineLayout VCreatePipelineLayoutX(const  VDeviceContext* _restrict vdevice,
                                        VkDescriptorSetLayout* descriptorset_array,
                                        u32 descriptorset_count,
                                        SPXData* spx_array,
                                        u32 spx_count);




VkDescriptorPool VCreateDescriptorPoolX(VDeviceContext* _in_ vdevice,
                                        VDescriptorPoolSpec poolspec,u32 flags = 0);
