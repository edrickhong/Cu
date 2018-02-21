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
                                    VK_VERTEX_INPUT_RATE_VERTEX);
            
            for(u32 i = 0; i < spx->instlayout.entry_count; i++){
                
                const auto entry = &spx->instlayout.entry_array[i];
                
                VPushBackVertexSpecAttrib(spec,vert_bindingno,
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
