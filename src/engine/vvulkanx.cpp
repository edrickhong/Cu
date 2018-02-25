#include "vvulkanx.h"

void VSetComputePipelineSpecShaderX(VComputePipelineSpec* spec,const s8* filepath,
                                    VkSpecializationInfo specialization){
    
    auto file = FOpenFile(filepath,F_FLAG_READONLY);
    
    ptrsize shadersize = FSeekFile(file, 0, F_METHOD_END);
    
    FSeekFile(file,0,F_METHOD_START);
    
    auto shaderdata = TAlloc(s8,shadersize);
    
    FRead(file,shaderdata,shadersize);
    
    
    VSetComputePipelineSpecShader(spec,shaderdata,shadersize,specialization);
    
    FCloseFile(file);
}

u32 VPushBackShaderPipelineSpecX(VGraphicsPipelineSpec* spec,const SPXData* spx,
                                 VkSpecializationInfo specialization,u32 vert_bindingno,u32 inst_bindingno){
    
    u32 hash = 0;
    
    if(spx->type == VK_SHADER_STAGE_VERTEX_BIT){
        
        VkFormat format_array[32] = {};
        u32 format_count = 0;
        
        VPushBackVertexSpecDesc(spec,vert_bindingno,spx->vlayout.size,
                                VK_VERTEX_INPUT_RATE_VERTEX);
        
        for(u32 i = 0; i < spx->vlayout.entry_count; i++){
            
            const auto entry = &spx->vlayout.entry_array[i];
            
            VPushBackVertexSpecAttrib(spec,vert_bindingno,
                                      entry->format,entry->size);
            
            format_array[format_count] = entry->format;
            format_count++;
            _kill("not enough space\n",format_count >= _arraycount(format_array));
        }
        
        if(spx->instlayout.size){
            
            VPushBackVertexSpecDesc(spec,inst_bindingno,spx->instlayout.size,
                                    VK_VERTEX_INPUT_RATE_INSTANCE);
            
            for(u32 i = 0; i < spx->instlayout.entry_count; i++){
                
                const auto entry = &spx->instlayout.entry_array[i];
                
                VPushBackVertexSpecAttrib(spec,inst_bindingno,
                                          entry->format,entry->size);
                
                format_array[format_count] = entry->format;
                format_count++;
                _kill("not enough space\n",format_count >= _arraycount(format_array));
            }
        }
        
        hash = VFormatHash(format_array,format_count);
    }
    
    VPushBackShaderPipelineSpec(spec,spx->spv,spx->spv_size,spx->type,specialization);
    
    return hash;
}


void VPushBackShaderArrayPipelineSpecX(VGraphicsPipelineSpec* spec,
                                       SPXData* spx_array,u32 spx_count,
                                       u32 vert_bindingno,u32 inst_bindingno){
    
    for(u32 i = 0; i < spx_count; i++){
        
        auto spx = &spx_array[i];
        
        VPushBackShaderPipelineSpecX(spec,spx,{},vert_bindingno,inst_bindingno);    
    }
    
}



VkDescriptorPool VCreateDescriptorPoolX(VDeviceContext* _in_ vdevice,
                                        VDescriptorPoolSpec poolspec,u32 flags){
    
    return VCreateDescriptorPool(vdevice,poolspec,flags,poolspec.desc_count);
}

void InternalPushBackVertexAttrib(SPX_GraphicsShaderObject* obj,u32 binding_no,VkFormat format,u32 attrib_size){
    
    _kill("cannot push more vert attrib\n",obj->vert_attrib_count >= _arraycount(obj->vert_attrib_array));
    
    u32 offset = 0;
    
    for(u32 i = 0; i < obj->vert_attrib_count; i++){
        
        if(binding_no == obj->vert_attrib_array[i].format){
            offset += obj->vert_attrib_size_array[i];
        }
    }
    
    obj->vert_attrib_array[obj->vert_attrib_count] = {obj->vert_attrib_count,binding_no,format,offset};
    obj->vert_attrib_count++;
    
}

void InternalPushBackVertexDesc(SPX_GraphicsShaderObject* obj,u32 binding_no,u32 vert_size,VkVertexInputRate inputrate){
    
    _kill("cannot push more vert desc\n",obj->vert_desc_count >= _arraycount(obj->vert_desc_array));
    
    obj->vert_desc_array[obj->vert_desc_count] = {binding_no,vert_size,inputrate};
    obj->vert_desc_count++;
    
}

void InternalHandleVertexBuilding(SPX_GraphicsShaderObject* obj,SPXData* spx,u32 vert_binding_no,u32 inst_binding_no){
    
    if(spx->type != VK_SHADER_STAGE_VERTEX_BIT){
        return;
    }
    
    VkFormat format_array[32] = {};
    u32 format_count = 0;
    u64 vert_hash = 0;
    
    
    InternalPushBackVertexDesc(obj,vert_binding_no,spx->vlayout.size,VK_VERTEX_INPUT_RATE_VERTEX);
    
    for(u32 j = 0; j < spx->vlayout.entry_count; j++){
        
        const auto entry = &spx->vlayout.entry_array[j];
        
        InternalPushBackVertexAttrib(obj,vert_binding_no,entry->format,entry->size);
        
        format_array[format_count] = entry->format;
        format_count++;
        _kill("not enough space\n",format_count >= _arraycount(format_array));
    }
    
    if(spx->instlayout.size){
        
        InternalPushBackVertexDesc(obj,inst_binding_no,spx->instlayout.size,VK_VERTEX_INPUT_RATE_INSTANCE);
        
        for(u32 j = 0; j < spx->instlayout.entry_count; j++){
            
            const auto entry = &spx->instlayout.entry_array[j];
            
            InternalPushBackVertexAttrib(obj,inst_binding_no,entry->format,entry->size);
            
            format_array[format_count] = entry->format;
            format_count++;
            _kill("not enough space\n",format_count >= _arraycount(format_array));
        }
        
    }
    
    vert_hash = VFormatHash(format_array,format_count);
    
    obj->vert_hash = vert_hash;
}

#include "aassettools.h"

void DebugCompareSet(SPX_GraphicsShaderObject::DescSetEntry* set,DescEntry* entry){
    
    for(u32 i = 0; i < set->element_count; i++){
        auto el = &set->element_array[i];
        
        if(el->binding_no == entry->bind){
            _kill("elements do not match\n",(el->type != entry->type) || (el->array_count != entry->array_count));
            return;
        }
    }
    
    _kill("element not found\n",1);
}

void SetAddElement(SPX_GraphicsShaderObject::DescSetEntry* set,DescEntry* entry){
    
    _kill("max desc set element reached\n",set->element_count >= _arraycount(set->element_array));
    
    set->element_array[set->element_count] = {entry->type,entry->bind,entry->array_count};
    set->element_count++;
}

SPX_GraphicsShaderObject::DescSetEntry* GetDescSet(SPX_GraphicsShaderObject* obj,u32 set_no){
    
    for(u32 i = 0; i < obj->descset_count; i++){
        auto entry = &obj->descset_array[i];
        if(set_no == entry->set_no){
            return entry;
        }
    }
    
    _kill("max desc set reached\n",obj->descset_count >= _arraycount(obj->descset_array));
    
    auto entry = &obj->descset_array[obj->descset_count];
    obj->descset_count++;
    
    entry->set_no = set_no;
    
    return entry;
}

void ConstructDescSets(SPX_GraphicsShaderObject* obj,SPXData* spx){
    
    for(u32 i = 0; i < spx->dlayout.entry_count; i++){
        
        auto entry = &spx->dlayout.entry_array[i];
        
        auto set = GetDescSet(obj,entry->set);
        
        set->shader_stage |= spx->type;
        
        //set has not been defined
        if(set->shader_stage == spx->type){
            SetAddElement(set,entry);
        }
        
#if _debug
        //set has been defined. Make sure it is the same across all shaders
        else{
            DebugCompareSet(set,entry);
        }
#endif
    }
    
}

void ConstructPushConsts(SPX_GraphicsShaderObject* _restrict obj,SPXData* _restrict spx){
    
    if(!spx->playout.size){
        return;
    }
    
    PushConstLayout* layout = &spx->playout;
    
    VkFormat format_array[16] = {};
    
    for(u32 i = 0; i < layout->entry_count; i++){
        format_array[i] = layout->entry_array[i].format;
    }
    
    auto hash = VFormatHash(&format_array[0],layout->entry_count);
    
    VkPushConstantRange* range = 0;
    
    for(u32 i = 0; i < obj->range_count; i++){
        
        if(obj->range_hash_array[i] == hash){
            range = &obj->range_array[i];
            break;
        }
    }
    
    if(!range){
        obj->range_hash_array[obj->range_count] = hash;
        range = &obj->range_array[obj->range_count];
        obj->range_count++;
        
        //MARK: for now we assume the offset is always 0
        range->offset = 0;
        range->size = layout->size;
    }
    
    range->stageFlags |= spx->type;
    
}


SPX_GraphicsShaderObject MakeShaderObjectSPX(SPXData* spx_array,
                                             u32 spx_count,VkSpecializationInfo* spec_array,
                                             u32 spec_count,u32 vert_binding_no,u32 inst_binding_no){
    
    _kill("max possible shaders reached\n",spx_count > 5);
    
    _kill("each specialization entry has to correspond to a shader\n",!spx_count && spec_count != spx_count);
    
    SPX_GraphicsShaderObject obj = {};
    
    if(spec_count){
        
        memcpy(&obj.spec_array[0],&spec_array[0],sizeof(VkSpecializationInfo) * spec_count);
        obj.spec_count = spec_count;
    }
    
    for(u32 i = 0; i < spx_count; i++){
        
        auto spx = &spx_array[i];
        
        InternalHandleVertexBuilding(&obj,spx,vert_binding_no,inst_binding_no);
        
        ConstructDescSets(&obj,spx);
        ConstructPushConsts(&obj,spx);
        
        obj.shaderstage_array[obj.shader_count] = spx->type;
        obj.shader_data_array[obj.shader_count] = spx->spv;
        obj.spv_size_array[obj.shader_count] = spx->spv_size;
        obj.shader_count++;
    }
    
    //sort descset elements
    
    for(u32 i = 0; i < obj.descset_count; i++){
        
        auto set = &obj.descset_array[i];
        
        qsort(set->element_array,set->element_count,sizeof(SPX_GraphicsShaderObject::DescSetElement),[](const void * a, const void* b) ->s32 {
              
              auto entry_a = (SPX_GraphicsShaderObject::DescSetElement*)a;
              auto entry_b = (SPX_GraphicsShaderObject::DescSetElement*)b;
              
              return entry_a->binding_no - entry_b->binding_no;
              });
    }
    
    _kill("no vertex shader found\n",!obj.vert_hash);
    
    return obj;
}

void VDescPushBackPoolSpecX(VDescriptorPoolSpec* spec,SPX_GraphicsShaderObject* obj,u32 descset_count,u32 desc_set){
    
    _kill("desc count cannot be 0\n",!descset_count);
    
    for(u32 i = 0; i < obj->descset_count; i++){
        
        auto set = &obj->descset_array[i];
        
        if(set->set_no == desc_set || desc_set == (u32)-1){
            
            for(u32 j = 0; j < set->element_count; j++){
                
                auto element = &set->element_array[j];
                
                VDescPushBackPoolSpec(spec,element->type,element->array_count * descset_count);
            }
        }
    }
    
    if(descset_count > spec->desc_count){
        spec->desc_count = descset_count;
    }
    
}

VkDescriptorSetLayout VCreateDescriptorSetLayoutX(const  VDeviceContext* vdevice,SPX_GraphicsShaderObject* obj,u32 descset_no){
    
    SPX_GraphicsShaderObject::DescSetEntry* set = 0;
    
    for(u32 i = 0; i < obj->descset_count; i++){
        
        auto dset = &obj->descset_array[i];
        
        if(dset->set_no == descset_no){
            set = dset;
            break;
        }
    }
    
    _kill("set not found\n",!set);
    
    
    VDescriptorBindingSpec bindingspec = {};
    
    for(u32 i = 0; i < set->element_count; i++){
        
        auto element = &set->element_array[i];
        
        VDescPushBackBindingSpec(&bindingspec,element->type,
                                 element->array_count,set->shader_stage);
    }
    
    return VCreateDescriptorSetLayout(vdevice,bindingspec);
}

VkPipelineLayout VCreatePipelineLayoutX(const  VDeviceContext* _restrict vdevice,VkDescriptorSetLayout* descriptorset_array,u32 descriptorset_count,SPX_GraphicsShaderObject* obj){
    
    return VCreatePipelineLayout(vdevice,descriptorset_array,descriptorset_count,obj->range_array,
                                 obj->range_count);
}

GraphicsPipelineSpecObject MakeGraphicsPipelineSpecObj(const  VDeviceContext* vdevice,SPX_GraphicsShaderObject* obj,VkPipelineLayout layout,
                                                       VkRenderPass renderpass,u32 subpass_index,VSwapchainContext* swap,u32 colorattachment_count,VkPipelineCreateFlags flags,
                                                       VkPipeline parent_pipeline,s32 parentpipeline_index){
    
    GraphicsPipelineSpecObject spec = {};
    
    if(obj->spec_count){
        
        memcpy(&spec.spec_array[0],&obj->spec_array[0],sizeof(VkSpecializationInfo) * obj->spec_count);
    }
    
    
    
    spec.subpass_index = subpass_index;
    spec.flags = flags;
    spec.layout = layout;
    spec.cur = &spec.buffer[0];
    spec.renderpass = renderpass;
    spec.parent_pipeline = parent_pipeline;
    spec.parentpipeline_index = parentpipeline_index;
    
    spec.vertexinput = {
        
        VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        0,
        0,
        obj->vert_desc_count,
        &obj->vert_desc_array[0],
        obj->vert_attrib_count,
        &obj->vert_attrib_array[0]
    };
    
    spec.assembly = {
        VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        0,
        0,
        VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        VK_FALSE //restart assembly of primitives for indexed draw if index(MAX_INT) is reached
    };
    
    spec.raster = {
        
        VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        0,
        0,
        VK_FALSE,
        VK_FALSE,
        VK_POLYGON_MODE_FILL,
        VK_CULL_MODE_BACK_BIT,
        VK_FRONT_FACE_CLOCKWISE,
        VK_FALSE,
        0.0f,
        0.0f,
        0.0f,
        1.0f
    };
    
    spec.viewport.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    
    if(swap){
        
        auto width = swap->width;
        auto height = swap->height;
        
        VkViewport viewport = {0.0f,0.0f,(f32)width,(f32)height,0.0f,1.0f};
        VkRect2D scissor = {width,height};
        
        spec.viewport.viewportCount = 1;
        spec.viewport.pViewports = &viewport;
        spec.viewport.scissorCount = 1;
        spec.viewport.pScissors = &scissor;
    }
    
    spec.multisample.sType =VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    
    spec.multisample.pNext = 0;
    spec.multisample.flags = 0;
    spec.multisample.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    spec.multisample.sampleShadingEnable = VK_FALSE;
    spec.multisample.minSampleShading = 1.0f;
    spec.multisample.pSampleMask= 0;
    spec.multisample.alphaToCoverageEnable = VK_FALSE;
    spec.multisample.alphaToOneEnable = VK_FALSE;
    
    
    //MARK: this is disabled by default
    spec.depthstencil.sType =
        VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    
    spec.depthstencil.pNext = 0;
    spec.depthstencil.flags = 0;
    spec.depthstencil.depthTestEnable = VK_FALSE;
    spec.depthstencil.depthWriteEnable = VK_FALSE;
    spec.depthstencil.depthCompareOp = VK_COMPARE_OP_NEVER;
    spec.depthstencil.depthBoundsTestEnable = VK_FALSE;
    spec.depthstencil.stencilTestEnable = VK_FALSE;
    spec.depthstencil.front = {};
    spec.depthstencil.back = {};
    spec.depthstencil.minDepthBounds = 0.0f;
    spec.depthstencil.maxDepthBounds = 1.0f;
    
    
    
    
    _kill("we do not support this many color attachments\n",
          colorattachment_count > _arraycount(spec.colorattachment_array));
    
    for(u32 i = 0; i < colorattachment_count; i++){
        
        spec.colorattachment_array[i] = {
            VK_FALSE,
            VK_BLEND_FACTOR_SRC_ALPHA,
            VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
            VK_BLEND_OP_ADD,
            VK_BLEND_FACTOR_ONE,
            VK_BLEND_FACTOR_ZERO,
            VK_BLEND_OP_ADD,0xf
        };
    }
    
    spec.colorblendstate.sType =
        VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    
    spec.colorblendstate.pNext = 0;
    spec.colorblendstate.flags = 0;
    spec.colorblendstate.logicOpEnable = VK_FALSE;
    spec.colorblendstate.logicOp = VK_LOGIC_OP_CLEAR;
    spec.colorblendstate.attachmentCount = colorattachment_count;
    spec.colorblendstate.pAttachments = &spec.colorattachment_array[0];
    
    memset(spec.colorblendstate.blendConstants,0,sizeof(f32) * 4);
    
    spec.shadermodule_count = obj->shader_count;
    
    for(u32 i = 0; i < obj->shader_count; i++){
        
        auto data = obj->shader_data_array[i];
        auto size = obj->spv_size_array[i];
        
        spec.shadermodule_array[i] = VCreateShaderModule(vdevice->device,data,size);
    }
    
    for(u32 i = 0; i < obj->shader_count; i++){
        
        VkSpecializationInfo* info = 0;
        
        if(spec.spec_array[i].mapEntryCount){
            info = &spec.spec_array[i];
        }
        
        spec.shaderinfo_array[i] = {
            VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            0,
            0,
            obj->shaderstage_array[i],
            spec.shadermodule_array[i],
            "main",
            info
        };
    }
    
    return spec;
}

void VCreateGraphicsPipelineArray(const  VDeviceContext* _restrict vdevice,GraphicsPipelineSpecObject* spec_array,u32 spec_count,VkPipeline* pipeline_array,VkPipelineCache cache){
    
    VkGraphicsPipelineCreateInfo info_array[16] = {};
    
    _kill("too many specs\n",spec_count > _arraycount(info_array));
    
    for(u32 i = 0; i < spec_count; i++){
        
        auto spec = &spec_array[i];
        
        VkPipelineTessellationStateCreateInfo* tessalationstate = 0;
        VkPipelineDynamicStateCreateInfo* dynamicstate = 0;
        
        if(spec->tessalationstate.sType){
            tessalationstate = &spec->tessalationstate;
        }
        
        if(spec->dynamicstate.sType){
            dynamicstate= &spec->dynamicstate;
        }
        
        info_array[i] = {
            VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
            0,
            spec->flags,
            spec->shadermodule_count,
            &spec->shaderinfo_array[0],
            &spec->vertexinput,
            &spec->assembly,
            
            tessalationstate,
            
            &spec->viewport,
            &spec->raster,
            &spec->multisample,
            &spec->depthstencil,
            &spec->colorblendstate,
            
            dynamicstate,
            
            spec->layout,
            spec->renderpass,
            spec->subpass_index,
            spec->parent_pipeline,
            spec->parentpipeline_index
        };
        
    }
    
    //MARK: pass the global allocator and use vktest
    vkCreateGraphicsPipelines(vdevice->device,cache,spec_count,&info_array[0],0,
                              pipeline_array);
    
    //destroy all modules
    for(u32 i = 0; i < spec_count; i++){
        
        auto spec = &spec_array[i];
        
        for(u32 j = 0; j < spec->shadermodule_count; j++){
            //MARK: pass the global allocator
            vkDestroyShaderModule(vdevice->device,spec->shadermodule_array[j],0);
        }
    }
}