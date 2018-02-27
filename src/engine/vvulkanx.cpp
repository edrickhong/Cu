#include "vvulkanx.h"
#include "aassettools.h"

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

//make functions that automate pipeline and descriptor specification


void InternalHandleVertexBuilding(VShaderObj* obj,SPXData* spx,u32 vert_binding_no,u32 inst_binding_no){
    
    if(spx->type != VK_SHADER_STAGE_VERTEX_BIT){
        return;
    }
    
    VkFormat format_array[32] = {};
    u32 format_count = 0;
    u64 vert_hash = 0;
    
    
    VPushBackVertexDesc(obj,vert_binding_no,spx->vlayout.size,VK_VERTEX_INPUT_RATE_VERTEX);
    
    for(u32 j = 0; j < spx->vlayout.entry_count; j++){
        
        const auto entry = &spx->vlayout.entry_array[j];
        
        VPushBackVertexAttrib(obj,vert_binding_no,entry->format,entry->size);
        
        format_array[format_count] = entry->format;
        format_count++;
        _kill("not enough space\n",format_count >= _arraycount(format_array));
    }
    
    if(spx->instlayout.size){
        
        VPushBackVertexDesc(obj,inst_binding_no,spx->instlayout.size,VK_VERTEX_INPUT_RATE_INSTANCE);
        
        for(u32 j = 0; j < spx->instlayout.entry_count; j++){
            
            const auto entry = &spx->instlayout.entry_array[j];
            
            VPushBackVertexAttrib(obj,inst_binding_no,entry->format,entry->size);
            
            format_array[format_count] = entry->format;
            format_count++;
            _kill("not enough space\n",format_count >= _arraycount(format_array));
        }
        
    }
    
    vert_hash = VFormatHash(format_array,format_count);
    
    obj->vert_hash = vert_hash;
}

void DebugCompareSet(VShaderObj::DescSetEntry* set,DescEntry* entry){
    
    for(u32 i = 0; i < set->element_count; i++){
        auto el = &set->element_array[i];
        
        if(el->binding_no == entry->bind){
            _kill("elements do not match\n",(el->type != entry->type) || (el->array_count != entry->array_count));
            return;
        }
    }
    
    _kill("element not found\n",1);
}


void _ainline SetAddElement(VShaderObj::DescSetEntry* set,DescEntry* entry){
    VPushBackSetElement(set,entry->type,entry->bind,entry->array_count);
}

void ConstructDescSets(VShaderObj* obj,SPXData* spx){
    
    for(u32 i = 0; i < spx->dlayout.entry_count; i++){
        
        auto entry = &spx->dlayout.entry_array[i];
        
        VPushBackDescSet(obj,entry->set,spx->type);
        
        auto set = VGetSet(obj,entry->set);
        
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

void ConstructPushConsts(VShaderObj* _restrict obj,SPXData* _restrict spx){
    
    if(!spx->playout.size){
        return;
    }
    
    PushConstLayout* layout = &spx->playout;
    
    VkFormat format_array[16] = {};
    
    for(u32 i = 0; i < layout->entry_count; i++){
        format_array[i] = layout->entry_array[i].format;
    }
    
    VPushBackPushConstRange(obj,&format_array[0],layout->entry_count,spx->playout.size,spx->type);
    
}


VShaderObj VMakeShaderObjSPX(SPXData* spx_array,
                                  u32 spx_count,VkSpecializationInfo* spec_array,
                                  u32 spec_count,u32 vert_binding_no,u32 inst_binding_no){
    
    _kill("each specialization entry has to correspond to a shader\n",!spx_count && spec_count != spx_count);
    
    VShaderObj obj = {};
    
    for(u32 i = 0; i < spx_count; i++){
        
        auto spx = &spx_array[i];
        
        InternalHandleVertexBuilding(&obj,spx,vert_binding_no,inst_binding_no);
        
        ConstructDescSets(&obj,spx);
        ConstructPushConsts(&obj,spx);
        
        VkSpecializationInfo spec = {};
        
        if(spec_array){
            spec = spec_array[i];
        }
        
        
        VPushBackShaderData(&obj,spx->type,spx->spv,spx->spv_size,spec);
        
    }
    
    //sort descset elements
    for(u32 i = 0; i < obj.descset_count; i++){
        
        auto set = &obj.descset_array[i];
        
        qsort(set->element_array,set->element_count,sizeof(VShaderObj::DescSetElement),[](const void * a, const void* b) ->s32 {
              
              auto entry_a = (VShaderObj::DescSetElement*)a;
              auto entry_b = (VShaderObj::DescSetElement*)b;
              
              return entry_a->binding_no - entry_b->binding_no;
              });
    }
    
    _kill("no vertex shader found\n",!obj.vert_hash);
    
    return obj;
}

//To remove
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