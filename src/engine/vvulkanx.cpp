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
        
        hash = VPipelineVertHash(format_array,format_count);
    }
    
    VPushBackShaderPipelineSpec(spec,spx->spv,spx->spv_size,spx->type,specialization);
    
    return hash;
}


void VDescPushBackPoolSpecX(VDescriptorPoolSpec* spec,const SPXData* spx_array,
                            u32 spx_count,u32 descset_count){
    
    _kill("can't be 0\n",!descset_count);
    
    for(u32 i = 0; i < spx_count; i++){
        
        const auto spx = &spx_array[i];
        
        for(u32 j = 0; j < spx->dlayout.entry_count; j++){
            
            const auto entry = &spx->dlayout.entry_array[j];
            
            VDescPushBackPoolSpec(spec,entry->type,entry->array_count * descset_count);
            
        }
        
    }
    
    spec->desc_count += descset_count;
    
}

VkDescriptorSetLayout VCreateDescriptorSetLayoutX(
const  VDeviceContext* _restrict vdevice,
SPXData* spx_array,u32 spx_count,u32 descset_no){
    
    struct DescEntryX : DescEntry{
        VkShaderStageFlags stage_flags;
    };
    
    DescEntryX entry_array[16];
    u32 entry_count = 0;
    
    for(u32 i = 0; i < spx_count; i++){
        
        const auto spx = &spx_array[i];
        
        for(u32 j = 0; j < spx->dlayout.entry_count; j++){
            
            const auto entry = &spx->dlayout.entry_array[j];
            
            if(entry->set == descset_no){
                
                entry_array[entry_count].type = entry->type;
                entry_array[entry_count].set = entry->set;
                entry_array[entry_count].bind = entry->bind;
                entry_array[entry_count].array_count = entry->array_count;
                entry_array[entry_count].stage_flags = spx->type;
                
                entry_count++;
                
            }
            
        }
        
    }
    
    qsort(entry_array,entry_count,sizeof(DescEntryX),[](const void * a, const void* b)->s32 {
          
          auto entry_a = (DescEntryX*)a;
          auto entry_b = (DescEntryX*)b;
          
          return entry_a->bind - entry_b->bind;
          });
    
    VDescriptorBindingSpec bindingspec;
    
    for(u32 i = 0; i < entry_count;i++){
        
        const auto entry = &entry_array[i];
        
        VDescPushBackBindingSpec(&bindingspec,entry->type,
                                 entry->array_count,entry->stage_flags);
    }
    
    return VCreateDescriptorSetLayout(vdevice,bindingspec);
}


VkPipelineLayout VCreatePipelineLayoutX(const  VDeviceContext* _restrict vdevice,
                                        VkDescriptorSetLayout* descriptorset_array,
                                        u32 descriptorset_count,
                                        SPXData* spx_array,
                                        u32 spx_count){
    
    VkPushConstantRange range_array[7];
    u32 range_count = 0;
    
    for(u32 i = 0; i < spx_count;i++){
        const auto spx = &spx_array[i];
        
        if(spx->playout.size){
            
            //MARK: should offset always be 0?
            range_array[range_count] = {(VkShaderStageFlags)spx->type,0,spx->playout.size};
            range_count++;
            
        }
        
    }
    
    
    
    return VCreatePipelineLayout(vdevice,descriptorset_array,descriptorset_count,range_array,
                                 range_count);  
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
    
    vert_hash = VPipelineVertHash(format_array,format_count);
    
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


SPX_GraphicsShaderObject MakeShaderObjectSPX(SPXData* spx_array,
                                             u32 spx_count,u32 vert_binding_no,u32 inst_binding_no){
    
    SPX_GraphicsShaderObject obj = {};
    
    for(u32 i = 0; i < spx_count; i++){
        
        auto spx = &spx_array[i];
        
        InternalHandleVertexBuilding(&obj,spx,vert_binding_no,inst_binding_no);
        
        ConstructDescSets(&obj,spx);
        
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

void VPushBackDescriptorPoolX(VDescriptorPoolSpec* spec,SPX_GraphicsShaderObject* obj,u32 descset_count,u32 desc_set){
    
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

VkDescriptorSetLayout VCreateDescriptorSetLayout(const  VDeviceContext* vdevice,SPX_GraphicsShaderObject* obj,u32 descset_no){
    
    SPX_GraphicsShaderObject::DescSetEntry* set = 0;
    
    for(u32 i = 0; i < obj->descset_count; i++){
        
        auto dset = &obj->descset_array[i];
        
        if(dset->set_no == descset_no){
            set = dset;
            break;
        }
    }
    
    VDescriptorBindingSpec bindingspec = {};
    
    for(u32 i = 0; i < set->element_count; i++){
        
        auto element = &set->element_array[i];
        
        VDescPushBackBindingSpec(&bindingspec,element->type,
                                 element->array_count,set->shader_stage);
    }
    
    
    _kill("set not found\n",!set);
    
    return VCreateDescriptorSetLayout(vdevice,bindingspec);
}