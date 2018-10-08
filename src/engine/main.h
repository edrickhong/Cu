#pragma once

#include "mode.h"


#ifndef CPP_PASS

#include "engine_meta.h"

#endif


#include "wwindow.h"

#include "aaudio.h"
#include "game.h"
#include "ccontainer.h"
#include "kkeycode.h"
#include "debugtimer.h"
#include "ssys.h"
#include "tthread.h"
#include "ffileio.h"
#include "libload.h"
#include "ttimer.h"
#include "pparse.h"

#include "dynamic_reload.cpp"
#include "tthreadx.h"

#include "settings.cpp"


#define _targetframerate 16.0f//33.33f
#define _ms2s(ms)  ((f32)ms/1000.0f)

#define _mute_sound 1

#define _max_swapchain_count 4

/*
  TODO:
  See if we can make render jobs just regular thread jobs
  
  NOTE: stb_vorbis_decode_filename("somefile.ogg", &channels, &sample_rate, &output);
  
  for our recording system, we are saving every frame already. we could reduce the requirements by doing a XOR on the whole struct and compress the rest of the data. XOR means only the bits that changed are encoded and the rest will be 0, making this very good for compression
*/


u32 GetExecFileAssetData(const s8* exec_filepath){
    
    _persist auto ustring = "patchthisvalueinatassetpacktime";
    
    if(ustring[0] != '!'){
        printf("binary has not been patched\n");
        return (u32)-1;
    }
    
    auto exec_size = *((u32*)&ustring[1]);
    
    printf("exec size %d\n",exec_size);
    
    auto file = FOpenFile(exec_filepath,F_FLAG_READONLY);
    
    FSeekFile(file,exec_size,F_METHOD_START);
    
    s8 buffer[1024] = {};
    u32 size;
    
    FRead(file,&size,sizeof(size));
    FRead(file,&buffer[0],size);
    
    printf("%s\n",&buffer[0]);
    
    FCloseFile(file);
    
    return 0;
}


#ifdef DEBUG


struct DebugRenderEntry{
    u32 batch_index;
    u32 group_no;
    u32 obj_index_in_batch;
    s8* obj_assetfile;
};

struct DebugRenderRefEntry{
    DebugRenderEntry* entry_array;
    u32 entry_count;
    ThreadID thread;
};


_persist DebugRenderRefEntry global_debugentry_array[32];
_persist u32 global_debugentry_count = 0;

void DebugSubmitDebugEntryRef(ThreadID tid,DebugRenderEntry* array,u32 count){
    
    u32 actual_count = TGetEntryIndexD(&global_debugentry_count,_arraycount(global_debugentry_array));
    
    global_debugentry_array[actual_count] = {array,count,tid};
    
}

void _ainline DebugPlane(Vertex* vert_array,u32* index_array){
    
    Vertex vert[] = {
        { {  -1.0f,  -1.0f, 1.0f ,1.0f }, { 1.0f, 1.0f, 1.0f ,1.0f }, {0.0f, 1.0f}},//a  0
        { { -1.0f,  1.0f, 1.0f ,1.0f }, { 1.0f, 1.0f, 1.0f ,1.0f }, {0.0f, 0.0f}},//b   1
        { { 1.0f, 1.0f, 1.0f ,1.0f }, { 1.0f, 1.0f, 1.0f ,1.0f }, {1.0f, 0.0f}},//c   2
        { {  1.0f, -1.0f, 1.0f ,1.0f }, { 1.0f, 1.0f, 1.0f ,1.0f }, {1.0f, 1.0f}},//d   3
    };
    
    u32 index[] = {
        0,1,2, 2,3,0,//front
    };
    
    memcpy(vert_array,vert,sizeof(vert));
    memcpy(index_array,index,sizeof(index));
}

#endif

struct PrimaryRenderCommandbuffer{
    VkCommandBuffer buffer;
};





struct RenderGroup{
    //we can force this to be contiguous
    struct obj{
        ModelAssetHandle* handle;
        u16 dyn_offset;
        u16 count;
    };
    
    obj* container;
    volatile u32 count;
    
    VkPipeline pipeline;
    VkPipelineLayout pipelinelayout;
    
    VkDescriptorSet* descriptorset_array;
    VkRect2D* scissor_array;
    VkViewport* viewport_array;
    s8* pushconst_data;
    
    u8 descriptorset_count;
    u8 scissor_count;
    u8 viewport_count;
    u8 pushconst_size;
    
    
    VThreadCommandbufferList cmdbufferlist;
};

struct RenderBatch{
    RenderGroup::obj* container;
    volatile u16 count;
    u16 group;
    
    VkPipeline pipeline;
    VkPipelineLayout pipelinelayout;
    
    VkDescriptorSet* descriptorset_array;
    VkRect2D* scissor_array;
    VkViewport* viewport_array;
    s8* pushconst_data;
    
    u8 descriptorset_count;
    u8 scissor_count;
    u8 viewport_count;
    u8 pushconst_size;
};

struct RenderContext{
    RenderGroup rendergroup[_rendergroupcount];
    volatile VkRenderPass renderpass;
    volatile VkFramebuffer framebuffer;
    volatile u16 subpass_index;
    volatile u16 swap_index;
    volatile Color clearcolor = {};
};

_persist RenderBatch* renderbatch_array[16];
_persist u32 renderbatch_cur = 0;
_persist u32 renderbatch_completed_count = 0;
_persist u32 renderbatch_total_count = 0;

void DumpRenderBatches(s8* file,s8* function,u32 line){
    
    for(u32 i = 0; i < renderbatch_total_count; i++){
        auto batch = renderbatch_array[i];
        
        printf("dump batch %p: %p %d %d | %d %s %s %d\n",(void*)batch,(void*)batch->container,
               batch->descriptorset_count,batch->pushconst_size,(u32)TGetThisThreadID(),file,function,
               line);
    }
    
    
}

void _ainline InitRenderContext(RenderContext* context,u32 count = 32){
    
    for(u32 i = 0; i < _arraycount(context->rendergroup); i++){
        
        context->rendergroup[i].container =
            (RenderGroup::obj*)alloc(sizeof(RenderGroup::obj) * count);
        
        context->rendergroup[i].cmdbufferlist.container =
            (CacheAlignedCommandbuffer*)alloc(sizeof(CacheAlignedCommandbuffer) * (count >> 2));
    }
    
}

void _ainline Clear(RenderContext* context){
    
    for(u32 i = 0; i < _arraycount(context->rendergroup); i++){
        auto group = &context->rendergroup[i];
        group->count = 0;
        group->cmdbufferlist.count = 0;    
    }
    
    renderbatch_cur = 0;
    renderbatch_completed_count = 0;
    renderbatch_total_count = 0;
}

void _ainline SetClearColor(RenderContext* context,f32 r,f32 g,f32 b,f32 a){
    
    context->clearcolor.R = r;
    context->clearcolor.G = g;
    context->clearcolor.B = b;
    context->clearcolor.A = a;
    
}

void _ainline SetClearColor(RenderContext* context,Color color){
    SetClearColor(context,color.R,color.G,color.B,color.A);
}

void _ainline InternalPushRenderEntry(RenderContext* context,u32 group_index,
                                      ModelAssetHandle* handle,u32 dyn_offset,u32 instance_count = 1){
    
#ifdef DEBUG
    if(handle->instancebuffer.buffer){
        _kill("invalid instance buffer use\n",instance_count > handle->instancebuffer.inst_count);  
    }
#endif
    
    auto group = &context->rendergroup[group_index];
    
    group->container[group->count].handle = handle;
    group->container[group->count].dyn_offset = dyn_offset;
    group->container[group->count].count = instance_count;
    group->count++;
    
}


void _ainline InternalDraw(VkCommandBuffer commandbuffer,
                           VBufferContext vertex_buffer,VBufferContext index_buffer,
                           VBufferContext instance_buffer = {},u32 instance_count = 1,
                           VkDeviceSize vb_offset = 0,VkDeviceSize ind_offset = 0,VkDeviceSize inst_offset = 0){
    
    if(instance_buffer.buffer){
        vkCmdBindVertexBuffers(commandbuffer,instance_buffer.bind_no,1,
                               &instance_buffer.buffer,
                               &inst_offset);
    }
    
    vkCmdBindVertexBuffers(commandbuffer,vertex_buffer.bind_no,1,
                           &vertex_buffer.buffer,
                           &vb_offset);
    
    vkCmdBindIndexBuffer(commandbuffer,index_buffer.buffer,
                         ind_offset,VX_INDEXBUFFER_GETINDEXTYPE(index_buffer.ind_count));
    
    vkCmdDrawIndexed(commandbuffer,VX_INDEXBUFFER_GETINDEX(index_buffer.ind_count),instance_count,0,0,0);  
}

struct ThreadRenderData{
    VkCommandPool pool;
    
    
    VkCommandBuffer cmdbuffer[_max_swapchain_count * _rendergroupcount];
    
    
    u32 active_group;
    u8 group_submit_count[4];
    
#ifdef DEBUG
    
    DebugRenderEntry debugentry_array[32] = {};
    u32 debugentry_count = 0;
    
#endif
};

ThreadRenderData CreateThreadRenderData(VDeviceContext* vdevice){
    
    ThreadRenderData data = {};
    
    data.pool = VCreateCommandPool(vdevice,
                                   VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
                                   VGetQueueFamilyIndex(VQUEUETYPE_ROOT));
    
    for(u32 i = 0; i < _arraycount(data.cmdbuffer); i++){
        
        data.cmdbuffer[i] =  VAllocateCommandBuffer(vdevice,data.pool,
                                                    VK_COMMAND_BUFFER_LEVEL_SECONDARY);
    }
    
    return data;
}

logic _ainline InternalExecuteRenderBatch(RenderContext* context,
                                          ThreadRenderData* render){
    
    TIMEBLOCK(Wheat);
    
    if(renderbatch_cur >= renderbatch_total_count){
        return false;
    }
    
    auto count = renderbatch_cur;
    
    auto actual_count = LockedCmpXchg(&renderbatch_cur,count,count + 1);
    
    if(count == actual_count && count < renderbatch_total_count){
        
        auto index = count;
        
        auto batch = renderbatch_array[index];
        
        auto cmdbuffer =
            render->cmdbuffer[_arraycount(context->rendergroup) * context->swap_index +
                batch->group];
        
        if(!(render->active_group & (1 << batch->group))){
            
            render->active_group |= (1 << batch->group);
            
            VStartCommandBuffer(cmdbuffer,
                                VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT,
                                context->renderpass,context->subpass_index,context->framebuffer,VK_FALSE,0,0);
            
#ifdef DEBUG
            
            render->debugentry_count = 0;
            
#endif
        }
        
        _kill("a trivial non program will always have these\n",
              !(batch->descriptorset_count && batch->pushconst_size));
        
        vkCmdBindPipeline(cmdbuffer,
                          VK_PIPELINE_BIND_POINT_GRAPHICS,batch->pipeline);
        
        vkCmdPushConstants(cmdbuffer,batch->pipelinelayout,
                           VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,0,batch->pushconst_size,
                           batch->pushconst_data);
        
        //slot 0 is reserved for dynamic ubo
        if(batch->descriptorset_count > 1){
            
            vkCmdBindDescriptorSets(cmdbuffer,
                                    VK_PIPELINE_BIND_POINT_GRAPHICS,
                                    batch->pipelinelayout,1,batch->descriptorset_count - 1,
                                    &batch->descriptorset_array[1],0,0);  
        }
        
        //MARK: we'll pretend these are not mutually exclusive
        if(batch->viewport_count & batch->scissor_count){
            
            vkCmdSetViewport(cmdbuffer,0,batch->viewport_count,batch->viewport_array);
            
            vkCmdSetScissor(cmdbuffer,0,batch->scissor_count,batch->scissor_array);
        }
        
        
        for(u32 i = 0; i < batch->count; i++){
            
            auto obj = batch->container[i];
            
            CommitModel(obj.handle,cmdbuffer);
            
            auto vertexbuffer = obj.handle->vertexbuffer;
            auto indexbuffer = obj.handle->indexbuffer;
            auto instancebuffer = obj.handle->instancebuffer;
            
            u32 offsets = obj.dyn_offset;
            
            vkCmdBindDescriptorSets(cmdbuffer,
                                    VK_PIPELINE_BIND_POINT_GRAPHICS,
                                    batch->pipelinelayout,0,1,
                                    &batch->descriptorset_array[0],1,&offsets);
            
            InternalDraw(cmdbuffer,vertexbuffer,indexbuffer,instancebuffer,obj.count);
            
#ifdef DEBUG
            
            render->debugentry_array[render->debugentry_count] = {
                index,batch->group,i,obj.handle->assetfile
            };
            render->debugentry_count++;
            
#endif
        }
        
        render->group_submit_count[batch->group]++;
    }
    
    return true;
}

void ExecuteRenderBatch(RenderContext* context,
                        ThreadRenderData* render){
    
    render->active_group ^=render->active_group;
    memset(render->group_submit_count,0,sizeof(render->group_submit_count));
    
    while(InternalExecuteRenderBatch(context,render)){}
    
    if(!render->active_group){
        return;
    }
    
#ifdef DEBUG
    
    DebugSubmitDebugEntryRef(TGetThisThreadID(),&render->debugentry_array[0],render->debugentry_count);
#endif
    
    u32 count = _typebitcount(render->active_group) - BSR(render->active_group);
    
    for(u32 i = 0; i < count; i++){
        
        if(render->active_group & (1 << i)){
            
            _kill("active group > actual number of groups\n", i >= _arraycount(context->rendergroup));
            
            auto cmdbuffer =
                render->cmdbuffer[_arraycount(context->rendergroup) * context->swap_index + i];
            
            VEndCommandBuffer(cmdbuffer);
            
            VPushThreadCommandbufferList(&context->rendergroup[i].cmdbufferlist,
                                         cmdbuffer);
            
            _kill("exceeds total\n",(renderbatch_completed_count + render->group_submit_count[i]) > renderbatch_total_count);
            
            LockedAdd(&renderbatch_completed_count,render->group_submit_count[i]);
        }
        
    }
    
}

void ThisThreadExecuteRenderBatch(RenderContext* context,
                                  ThreadRenderData* render){
    
    TIMEBLOCK(DeepSkyBlue);
    ExecuteRenderBatch(context,render);
    
    
    while(renderbatch_completed_count != renderbatch_total_count){
        _kill("",renderbatch_completed_count > renderbatch_total_count);
        _mm_pause();
    }
    
#ifdef DEBUG
    
    global_debugentry_count = 0;
    
#endif
}

void _ainline InternalDispatchRenderBatch(RenderBatch* batch,TSemaphore sem){
    
    _kill("too many batches\n",renderbatch_total_count >= _arraycount(renderbatch_array));
    
    renderbatch_array[renderbatch_total_count] = batch;
    renderbatch_total_count++;
    
    TSignalSemaphore(sem);
}

void _ainline DispatchRenderContext(RenderContext* context,TSemaphore sem){
    
    for(u32 i = 0; i < _arraycount(context->rendergroup); i++){
        
        auto group = &context->rendergroup[i];
        
        auto count = group->count;
        
        for(;count >= _renderbatchsize; count -= _renderbatchsize){
            
            auto batch = TAlloc(RenderBatch,1);
            
            batch->container = &group->container[group->count - count];
            
            batch->count = _renderbatchsize;
            
            batch->group = i;
            
            batch->pipeline = group->pipeline;
            batch->pipelinelayout = group->pipelinelayout;
            
            batch->descriptorset_array = group->descriptorset_array;
            batch->scissor_array = group->scissor_array;
            batch->viewport_array = group->viewport_array;
            batch->pushconst_data = group->pushconst_data;
            
            batch->descriptorset_count = group->descriptorset_count;
            batch->scissor_count = group->scissor_count;
            batch->viewport_count = group->viewport_count;
            batch->pushconst_size = group->pushconst_size;
            
            
            InternalDispatchRenderBatch(batch,sem);
        }
        
        if(count){
            
            auto batch = TAlloc(RenderBatch,1);
            
            batch->container = &group->container[group->count - count];
            batch->group = i;
            batch->count = count;
            
            batch->pipeline = group->pipeline;
            batch->pipelinelayout = group->pipelinelayout;
            
            batch->descriptorset_array = group->descriptorset_array;
            batch->scissor_array = group->scissor_array;
            batch->viewport_array = group->viewport_array;
            batch->pushconst_data = group->pushconst_data;
            
            batch->descriptorset_count = group->descriptorset_count;
            batch->scissor_count = group->scissor_count;
            batch->viewport_count = group->viewport_count;
            batch->pushconst_size = group->pushconst_size;
            
            InternalDispatchRenderBatch(batch,sem);
        }
        
    }
    
}

void _ainline GetCmdBufferArray(RenderContext* context,
                                VkCommandBuffer** cmdbuffer_array,u32* cmdbuffer_count){
    
    u32 count = 0;
    
    for(u32 i = 0; i < _arraycount(context->rendergroup); i++){
        count += context->rendergroup[i].cmdbufferlist.count;
    }
    
    //NOTE: Handles the case where there are no cmdbuffers
    if(!count){
        return;
    }
    
    auto cmdbuffers = TAlloc(VkCommandBuffer,count);
    
    count = 0;
    
    for(u32 i = 0; i < _arraycount(context->rendergroup); i++){
        
        auto group = &context->rendergroup[i];
        
        for(u32 j = 0; j < group->cmdbufferlist.count; j++){
            cmdbuffers[count] = group->cmdbufferlist.container[j].cmdbuffer;
            count++;
        }
        
    }
    
    *cmdbuffer_array = cmdbuffers;
    *cmdbuffer_count = count;
}



struct Threadinfo{
    ThreadWorkQueue* queue;
    ThreadTextureFetchQueue* fetchqueue;
    TSemaphore this_sem;
    TSemaphore main_sem;
    VDeviceContext vdevicecontext;
    RenderContext* rendercontext;
};

struct ObjUpdateEntry{
    u16 offset;
    u16 data_size;
};

struct ThreadLinearBlendRes{
    u32 id;
    f32 time;
    u32 bone_count;
    u32 animation_index;
    AAnimationSet* set_array;
    ALinearBone* root;
    DEBUGPTR(Matrix4b4) result;//filled by the platform
};

struct LightUBO{
    
#define _lightcount 1024
    
    struct PointLight{
        Vector4 pos;
        Color color;
        
        f32 radius;
    };
    
    struct SpotLight{
        
        Vector4 pos;
        Vector4 dir;
        Color color;
        
        f32 cos_angle;
        f32 hard_cos_angle;
        
        f32 radius;
    };
    
    u32 dir_count;
    u32 point_count;
    u32 spot_count;
    
    
    DirLight dir_array[_lightcount];
    PointLight point_array[_lightcount];
    SpotLight spot_array[_lightcount];
    
    Color ambient_color;
    
}_align(128);

struct PlatformData{
    
    WWindowContext window;
    AAudioContext audio;
    VDeviceContext vdevice;
    VSwapchainContext swapchain;
    VkFence present_fence;
    
    
    VkQueue root_queue;
    VkQueue compute_queue;
    VkQueue transfer_queue;
    ThreadRenderData drawcmdbuffer;
    VkSemaphore waitacquireimage_semaphore;
    VkSemaphore waitfinishrender_semaphore;
    VkRenderPass renderpass;
    
    VkDescriptorPool descriptorpool;
    
    
    VkDescriptorSetLayout dynuniform_descriptorlayout;
    VkDescriptorSetLayout vt_descriptorlayout;
    
    
    VkDescriptorSet dynuniform_skel_descriptorset;
    VkDescriptorSet vt_descriptorset;
    
    VkPipelineCache pipelinecache;
    VkPipeline pipeline_array[2];
    VkPipelineLayout pipelinelayout;
    
    VBufferContext skel_ubo;
    VBufferContext light_ubo;
    
    Matrix4b4 view;
    Matrix4b4 proj;
    
    Vector4 camerapos;
    
    AAudioBuffer submit_audiobuffer;
    TSemaphore worker_sem;
    TSemaphore main_sem;
    ThreadWorkQueue threadqueue;
    ThreadTextureFetchQueue fetchqueue;
    u32 threadcount;
    PrimaryRenderCommandbuffer* rendercmdbuffer_array;
    u32 prevdrawcount[2] = {};
    GameLib lib;
    f32 deltatime;
    KeyboardState keyboardstate;
    MouseState mousestate;
    
    
    SceneContext scenecontext;
    RenderContext rendercontext;
    
    //MARK:
    s8* objupdate_ptr;
    s8* lightupdate_ptr;
    ObjUpdateEntry objupdate_array[256];
    u32 objupdate_count;
    
    //MARK:
    u32 point_count;
    u32 spot_count;
    
};

_persist PlatformData* pdata;
_persist GameData* gdata;


void _ainline PushUpdateEntry(u32 id,u32 offset,u32 data_size,void* data){
    
    _kill("too many updates\n",pdata->objupdate_count >= _arraycount(pdata->objupdate_array));
    
    auto effective_offset = (id * sizeof(SkelUBO)) + offset;
    
    auto ptr = ((s8*)pdata->objupdate_ptr) + effective_offset;
    memcpy(ptr,data,data_size);
    
    pdata->objupdate_array[pdata->objupdate_count] = {
        (u16)effective_offset,
        (u16)data_size
    };
    
    pdata->objupdate_count++;
}

_persist u32 gui_draw_is_locked = 0;

void GameDrawGUI(RenderContext* context,ThreadRenderData* render,u32 group_index){
    
    if(gui_draw_is_locked){
        return;
    }
    
    auto islocked = gui_draw_is_locked;
    
    u32 actual_islocked = LockedCmpXchg(&gui_draw_is_locked,islocked,islocked + 1);
    
    if(islocked != actual_islocked){
        return;
    }
    
    auto cmdbuffer =
        render->cmdbuffer[_arraycount(context->rendergroup) * context->swap_index +
            group_index];
    
    VStartCommandBuffer(cmdbuffer,
                        VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT,
                        context->renderpass,context->subpass_index,context->framebuffer,VK_FALSE,0,0);
    
    GUIDraw(cmdbuffer);
    
    VEndCommandBuffer(cmdbuffer);
    
    {
        TIMEBLOCKTAGGED("VThreadEndRender::Submit",Turquoise);
        VPushThreadCommandbufferList(&context->rendergroup[group_index].cmdbufferlist,
                                     cmdbuffer);
    }
}


void _ainline BuildRenderCommandBuffer(PlatformData* pdata){
    
    TIMEBLOCK(Silver);  
    
    u32 frameindex = pdata->swapchain.image_index;
    
    auto framebuffer = pdata->swapchain.presentresource_array[frameindex].framebuffer;
    
    auto context = &pdata->rendercontext;
    
    SetClearColor(context,0.0f,0.0f,1.0f,0.0f);
    
    auto cmdbuffer = pdata->rendercmdbuffer_array[frameindex].buffer;
    
    auto renderpass = pdata->renderpass;
    
    context->renderpass = pdata->renderpass;
    context->framebuffer = framebuffer;
    context->subpass_index = 0;
    context->swap_index = frameindex;
    
    auto pushconst = TAlloc(PushConst,1);
    
    *pushconst = {
        
#if _row_major
        
        Transpose(pdata->proj * pdata->view),
        
#else
        
        pdata->proj * pdata->view,
        
#endif
        pdata->camerapos
    };
    
    //MARK:
    for(u32 i = 0; i < 2; i++){
        context->rendergroup[i].pushconst_data = (s8*)pushconst;
        context->rendergroup[i].pushconst_size = sizeof(PushConst);
    }
    
    
    VStartCommandBuffer(cmdbuffer,0);
    
    
    VTStart(cmdbuffer);
    
#if _enable_gui
    
    GameDrawGUI(context,&pdata->drawcmdbuffer,2);
    
#endif
    
    VkImageMemoryBarrier present_membarrier[] = {
        {
            VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            0,
            0,//srcAccessMask
            VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            VK_QUEUE_FAMILY_IGNORED,
            VK_QUEUE_FAMILY_IGNORED,
            pdata->swapchain.presentresource_array[frameindex].image,
            {
                VK_IMAGE_ASPECT_COLOR_BIT,
                0,
                1,
                0,
                1
            } 
        }
    };
    
    vkCmdPipelineBarrier(cmdbuffer,
                         VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                         VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                         0,
                         0,0,0,0,_arraycount(present_membarrier),&present_membarrier[0]);
    
    
    VkClearValue clearvalue[2] = {};
    
    clearvalue[0] = {
        {{context->clearcolor.R,context->clearcolor.G,context->clearcolor.B,context->clearcolor.A}},
    };
    
    clearvalue[1].color = {};
    clearvalue[1].depthStencil = {1.0f,0};
    
    VStartRenderpass(cmdbuffer,
                     VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS,renderpass,
                     framebuffer,
                     {{0,0},{pdata->swapchain.width,
                     pdata->swapchain.height}},
                     &clearvalue[0],_arraycount(clearvalue));
    
    _vthreaddump("--------new frame-------------------%s\n","");
    
    _vthreaddump("prim framebuffer %p\n",(void*)framebuffer);
    
    DispatchRenderContext(context,pdata->worker_sem);
    
    {
        TIMEBLOCKTAGGED("THREADED_DRAW",SteelBlue);
        ThisThreadExecuteRenderBatch(context,&pdata->drawcmdbuffer);
    }
    
    VkCommandBuffer* cmdbuffers = 0;
    u32 cmdbuffers_count = 0;
    
    
    GetCmdBufferArray(context,&cmdbuffers,&cmdbuffers_count);
    
    
    _vthreaddump("cmdbufferlistcount %d\n",cmdbuffers_count);
    
    if(cmdbuffers_count){
        vkCmdExecuteCommands(cmdbuffer,cmdbuffers_count,cmdbuffers);
    }
    
    
    
    VEndRenderPass(cmdbuffer);
    
    VTEnd(cmdbuffer);  
    
}

void SetupData(void** platform,void** game){
    
    s8* data = (s8*)alloc(sizeof(PlatformData) + sizeof(GameData));
    
    *platform = (void*)data;
    
    *game = (void*)(data + sizeof(PlatformData));
    
    printf("platform %d game %d total %d\n",(u32)sizeof(PlatformData),(u32)sizeof(GameData),
           (u32)(sizeof(PlatformData) + sizeof(GameData)));
    
}


PrimaryRenderCommandbuffer* CreatePrimaryRenderCommandbuffer(
VDeviceContext* device,
VkCommandPool pool,u32 count){
    
    PrimaryRenderCommandbuffer* array =
        (PrimaryRenderCommandbuffer*)alloc(sizeof(PrimaryRenderCommandbuffer) * count);
    
    for(u32 i = 0; i < count; i++){
        
        array[i].buffer =
            VAllocateCommandBuffer(device,pool,VK_COMMAND_BUFFER_LEVEL_PRIMARY);
    }
    
    return array;
}


u32 _ainline GenRenderKey(u64 val1){
    
    u64 t = val1 >> 32;
    
    t ^= val1;
    
    return (u32)t;
}

void ThreadSingleEntryProc(Threadinfo info){
    
    _persist volatile  _cachealign u32 single_entry_lock = 0;
    
    if(single_entry_lock){
        return;
    }
    
    auto is_locked = single_entry_lock;
    
    u32 actual_islocked = LockedCmpXchg(&single_entry_lock,is_locked,is_locked + 1);
    
    if((is_locked != actual_islocked) || is_locked){
        return;
    }
    
    //TODO: do all asset transfers here too
    
    
    ExecuteThreadTextureFetchQueue(info.fetchqueue);
    
    
    //TODO: only do this once the device is not busy
    //(need a way to ask if last batch was done)
    //ResetAsyncTransferBuffer();
    
    single_entry_lock = 0;
}

s64 ThreadProc(void* args){
    
    RECORDTHREAD();
    
    Threadinfo info;
    
    memcpy(&info,args,sizeof(info));
    
    auto drawbuffers = CreateThreadRenderData(&info.vdevicecontext);
    
    for(;;){
        
        TWaitSemaphore(info.this_sem);
        
        TIMEBLOCKTAGGED("THREADTOTALTIME",Crimson);
        
        while(ExecuteThreadWorkQueue(info.queue,(void*)&drawbuffers)){}
        
        ExecuteRenderBatch(info.rendercontext,&drawbuffers);
        
        ThreadSingleEntryProc(info);
        
    }
    
    return 0;
}

u32 DeployAllThreads(Threadinfo* info){
    
    RECORDTHREAD();
    
#ifdef DEBUG
    
    u32 total_threads = 3;
    
#else
    
    u32 total_threads = SGetTotalThreads() - 1;
    
#endif
    
    
    
    for(u32 i = 0; i < total_threads;i++){
        
        TCreateThread(ThreadProc,_megabytes(22),(void*)info);
    }
    
    return total_threads;
}

struct AudioArgsLayout{
    u32 towrite;
    AAudioBuffer* musicbuffer;
    AAudioBuffer* submitbuffer;
    
};

struct MixAudioLayout{
    AAudioBuffer* submitbuffer;
    EntityAudioData* audio_data;
    u32 audio_count;
};

void MixAudio(void* data,void*){
    
    auto layout = (MixAudioLayout*)data;
    
    auto submitbuffer = layout->submitbuffer;
    auto audio_data = layout->audio_data;
    auto audio_count = layout->audio_count;
    
    TIMEBLOCK(BlueViolet);
    
    memset(submitbuffer->data,0,submitbuffer->size);
    
    for(u32 i = 0; i < audio_count; i++){
        
        auto audio = &audio_data[i];
        
        CommitAudio(&audio->audioasset);
        
        //TODO: Restructure this properly
        
        if(submitbuffer->size > audio->audioasset.avail_size){
            
            auto dst = (s8*)audio->audioasset.ptr;
            u32 readsize = _fixed_audio - audio->audioasset.avail_size;
            
            if((audio->audioasset.file_offset + readsize) > audio->audioasset.file_size){
                
                u32 remaining = audio->audioasset.file_size - audio->audioasset.file_offset;
                
                ADFGetData(audio->audioasset.assetfile,(dst + audio->audioasset.avail_size),
                           &audio->audioasset.file_offset,remaining);
                
                audio->audioasset.avail_size +=remaining;
                
                if(audio->islooping){
                    readsize -= remaining;
                    audio->audioasset.file_offset = 0;
                }
                
                else{
                    
                    audio->toremove++;//if this is our first round here, don't remove
                    
                    u32 zerosize = _fixed_audio - audio->audioasset.avail_size;	  
                    memset((dst + audio->audioasset.avail_size),0,zerosize);
                    
                    readsize = 0;
                }
                
            }
            
            ADFGetData(audio->audioasset.assetfile,(dst + audio->audioasset.avail_size),
                       &audio->audioasset.file_offset,readsize);
            
            audio->audioasset.avail_size = _fixed_audio;
            
        }
        
        //TODO:Use simd
        
        auto submitdata = (s16*)submitbuffer->data;
        auto cpydata = (s16*)audio->audioasset.ptr;
        
        u32 len = submitbuffer->size/2;
        
        for(u32 i = 0; i < len; i++){
            submitdata[i] += cpydata[i];
        }
        
        auto tcpy = (s8*)cpydata;
        
        audio->audioasset.avail_size -= submitbuffer->size;
        
        //To side step overlap in sanitizers
#if 0
        memcpy(tcpy,(tcpy + submitbuffer->size),(audio->audioasset.avail_size));
#else
        memmove(tcpy,(tcpy + submitbuffer->size),(audio->audioasset.avail_size));
#endif
    }
    
    {
        TIMEBLOCKTAGGED("PlayAudio",Red);
        
#if _mute_sound
        memset(pdata->submit_audiobuffer.data,0,pdata->submit_audiobuffer.size_frames * sizeof(u32));
#endif
        
        APlayAudioDevice(pdata->audio,pdata->submit_audiobuffer.data,
                         pdata->submit_audiobuffer.size_frames);  
    }
    
}


void FillAudioBuffers(void* args){
    
    AudioArgsLayout* audioargs = (AudioArgsLayout*)args;
    
    u32 towrite = audioargs->towrite;
    AAudioBuffer* music_audiobuffer = audioargs->musicbuffer;
    AAudioBuffer* submit_audiobuffer = audioargs->submitbuffer;
    
    if(music_audiobuffer->cur_pos ==  music_audiobuffer->size){
        music_audiobuffer->cur_pos = 0;
    }
    
    s8* curpos = ((s8*)music_audiobuffer->data) + music_audiobuffer->cur_pos;
    
    if((music_audiobuffer->cur_pos + towrite) > music_audiobuffer->size){
        
        u32 overwrite = (music_audiobuffer->cur_pos + towrite) - music_audiobuffer->size;
        
        towrite-= overwrite;
        
        memcpy(submit_audiobuffer->data,curpos,towrite);
        
        
        void* ptr = ((s8*)submit_audiobuffer->data) + towrite;
        
        memset(ptr,0,overwrite);
        
    }
    else{
        memcpy(submit_audiobuffer->data,curpos,towrite);
    }
    
    music_audiobuffer->cur_pos += towrite;
    
}

void ProcessObjUpdateList(){
    
    TIMEBLOCK(Purple);
    
    if(!pdata->objupdate_count){
        return;
    }
    
    
    
    //MARK: why do we have to sort?
    qsort(pdata->objupdate_array,pdata->objupdate_count,
          sizeof(ObjUpdateEntry),
          [](const void * a, const void* b)->s32 {
          
          auto entry_a = (ObjUpdateEntry*)a;
          auto entry_b = (ObjUpdateEntry*)b;
          
          return entry_a->offset - entry_b->offset;
          });
    
    auto light_ubo = (LightUBO*)pdata->lightupdate_ptr;
    
    //write the light list
    light_ubo->point_count = pdata->point_count;
    light_ubo->spot_count = pdata->spot_count;
    
    VMemoryRangesPtr ranges = {TAlloc(VkMappedMemoryRange,pdata->objupdate_count + 1 + light_ubo->dir_count + pdata->point_count + pdata->spot_count),0};
    
    
    for(u32 i = 0; i < pdata->objupdate_count; i++){
        
        auto entry = pdata->objupdate_array[i];
        
        VPushBackMemoryRanges(&ranges,pdata->skel_ubo.memory,
                              entry.offset,entry.data_size);
        
    }
    
    VPushBackMemoryRanges(&ranges,pdata->light_ubo.memory,
                          0,sizeof(u32) * 4);
    
    //MARK: hardcoded for now
    VPushBackMemoryRanges(&ranges,pdata->light_ubo.memory,
                          offsetof(LightUBO,point_array),sizeof(LightUBO::PointLight) * pdata->point_count);
    
    pdata->objupdate_count++;
    
    
    {
        TIMEBLOCKTAGGED("vkFlush",Green);
        VFlushMemoryRanges(&pdata->vdevice,&ranges);
        
    }
    
}

void ThreadUpdateUniformBuffer(void*,void*){
    
    TIMEBLOCKTAGGED("UPDATEUBUFFER",Green);
    
    ProcessObjUpdateList();
}

void ThreadLinearBlend(void* args,void*){
    
    TIMEBLOCK(Violet);
    
    ThreadLinearBlendRes* data = (ThreadLinearBlendRes*)args;
    
    ALinearBlend(data->time,data->animation_index,data->set_array,data->root,data->result);
    
    PushUpdateEntry(data->id,offsetof(SkelUBO,bone_array),
                    data->bone_count * sizeof(Matrix4b4),data->result);
}

_persist u32 is_first_present = true;

void PresentBuffer(PlatformData* pdata){
    
    auto frameindex = pdata->swapchain.image_index;
    
    VkDevice device = pdata->vdevice.device;
    VSwapchainContext swapchain = pdata->swapchain;
    VkQueue queue = pdata->root_queue;
    VkCommandBuffer commandbuffer = pdata->rendercmdbuffer_array[frameindex].buffer;
    VkFence fence = pdata->present_fence;
    VkSemaphore wait_semaphore = pdata->waitacquireimage_semaphore;
    VkSemaphore signal_semaphore = pdata->waitfinishrender_semaphore;
    
    TIMEBLOCK(Magenta);
    
    auto cmdbuffer = pdata->rendercmdbuffer_array[frameindex].buffer;
    
    if(is_first_present){
        is_first_present = false;
    }
    
    else{
        
        {
            TIMEBLOCKTAGGED("Wait for fence",Pink);
            vkWaitForFences(device,1,&fence,VK_TRUE,0xFFFFFFFFFFFFFFFF);
            vkResetFences(device,1,&fence);  
        }
        
        auto fetch_cmdbuffer =
            GenerateTextureFetchRequests(&pdata->fetchqueue,pdata->worker_sem);
        
        if(fetch_cmdbuffer){
            vkCmdExecuteCommands(cmdbuffer,1,&fetch_cmdbuffer);    
        }
        
    }
    
    
    
    VEndCommandBuffer(cmdbuffer);
    
    /*
      tell queue to wait at the transfer stage until presentation engine has gotten us an image to render to. Signal the semaphore to start presenting when we are done
    */
    
    {
        TIMEBLOCKTAGGED("PSubmit",Green);
        
        VkSemaphore waitsem[] = {wait_semaphore};
        
        VkPipelineStageFlags stage[] =
        {VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,VK_PIPELINE_STAGE_TRANSFER_BIT};
        
        VSubmitCommandBuffer(queue,commandbuffer,
                             waitsem,_arraycount(waitsem),
                             &signal_semaphore,1,
                             stage,fence);
    }
    
    {
        TIMEBLOCKTAGGED("QPresent",Gold);
        VQueuePresent(queue,swapchain.image_index,swapchain.swap,
                      signal_semaphore); 
    }
    
}

struct PresentLayout{
    VkDevice device;
    VSwapchainContext swapchain;
    VkQueue queue;
    VkCommandBuffer commandbuffer;
    VkFence fence;
    VkSemaphore wait_semaphore;
    VkSemaphore signal_semaphore;
};

VkRenderPass SetupRenderPass(VDeviceContext* vdevicecontext,
                             VSwapchainContext swapchaincontext){
    
    VAttachmentSpec attchement_spec;
    
    VPushBackAttachmentSpec(&attchement_spec,0,swapchaincontext.internal->format,
                            VK_SAMPLE_COUNT_1_BIT,VK_ATTACHMENT_LOAD_OP_CLEAR,
                            VK_ATTACHMENT_STORE_OP_STORE,VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                            VK_ATTACHMENT_STORE_OP_DONT_CARE,
                            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                            VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
    
    VPushBackAttachmentSpec(&attchement_spec,0,
                            swapchaincontext.internal->depthstencil.format,
                            VK_SAMPLE_COUNT_1_BIT,VK_ATTACHMENT_LOAD_OP_CLEAR,
                            VK_ATTACHMENT_STORE_OP_DONT_CARE,
                            VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                            VK_ATTACHMENT_STORE_OP_DONT_CARE,
                            VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                            VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
    
    
    
    VSubpassDescriptionSpec subpassdesc_spec;
    
    VPushBackSubpassDescriptionSpec(&subpassdesc_spec,0,
                                    VK_PIPELINE_BIND_POINT_GRAPHICS,
                                    0,
                                    0,
                                    1,
                                    &attchement_spec.ref_container[0],
                                    0,
                                    &attchement_spec.ref_container[1],
                                    0,
                                    0);
    
    VSubpassDependencySpec subpassdep_spec = {};
    
    return VCreateRenderPass(vdevicecontext,0,attchement_spec,subpassdesc_spec,
                             subpassdep_spec);
}

void SetupFrameBuffers(VDeviceContext* _restrict  device,
                       VkCommandBuffer commandbuffer,VkQueue queue,VkRenderPass renderpass,
                       VSwapchainContext* _restrict swapchain){
    
    VkImageMemoryBarrier imagememorybarrier;
    
    for(u32 i = 0; i < swapchain->image_count; i++){
        
        
        VkImageView imageview_array[] = {swapchain->presentresource_array[i].view,
            swapchain->internal->depthstencil.view};
        
        swapchain->presentresource_array[i].framebuffer =
            VCreateFrameBuffer(device,0,renderpass,
                               imageview_array,_arraycount(imageview_array),
                               swapchain->width,
                               swapchain->height,1);
        
    }
    
    imagememorybarrier =
    {
        VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        0,
        0,
        0,
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
        VK_QUEUE_FAMILY_IGNORED,
        VK_QUEUE_FAMILY_IGNORED,
        swapchain->internal->depthstencil.image,
        {VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT,0,1,0,1}
    };
    
    
    VStartCommandBuffer(commandbuffer,
                        VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
    
    vkCmdPipelineBarrier(commandbuffer,VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                         VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,0,
                         0,0,0,0,1,
                         &imagememorybarrier);
    
    
    VEndCommandBuffer(commandbuffer);
    
    VSubmitCommandBuffer(queue,commandbuffer);
    
    vkQueueWaitIdle(queue);
}



#define _test(call) if(call < 0){_kill("",1);}
#define _ms2frames(ms) (((f32)(ms) * 48.0f) + 0.5f)
#define _frames2ms(frames) (((f32)frames)/48.0f)

void _ainline ProcessEvents(WWindowContext* windowcontext,KeyboardState* keyboardstate,
                            MouseState* mousestate,void* args){
    
    TIMEBLOCK(Green);
    
    memcpy(keyboardstate->prevkeystate,keyboardstate->curkeystate,
           sizeof(keyboardstate->prevkeystate));
    
    memcpy(mousestate->prevstate,mousestate->curstate,
           sizeof(mousestate->prevstate));
    
    WWindowEvent event;
    
    while( WWaitForWindowEvent(windowcontext,&event)){
        
        switch(event.type){
            
            
            //MARK: shut up compiler!
            case W_EVENT_NONE:{}break;
            
            case W_EVENT_EXPOSE:{
                printf("window exposed\n");
            }
            break;
            
            case W_EVENT_RESIZE:{
                //recreate swapbuffer whenever this happens.
            }
            break;
            
            case W_EVENT_CLOSE:{
                printf("window close\n");
                *((logic*)args) = 0;
            }
            break;
            
            case W_EVENT_KBEVENT_KEYDOWN:{
                
                keyboardstate->curkeystate[event.keyboard_event.keycode] = 1;
            }
            break;
            
            case W_EVENT_KBEVENT_KEYUP:{
                
                keyboardstate->curkeystate[event.keyboard_event.keycode] = 0;
            }
            break;
            
            case W_EVENT_MSEVENT_MOVE:{
                mousestate->x = event.mouse_event.x;
                mousestate->y = event.mouse_event.y;
            }break;
            
            case W_EVENT_MSEVENT_DOWN:{
                mousestate->curstate[event.mouse_event.keycode] = 1;
                
            }break;
            
            case W_EVENT_MSEVENT_UP:{
                mousestate->curstate[event.mouse_event.keycode] = 0;
            }break;
            
        }
        
    }
    
}

Vector3 TranslateWorldSpaceToClipSpace(Vector3 pos){
    
    return WorldSpaceToClipSpace(pos,pdata->proj * pdata->view);
}



Vector3 TranslateClipSpaceToWorldSpace(Vector3 pos){
    
    return ClipSpaceToWorldSpace(pos,pdata->proj * pdata->view);
}


void SetActiveCameraOrientation(Vector3 pos,Vector3 lookdir){  
    
    pdata->camerapos = ToVec4(pos);
    
    pdata->view = ViewMatrix(pos,pos + lookdir,Vector3{0.0f,-1.0f,0.0f});
}

void SetObjectOrientation(u32 obj_id,Vector3 pos,Quaternion rot,f32 scale){
    
    _kill("too many entries\n",
          pdata->objupdate_count >= _arraycount(PlatformData::objupdate_array));
    
    auto orientation = TAlloc(Matrix4b4,1);
    
    *orientation =
    
#if _row_major
    
        Transpose(WorldMatrix(pos,rot,Vector3{scale,scale,scale}));
    
#else
    
    WorldMatrix(pos,rot,Vector3{scale,scale,scale});
    
#endif
    
    PushUpdateEntry(obj_id,offsetof(SkelUBO,world),sizeof(SkelUBO::world),orientation);
}

void SetObjectMaterial(u32 obj_id,u32 mat_id){
    
    auto mat = &pdata->scenecontext.materialasset_array[mat_id];
    
    PushUpdateEntry(obj_id,offsetof(SkelUBO,texture_array),
                    sizeof(u32) * mat->textureid_count,&mat->textureid_array);
}

#define _PIPELINECACHE_FILE "pipelinecache_file.txt"

void SetupPipelineCache(){
    
    void* cache_data = 0;
    ptrsize cache_size = 0;
    
    if(FIsFileExists(_PIPELINECACHE_FILE)){
        
        auto file = FOpenFile(_PIPELINECACHE_FILE,F_FLAG_READWRITE);
        cache_data = FReadFileToBuffer(file,&cache_size);
        
        FCloseFile(file);
    }
    
    pdata->pipelinecache = VCreatePipelineCache(&pdata->vdevice,cache_data,cache_size);
    
    if(cache_data){
        unalloc(cache_data);
    }
}


/*
FIXME: 
Run-Time Check Failure #2 - Stack around the variable 'write_cache_size' was corrupted
*/
void WritePipelineCache(){
    
    ptrsize write_cache_size = 0;
    
    //write the cache data
    VGetPipelineCacheData(&pdata->vdevice,pdata->pipelinecache,0,&write_cache_size);
    
    auto write_cache_data = TAlloc(s8,write_cache_size);
    
    VGetPipelineCacheData(&pdata->vdevice,pdata->pipelinecache,write_cache_data,&write_cache_size);
    
    
    auto file = FOpenFile(_PIPELINECACHE_FILE,F_FLAG_READWRITE | F_FLAG_CREATE | F_FLAG_TRUNCATE);
    
    FWrite(file,write_cache_data,write_cache_size);
    
    FCloseFile(file);
}


void CompileAllPipelines(PlatformData* pdata){
    
    if(pdata->pipeline_array[PSTATIC]){
        VDestroyPipeline(&pdata->vdevice,pdata->pipeline_array[PSTATIC]);
    }
    
    if(pdata->pipeline_array[PSKEL]){
        VDestroyPipeline(&pdata->vdevice,pdata->pipeline_array[PSKEL]);
    }
    
    
    SPXData shader_data_1[] = {
        LoadSPX(SHADER_PATH(model_skel.vert.spx)),
        LoadSPX(SHADER_PATH(vt_generic.frag.spx)),
        
    };
    
    SPXData shader_data_2[] = {
        LoadSPX(SHADER_PATH(model.vert.spx)),
        LoadSPX(SHADER_PATH(vt_generic.frag.spx)),
        
    };
    
    VGraphicsPipelineSpecObj spec_array[2] = {};
    
    {
        
        VkSpecializationInfo info[] = {
            {},
            VTFragmentShaderSpecConst()
        };
        
        
        auto shaderobj = VMakeShaderObjSPX(&shader_data_1[0],2,&info[0],_arraycount(info));
        
        spec_array[0] = VMakeGraphicsPipelineSpecObj(&pdata->vdevice,&shaderobj,pdata->pipelinelayout,pdata->renderpass,0,&pdata->swapchain);
        
        VSetDepthStencilGraphicsPipelineSpec(&spec_array[0],
                                             VK_TRUE,
                                             VK_TRUE,VK_COMPARE_OP_LESS_OR_EQUAL,
                                             VK_TRUE);
    }
    
    
    {
        
        VkSpecializationInfo info[] = {
            {},
            VTFragmentShaderSpecConst()
        };
        
        
        auto shaderobj = VMakeShaderObjSPX(&shader_data_2[0],2,&info[0],_arraycount(info));
        
        spec_array[1] = VMakeGraphicsPipelineSpecObj(&pdata->vdevice,&shaderobj,pdata->pipelinelayout,pdata->renderpass,0,&pdata->swapchain);
        
        VSetDepthStencilGraphicsPipelineSpec(&spec_array[1],
                                             VK_TRUE,
                                             VK_TRUE,VK_COMPARE_OP_LESS_OR_EQUAL,
                                             VK_TRUE);
        
        
    }
    
    VCreateGraphicsPipelineArray(&pdata->vdevice,&spec_array[0],_arraycount(spec_array),&pdata->pipeline_array[0],pdata->pipelinecache);
    
    //set rendercontext resources
    {
        pdata->rendercontext.rendergroup[0].pipeline = pdata->pipeline_array[PSKEL];
        pdata->rendercontext.rendergroup[0].pipelinelayout = pdata->pipelinelayout;
        
        pdata->rendercontext.rendergroup[0].descriptorset_array =
            &pdata->dynuniform_skel_descriptorset;
        
        pdata->rendercontext.rendergroup[0].scissor_array = 0;
        pdata->rendercontext.rendergroup[0].viewport_array = 0;
        pdata->rendercontext.rendergroup[0].descriptorset_count = 2;
        pdata->rendercontext.rendergroup[0].scissor_count = 0;
        pdata->rendercontext.rendergroup[0].viewport_count = 0;
        
        pdata->rendercontext.rendergroup[1].pipeline = pdata->pipeline_array[PSTATIC];
        pdata->rendercontext.rendergroup[1].pipelinelayout = pdata->pipelinelayout;
        pdata->rendercontext.rendergroup[1].descriptorset_array =
            &pdata->dynuniform_skel_descriptorset;
        pdata->rendercontext.rendergroup[1].scissor_array = 0;
        pdata->rendercontext.rendergroup[1].viewport_array = 0;
        pdata->rendercontext.rendergroup[1].descriptorset_count = 2;
        pdata->rendercontext.rendergroup[1].scissor_count = 0;
        pdata->rendercontext.rendergroup[1].viewport_count = 0;
    }
}



void ClearLightList(){
    
    pdata->point_count = 0;
    pdata->spot_count = 0;
}

void SetAmbientColor(Color color ,f32 intensity){
    
    auto light_ubo = (LightUBO*)pdata->lightupdate_ptr;
    
    Vector4 c = Vector4{color.R,color.G,color.B,color.A} * intensity;
    
    light_ubo->ambient_color = Color{c.x,c.y,c.z,1.0f};
}

void GetDirLightList(DirLight** array,u32** count){
    
    auto light_ubo = (LightUBO*)pdata->lightupdate_ptr;
    
    *count = &light_ubo->dir_count;
    *array = &light_ubo->dir_array[0];
}

void AddPointLight(Vector3 pos,Color color,f32 radius){
    
    auto light_ubo = (LightUBO*)pdata->lightupdate_ptr;
    
    //TODO: make radius into a proper distance cut off
    
    light_ubo->point_array[pdata->point_count] = {
        ToVec4(pos),color,radius
    };
    
    pdata->point_count++;
}

void AddSpotLight(Vector3 pos,Vector3 dir,Color color,f32 full_angle,f32 hard_angle,f32 radius){
    
    _kill("hard must be less than full\n",hard_angle > full_angle);
    
    auto light_ubo = (LightUBO*)pdata->lightupdate_ptr;
    
    light_ubo->spot_array[pdata->spot_count] = {ToVec4(pos),ToVec4(dir),color,cosf(_radians(full_angle * 0.5f)),cosf(_radians(hard_angle * 0.5f)),radius};
    
    pdata->spot_count ++;
    
}


void AddModel(const s8* filepath,VkQueue queue,VkCommandBuffer cmdbuffer){
    
    pdata->scenecontext.modelasset_array[pdata->scenecontext.modelasset_count] =
        AllocateAssetModel(filepath,&pdata->vdevice,
                           queue,cmdbuffer,_vertexbindingno);
    
    pdata->scenecontext.modelasset_count++;
}

void AddAnimatedModel(const s8* filepath,VkQueue queue,VkCommandBuffer cmdbuffer){
    
    AllocateAssetAnimated(filepath,
                          &pdata->vdevice,queue,cmdbuffer,_vertexbindingno,
                          &pdata->scenecontext.animatedasset_array[
                          pdata->scenecontext.animatedasset_count],
                          &pdata->scenecontext.modelasset_array[pdata->scenecontext.modelasset_count]);
    
    auto model =
        &pdata->scenecontext.modelasset_array[pdata->scenecontext.modelasset_count];
    
    model->animation_id = pdata->scenecontext.animatedasset_count;
    
    pdata->scenecontext.animatedasset_count++;
    pdata->scenecontext.modelasset_count++;
    
}

//MARK:
void InitSceneContext(PlatformData* pdata,VkCommandBuffer cmdbuffer,
                      VkQueue queue){
    
    
    f32 aspectratio = ((f32)pdata->window.width)/((f32)pdata->window.height);
    
    pdata->proj = ProjectionMatrix(_radians(90.0f),aspectratio,0.1f,256.0f);
    
    {
        
        VStartCommandBuffer(cmdbuffer,
                            VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
        
        pdata->scenecontext.animatedasset_count = 0;
        pdata->scenecontext.modelasset_count = 0;
        
        pdata->scenecontext.textureasset_array[pdata->scenecontext.textureasset_count] =
            AllocateAssetTexture(IMAGE_PATH(goblin.tdf),&pdata->vdevice,cmdbuffer);
        
        pdata->scenecontext.textureasset_count++;
        
        pdata->scenecontext.textureasset_array[pdata->scenecontext.textureasset_count] =
            AllocateAssetTexture(IMAGE_PATH(jack_o_lantern.tdf),&pdata->vdevice,cmdbuffer);
        
        pdata->scenecontext.textureasset_count++;
        
        //MARK:Allocate Materials
        {
            
            auto mat1 = &pdata->scenecontext.materialasset_array[0];
            auto mat2 = &pdata->scenecontext.materialasset_array[1];
            pdata->scenecontext.materialasset_count = 2;
            
            *mat1 = AllocateAssetMaterial(&pdata->vdevice);
            *mat2 = AllocateAssetMaterial(&pdata->vdevice);
            
            MaterialAddTexture(mat1,TextureType_Diffuse,0);
            MaterialAddTexture(mat2,TextureType_Diffuse,1);
        }
        
        
        AddAnimatedModel(MODEL_PATH(goblin.mdf),queue,cmdbuffer);
        //AddAnimatedModel(MODEL_PATH(knight.mdf),queue,cmdbuffer);
        //AddAnimatedModel(MODEL_PATH(golem_clean.mdf),queue,cmdbuffer);
        
        
        //AddModel(MODEL_PATH(teapot.mdf),queue,cmdbuffer);
        //AddModel(MODEL_PATH(box.mdf),queue,cmdbuffer);
        
        
        VEndCommandBuffer(cmdbuffer);
        
        VSubmitCommandBuffer(queue,cmdbuffer);
        
        vkQueueWaitIdle(queue);
    }
    
    
    SPXData shader_data[] = {
        LoadSPX(SHADER_PATH(model_skel.vert.spx)),
        LoadSPX(SHADER_PATH(vt_generic.frag.spx))
    };
    
    auto shader_obj = VMakeShaderObjSPX(shader_data,_arraycount(shader_data));
    
    
    
    VDescriptorPoolSpec poolspec;
    
    VDescPushBackPoolSpec(&poolspec,&shader_obj,2);
    
    pdata->descriptorpool = VCreateDescriptorPoolX(&pdata->vdevice,poolspec);
    
    
    pdata->dynuniform_descriptorlayout =
        VCreateDescriptorSetLayout(&pdata->vdevice,&shader_obj,0);
    
    
    pdata->vt_descriptorlayout =
        VCreateDescriptorSetLayout(&pdata->vdevice,&shader_obj,1);
    
    VkDescriptorSetLayout desclayout_array[] = {
        pdata->dynuniform_descriptorlayout,
        pdata->vt_descriptorlayout,
    };
    
    
    
    VAllocDescriptorSetArray(&pdata->vdevice,pdata->descriptorpool,
                             _arraycount(desclayout_array),&desclayout_array[0],
                             &pdata->dynuniform_skel_descriptorset);
    
    
    
    VkDescriptorSetLayout desc_layout[] = {
        pdata->dynuniform_descriptorlayout,
        pdata->vt_descriptorlayout,
    };
    
    
    
    pdata->pipelinelayout = VCreatePipelineLayout(&pdata->vdevice,
                                                  &desc_layout[0],_arraycount(desc_layout),&shader_obj);
    
    VDescriptorWriteSpec writespec;
    
    
    auto skel_binfo = VGetBufferInfo(&pdata->skel_ubo,0,sizeof(SkelUBO));
    
    VDescPushBackWriteSpecBuffer(&writespec,pdata->dynuniform_skel_descriptorset,0,0,1,
                                 VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,&skel_binfo);
    
    auto light_binfo = VGetBufferInfo(&pdata->light_ubo,0,sizeof(LightUBO));
    
    VDescPushBackWriteSpecBuffer(&writespec,pdata->vt_descriptorset,3,0,1,
                                 VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,&light_binfo);
    
    VkDescriptorImageInfo image_info[17] =
    {
        {
            GetTextureCache()->sampler,
            GetTextureCache()->view,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL  
        }
        
    };
    
    for(u32 i = 0; i < pdata->scenecontext.textureasset_count; i++){
        
        const auto tasset = pdata->scenecontext.textureasset_array[i];
        
        image_info[i + 1] = {
            tasset->pagetable.sampler,
            tasset->pagetable.view,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        };
        
    }
    
    VDescPushBackWriteSpecImage(&writespec,pdata->vt_descriptorset,0,0,1,
                                VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                &image_info[0]);
    
    
    VDescPushBackWriteSpecImage(&writespec,pdata->vt_descriptorset,1,0,
                                pdata->scenecontext.textureasset_count,
                                VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                &image_info[1]);
    
    
    VkDescriptorImageInfo vt_readbackbufferinfo = {
        0,
        GetVTReadbackBuffer().view,
        VK_IMAGE_LAYOUT_GENERAL
    };
    
    VDescPushBackWriteSpecImage(&writespec,pdata->vt_descriptorset,
                                2,0,1,VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
                                &vt_readbackbufferinfo);
    
    
    VUpdateDescriptorSets(&pdata->vdevice,writespec);
    
    InitRenderContext(&pdata->rendercontext);
    
    pdata->scenecontext.TranslateClipSpaceToWorldSpace = TranslateClipSpaceToWorldSpace;
    pdata->scenecontext.TranslateWorldSpaceToClipSpace = TranslateWorldSpaceToClipSpace;
    pdata->scenecontext.SetObjectMaterial = SetObjectMaterial;
    pdata->scenecontext.SetActiveCameraOrientation = SetActiveCameraOrientation;
    pdata->scenecontext.SetObjectOrientation = SetObjectOrientation;
    pdata->scenecontext.AddPointLight = AddPointLight;
    pdata->scenecontext.AddSpotLight = AddSpotLight;
    
    pdata->scenecontext.GetDirLightList = GetDirLightList;
    pdata->scenecontext.SetAmbientColor = SetAmbientColor;
    
    
    //asset stuff
    pdata->scenecontext.AllocateAssetAudio = AllocateAssetAudio;
    pdata->scenecontext.CommitAnimated = CommitAnimated;
    pdata->scenecontext.UnallocateAsset = UnallocateAsset;
    
}


void _ainline DispatchSkelLinearBlend(EntityAnimationData* anim){
    
    auto context = &pdata->scenecontext;
    
    anim->animationtime += anim->speed * context->prev_frametime;
    
    auto anim_handle = &context->animatedasset_array[anim->animdata_id];
    
    CommitAnimated(anim_handle);
    
    auto blend = TAlloc(ThreadLinearBlendRes,1);
    
    blend->id = anim->id;
    blend->time = anim->animationtime;
    blend->bone_count = anim_handle->bone_count;
    blend->animation_index = anim->animationindex;
    blend->set_array = anim_handle->animationset_array;
    blend->root = anim_handle->rootbone;
    
    blend->result = TAlloc(Matrix4b4,blend->bone_count);
    
    PushThreadWorkQueue(&pdata->threadqueue,
                        ThreadLinearBlend,(void*)blend,pdata->worker_sem);
}

void _ainline ProcessDrawList(){
    
    auto array = pdata->scenecontext.draw_array;
    auto count = pdata->scenecontext.draw_count;
    
    for(u32 i = 0; i < count; i++){
        
        auto entry = &array[i];
        
        auto offset = entry->id * sizeof(SkelUBO);
        auto model = &pdata->scenecontext.modelasset_array[entry->model];
        auto group = entry->group;
        
        _kill("offset is not aligned\n",(offset % 256) != 0);
        
        InternalPushRenderEntry(&pdata->rendercontext,group,model,offset);
        
        if(model->animation_id != (u32)-1){
            
            auto anim_array = pdata->scenecontext.animationdata_array;
            auto anim_count = pdata->scenecontext.animationdata_count;
            
            for(u32 j = 0; j < anim_count; j++){
                auto anim = &anim_array[j];
                
                if(anim->id == entry->id){
                    DispatchSkelLinearBlend(anim);
                    break;  
                }
                
            }
            
        }
        
    }
    
}



void PrintFileAsArray(const s8* filepath){
    
    auto file = FOpenFile(filepath,F_FLAG_READONLY);
    
    ptrsize size;
    
    auto src = FReadFileToBuffer(file,&size);
    
    auto dst = (s8*)alloc(_megabytes(22));
    
    PBufferListToArrayString((s8*)"hash_array",src,size,dst,0);
    
    printf("%s\n",dst);
    
    FCloseFile(file);
    unalloc(dst);
}



u64 GenGPUHash(VkPhysicalDeviceProperties* prop){
    
    return (PHashString(prop->deviceName) * (prop->deviceType + 13)) ^ ((prop->vendorID + 1) * (prop->deviceID + 1) * 19);
}


void InitAllSystems(){
    
    //Test();
    
    auto settings = ParseSettings();
    
    TInitTimer();
    INIT_DEBUG_TIMER();
    
    InitInternalAllocator();
    InitTAlloc(_megabytes(settings.frame_alloc_size));
    
    SetupData((void**)&pdata,(void**)&gdata);
    
    
    {
        pdata->submit_audiobuffer.size_frames =
            (u32)(_48ms2frames(settings.playbuffer_size_ms));
        
        pdata->submit_audiobuffer.size =
            pdata->submit_audiobuffer.size_frames * sizeof(s16) * 2;
        
        pdata->submit_audiobuffer.data = alloc(pdata->submit_audiobuffer.size); 
    }
    
    pdata->audio =
        ACreateAudioDevice(A_DEVICE_DEFAULT,settings.audio_frequency,settings.audio_channels,settings.audio_format);
    
    
    VInitVulkan();
    
    pdata->window = WCreateVulkanWindow("Cu",(WCreateFlags)(W_CREATE_NORESIZE | settings.backend),settings.window_x,settings.window_y,settings.window_width,settings.window_height);
    
    auto loaded_version = VCreateInstance("eengine",true,VK_MAKE_VERSION(1,0,0),&pdata->window,V_INSTANCE_FLAGS_SINGLE_VKDEVICE);
    
    _kill("requested vulkan version not found\n",loaded_version == (u32)-1);
    
    {
        
        VkPhysicalDevice phys_array[16] = {};
        u32 phys_count = 0;
        
        VEnumeratePhysicalDevices(&phys_array[0],&phys_count,&pdata->window);
        
        //search thru and find device by hash
        
        u32 phys_index = 0;
        
        for(u32 i = 0; i < phys_count; i++){
            
            auto phys = phys_array[i];
            
            VkPhysicalDeviceProperties prop;
            
            vkGetPhysicalDeviceProperties(phys,
                                          &prop);
            
            auto hash = GenGPUHash(&prop);
            
            printf("GPUHASH %d\n",(u32)hash);
            
            if(hash == settings.gpu_device_hash){
                phys_index = i;
                break;
            }
        }
        
        
        pdata->vdevice = VCreateDeviceContext(&phys_array[0]);
        
    }
    
    
    _kill("cannot exceed the max allocated swapchain\n",settings.swapchain_depth > _max_swapchain_count);
    
    
    //MARK: maybe we should separate surface creation and swapchaiun creation to make it possible to query present modes
    pdata->swapchain = VCreateSwapchainContext(&pdata->vdevice,settings.swapchain_depth,&pdata->window,
                                               (VPresentSyncType)settings.vsync_mode);
    
    
    InitAssetAllocator(_megabytes(settings.host_alloc_size),_megabytes(settings.gpu_alloc_size),settings.vt_width,settings.vt_height,&pdata->vdevice,
                       &pdata->swapchain);
    
    
    
    {
        
        
        
        pdata->present_fence = VCreateFence(&pdata->vdevice,(VkFenceCreateFlagBits)0);
        
        printf("swapchain count %d\n",pdata->swapchain.image_count);
        
        pdata->root_queue = VGetQueue(&pdata->vdevice,VQUEUETYPE_ROOT);
        
        pdata->drawcmdbuffer =
            CreateThreadRenderData(&pdata->vdevice);
        
        pdata->transfer_queue = VGetQueue(&pdata->vdevice,VQUEUETYPE_ROOT);
        
        //MARK: just reuse a cmdbuffer
        VkCommandBuffer transfercmdbuffer =
            VAllocateCommandBuffer(&pdata->vdevice,pdata->drawcmdbuffer.pool,
                                   VK_COMMAND_BUFFER_LEVEL_PRIMARY);
        
        //MARK: Handle case where a compute queue doesn't exist
        pdata->compute_queue = VGetQueue(&pdata->vdevice,VQUEUETYPE_COMPUTE);
        
        pdata->waitacquireimage_semaphore = VCreateSemaphore(&pdata->vdevice);
        pdata->waitfinishrender_semaphore = VCreateSemaphore(&pdata->vdevice);
        
        pdata->renderpass = SetupRenderPass(&pdata->vdevice,pdata->swapchain);
        
        SetupFrameBuffers(&pdata->vdevice,transfercmdbuffer,pdata->transfer_queue,
                          pdata->renderpass,&pdata->swapchain);
        
        SetupPipelineCache();
        
        pdata->skel_ubo = VCreateUniformBufferContext(&pdata->vdevice,sizeof(SkelUBO[64]),VMAPPED_NONE);
        
        pdata->light_ubo = VCreateUniformBufferContext(&pdata->vdevice,sizeof(LightUBO),VMAPPED_NONE);
        
        //MARK: keep the obj buffer permanently mapped
        
        VMapMemory(&pdata->vdevice,pdata->skel_ubo.memory,
                   0,pdata->skel_ubo.size,(void**)&pdata->objupdate_ptr);
        
        VMapMemory(&pdata->vdevice,pdata->light_ubo.memory,
                   0,pdata->light_ubo.size,(void**)&pdata->lightupdate_ptr);
        
        memset(pdata->lightupdate_ptr,0,pdata->light_ubo.size);
        
        InitSceneContext(pdata,transfercmdbuffer,pdata->transfer_queue);
        
        
        pdata->rendercmdbuffer_array =
            CreatePrimaryRenderCommandbuffer(&pdata->vdevice,
                                             pdata->drawcmdbuffer.pool,pdata->swapchain.image_count);
        
#if _enable_gui
        
        GUIInit(&pdata->vdevice,&pdata->swapchain,pdata->renderpass,pdata->transfer_queue,
                transfercmdbuffer,pdata->pipelinecache);
        
#endif
        
        GameInitData initdata = {
            &pdata->scenecontext,gdata,&pdata->window,pdata->vdevice,pdata->renderpass,
            transfercmdbuffer,pdata->transfer_queue
        };
        
        
        void (*gameinit_funptr)(GameInitData*);
        pdata->lib = InitGameLibrary((void**)(&gameinit_funptr));
        
        
        gameinit_funptr(&initdata);
        
        pdata->deltatime = 0;
        
        ResetTransferBuffer();
    }
    
    
    //Kickoff worker threads
    {
        
        pdata->worker_sem = TCreateSemaphore();
        pdata->main_sem = TCreateSemaphore();
        
        auto info = TAlloc(Threadinfo,1);
        
        info->this_sem = pdata->worker_sem;
        info->main_sem = pdata->main_sem;
        info->queue = &pdata->threadqueue;
        info->rendercontext = &pdata->rendercontext;
        info->vdevicecontext = pdata->vdevice;
        pdata->fetchqueue = {};
        pdata->fetchqueue.buffer = ((ThreadFetchBatch*)alloc(_FetchqueueSize *
                                                             sizeof(ThreadFetchBatch)));
        info->fetchqueue = &pdata->fetchqueue;
        
        
        if(settings.launch_threads == (u32)-1){
            pdata->threadcount = DeployAllThreads(info);
        }
        
        else{
            pdata->threadcount = settings.launch_threads;
        }
        
    }
}