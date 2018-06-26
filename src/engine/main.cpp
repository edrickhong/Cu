#include "main.h"

#define _targetframerate 16.0f//33.33f
#define _ms2s(ms)  ((f32)ms/1000.0f)

_persist auto ustring = "patchthisvalueinatassetpacktime";


//TODO: move this to pparse (rename it to BufferList to Array String)
void PrintFileListAsArray(const s8* filepath, const s8* name = "array"){
    
    auto file = FOpenFile(filepath,F_FLAG_READONLY);
    
    ptrsize size = 0;
    
    auto buffer = FReadFileToBuffer(file,&size);
    
    FCloseFile(file);
    
    printf("const s8* %s[] = {\n",name);
    
    for(u32 i = 0;;){
        
        if(i >= size){
            break;
        }
        
        s8 dst_buffer[512] ={};
        u32 len = 0;
        
        PGetLine(&dst_buffer[0],&buffer[0],&i,&len);
        
        if(len){
            
            s8 out_string[512] = {};
            out_string[0] = '"';
            
            for(u32 j = 0; j < len;j++){
                
                if(dst_buffer[j] == '\r'){
                    continue;
                }
                
                out_string[j + 1] = dst_buffer[j];
            }
            
            out_string[len] = '"';
            out_string[len + 1] = ',';
            out_string[len + 2] = '\n';
            
            printf(&out_string[0]);
        }
    }
    
    printf("}\n");
    
    unalloc(buffer);
    
}

//MARK: it should fail with pointer or array types
#include "function_refl.h"

#ifndef CPP_PASS

#include "meta.h"

#endif

s32 main(s32 argc,s8** argv){
    
    
    
#if  0
    
#ifdef _WIN32
#define _fontfile FONT_PATH(arial.ttf)
#else
#define _fontfile FONT_PATH(ubuntu-font-family/Ubuntu-B.ttf)
#endif
    
    GUIGenFontFile(_fontfile,"Ubuntu-B.fbmp",100.0f);//200.0f
    exit(0);
    
#endif
    
#if _debug && 0
    {
        
        if(ustring[0] != '!'){
            printf("binary has not been patched\n");
            return -1;
        }
        
        auto exec_size = *((u32*)&ustring[1]);
        
        printf("exec size %d\n",exec_size);
        
        auto file = FOpenFile(&argv[0][2],F_FLAG_READONLY);
        
        FSeekFile(file,exec_size,F_METHOD_START);
        
        s8 buffer[1024] = {};
        u32 size;
        
        FRead(file,&size,sizeof(size));
        FRead(file,&buffer[0],size);
        
        printf("%s\n",&buffer[0]);
        
        FCloseFile(file);
    }
#endif
    
    _kill("ubo too big",sizeof(SkelUBO) > _kilobytes(64));
    _kill("ubo too big",sizeof(PushConst) > 128);
    
    void(*reload)(GameReloadData*) = 0;
    
    //init code
    
    InitInternalAllocator();
    InitTAlloc(_megabytes(32));
    
    SetupData((void**)&pdata,(void**)&gdata);
    
    TInitTimer();
    INIT_DEBUG_TIMER();
    
    auto loaded_version = VCreateInstance("eengine",false,VK_MAKE_VERSION(1,0,0),V_INSTANCE_FLAGS_SINGLE_VKDEVICE);
    
    pdata->window = WCreateVulkanWindow("Cu",W_CREATE_NORESIZE,100,100,1280,720);
    
    _kill("requested vulkan version not found\n",loaded_version == (u32)-1);
    
    {
        
        VkPhysicalDevice phys_array[16];
        u32 phys_count;
        
        VEnumeratePhysicalDevices(&phys_array[0],&phys_count,&pdata->window);
        
        pdata->vdevice = VCreateDeviceContext(&phys_array[0]);
        
    }
    
    
    pdata->swapchain = VCreateSwapchainContext(&pdata->vdevice,2,pdata->window,
                                               VSYNC_NORMAL);
    
    InitAssetAllocator(_gigabytes(1),_megabytes(4),&pdata->vdevice,
                       &pdata->swapchain);
    
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
    
    
    {
        pdata->submit_audiobuffer.size_frames =
            (u32)(_48ms2frames(_targetframerate) + _48ms2frames(8));
        
        pdata->submit_audiobuffer.size =
            pdata->submit_audiobuffer.size_frames * sizeof(s16) * 2;
        
        pdata->submit_audiobuffer.data = alloc(pdata->submit_audiobuffer.size); 
    }
    
    pdata->audio =
        ACreateAudioDevice(A_DEVICE_DEFAULT,48000,2,A_FORMAT_S16LE);
    
    InitSceneContext(pdata,transfercmdbuffer,pdata->transfer_queue);
    
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
        
        pdata->threadcount = DeployAllThreads(info);
    }
    
    
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
    
    while(gdata->running){
        
        {
            
            pdata->objupdate_count = 0;
            gui_draw_is_locked = 0;
            
            Clear(&pdata->rendercontext);
            ClearLightList();
            
#if _enable_gui
            
            GUIUpdate(&pdata->window,&pdata->keyboardstate,&pdata->mousestate,
                      pdata->view,pdata->proj);
            
            GUIBegin();
            
#endif
            
            BUILDGUIGRAPH(gdata->draw_profiler);
            
            
            
            TimeSpec start,end;
            
            MASTERTIMEBLOCKSTART(Aqua);
            
            GetTime(&start);
            
            {
                
                EXECTIMEBLOCK(Black);
                
                
                //MARK: we should move this to buildrendercmdbuffer
                {
                    
                    TIMEBLOCKTAGGED("AcquireImage",Orange);
                    vkAcquireNextImageKHR(pdata->vdevice.device,
                                          pdata->swapchain.swap,0xFFFFFFFFFFFFFFFF,
                                          pdata->waitacquireimage_semaphore,
                                          0,(u32*)&pdata->swapchain.image_index);  
                    
                }
                
                
                UpdateAllocatorTimeStamp();
                Clear(&pdata->threadqueue);
                
                ProcessEvents(&pdata->window,&pdata->keyboardstate,&pdata->mousestate,
                              &gdata->running);
                
                {
                    TIMEBLOCKTAGGED("RELOADLIB",Firebrick);
                    pdata->lib = ReloadGameLibrary(pdata->lib,(void**)&reload,&pdata->scenecontext);
                }
                
                
                if(reload){
                    
                    auto context = &pdata->scenecontext;
                    
                    GameReloadData reloaddata = {
                        gdata,pdata->vdevice,pdata->renderpass,pdata->window,context,GetGUIContext(),
                        GetAAllocatorContext()
                    };
                    
                    vkDeviceWaitIdle(pdata->vdevice.device);
                    
                    reload(&reloaddata);
                    reload = 0;
                    
                    CompileAllPipelines(pdata);
                }
                
                ThreadLinearBlendRes blendres_array[24] = {};
                u32 linearblend_count = 0;
                u32 audio_count = 0;
                EntityAudioData* audio_data;
                
                
                if(pdata->lib.updaterender){
                    
                    auto context = &pdata->scenecontext;
                    context->prev_frametime = pdata->deltatime;
                    context->keyboardstate = &pdata->keyboardstate;
                    context->mousestate = &pdata->mousestate;
                    context->audiocontext = &audio_data;
                    context->audiocontext_count = &audio_count;
                    
                    auto ptr =
                        (void (*)(SceneContext*))pdata->lib.updaterender;
                    
                    ptr(context);
                }
                
                auto frames = AAudioDeviceWriteAvailable(pdata->audio);
                
                if(frames >= pdata->submit_audiobuffer.size_frames){
                    
                    auto args = TAlloc(MixAudioLayout,1);
                    
                    *args = {&pdata->submit_audiobuffer,audio_data,audio_count};
                    
                    PushThreadWorkQueue(&pdata->threadqueue,
                                        MixAudio,(void*)args,pdata->worker_sem);
                    
                }
                
                ProcessDrawList();
                
                MainThreadDoWorkQueue(&pdata->threadqueue,0);
                
                //FIXME: if there is corruption, it is either because: 1. Flush is too slow,
                //2. MainThreadDoWorkQueue is not synced
                //3. Someone is writing into another person's data
                
                ProcessObjUpdateList();
                
                
#if _enable_gui
                
                GUIEnd();
                
#endif
                //we can thread this
                BuildRenderCommandBuffer(pdata);
                
                PresentBuffer(pdata);
            }
            
            GetTime(&end);
            
            auto diff = GetTimeDifferenceMS(start,end);
            
            
            f32 sleeptime = (_targetframerate - diff);
            
            
            if(sleeptime > 0 ){
                TIMEBLOCKTAGGED("Sleep",DarkGray);
                SleepMS(sleeptime);
            }
            
            GetTime(&end);
            
            pdata->deltatime = GetTimeDifferenceMS(start,end);
            
        }
        
    }
    
    WritePipelineCache();
    
    return 0;
    
}


/*
  TODO: 
  
  Setup a standard build environment for linux:
  https://linuxconfig.org/how-to-debootstrap-on-centos-linux
  
  Compile all assets into an adb file (asset data base). We will build a function a constexpr
  function at compile time which translates filepaths to indices and an adb file w raw data.
  We will keep the adb file open at all times and just read from the offset as required.
  
  Beef up our profiler
  We can profile the cmdbuffers using this. We should inject and record this too
  
  vkCmdWriteTimestamp();
  
  
  memory protect our allocations
  Transition to avx
  Support drawing multiple objects off a single buffer(use vert & index buffer offsets into buffer)
  implement quaternion double cover
  Implement Dual quaternion blending in MDF
  
  multi gpu functions
  
  void vkCmdSetDeviceMask(
VkCommandBuffer commandBuffer, uint32_t deviceMask);

VkResult vkAcquireNextImage2KHR(
VkDevice device, const VkAcquireNextImageInfoKHR* pAcquireInfo, uint32_t* pImageIndex);
typedef struct VkAcquireNextImageInfoKHR {
VkStructureType sType; const void* pNext;
VkSwapchainKHR swapchain;
uint64_t timeout;
VkSemaphore semaphore;
VkFence fence;
uint32_t deviceMask;
} VkAcquireNextImageInfoKHR;


typedef struct VkDeviceGroupPresentInfoKHR {
VkStructureType sType; const void* pNext;
uint32_t swapchainCount;
const uint32_t* pDeviceMasks;
VkDeviceGroupPresentModeFlagBitsKHR mode;
} VkDeviceGroupPresentInfoKHR;

typedef struct VkDeviceGroupCommandBufferBeginInfo {
VkStructureType sType; const void* pNext;
uint32_t deviceMask;
} VkDeviceGroupCommandBufferBeginInfo;

enum VkDeviceGroupPresentModeFlagBitsKHR:
VK_DEVICE_GROUP_PRESENT_MODE_X_BIT_KHR where X is
LOCAL,
REMOTE,
SUM,
LOCAL_MULTI_DEVICE
typedef struct VkDeviceGroupRenderPassBeginInfo {
VkStructureType sType; const void* pNext;
uint32_t deviceMask;
uint32_t deviceRenderAreaCount;
const VkRect2D* pDeviceRenderAreas; P.15
} VkDeviceGroupRenderPassBeginInfo;
typedef struct VkDeviceGroupSubmitInfo {
VkStructureType sType; const void* pNext;
uint32_t waitSemaphoreCount;
const uint32_t* pWaitSemaphoreDeviceIndices;
uint32_t commandBufferCount;
const uint32_t* pCommandBufferDeviceMasks;
uint32_t signalSemaphoreCount;
const uint32_t* pSignalSemaphoreDeviceIndices;
} VkDeviceGroupSubmitInfo;


typedef struct VkMemoryAllocateFlagsInfo {
VkStructureType sType; const void* pNext;
VkMemoryAllocateFlags flags;
uint32_t deviceMask;
} VkMemoryAllocateFlagsInfo;

*/
