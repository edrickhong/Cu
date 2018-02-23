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


struct SPX_GraphicsShaderObject{
    
#if _debug
    u64 vert_hash;
#endif
    
    u32 vert_desc_count = 0;
    u32 vert_attrib_count = 0;
    u32 descset_count;
    
    VkVertexInputBindingDescription vert_desc_array[4];
    VkVertexInputAttributeDescription vert_attrib_array[16];
    u32 vert_attrib_size_array[16];
    
    
    struct DescSetElement{
        VkDescriptorType type;
        u32 binding_no;
        u32 array_count;
    };
    
    struct DescSetEntry{
        u32 shader_stage;
        u32 set_no;
        DescSetElement element_array[32];
        u32 element_count;
        
    };
    
    DescSetEntry descset_array[16];
};

struct GraphicsPipelineSpecObject{
    
    
    VkPipelineVertexInputStateCreateInfo vertexinput;
    VkPipelineInputAssemblyStateCreateInfo assembly;
    VkPipelineRasterizationStateCreateInfo raster;
    
    
    VkPipelineViewportStateCreateInfo viewport;
    VkPipelineMultisampleStateCreateInfo multisample;
    VkPipelineDepthStencilStateCreateInfo depthstencil;
    VkPipelineColorBlendStateCreateInfo colorblendstate;
    
    //everything here is optional
    VkPipelineTessellationStateCreateInfo tessalationstate;
    VkPipelineDynamicStateCreateInfo dynamicstate;
    
    void* shaderdata_array[6];
    u32 shaderdatasize_array[6];
    VkShaderStageFlagBits shaderstage_array[6];
    VkSpecializationInfo shaderspecialization_array[6];
    u32 shaderdata_count = 0;
    
    VkPipelineCreateFlags flags;
    
    VkPipelineLayout layout;
    VkRenderPass renderpass;
    u32 subpass_index;
    
    VkPipeline parent_pipeline;
    s32 parentpipeline_index;
    
    s8 buffer[_kilobytes(4)];
    
    s8* cur;
};

SPX_GraphicsShaderObject MakeShaderObjectSPX(SPXData* spx_array,u32 spx_count);

void VPushBackDescriptorPoolX(VDescriptorPoolSpec* spec,SPX_GraphicsShaderObject* obj,u32 descset_count = 1,u32 desc_set = (u32)-1);

VkDescriptorSetLayout VCreateDescriptorSetLayout(SPX_GraphicsShaderObject* obj,u32 descset_no);

u32 VGetDescriptorSetLayoutHash(SPX_GraphicsShaderObject* obj,u32 descset_no);

GraphicsPipelineSpecObject MakeGraphicsPipelineSpecObj(SPX_GraphicsShaderObject* obj);
