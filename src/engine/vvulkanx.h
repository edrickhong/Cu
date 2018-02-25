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

//MARK: remove
u32 VPushBackShaderPipelineSpecX(VGraphicsPipelineSpec* spec,const SPXData* spx,
                                 VkSpecializationInfo specialization = {},
                                 u32 vert_bindingno = 0,u32 inst_bindingno = 1);

//MARK: remove
void VPushBackShaderArrayPipelineSpecX(VGraphicsPipelineSpec* spec,
                                       SPXData* spx_array,u32 spx_count,
                                       u32 vert_bindingno = 0,u32 inst_bindingno = 1);



VkDescriptorPool VCreateDescriptorPoolX(VDeviceContext* _in_ vdevice,
                                        VDescriptorPoolSpec poolspec,u32 flags = 0);


struct SPX_GraphicsShaderObject{
    
#if _debug
    u64 vert_hash;
#endif
    
    u8 vert_desc_count = 0;
    u8 vert_attrib_count = 0;
    u8 descset_count;
    u8 range_count = 0;
    u16 shader_count = 0;
    u16 spec_count = 0;
    u32 range_hash_array[16];
    u32 spv_size_array[8];
    
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
    VkPushConstantRange range_array[16];
    
    void* shader_data_array[8];
    VkShaderStageFlagBits shaderstage_array[8];
    VkSpecializationInfo spec_array[8];
    
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
    
    VkPipelineCreateFlags flags;
    
    VkPipelineLayout layout;
    VkRenderPass renderpass;
    u32 subpass_index;
    
    VkPipeline parent_pipeline;
    s32 parentpipeline_index;
    
    VkPipelineColorBlendAttachmentState colorattachment_array[16] = {};
    
    VkVertexInputBindingDescription vert_desc_array[4];
    VkVertexInputAttributeDescription vert_attrib_array[16];
    
    VkDynamicState dynamic_array[16];
    
    VkViewport viewport_array[8];
    VkRect2D scissor_array[8];
    
    VkPipelineShaderStageCreateInfo shaderinfo_array[8];
    VkSpecializationInfo spec_array[8];
    VkShaderModule shadermodule_array[8];
    u32 shadermodule_count = 0;
};

SPX_GraphicsShaderObject MakeShaderObjectSPX(SPXData* spx_array,u32 spx_count,VkSpecializationInfo* spec_array = 0,u32 spec_count = 0,u32 vert_binding_no = 0,u32 inst_binding_no = 1);

void VDescPushBackPoolSpecX(VDescriptorPoolSpec* spec,SPX_GraphicsShaderObject* obj,u32 descset_count = 1,u32 desc_set = (u32)-1);

VkDescriptorSetLayout VCreateDescriptorSetLayoutX(const  VDeviceContext* vdevice,SPX_GraphicsShaderObject* obj,u32 descset_no);

VkPipelineLayout VCreatePipelineLayoutX(const  VDeviceContext* _restrict vdevice,
                                        VkDescriptorSetLayout* descriptorset_array,
                                        u32 descriptorset_count,
                                        SPX_GraphicsShaderObject* obj);

u32 VGetDescriptorSetLayoutHash(SPX_GraphicsShaderObject* obj,u32 descset_no);

GraphicsPipelineSpecObject MakeGraphicsPipelineSpecObj(const  VDeviceContext* vdevice,SPX_GraphicsShaderObject* obj,VkPipelineLayout layout,VkRenderPass renderpass,u32 subpass_index = 0,VSwapchainContext* swap = 0,u32 colorattachment_count = 1,VkPipelineCreateFlags flags = 0,
                                                       VkPipeline parent_pipeline = 0,s32 parentpipeline_index = -1);

void VCreateGraphicsPipelineArray(const  VDeviceContext* _restrict vdevice,GraphicsPipelineSpecObject* spec_array,u32 spec_count,VkPipeline* pipeline_array,VkPipelineCache cache = 0);



void VSetFixedViewportGraphicsPipelineSpec(GraphicsPipelineSpecObject* spec,
                                           VkViewport* viewport,u32 viewport_count,VkRect2D* scissor,
                                           u32 scissor_count);

void VSetFixedViewportGraphicsPipelineSpec(GraphicsPipelineSpecObject* spec,
                                           u16 width,u16 height);

void VSetMultisampleGraphicsPipelineSpec(GraphicsPipelineSpecObject* spec,
                                         VkSampleCountFlagBits samplecount_bits = VK_SAMPLE_COUNT_1_BIT,
                                         VkBool32 is_persample_perfragment = VK_FALSE,//true = sample,else frag
                                         f32 minsampleshading = 1.0f,
                                         VkSampleMask* samplemask = 0,
                                         VkBool32 enable_alpha_to_coverage = VK_FALSE,
                                         VkBool32 enable_alpha_to_one = VK_FALSE);


void VSetDepthStencilGraphicsPipelineSpec(GraphicsPipelineSpecObject* spec,
                                          VkBool32 depthtest_enable = VK_FALSE,
                                          VkBool32 depthwrite_enable = VK_FALSE,VkCompareOp depthtest_op = VK_COMPARE_OP_NEVER,
                                          VkBool32 depthboundstest_enable = VK_FALSE,
                                          f32 min_depthbounds = 0.0f,
                                          f32 max_depthbounds = 1.0f,
                                          VkBool32 stencil_enable = false,
                                          VkStencilOpState front = {},
                                          VkStencilOpState back = {});

void VSetColorBlend(GraphicsPipelineSpecObject* spec,
                    VkPipelineColorBlendAttachmentState* attachment_array,u32 attachment_count,
                    VkBool32 logicop_enable = VK_FALSE,VkLogicOp logic_op = VK_LOGIC_OP_CLEAR,
                    f32 blendconstants[4] = {});

void VEnableColorBlendTransparency(GraphicsPipelineSpecObject* spec,
                                   u32 colorattachment_bitmask = 1,
                                   VkBlendFactor srccolor_blendfactor = VK_BLEND_FACTOR_SRC_ALPHA,
                                   VkBlendFactor dstcolor_blendfactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
                                   VkBlendOp colorblend_op = VK_BLEND_OP_ADD,
                                   VkBlendFactor srcalpha_blendfactor = VK_BLEND_FACTOR_ONE,
                                   VkBlendFactor dst_alphablendfactor = VK_BLEND_FACTOR_ZERO,
                                   VkBlendOp alphablend_op = VK_BLEND_OP_ADD,
                                   VkColorComponentFlags colorWriteMask = 0xf);

void VEnableDynamicStateGraphicsPipelineSpec(GraphicsPipelineSpecObject* spec,
                                             VkDynamicState* dynamic_array,u32 dynamic_count);
