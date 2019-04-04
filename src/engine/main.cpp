#include "main.h"

_compile_kill(sizeof(SkelUBO) > _kilobytes(64));
_compile_kill(sizeof(LightUBO) > _kilobytes(64));
_compile_kill(sizeof(PushConst) > 128);
_compile_kill(VK_INDEX_TYPE_UINT16 != 0);
_compile_kill(VK_INDEX_TYPE_UINT32 != 1);


s32 main(s32 argc,s8** argv){
    
    InitAllSystems();
    
#ifdef DEBUG
    
#if 0
    GetExecFileAssetData();
#endif
    
#endif
    
    void(*reload)(GameReloadData*) = 0;
    
    
    while(gdata->running){
        
        {
            
            pdata->objupdate_count = 0;
            TSingleEntryUnlock(&gui_draw_is_locked);
            
            Clear(&pdata->rendercontext);
            ClearLightList();
            
#if _enable_gui
            
            s8 ascii_buffer[256] = {};
            u32 ascii_count = 0;
            
            if(GUIIsAnyElementActive()){
                
                for(u32 i = 0; i < _arraycount(pdata->keyboardstate.curkeystate); i++){
                    
                    if(IsKeyPressed(&pdata->keyboardstate,i)){
                        
                        auto c = WKeyCodeToASCII(i);
                        
                        if(PIsVisibleChar(c)){
                            ascii_buffer[ascii_count] = c;
                            ascii_count++;
                        }
                        
                    }
                    
                }
                
            }
            
            GUIUpdate(&pdata->swapchain,&pdata->keyboardstate,ascii_buffer,ascii_count,&pdata->mousestate,
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
                //FIXME: turning on vsync has frame hitches
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
                    
					//FIXME(WIN32): we are reloading every frame. seems like a problem with FFileChanged
                    
                    auto context = &pdata->scenecontext;
                    
                    GameReloadData reloaddata = {
                        gdata,pdata->vdevice,pdata->renderpass,&pdata->window,context,GetGUIContext(),
                        GetAAllocatorContext(),DEBUGTIMERGETCONTEXT()
                    };
                    
                    vkDeviceWaitIdle(pdata->vdevice.device);
                    
                    reload(&reloaddata);
                    reload = 0;
                    
                    CompileAllPipelines(pdata);
                }
                
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

//add cpp's here to limit scope
#include "aassettools.cpp"
#include "vvulkanx.cpp"
#include "aassetmanager.cpp"

#ifndef CPP_PASS

#include "engine_meta.cpp"

#endif


/*

FIXME: renderdoc does not work with our app anymore. it refuses to create a device

TODO: 

  Make keyboardstate and mousestate a bit array
  
  Compile all assets into an adb file (asset data base). We will build a function a constexpr
  function at compile time which translates filepaths to indices and an adb file w raw data.
  We will keep the adb file open at all times and just read from the offset as required.
  
  Beef up our profiler
  We can profile the cmdbuffers using this. We should inject and record this too
  
  vkCmdWriteTimestamp();
  
  
  memory protect our allocations
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



// libC dependencies

free
pthread_create
__errno_location
stdout
strcpy
cosf
qsort
abs
atof
clock_gettime
write
opendir
strlen
mmap
pthread_setaffinity_np
printf
nanosleep
lseek
__assert_fail
sinf
sem_timedwait
memset
close
read
__libc_start_main
pthread_attr_init
ceilf
acosf
sem_wait
tanf
fprintf
__gmon_start__
memcpy
sqrtf
__xstat
readdir
pthread_attr_setdetachstate
dlopen
malloc
fflush
sem_post
dlclose
pthread_attr_setstacksize
realloc
atan2f
munmap
poll
memmove
pthread_self
sem_init
open
floorf
pthread_attr_destroy
atoi
sprintf
exit
sem_destroy
dlsym
fmodf
dlerror
stderr



*/
