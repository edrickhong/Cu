#include "libload.h"

void* vkenumerateinstanceextensionproperties;
void* vkenumerateinstancelayerproperties;
void* vkcreateinstance;
void* vkgetinstanceprocaddress;
void* vkgetdeviceprocaddress;

void* vkenumeratephysicaldevices;
void* vkgetphysicaldeviceproperties;
void* vkenumeratedevicelayerproperties;
void* vkenumeratedeviceextensionproperties;
void* vkgetphysicaldevicequeuefamilyproperties;
void* vkgetphysicaldevicefeatures;
void* vkcreatedevice;
void* vkgetphysicaldeviceformatproperties;
void* vkgetphysicaldevicememoryproperties;
void* vkcmdpipelinebarrier;
void* vkcreateshadermodule;
void* vkcreatebuffer;
void* vkgetbuffermemoryrequirements;
void* vkmapmemory;
void* vkunmapmemory;
void* vkflushmappedmemoryranges;
void* vkinvalidatemappedmemoryranges;
void* vkbindbuffermemory;
void* vkdestroybuffer;
void* vkallocatememory;
void* vkfreememory;
void* vkcreaterenderpass;
void* vkcmdbeginrenderpass;
void* vkcmdendrenderpass;
void* vkcmdnextsubpass;
void* vkcmdexecutecommands;
void* vkcreateimage;
void* vkgetimagememoryrequirements;
void* vkcreateimageview;
void* vkdestroyimageview;
void* vkbindimagememory;
void* vkgetimagesubresourcelayout;
void* vkcmdcopyimage;
void* vkcmdblitimage;
void* vkdestroyimage;
void* vkcmdclearattachments;
void* vkcmdcopybuffer;
void* vkcmdcopybuffertoimage;
void* vkcreatesampler;
void* vkdestroysampler;
void* vkcreatesemaphore;
void* vkdestroysemaphore;
void* vkcreatefence;
void* vkdestroyfence;
void* vkwaitforfences;
void* vkresetfences;
void* vkcreatecommandpool;
void* vkdestroycommandpool;
void* vkallocatecommandbuffers;
void* vkbegincommandbuffer;
void* vkendcommandbuffer;
void* vkgetdevicequeue;
void* vkqueuesubmit;
void* vkqueuewaitidle;
void* vkdevicewaitidle;
void* vkcreateframebuffer;
void* vkcreatepipelinecache;
void* vkcreatepipelinelayout;
void* vkcreategraphicspipelines;
void* vkcreatecomputepipelines;
void* vkcreatedescriptorpool;
void* vkcreatedescriptorsetlayout;
void* vkallocatedescriptorsets;
void* vkupdatedescriptorsets;
void* vkcmdbinddescriptorsets;
void* vkcmdbindpipeline;
void* vkcmdbindvertexbuffers;
void* vkcmdbindindexbuffer;
void* vkcmdsetviewport;
void* vkcmdsetscissor;
void* vkcmdsetlinewidth;
void* vkcmdsetdepthbias;
void* vkcmdpushconstants;
void* vkcmddrawindexed;
void* vkcmddraw;
void* vkcmddrawindexedindirect;
void* vkcmddrawindirect;
void* vkcmddispatch;
void* vkdestroypipeline;
void* vkdestroypipelinelayout;
void* vkdestroydescriptorsetlayout;
void* vkdestroydevice;
void* vkdestroyinstance;
void* vkdestroydescriptorpool;
void* vkfreecommandbuffers;
void* vkdestroyrenderpass;
void* vkdestroyframebuffer;
void* vkdestroyshadermodule;
void* vkdestroypipelinecache;
void* vkcreatequerypool;
void* vkdestroyquerypool;
void* vkgetquerypoolresults;
void* vkcmdbeginquery;
void* vkcmdendquery;
void* vkcmdresetquerypool;
void* vkcmdcopyquerypoolresults;
void* vkcreate_xlib_wayland_win32surfacekhr;
void* vkdestroysurfacekhr;
void* vkcmdfillbuffer;
void* vkacquirenextimagekhr;
void* vkgetfencestatus;
void* vkcreateswapchainkhr;
void* vkgetswapchainimageskhr;
void* vkqueuepresentkhr;
void* vkgetphysicaldevice_xlib_wayland_win32_presentationsupportkhr;
void* vkgetphysicaldevicesurfacesupportkhr;
void* vkcmdclearcolorimage;
void* vkgetphysicaldeviceimageformatproperties;
void* vkcmdcopyimagetobuffer;
void* vkgetpipelinecachedata;

#ifdef _WIN32

#define _surface_extension VK_KHR_WIN32_SURFACE_EXTENSION_NAME
#define _get_present_support(pdevice,window,queuefam)			\
vkGetPhysicalDeviceWin32PresentationSupportKHR(pdevice,queuefam)


#else

#define _surface_extension VK_KHR_XLIB_SURFACE_EXTENSION_NAME
#define _get_present_support(pdevice,window,queuefam)			\
vkGetPhysicalDeviceXlibPresentationSupportKHR(pdevice,queuefam,window->x11_handle,window->x11_visualid)

#endif



//MARK: every function that uses VCreateImage/Buffer etc does not handle for shared resources
//Look for VkSharingMode


_persist VkAllocationCallbacks* global_allocator = 0;

//MARK: Implementation code

void ErrorString(VkResult errorCode){
    switch (errorCode)
    {
#define STR(r) case VK_ ##r: printf("VK Error:%s",#r); break;
        STR(NOT_READY);
        STR(TIMEOUT);
        STR(EVENT_SET);
        STR(EVENT_RESET);
        STR(INCOMPLETE);
        STR(ERROR_OUT_OF_HOST_MEMORY);
        STR(ERROR_OUT_OF_DEVICE_MEMORY);
        STR(ERROR_INITIALIZATION_FAILED);
        STR(ERROR_DEVICE_LOST);
        STR(ERROR_MEMORY_MAP_FAILED);
        STR(ERROR_LAYER_NOT_PRESENT);
        STR(ERROR_EXTENSION_NOT_PRESENT);
        STR(ERROR_FEATURE_NOT_PRESENT);
        STR(ERROR_INCOMPATIBLE_DRIVER);
        STR(ERROR_TOO_MANY_OBJECTS);
        STR(ERROR_FORMAT_NOT_SUPPORTED);
        STR(ERROR_SURFACE_LOST_KHR);
        STR(ERROR_NATIVE_WINDOW_IN_USE_KHR);
        STR(SUBOPTIMAL_KHR);
        STR(ERROR_OUT_OF_DATE_KHR);
        STR(ERROR_INCOMPATIBLE_DISPLAY_KHR);
        STR(ERROR_VALIDATION_FAILED_EXT);
        STR(ERROR_INVALID_SHADER_NV);
#undef STR
        default:
        printf("VK Error:%s","UNKNOWN_ERROR"); break;
    }
}


#define _vktest(condition) {VkResult result = condition;if((result != VK_SUCCESS)) {ErrorString(condition);*(s32 *)0 = 0;}}

#define _instproc(fptr,inst,entrypoint)				\
{									\
    fptr = (void*)vkGetInstanceProcAddr(inst, ""#entrypoint); \
    if (!fptr)								\
    {									\
        _kill("",1);							\
    }									\
}

#define _instanceproc(fptr,inst,entrypoint)				\
{									\
    fptr = (PFN_vk##entrypoint) vkGetInstanceProcAddr(inst, "vk"#entrypoint); \
    if (!fptr)								\
    {									\
        _kill("",1);							\
    }									\
}

// Macro to get a procedure address based on a vulkan device
#define _deviceproc(fptr,dev,entrypoint)				\
{									\
    fptr = (PFN_vk##entrypoint) vkGetDeviceProcAddr(dev, "vk"#entrypoint); \
    if (!fptr)								\
    {									\
        _kill("",1)							\
    }								\
}



//MARK: Implementation

_persist LibHandle vklib = 0;

void InternalLoadVulkanLib(){
    
    //TODO: expand this list
    const s8* vklib_array[] = {
        
#ifdef _WIN32
        "vulkan-1.dll",
        "vulkan.dll",
#else
        "libvulkan.so.1.1.70", // required for 1.1
        
        "libvulkan.so.1.0.66",
        "libvulkan.so.1.0.65",
        "libvulkan.so.1.0.61",
        "libvulkan.so.1.0.57",
        "libvulkan.so.1.0.54",
        "libvulkan.so.1.0.51",
        "libvulkan.so.1.0.49",
        "libvulkan.so.1.0.46",
        "libvulkan.so.1.0.42",
        "libvulkan.so.1.0.39",
        "libvulkan.so.1.0.37",
        "libvulkan.so.1.0.33",
        "libvulkan.so.1.0.30",
        "libvulkan.so.1.0.26",
        "libvulkan.so.1.0.24",
        "libvulkan.so.1.0.21",
        "libvulkan.so.1.0.17",
        "libvulkan.so.1.0.13",
        "libvulkan.so.1.0.11",
        "libvulkan.so.1.0.08",
        "libvulkan.so.1.0.05",
        "libvulkan.so.1.0.03",
        "libvulkan.so.1",
        "libvulkan.so",
        
        
#endif
        
    };
    
    for(u32 i = 0; i < _arraycount(vklib_array); i++){
        
        vklib = LLoadLibrary(vklib_array[i]);
        
        if(vklib){
            break;
        }
        
    }
    
    _kill("faild to load vulkan lib\n",!vklib);
    
    vkenumerateinstanceextensionproperties =
        LGetLibFunction(vklib,"vkEnumerateInstanceExtensionProperties");
    
    vkenumerateinstancelayerproperties =
        LGetLibFunction(vklib,"vkEnumerateInstanceLayerProperties");
    
    vkcreateinstance = LGetLibFunction(vklib,"vkCreateInstance");
    
    
    vkgetinstanceprocaddress = LGetLibFunction(vklib,"vkGetInstanceProcAddr");
    vkgetdeviceprocaddress = LGetLibFunction(vklib,"vkGetDeviceProcAddr");
    
}

_persist VkInstance global_instance = 0;

#if _debug 
_persist VkDevice global_device = 0;
#endif

void InternalLoadVulkanInstanceLevelFunctions(){
    
    _instproc(vkenumeratephysicaldevices,global_instance,vkEnumeratePhysicalDevices);
    
    _instproc(vkgetphysicaldevicequeuefamilyproperties,global_instance,vkGetPhysicalDeviceQueueFamilyProperties);
    
    _instproc(vkgetphysicaldeviceproperties,global_instance,vkGetPhysicalDeviceProperties);
}

void InternalLoadVulkanFunctions(void* k,void* load_fptr){
    _kill("",!vkenumeratephysicaldevices);
    
    auto load = (void* (*)(void*,const s8*))load_fptr;
    
#define _initfunc(func,var) var = (void*)load(k,""#func); _kill("failed to load function\n",!func)
    
    //TODO: remove instance level functions and run them in instance creation
    _initfunc(vkEnumerateDeviceLayerProperties,vkenumeratedevicelayerproperties);
    _initfunc(vkEnumerateDeviceExtensionProperties,vkenumeratedeviceextensionproperties);
    
    _initfunc(vkGetPhysicalDeviceFeatures,vkgetphysicaldevicefeatures);
    _initfunc(vkCreateDevice,vkcreatedevice);
    _initfunc(vkGetPhysicalDeviceFormatProperties,vkgetphysicaldeviceformatproperties);
    _initfunc(vkGetPhysicalDeviceMemoryProperties,vkgetphysicaldevicememoryproperties);
    _initfunc(vkCmdPipelineBarrier,vkcmdpipelinebarrier);
    _initfunc(vkCreateShaderModule,vkcreateshadermodule);
    _initfunc(vkCreateBuffer,vkcreatebuffer);
    _initfunc(vkGetBufferMemoryRequirements,vkgetbuffermemoryrequirements);
    _initfunc(vkMapMemory,vkmapmemory);
    _initfunc(vkUnmapMemory,vkunmapmemory);
    _initfunc(vkFlushMappedMemoryRanges,vkflushmappedmemoryranges);
    _initfunc(vkInvalidateMappedMemoryRanges,vkinvalidatemappedmemoryranges);
    _initfunc(vkBindBufferMemory,vkbindbuffermemory);
    _initfunc(vkDestroyBuffer,vkdestroybuffer);
    _initfunc(vkAllocateMemory,vkallocatememory);
    _initfunc(vkFreeMemory,vkfreememory);
    _initfunc(vkCreateRenderPass,vkcreaterenderpass);
    _initfunc(vkCmdBeginRenderPass,vkcmdbeginrenderpass);
    _initfunc(vkCmdEndRenderPass,vkcmdendrenderpass);
    _initfunc(vkCmdNextSubpass,vkcmdnextsubpass);
    _initfunc(vkCmdExecuteCommands,vkcmdexecutecommands);
    _initfunc(vkCreateImage,vkcreateimage);
    _initfunc(vkGetImageMemoryRequirements,vkgetimagememoryrequirements);
    _initfunc(vkCreateImageView,vkcreateimageview);
    _initfunc(vkDestroyImageView,vkdestroyimageview);
    _initfunc(vkBindImageMemory,vkbindimagememory);
    _initfunc(vkGetImageSubresourceLayout,vkgetimagesubresourcelayout);
    _initfunc(vkCmdCopyImage,vkcmdcopyimage);
    _initfunc(vkCmdBlitImage,vkcmdblitimage);
    _initfunc(vkDestroyImage,vkdestroyimage);
    _initfunc(vkCmdClearAttachments,vkcmdclearattachments);
    _initfunc(vkCmdCopyBuffer,vkcmdcopybuffer);
    _initfunc(vkCmdCopyBufferToImage,vkcmdcopybuffertoimage);
    _initfunc(vkCreateSampler,vkcreatesampler);
    _initfunc(vkDestroySampler,vkdestroysampler);
    _initfunc(vkCreateSemaphore,vkcreatesemaphore);
    _initfunc(vkDestroySemaphore,vkdestroysemaphore);
    _initfunc(vkCreateFence,vkcreatefence);
    _initfunc(vkDestroyFence,vkdestroyfence);
    _initfunc(vkWaitForFences,vkwaitforfences);
    _initfunc(vkResetFences,vkresetfences);
    _initfunc(vkCreateCommandPool,vkcreatecommandpool);
    _initfunc(vkDestroyCommandPool,vkdestroycommandpool);
    _initfunc(vkAllocateCommandBuffers,vkallocatecommandbuffers);
    _initfunc(vkBeginCommandBuffer,vkbegincommandbuffer);
    _initfunc(vkEndCommandBuffer,vkendcommandbuffer);
    _initfunc(vkGetDeviceQueue,vkgetdevicequeue);
    _initfunc(vkQueueSubmit,vkqueuesubmit);
    _initfunc(vkQueueWaitIdle,vkqueuewaitidle);
    _initfunc(vkDeviceWaitIdle,vkdevicewaitidle);
    _initfunc(vkCreateFramebuffer,vkcreateframebuffer);
    _initfunc(vkCreatePipelineCache,vkcreatepipelinecache);
    _initfunc(vkCreatePipelineLayout,vkcreatepipelinelayout);
    _initfunc(vkCreateGraphicsPipelines,vkcreategraphicspipelines);
    _initfunc(vkCreateComputePipelines,vkcreatecomputepipelines);
    _initfunc(vkCreateDescriptorPool,vkcreatedescriptorpool);
    _initfunc(vkCreateDescriptorSetLayout,vkcreatedescriptorsetlayout);
    _initfunc(vkAllocateDescriptorSets,vkallocatedescriptorsets);
    _initfunc(vkUpdateDescriptorSets,vkupdatedescriptorsets);
    _initfunc(vkCmdBindDescriptorSets,vkcmdbinddescriptorsets);
    _initfunc(vkCmdBindPipeline,vkcmdbindpipeline);
    _initfunc(vkCmdBindVertexBuffers,vkcmdbindvertexbuffers);
    _initfunc(vkCmdBindIndexBuffer,vkcmdbindindexbuffer);
    _initfunc(vkCmdSetViewport,vkcmdsetviewport);
    _initfunc(vkCmdSetScissor,vkcmdsetscissor);
    _initfunc(vkCmdSetLineWidth,vkcmdsetlinewidth);
    _initfunc(vkCmdSetDepthBias,vkcmdsetdepthbias);
    _initfunc(vkCmdPushConstants,vkcmdpushconstants);
    _initfunc(vkCmdDrawIndexed,vkcmddrawindexed);
    _initfunc(vkCmdDraw,vkcmddraw);
    _initfunc(vkCmdDrawIndexedIndirect,vkcmddrawindexedindirect);
    _initfunc(vkCmdDrawIndirect,vkcmddrawindirect);
    _initfunc(vkCmdDispatch,vkcmddispatch);
    _initfunc(vkDestroyPipeline,vkdestroypipeline);
    _initfunc(vkDestroyPipelineLayout,vkdestroypipelinelayout);
    _initfunc(vkDestroyDescriptorSetLayout,vkdestroydescriptorsetlayout);
    _initfunc(vkDestroyDevice,vkdestroydevice);
    _initfunc(vkDestroyInstance,vkdestroyinstance);
    _initfunc(vkDestroyDescriptorPool,vkdestroydescriptorpool);
    _initfunc(vkFreeCommandBuffers,vkfreecommandbuffers);
    _initfunc(vkDestroyRenderPass,vkdestroyrenderpass);
    _initfunc(vkDestroyFramebuffer,vkdestroyframebuffer);
    _initfunc(vkDestroyShaderModule,vkdestroyshadermodule);
    _initfunc(vkDestroyPipelineCache,vkdestroypipelinecache);
    _initfunc(vkCreateQueryPool,vkcreatequerypool);
    _initfunc(vkDestroyQueryPool,vkdestroyquerypool);
    _initfunc(vkGetQueryPoolResults,vkgetquerypoolresults);
    _initfunc(vkCmdBeginQuery,vkcmdbeginquery);
    _initfunc(vkCmdEndQuery,vkcmdendquery);
    _initfunc(vkCmdResetQueryPool,vkcmdresetquerypool);
    _initfunc(vkCmdCopyQueryPoolResults,vkcmdcopyquerypoolresults);
    
    _initfunc(vkDestroySurfaceKHR,vkdestroysurfacekhr);
    _initfunc(vkCmdFillBuffer,vkcmdfillbuffer);
    _initfunc(vkAcquireNextImageKHR,vkacquirenextimagekhr);
    _initfunc(vkGetFenceStatus,vkgetfencestatus);
    _initfunc(vkCreateSwapchainKHR,vkcreateswapchainkhr);
    _initfunc(vkGetSwapchainImagesKHR,vkgetswapchainimageskhr);
    _initfunc(vkQueuePresentKHR,vkqueuepresentkhr);
    
    
    _initfunc(vkGetPhysicalDeviceSurfaceSupportKHR,vkgetphysicaldevicesurfacesupportkhr);
    
    _initfunc(vkCmdClearColorImage,vkcmdclearcolorimage);
    _initfunc(vkGetPhysicalDeviceImageFormatProperties,vkgetphysicaldeviceimageformatproperties);
    
    _initfunc(vkCmdCopyImageToBuffer,vkcmdcopyimagetobuffer);
    
    _initfunc(vkGetPipelineCacheData,vkgetpipelinecachedata);
    
#undef _initfunc
    
}


VkBool32 VkDebugMessageCallback(VkDebugReportFlagsEXT flags,
                                VkDebugReportObjectTypeEXT objType,
                                uint64_t srcObject,u32 location,u32 msgCode,
                                const s8* pLayerPrefix,
                                const s8* pMsg,
                                void* pUserData){
    
#ifdef _WIN32 //we leave some items in our desc set empty
    
    auto tokill = msgCode != 59 && msgCode != 61;
    
#else
    
    auto tokill = msgCode != 61;
    
#endif
    
    
    
    if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT && tokill){
        
        printf("ERROR: %s Code: %d:%s\n\n",pLayerPrefix,msgCode,pMsg);
        
        _kill("",tokill);
    }
    else if (flags & VK_DEBUG_REPORT_WARNING_BIT_EXT){
        
        printf("WARNING: %s Code: %d:%s\n\n",pLayerPrefix,msgCode,pMsg);
        // _kill("",tokill);
    }
    
    else
    {
        return false;
    }
    
    fflush(stdout);
    
    return false;
}


PFN_vkDestroyDebugReportCallbackEXT CreateVkDebug(VkInstance instance){
    
    PFN_vkCreateDebugReportCallbackEXT CreateDebugReportCallback;
    PFN_vkDestroyDebugReportCallbackEXT DestroyDebugReportCallback;
    VkDebugReportCallbackEXT DebugReportCallback;
    
    _instanceproc(CreateDebugReportCallback,instance,
                  CreateDebugReportCallbackEXT);
    
    _instanceproc(DestroyDebugReportCallback,instance,
                  DestroyDebugReportCallbackEXT);
    
    VkDebugReportCallbackCreateInfoEXT debug_info = {};
    debug_info.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT;
    debug_info.pfnCallback = 
        (PFN_vkDebugReportCallbackEXT)VkDebugMessageCallback;
    debug_info.flags =
        VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
    
    _vktest(CreateDebugReportCallback(instance,&debug_info,0,
                                      &DebugReportCallback));
    
    return DestroyDebugReportCallback;
    
}

//Unmanaged
VkDeviceMemory  
VRawDeviceAlloc(VkDevice device,VkDeviceSize alloc_size,u32 memorytype_index){
    
#if _debug && 0
    PRINTTIMEBLOCK();
    printf("DEVICEALLOCED: TYPE %d SIZE %d\n",memorytype_index,(u32)alloc_size);
#endif
    
    VkDeviceMemory memory;
    
    VkMemoryAllocateInfo info = {
        VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        0,
        alloc_size,
        memorytype_index
    };
    
    _vktest(vkAllocateMemory(device,&info,global_allocator,&memory));
    
    return memory;
}

_persist VkDeviceMemory (*deviceallocator)(VkDevice,VkDeviceSize,u32) = VRawDeviceAlloc;

VkSampler CreateSampler(VkDevice device,VkSamplerCreateFlags flags,
                        VkFilter mag_filter,VkFilter min_filter,
                        VkSamplerMipmapMode mipmapmode,
                        VkSamplerAddressMode u_addressmode,
                        VkSamplerAddressMode v_addressmode,
                        VkSamplerAddressMode w_addressmode,f32 miplod_bias,
                        VkBool32 anisotropy_enable,f32 max_anisotropy,
                        VkBool32 compare_enable,VkCompareOp compareop,
                        f32 min_lod,f32 max_lod,VkBorderColor bordercolor,
                        VkBool32 unnormalizedcoordinates_enable){
    
    VkSampler sampler;
    
    VkSamplerCreateInfo info = {
        VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
        0,
        flags,
        mag_filter,
        min_filter,
        mipmapmode,
        u_addressmode,
        v_addressmode,
        w_addressmode,
        miplod_bias,
        anisotropy_enable,
        max_anisotropy,
        compare_enable,
        compareop,
        min_lod,
        max_lod,
        bordercolor,
        unnormalizedcoordinates_enable,
    };
    
    _vktest(vkCreateSampler(device,&info,global_allocator,&sampler));
    
    return sampler;
}


//we can optimize this
u32 VGetMemoryTypeIndex(VkPhysicalDeviceMemoryProperties properties,
                        u32 typebits,u32 flags){
    
    //typebits is a bitfield that lists all usable memory types
    //we iterate over each bit until we find one that is supported and fits
    //the specifications we set in flags
    
    for (u32 i = 0; i < properties.memoryTypeCount; i++){
        if (typebits & (1 << i)){
            if ((properties.memoryTypes[i].propertyFlags & flags) == 
                flags){
                return i;
            }
        }
    }
    
    return -1;
}

VkShaderModule VCreateShaderModule(VkDevice device,void* data,
                                   ptrsize size,
                                   VkShaderModuleCreateFlags flags){
    
    VkShaderModule shader;
    
    VkShaderModuleCreateInfo info = {};
    
    info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    info.flags = flags;
    info.codeSize = size;
    info.pCode = (u32*)data;
    
    _vktest(vkCreateShaderModule(device,&info,global_allocator,&shader));
    
    return shader;
}


VkBuffer CreateBuffer(VkDevice device,VkBufferCreateFlags flags,
                      VkDeviceSize size,VkBufferUsageFlags usage,
                      VkSharingMode sharingmode,
                      u32* queuefamilyindex_array ,
                      ptrsize queuefamilyindex_count){
    
    VkBuffer buffer;
    
    VkBufferCreateInfo info = {};
    
    info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    info.flags = flags;
    info.size = size;
    info.usage = usage;
    info.sharingMode = sharingmode;
    info.queueFamilyIndexCount = queuefamilyindex_count;
    info.pQueueFamilyIndices = queuefamilyindex_array;
    
    _vktest(vkCreateBuffer(device,&info,global_allocator,&buffer));
    
    return buffer;
}

VkBuffer VRawCreateBuffer(const  VDeviceContext* _restrict vdevice,
                          VkBufferCreateFlags flags,
                          VkDeviceSize size,VkBufferUsageFlags usage,
                          VkSharingMode sharingmode,
                          u32* queuefamilyindex_array ,
                          ptrsize queuefamilyindex_count){
    
    return CreateBuffer(vdevice->device,flags,size,usage,sharingmode,
                        queuefamilyindex_array ,queuefamilyindex_count);
}

VkBuffer VRawCreateBuffer(VkDevice device,
                          VkBufferCreateFlags flags,
                          VkDeviceSize size,VkBufferUsageFlags usage,
                          VkSharingMode sharingmode,
                          u32* queuefamilyindex_array ,
                          ptrsize queuefamilyindex_count){
    return CreateBuffer(device,flags,size,usage,sharingmode,
                        queuefamilyindex_array ,queuefamilyindex_count);
}



VBufferContext InternalCreateStaticBufferContext(
const  VDeviceContext* _restrict vdevice,
VkCommandBuffer commandbuffer,
VBufferContext src,VkDeviceSize src_offset,
VkDeviceMemory memory,
VkDeviceSize offset,
void* data,ptrsize data_size,
VkBufferUsageFlags usage){
    
    auto device = vdevice->device;
    
    VBufferContext context;
    
    VkMemoryRequirements memoryreq;
    
    void* mappedmemory_ptr;
    
    context.buffer =
        CreateBuffer(device,0,
                     data_size,usage | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                     VK_SHARING_MODE_EXCLUSIVE,0,0);
    
    vkGetBufferMemoryRequirements(device,context.buffer,&memoryreq);
    
    context.size = memoryreq.size;
    
    context.memory = memory;
    
    vkBindBufferMemory(device,context.buffer,context.memory,offset);
    
    vkMapMemory(device,src.memory,src_offset,context.size,0,&mappedmemory_ptr);
    
    memcpy(mappedmemory_ptr,data,data_size);
    
    vkUnmapMemory(device,src.memory);
    
    
    VkBufferCopy copyregion;
    
    copyregion.srcOffset = src_offset;
    copyregion.dstOffset = 0;
    copyregion.size = context.size;
    
    vkCmdCopyBuffer(commandbuffer,src.buffer,context.buffer,1,&copyregion);
    
    return context;
}

VBufferContext InternalCreateStaticBufferContext(
const  VDeviceContext* _restrict vdevice,
ptrsize data_size,VkBufferUsageFlags usage,
u32 memtype = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT){
    
    VBufferContext context;
    
    auto device = vdevice->device;
    
    VkPhysicalDeviceMemoryProperties 
        memoryproperties = *(vdevice->memoryproperties);
    
    context.buffer = 
        CreateBuffer(device,0,data_size,usage,VK_SHARING_MODE_EXCLUSIVE,0,0);
    
    VkMemoryRequirements memoryreq;
    
    vkGetBufferMemoryRequirements(device,context.buffer,
                                  &memoryreq);
    
    context.size = memoryreq.size;
    
    auto typeindex = VGetMemoryTypeIndex(memoryproperties,
                                         memoryreq.memoryTypeBits,memtype);
    
    _kill("invalid memory type\n",typeindex == (u32)-1);
    
    context.memory = deviceallocator(device,memoryreq.size,typeindex);
    
    vkBindBufferMemory(device,context.buffer,context.memory,0);
    
    return context;
}




VBufferContext VCreateStaticVertexBuffer(const  VDeviceContext* _restrict vdevice,
                                         VkCommandBuffer commandbuffer,
                                         VkDeviceMemory memory,
                                         VkDeviceSize offset,
                                         VBufferContext src,VkDeviceSize src_offset,void* data,
                                         ptrsize data_size,u32 bindingno){
    
    auto context =
        InternalCreateStaticBufferContext(
        vdevice,
        commandbuffer,
        src,src_offset,
        memory,
        offset,
        data,data_size,
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
        VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    
    context.attrib = bindingno;
    
    return context;
}

VBufferContext VCreateStaticIndexBuffer(const  VDeviceContext* _restrict vdevice,
                                        VkCommandBuffer commandbuffer,
                                        VkDeviceMemory memory,
                                        VkDeviceSize offset,
                                        VBufferContext src,VkDeviceSize src_offset,void* data,
                                        ptrsize data_size){
    
    auto context =
        InternalCreateStaticBufferContext(vdevice,
                                          commandbuffer,
                                          src,src_offset,
                                          memory,
                                          offset,
                                          data,data_size,
                                          VK_BUFFER_USAGE_INDEX_BUFFER_BIT |
                                          VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    
    context.attrib = _countsize(data_size,u32);
    
    return context;
    
}


VBufferContext VCreateStaticVertexBuffer(const  VDeviceContext* _restrict vdevice,
                                         ptrsize data_size,u32 bindingno,logic isdevice_local){
    
    u32 memtype = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    
    if(!isdevice_local){
        memtype = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
            VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    }
    
    
    
    auto context = InternalCreateStaticBufferContext(vdevice,data_size,
                                                     VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
                                                     VK_BUFFER_USAGE_TRANSFER_DST_BIT,memtype);
    
    context.attrib = bindingno;
    
    return context;
}

VBufferContext VCreateStaticIndexBuffer(const  VDeviceContext* _restrict vdevice,
                                        ptrsize size,logic isdevice_local){
    
    u32 memtype = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    
    if(!isdevice_local){
        memtype = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
            VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    }
    
    auto context = InternalCreateStaticBufferContext(vdevice,size,
                                                     VK_BUFFER_USAGE_INDEX_BUFFER_BIT |
                                                     VK_BUFFER_USAGE_TRANSFER_DST_BIT,memtype);
    
    context.attrib = _countsize(size,u32);
    
    return context;
    
}

VBufferContext VCreateTransferBuffer(const  VDeviceContext* _restrict vdevice,
                                     ptrsize size,u32 add_flags){
    
    auto flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | add_flags;
    
    VBufferContext context = {};
    
    context.buffer = 
        CreateBuffer(vdevice->device,0,size,VK_BUFFER_USAGE_TRANSFER_SRC_BIT | 
                     VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                     VK_SHARING_MODE_EXCLUSIVE,0,0);
    
    VkMemoryRequirements memoryreq;
    
    vkGetBufferMemoryRequirements(vdevice->device,context.buffer,
                                  &memoryreq);
    
    context.size = memoryreq.size;
    
    auto typeindex = VGetMemoryTypeIndex(*vdevice->memoryproperties,
                                         memoryreq.memoryTypeBits,flags);
    
    if(typeindex == (u32)-1 && (add_flags == V_AMD_DEVICE_HOST_VISIBLE)){
        typeindex = VGetMemoryTypeIndex(*vdevice->memoryproperties,
                                        memoryreq.memoryTypeBits,flags);
    }
    
    _kill("invalid memory type\n",typeindex == (u32)-1);
    
    context.memory = deviceallocator(vdevice->device,memoryreq.size,typeindex);
    vkBindBufferMemory(vdevice->device,context.buffer,context.memory,0);
    
    return context;
}


VkInstance CreateInstance(const s8* _restrict name,
                          u32 firstno_apiver,u32 secondno_apiver,u32 thirdno_apiver,
                          const s8** layer_array,ptrsize layer_count,
                          const s8** extension_array,ptrsize extension_count){
    
    VkApplicationInfo app_info = {};
    
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pApplicationName = name;
    app_info.pEngineName = name;
    app_info.apiVersion = VK_MAKE_VERSION(firstno_apiver,secondno_apiver,thirdno_apiver);
    
    VkInstanceCreateInfo instance_info = {};
    
    instance_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instance_info.pNext = 0;
    instance_info.flags = 0;
    instance_info.pApplicationInfo = &app_info;
    
    instance_info.ppEnabledLayerNames = layer_array;
    instance_info.enabledLayerCount = layer_count;
    
    
    instance_info.ppEnabledExtensionNames = extension_array;
    instance_info.enabledExtensionCount = extension_count;
    
    VkInstance instance = 0;
    
    _vktest(vkCreateInstance(&instance_info, global_allocator,&instance));
    
    return instance;
}


VkDevice CreateDevice(VkPhysicalDevice physicaldevice,
                      VkDeviceQueueCreateInfo* queueinfo_array,
                      ptrsize queueinfo_count,
                      const s8** layer_array,ptrsize layer_count,
                      const s8** extension_array,ptrsize extension_count,
                      VkPhysicalDeviceFeatures* _restrict devicefeatures){
    
    VkDevice device;
    
    VkDeviceCreateInfo device_info = {};
    device_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    device_info.queueCreateInfoCount = queueinfo_count;
    device_info.pQueueCreateInfos = queueinfo_array;
    
    
    device_info.enabledExtensionCount = extension_count;
    device_info.ppEnabledExtensionNames = extension_array;
    
    device_info.ppEnabledLayerNames = layer_array;
    device_info.enabledLayerCount = layer_count;
    
    device_info.pEnabledFeatures = devicefeatures;
    
    
    _vktest(vkCreateDevice(physicaldevice,&device_info,global_allocator,&device));
    
    return device;
}

#ifdef _WIN32

VkSurfaceKHR CreateSurface(VkInstance instance,HINSTANCE connection,HWND window){
    
    VkWin32SurfaceCreateInfoKHR surface_info = {};
    
    VkSurfaceKHR surface;
    
    surface_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    surface_info.pNext = 0;
    surface_info.hinstance = connection;
    surface_info.hwnd = window;
    
    _vktest(vkCreateWin32SurfaceKHR(instance,&surface_info,global_allocator,&surface));
    
    return surface;
}


#else

VkSurfaceKHR CreateSurface(VkInstance instance,Display* display,Window window){
    
    VkXlibSurfaceCreateInfoKHR surface_info = {};
    
    VkSurfaceKHR surface;
    
    surface_info.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
    surface_info.pNext = 0;
    surface_info.dpy = display;
    surface_info.window = window;
    
    _vktest(vkCreateXlibSurfaceKHR(instance, &surface_info,global_allocator,&surface));
    
    return surface;
}

#endif

VkImageView CreateImageView(VkDevice device,
                            VkImageViewCreateFlags flags,
                            VkImage image,
                            VkImageViewType viewtype,
                            VkFormat format,
                            VkComponentMapping components,
                            VkImageSubresourceRange subresourcerange){
    
    VkImageView view;
    
    VkImageViewCreateInfo view_createinfo = {};
    
    view_createinfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    view_createinfo.flags = flags;
    view_createinfo.image = image;
    view_createinfo.viewType = viewtype;
    view_createinfo.format = format;
    view_createinfo.components = components;
    view_createinfo.subresourceRange = subresourcerange;
    
    _vktest(vkCreateImageView(device,&view_createinfo,global_allocator,&view));
    
    return view;
}

VkImage CreateImage(VkDevice device,VkImageCreateFlags flags,
                    VkImageType imagetype, VkFormat format,VkExtent3D extent,
                    u32 miplevels,u32 arraylayers,
                    VkSampleCountFlagBits samples,VkImageTiling tiling,
                    VkImageUsageFlags usage,VkSharingMode sharingmode,
                    u32* queuefamilyindex_array,ptrsize queuefamilyindex_count,
                    VkImageLayout initial_layout,VkPhysicalDevice phys_device){
    
    VkImage image;
    
    VkImageCreateInfo info = {};
    
    info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    info.flags = flags;
    info.imageType = imagetype;
    info.format = format;
    info.extent = extent;
    info.mipLevels = miplevels;
    info.arrayLayers = arraylayers;
    info.samples = samples;
    info.tiling = tiling;
    info.usage = usage;
    info.sharingMode = sharingmode;
    info.queueFamilyIndexCount = queuefamilyindex_count;
    info.pQueueFamilyIndices = queuefamilyindex_array;
    info.initialLayout = initial_layout;
    
#if _debug
    {
        
        VkImageFormatProperties props = {};
        
        vkGetPhysicalDeviceImageFormatProperties(
            phys_device,
            format,
            imagetype,
            tiling,
            usage,
            flags,&props);
        
        _kill("cannot create this image\n",extent.width > props.maxExtent.width ||
              extent.height > props.maxExtent.height || extent.depth > props.maxExtent.depth);
    }
#endif
    
    _vktest(vkCreateImage(device,&info,global_allocator,&image));
    
    return image;
}


VSwapchainContext CreateSwapchain(VkInstance instance,VkPhysicalDevice physicaldevice,
                                  VkDevice device,
                                  VkPhysicalDeviceMemoryProperties memoryproperties,
                                  VkSurfaceKHR surface,u32 width,u32 height,u32 swapcount,
                                  VSwapchainContext* oldswapchain,VPresentSyncType sync_type){
    
    //requesting the array size is unnecessary
    
    PFN_vkGetPhysicalDeviceSurfaceFormatsKHR GetSurfaceFormats;
    PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR GetSurfaceCapabilities;
    PFN_vkGetPhysicalDeviceSurfacePresentModesKHR GetSurfacePresentModes;  
    
    _instanceproc(GetSurfaceCapabilities,instance,
                  GetPhysicalDeviceSurfaceCapabilitiesKHR);
    
    _instanceproc(GetSurfaceFormats,instance,
                  GetPhysicalDeviceSurfaceFormatsKHR);
    
    _instanceproc(GetSurfacePresentModes,instance,
                  GetPhysicalDeviceSurfacePresentModesKHR);
    
    VkSurfaceCapabilitiesKHR surfacecapabilities;
    
    _vktest(GetSurfaceCapabilities(physicaldevice,surface,&surfacecapabilities));
    
    VkSurfaceFormatKHR surfaceformat_array[20] = {};
    ptrsize surfaceformat_count = 0;
    
    GetSurfaceFormats(physicaldevice,surface,(u32*)&surfaceformat_count,0);
    
    _kill("", surfaceformat_count > _arraycount(surfaceformat_array));
    
    GetSurfaceFormats(physicaldevice,surface,(u32*)&surfaceformat_count,surfaceformat_array);
    
    u32 image_count = swapcount;
    
    
    if(image_count > surfacecapabilities.maxImageCount){
        
        image_count = surfacecapabilities.maxImageCount;
    }
    
    if(image_count < surfacecapabilities.minImageCount){
        image_count = surfacecapabilities.minImageCount;
    }
    
    VkSurfaceFormatKHR surfaceformat = {};
    
    //Any format supported
    if(surfaceformat_count == 1 && surfaceformat_array[0].format == 
       VK_FORMAT_UNDEFINED){
        
        surfaceformat.format = VK_FORMAT_B8G8R8A8_UNORM;
        surfaceformat.colorSpace =  VK_COLORSPACE_SRGB_NONLINEAR_KHR;
    }
    
    else{
        
        logic found = false;
        
        for(ptrsize i = 0; i < surfaceformat_count; i++){
            
            if(surfaceformat_array[i].format == VK_FORMAT_B8G8R8A8_UNORM){
                
                surfaceformat = surfaceformat_array[i];
                found = true;
                break;
            }
        }
        
        if(!found)
            surfaceformat = surfaceformat_array[0];
        
    }
    
    VkExtent2D extent = surfacecapabilities.currentExtent;
    
    //we can do a simd compare here
    if(surfacecapabilities.currentExtent.width == 0xFFFFFFFF){
        
        extent = {width,height};
        
        if(extent.width < surfacecapabilities.minImageExtent.width)
            extent.width = surfacecapabilities.minImageExtent.width;
        
        if(extent.height < surfacecapabilities.minImageExtent.height)
            extent.height = surfacecapabilities.minImageExtent.height;
        
        if(extent.width > surfacecapabilities.maxImageExtent.width)
            extent.width = surfacecapabilities.maxImageExtent.width;
        
        if(extent.height > surfacecapabilities.maxImageExtent.height)
            extent.height = surfacecapabilities.maxImageExtent.height;
        
    }
    
    
    //will be used as our color attachment
    VkImageUsageFlags usageflags =
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    
    if(surfacecapabilities.supportedUsageFlags & 
       VK_IMAGE_USAGE_TRANSFER_DST_BIT){
        
        //our transfer commands are copied to it
        usageflags = usageflags | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        
    }
    
    VkSurfaceTransformFlagBitsKHR pretransform = 
        surfacecapabilities.currentTransform;
    
    if(surfacecapabilities.supportedTransforms & 
       VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR){
        
        pretransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
        
    }
    
    
    VkPresentModeKHR presentmode = (VkPresentModeKHR)sync_type;
    
#if _debug
    
    VkPresentModeKHR presentmode_array[10] = {};
    u32 presentmode_count = 0;
    
    GetSurfacePresentModes(physicaldevice,surface,(u32*)&presentmode_count,0);
    
    _kill("", presentmode_count > 10);
    
    GetSurfacePresentModes(physicaldevice,surface,(u32*)&presentmode_count,
                           presentmode_array);
    
    u32 not_available = true;
    
    for(u32 i = 0; i < presentmode_count; i++){
        
        if(presentmode_array[i] == presentmode){
            
            not_available = false;
            break;
        }
        
    }
    
    _kill("Passed invalid present mode\n",not_available);
    
#endif
    
    VSwapchainContext swapchain;
    
    if(oldswapchain){
        swapchain = *oldswapchain;
    }
    
    else{
        swapchain = {};
        swapchain.internal =
            (VSwapchainContext::InternalData*)alloc(sizeof(VSwapchainContext::InternalData));
    }
    
    VkSwapchainCreateInfoKHR swapchain_info = {};
    
    swapchain_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchain_info.pNext = 0;
    swapchain_info.flags = 0;
    swapchain_info.surface = surface;
    swapchain_info.minImageCount = image_count;
    swapchain_info.imageFormat = surfaceformat.format;
    swapchain_info.imageColorSpace = surfaceformat.colorSpace;
    swapchain_info.imageExtent = extent;
    
    
    /*
      MARK: might want to set this according to maxImageArrayLayers from surfacecaps
      Validation layer is crazy. min 1 max 0??
    */
    swapchain_info.imageArrayLayers = 1;
    
    swapchain_info.imageUsage = usageflags;
    
    //Are we gonna share this between different queue families
    swapchain_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapchain_info.queueFamilyIndexCount = 0;
    swapchain_info.pQueueFamilyIndices = 0;
    swapchain_info.preTransform = pretransform;
    swapchain_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchain_info.presentMode = presentmode;
    swapchain_info.clipped = VK_TRUE; //discards pixels that are obscured
    
    //NOTE: While it is not obvious now, we might want to support this
    swapchain_info.oldSwapchain = swapchain.swap;
    
    _vktest(vkCreateSwapchainKHR(device, &swapchain_info, global_allocator, &swapchain.swap));
    
    
    //create depthstencil buffer
    {
        //get depthstencil format
        
        VkFormat depthformat_array[] = {
            VK_FORMAT_D24_UNORM_S8_UINT, 
            VK_FORMAT_D32_SFLOAT_S8_UINT, 
            VK_FORMAT_D32_SFLOAT,
            VK_FORMAT_D16_UNORM_S8_UINT, 
            VK_FORMAT_D16_UNORM 
        };
        
        for(u32 i = 0; i < 5; i++){
            
            VkFormatProperties prop;
            
            vkGetPhysicalDeviceFormatProperties(physicaldevice,depthformat_array[i],
                                                &prop);
            if (prop.optimalTilingFeatures &
                VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT){
                swapchain.internal->depthstencil.format = depthformat_array[i];
                break;
            }
            
        }
        
        swapchain.internal->depthstencil.image = 
            CreateImage(device,0,VK_IMAGE_TYPE_2D,swapchain.internal->depthstencil.format,
                        {extent.width,extent.height,1},1,1,VK_SAMPLE_COUNT_1_BIT,
                        VK_IMAGE_TILING_OPTIMAL,
                        VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | 
                        VK_IMAGE_USAGE_TRANSFER_SRC_BIT,VK_SHARING_MODE_EXCLUSIVE,
                        0,0,VK_IMAGE_LAYOUT_UNDEFINED,physicaldevice);
        
        
        
        VkMemoryRequirements memoryreq = {};
        
        vkGetImageMemoryRequirements(device,swapchain.internal->depthstencil.image,
                                     &memoryreq);
        
        auto typeindex =
            VGetMemoryTypeIndex(memoryproperties,memoryreq.memoryTypeBits,
                                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        
        _kill("invalid memory type\n",typeindex == (u32)-1);
        
        swapchain.internal->depthstencil.memory = 
            deviceallocator(device,memoryreq.size,typeindex);
        
        vkBindImageMemory(device,swapchain.internal->depthstencil.image,
                          swapchain.internal->depthstencil.memory,0);
        
        swapchain.internal->depthstencil.view = 
            CreateImageView(device,0,swapchain.internal->depthstencil.image,
                            VK_IMAGE_VIEW_TYPE_2D,
                            swapchain.internal->depthstencil.format,
                            {VK_COMPONENT_SWIZZLE_IDENTITY,
                            VK_COMPONENT_SWIZZLE_IDENTITY,
                            VK_COMPONENT_SWIZZLE_IDENTITY,
                            VK_COMPONENT_SWIZZLE_IDENTITY},
                            {VK_IMAGE_ASPECT_DEPTH_BIT | 
                            VK_IMAGE_ASPECT_STENCIL_BIT,
                            0,1,0,1});
    }
    
    //get a context to all the swapchain images
    
    vkGetSwapchainImagesKHR(device, swapchain.swap, &image_count, 0);
    
    swapchain.image_count = image_count;
    
    //alloced here
    swapchain.presentresource_array =
        _alloc(VSwapchainContext::PresentImageResource,image_count);
    
    swapchain.width = extent.width;
    swapchain.height = extent.height;
    
    VkImage image_array[10];
    vkGetSwapchainImagesKHR(device, swapchain.swap, &image_count,image_array);
    
    swapchain.internal->format = surfaceformat.format;
    
    
    for(u32 i = 0; i < image_count; i++){
        
        swapchain.presentresource_array[i].image = image_array[i];
        
        swapchain.presentresource_array[i].view = 
            CreateImageView(device,
                            0,image_array[i],VK_IMAGE_VIEW_TYPE_2D,
                            swapchain.internal->format,{VK_COMPONENT_SWIZZLE_IDENTITY,
                            VK_COMPONENT_SWIZZLE_IDENTITY,
                            VK_COMPONENT_SWIZZLE_IDENTITY,
                            VK_COMPONENT_SWIZZLE_IDENTITY},
                            {VK_IMAGE_ASPECT_COLOR_BIT,0,1,0,1});
    }
    
    return swapchain;
}



//MARK:User safe functions

struct VCreatedQueueFamilyInfo{
    u32 familyindex;
    VkQueueFlags capabilities;
    u32 createdqueue_count;
};

_persist VCreatedQueueFamilyInfo global_queuefamilyinfo_array[5] = {};
_persist u32 global_queuefamilyinfo_count = 0;



_persist logic global_validation_enable = false;


void VEnumerateCreatedFamilyQueues(VCreatedQueueFamilyInfo* info_array,u32* count){
    
    if(!info_array){
        *count = global_queuefamilyinfo_count;
    }
    
    else{
        
        u32 loopcount = *count;
        
        if(loopcount > global_queuefamilyinfo_count){
            loopcount = global_queuefamilyinfo_count;
        }
        
        for(u32 i = 0; i < loopcount; i++){
            info_array[i] = global_queuefamilyinfo_array[i];
        }
        
    }
    
}

void VSetDriverAllocator(VkAllocationCallbacks allocator){
    
    if(!global_allocator){
        global_allocator = (VkAllocationCallbacks*)alloc(sizeof(VkAllocationCallbacks));
    }
    
    *global_allocator = allocator;
}

void VSetDeviceAllocator(VkDeviceMemory (*allocator)(VkDevice,VkDeviceSize,u32)){
    deviceallocator = allocator;
}

void VCreateInstance(const s8* applicationname_string,logic validation_enable,
                     u32 firstno_apiver,u32 secondno_apiver,u32 thirdno_apiver,u32 lflags){
    
    _kill("instance already active\n",global_instance);
    
#if  1
    
    //use this for runtime resource validation
    const s8* layer_array[] = {
        "VK_LAYER_LUNARG_standard_validation",
#if 0
        "VK_LAYER_LUNARG_device_simulation"
#endif
    };
    
#else
    
    //This is useful for optimized builds
    const s8* layer_array[] = {"VK_LAYER_LUNARG_api_dump"};
    
#endif
    
    u32 layer_count = _arraycount(layer_array);
    
    
    const s8* extension_array[] = {
        VK_KHR_SURFACE_EXTENSION_NAME,
        _surface_extension,
        VK_EXT_DEBUG_REPORT_EXTENSION_NAME
    };
    
    u32 extension_count = 2;
    
    if(validation_enable){
        extension_count++;
        global_validation_enable = true;
    }
    else{
        layer_count = 0;
    }
    
    InternalLoadVulkanLib();
    
    _kill("failed to load lib\n",!vkcreateinstance);
    
    
    global_instance = CreateInstance(applicationname_string,
                                     firstno_apiver,secondno_apiver,thirdno_apiver,
                                     layer_array,layer_count,
                                     extension_array,extension_count);
    
    InternalLoadVulkanInstanceLevelFunctions();
    
    if(!(lflags & V_L_SINGLE_VKDEVICE)){
        InternalLoadVulkanFunctions(global_instance,(void*)vkGetInstanceProcAddr);
    }
    
    if(validation_enable){
        CreateVkDebug(global_instance);
    }
}

VkQueue VGetQueue(const VDeviceContext* _restrict vdevice,VQueueType type){
    
    VkQueue queue;
    
    auto info = global_queuefamilyinfo_array[type];
    
    if(!info.capabilities){
        return 0;
    }
    
    vkGetDeviceQueue(vdevice->device,info.familyindex,0,&queue);
    
    return queue;
}

u32 VGetQueueFamilyIndex(VQueueType type){
    return global_queuefamilyinfo_array[type].familyindex;
}


VDeviceContext VCreateDeviceContext(WWindowContext* window,u32 createqueue_bits,
                                    u32 physicaldevice_index){
    
    VDeviceContext context;
    
    VkPhysicalDevice device_array[3];
    u32 count;
    
    u32 swapchain_enable = false;
    
    _vktest(vkEnumeratePhysicalDevices(global_instance,(u32*)&count,0));
    
    _vktest(vkEnumeratePhysicalDevices(global_instance,(u32*)&count,device_array));
    
    if(window){
        
        swapchain_enable = true;
        
        auto famindex = global_queuefamilyinfo_array[VQUEUETYPE_ROOT].familyindex;
        
        for(u32 i = 0; i < count; i++){
            
            auto physdevice = device_array[i];
            
#if _debug
            
            u32 tcount;
            
            vkGetPhysicalDeviceQueueFamilyProperties(physdevice,&tcount,0);
            
#endif
            
#ifdef _WIN32
            
            _instproc(vkgetphysicaldevice_xlib_wayland_win32_presentationsupportkhr,global_instance,vkGetPhysicalDeviceWin32PresentationSupportKHR);
#else
            
            
            _instproc(vkgetphysicaldevice_xlib_wayland_win32_presentationsupportkhr,global_instance,vkGetPhysicalDeviceXlibPresentationSupportKHR);
#endif
            
            auto res = _get_present_support(physdevice,window,famindex);
            
            if(res){
                physicaldevice_index = i;
                break;
            }
            
        }
        
    }
    
    context.physicaldevice = device_array[physicaldevice_index];
    
#if 1
    VkPhysicalDeviceProperties physproperties;
    vkGetPhysicalDeviceProperties(context.physicaldevice,&physproperties);
    
    _dprint("gpu:%s\n",physproperties.deviceName);
#endif
    
    context.memoryproperties =
        (VkPhysicalDeviceMemoryProperties*)alloc(sizeof(VkPhysicalDeviceMemoryProperties));
    
    vkGetPhysicalDeviceMemoryProperties(context.physicaldevice,context.memoryproperties);
    
    VkDeviceQueueCreateInfo queueinfo_array[5];
    u32 queueinfo_count = 0;
    
    VkQueueFamilyProperties queue_properties[5] = {};
    u32 queueproperties_count = 0;
    
    vkGetPhysicalDeviceQueueFamilyProperties(context.physicaldevice,
                                             &queueproperties_count,0);
    
    vkGetPhysicalDeviceQueueFamilyProperties(context.physicaldevice,
                                             &queueproperties_count,queue_properties);
    
    f32 queue_priority = 1.0f;
    
    for(u32 i = 0; i < queueproperties_count; i++){
        
        //Usually only the root queue has graphics capabilities (NV,AMD,Intel)
        if( 
            (queue_properties[i].queueFlags  & VK_QUEUE_GRAPHICS_BIT) &&
            (createqueue_bits & VCREATEQUEUEBIT_ROOT)){
            
            queueinfo_array[queueinfo_count] = {
                VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                0,
                0,
                i,
                1,//MARK:this is the queuecount of this type. we should handle this case
                &queue_priority
            };
            
            queueinfo_count++;
            
            global_queuefamilyinfo_array[VQUEUETYPE_ROOT] = {
                i,
                queue_properties[i].queueFlags,
                1 //MARK:this is the queuecount of this type. we should handle this case
            };
            
            global_queuefamilyinfo_count++;
            
        }
        
        else{
            
            if( 
                (queue_properties[i].queueFlags  & VK_QUEUE_COMPUTE_BIT) &&
                (createqueue_bits & VCREATEQUEUEBIT_COMPUTE)){
                
                queueinfo_array[queueinfo_count] = {
                    VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                    0,
                    0,
                    i,
                    1,//MARK:this is the queuecount of this type. we should handle this case
                    &queue_priority
                };
                
                queueinfo_count++;
                
                global_queuefamilyinfo_array[VQUEUETYPE_COMPUTE] = {
                    i,
                    queue_properties[i].queueFlags,
                    1 //MARK:this is the queuecount of this type. we should handle this case
                };
                
                global_queuefamilyinfo_count++;
                
            }
            
            else if(
                (queue_properties[i].queueFlags  & VK_QUEUE_TRANSFER_BIT) &&
                (createqueue_bits & VCREATEQUEUEBIT_TRANSFER)){
                
                queueinfo_array[queueinfo_count] = {
                    VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                    0,
                    0,
                    i,
                    1,//MARK:this is the queuecount of this type. we should handle this case
                    &queue_priority
                };
                
                queueinfo_count++;
                
                global_queuefamilyinfo_array[VQUEUETYPE_TRANSFER] = {
                    i,
                    queue_properties[i].queueFlags,
                    1 //MARK:this is the queuecount of this type. we should handle this case
                };
                
                global_queuefamilyinfo_count++;
                
            }
            
        }
        
    }
    
    VkPhysicalDeviceFeatures devicefeatures;
    vkGetPhysicalDeviceFeatures(context.physicaldevice,&devicefeatures);
    
    
    const s8* extension_array[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
    u32 extension_count = 0;
    
    const s8* layer_array[] = {"VK_LAYER_LUNARG_standard_validation"};
    u32 layer_count = 0;
    
    if(swapchain_enable){
        extension_count++;
    }
    
    if(global_validation_enable){
        layer_count++;
    }
    
    context.device =
        CreateDevice(context.physicaldevice,queueinfo_array,queueinfo_count,layer_array,
                     layer_count,extension_array,extension_count,&devicefeatures); 
    
    if(!vkcreategraphicspipelines){
        
#if _debug
        
        _kill("V_L_SINGLE_VKDEVICE specified but another VkDevice was created\n",global_device);
        global_device = context.device;
        
        InternalLoadVulkanFunctions(context.device,(void*)vkGetDeviceProcAddr);
        
#endif
        
    }
    
    return context;
}

VSwapchainContext VCreateSwapchainContext(const VDeviceContext* _restrict vdevice,
                                          u32 swapcount,WWindowContext windowcontext,
                                          VPresentSyncType sync_type,
                                          VSwapchainContext* oldswapchain){
    
    //combine surface and swapchain creation, pull CreateSwapchain into here
    
#ifdef _WIN32
    
    _instproc(vkcreate_xlib_wayland_win32surfacekhr,global_instance,vkCreateWin32SurfaceKHR);
    
    VkSurfaceKHR surface =
        CreateSurface(global_instance,windowcontext.handle, windowcontext.rootwindow);
    
#else
    
    _instproc(vkcreate_xlib_wayland_win32surfacekhr,global_instance,vkCreateXlibSurfaceKHR);
    
    VkSurfaceKHR surface =
        CreateSurface(global_instance,windowcontext.x11_handle, windowcontext.x11_rootwindow);
    
#endif
    
    VkBool32 supported;
    
#if _debug
    
    //this is to shut the validation layer up
    
    vkGetPhysicalDeviceSurfaceSupportKHR(vdevice->physicaldevice,
                                         0,surface,&supported);
#endif
    
    
    VSwapchainContext context =
        CreateSwapchain(global_instance,vdevice->physicaldevice,vdevice->device,
                        *(vdevice->memoryproperties),surface,windowcontext.width,
                        windowcontext.height,swapcount,oldswapchain,sync_type);
    
    context.internal->surface = surface;
    
    return context;
}

VkFence VCreateFence(VDeviceContext* _in_ vdevice,VkFenceCreateFlags flags){
    
    VkFence fence;
    
    VkFenceCreateInfo createinfo = {VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,0,flags};
    
    _vktest(vkCreateFence(vdevice->device,&createinfo,global_allocator,&fence));
    
    return fence;
}


VkDescriptorPool VCreateDescriptorPool(VDeviceContext* _in_ vdevice,
                                       VDescriptorPoolSpec poolspec,u32 flags,u32 max_sets){
    
    VkDescriptorPool pool;
    
    VkDescriptorPoolCreateInfo createinfo = {
        VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        0,flags,max_sets,(u32)poolspec.count,poolspec.container
    };
    
    _vktest(vkCreateDescriptorPool(vdevice->device,&createinfo,global_allocator,&pool));
    
    return pool;
}


void VDescPushBackPoolSpec(VDescriptorPoolSpec* poolspec,VkDescriptorType type,u32 count){
    
    _kill("exceeded poolspec types. do not submit the same type more than once\n",
          poolspec->count >= 10);
    
    poolspec->container[poolspec->count] = {type,count};
    
    poolspec->count++;
}

void VDescPushBackBindingSpec(VDescriptorBindingSpec* bindingspec,VkDescriptorType type,
                              u32 count,VkShaderStageFlags stage_flags,VkSampler* immutable_samplers){
    
    _kill("exceeded bindingspec types\n",
          bindingspec->count >= 20);
    
    bindingspec->container[bindingspec->count] =
    {bindingspec->count,type,count,stage_flags,immutable_samplers};
    
    bindingspec->count++;
    
}

VkDescriptorSetLayout VCreateDescriptorSetLayout(const  VDeviceContext* _restrict vdevice,
                                                 VDescriptorBindingSpec bindingspec){
    
    VkDescriptorSetLayout layout;
    
    VkDescriptorSetLayoutCreateInfo info =
    {
        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        0,0,bindingspec.count,bindingspec.container
    };
    
    
    _vktest(vkCreateDescriptorSetLayout(vdevice->device,&info,global_allocator,&layout));
    
    
    return layout;
}


void VAllocDescriptorSetArray(const  VDeviceContext* _restrict vdevice,
                              VkDescriptorPool pool,u32 count,
                              VkDescriptorSetLayout* layout_array,VkDescriptorSet* set_array){
    
    VkDescriptorSetAllocateInfo info =
    {
        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        0,pool,count,layout_array
    };
    
    _vktest(vkAllocateDescriptorSets(vdevice->device,&info,set_array));
}

void VDescPushBackWriteSpecImage(VDescriptorWriteSpec* spec,VkDescriptorSet dst_set,
                                 u32 dst_binding,u32 dst_startindex,u32 count,VkDescriptorType type,
                                 VkDescriptorImageInfo* imageinfo_array){
    
    _kill("exceeded bindingspec types\n",
          spec->count >= 20);
    
    spec->container[spec->count] = {
        VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        0,dst_set,dst_binding,dst_startindex,count,type,imageinfo_array,0,0
    };
    
    spec->count++;
}

void VDescPushBackWriteSpecBuffer(VDescriptorWriteSpec* spec,VkDescriptorSet dst_set,
                                  u32 dst_binding,u32 dst_startindex,u32 count,VkDescriptorType type,
                                  VkDescriptorBufferInfo* bufferinfo_array){
    _kill("exceeded bindingspec types\n",
          spec->count >= 20);
    
    spec->container[spec->count] = {
        VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        0,dst_set,dst_binding,dst_startindex,count,type,0,bufferinfo_array,0
    };
    
    spec->count++;  
}

void VDescPushBackWriteSpecView(VDescriptorWriteSpec* spec,VkDescriptorSet dst_set,
                                u32 dst_binding,u32 dst_startindex,u32 count,VkDescriptorType type,
                                VkBufferView* bufferview_array){
    _kill("exceeded bindingspec types\n",
          spec->count >=20);
    
    spec->container[spec->count] = {
        VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        0,dst_set,dst_binding,dst_startindex,count,type,0,0,bufferview_array
    };
    
    spec->count++;  
}

void VPushBackAttachmentSpec(VAttachmentSpec* spec,
                             VkAttachmentDescriptionFlags flags,VkFormat format,
                             VkSampleCountFlagBits samples,VkAttachmentLoadOp loadop,
                             VkAttachmentStoreOp storeop,VkAttachmentLoadOp stencil_loadop,
                             VkAttachmentStoreOp stencil_storeop,VkImageLayout initial,
                             VkImageLayout final){
    
    _kill("exceeded bindingspec types\n",
          spec->count >= 10);
    
    u32 index = spec->count;
    
    spec->des_container[index] =
    {flags,format,samples,loadop,storeop,stencil_loadop,stencil_storeop,initial,final};
    
    spec->ref_container[index] = {index,initial};
    spec->count++;
}

void VPushBackSubpassDescriptionSpec(VSubpassDescriptionSpec* spec,
                                     VkSubpassDescriptionFlags flags,VkPipelineBindPoint bindpoint,
                                     u32 inputattachment_count,
                                     const VkAttachmentReference* inputattachment_array,
                                     u32 colorattachment_count,
                                     const VkAttachmentReference* colorattachment_array,
                                     const VkAttachmentReference* resolveattachment_array,
                                     const VkAttachmentReference* depthstencilattachment_array,
                                     u32 preserveattachment_count,
                                     const u32* preserveattachment_array){
    
    _kill("exceeded bindingspec types\n",
          spec->count >= 10);
    
    spec->container[spec->count] = {flags,bindpoint,inputattachment_count,inputattachment_array,
        colorattachment_count,colorattachment_array,resolveattachment_array,
        depthstencilattachment_array,preserveattachment_count,
        preserveattachment_array};
    
    spec->count++;
}

void VPushBackSubpassDependencySpec(VSubpassDependencySpec* spec,
                                    u32 srcsubpass_index,u32 dstsubpass_index,
                                    VkPipelineStageFlags src_stagemask,VkPipelineStageFlags dst_stagemask,
                                    VkAccessFlags src_accessmask,VkAccessFlags dst_accessmask,
                                    VkDependencyFlags dependencyflags){
    
    _kill("exceeded bindingspec types\n",
          spec->count >= 20);
    
    spec->container[spec->count] = {srcsubpass_index,dstsubpass_index,src_stagemask,
        dst_stagemask,src_accessmask,dst_accessmask,dependencyflags};
    
    spec->count++;
}

VkRenderPass VCreateRenderPass(const  VDeviceContext* _restrict vdevice,
                               VkRenderPassCreateFlags flags,VAttachmentSpec attachmentspec,
                               VSubpassDescriptionSpec subpassdescspec,
                               VSubpassDependencySpec subpassdepspec){
    
    VkRenderPass renderpass;
    
    
    VkRenderPassCreateInfo renderpass_info = {};
    
    renderpass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderpass_info.pNext = 0;
    renderpass_info.flags = flags;
    renderpass_info.attachmentCount =attachmentspec.count;
    renderpass_info.pAttachments = attachmentspec.des_container;
    renderpass_info.subpassCount = subpassdescspec.count;
    renderpass_info.pSubpasses = subpassdescspec.container;
    renderpass_info.dependencyCount = subpassdepspec.count;
    renderpass_info.pDependencies = subpassdepspec.container;
    
    
    
    _vktest(vkCreateRenderPass(vdevice->device,&renderpass_info,
                               global_allocator ,&renderpass));
    
    return renderpass;
}



void VStartCommandBuffer(VkCommandBuffer cmdbuffer,
                         VkCommandBufferUsageFlags flags,
                         VkRenderPass renderpass,u32 subpass,
                         VkFramebuffer framebuffer,
                         VkBool32 occlusion_enable,
                         VkQueryControlFlags queryflags,
                         VkQueryPipelineStatisticFlags querypipelineflags){
    
    VkCommandBufferInheritanceInfo inherit_info = {};
    
    inherit_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
    inherit_info.renderPass = renderpass;
    inherit_info.subpass = subpass;
    inherit_info.framebuffer = framebuffer;
    inherit_info.occlusionQueryEnable = occlusion_enable;
    inherit_info.queryFlags = queryflags;
    inherit_info.pipelineStatistics = querypipelineflags;
    
    VkCommandBufferBeginInfo begin_info = {};
    
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin_info.flags = flags;
    begin_info.pInheritanceInfo = &inherit_info;
    
    _vktest(vkBeginCommandBuffer(cmdbuffer, &begin_info));
}

void VStartCommandBuffer(VkCommandBuffer cmdbuffer,
                         VkCommandBufferUsageFlags flags){
    
    VkCommandBufferBeginInfo begin_info = {};
    
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin_info.flags = flags;
    begin_info.pInheritanceInfo = 0;
    
    _vktest(vkBeginCommandBuffer(cmdbuffer, &begin_info));
    
}

void VEndCommandBuffer(VkCommandBuffer cmdbuffer){
    _vktest(vkEndCommandBuffer(cmdbuffer));
}

void VSubmitCommandBufferArray(VkQueue queue,VkCommandBuffer* commandbuffer,
                               u32 buffer_count,VkSemaphore* wait_semaphore,u32 wait_count,
                               VkSemaphore* signal_semaphore,u32 signal_count,
                               VkPipelineStageFlags* wait_dstmask,VkFence fence){
    
    VkSubmitInfo info =   {
        VK_STRUCTURE_TYPE_SUBMIT_INFO,
        0,
        wait_count,
        wait_semaphore,
        wait_dstmask,
        buffer_count,
        commandbuffer,
        signal_count,
        signal_semaphore
    };
    
    _vktest(vkQueueSubmit(queue,1,&info,fence));
}

void VSubmitCommandBufferArray(VkQueue queue,VkCommandBuffer* commandbuffer,
                               u32 buffer_count,VkSemaphore wait_semaphore,
                               VkSemaphore signal_semaphore,VkPipelineStageFlags wait_dstmask,
                               VkFence fence){
    
    VSubmitCommandBufferArray(queue,commandbuffer,buffer_count,&wait_semaphore,1,
                              &signal_semaphore,1,&wait_dstmask,fence);
    
}

void VSubmitCommandBuffer(VkQueue queue,VkCommandBuffer commandbuffer,
                          VkSemaphore* wait_semaphore,u32 wait_count,
                          VkSemaphore* signal_semaphore,u32 signal_count,
                          VkPipelineStageFlags* wait_dstmask,VkFence fence){
    
    VSubmitCommandBufferArray(queue,&commandbuffer,1,wait_semaphore,wait_count,
                              signal_semaphore,signal_count,wait_dstmask,fence);
}

void VSubmitCommandBuffer(VkQueue queue,VkCommandBuffer commandbuffer,
                          VkSemaphore wait_semaphore,
                          VkSemaphore signal_semaphore,VkPipelineStageFlags wait_dstmask,
                          VkFence fence){
    
    VSubmitCommandBuffer(queue,commandbuffer,&wait_semaphore,1,&signal_semaphore,1,
                         &wait_dstmask,fence);
    
}

void VSubmitCommandBufferBatch(VkQueue queue,VSubmitBatch batch,VkFence fence){
    
    _vktest(vkQueueSubmit(queue,batch.count,batch.container,fence));
    
}


void VQueuePresentArray(VkQueue queue,u32* imageindex_array,
                        VkSwapchainKHR* swapchain_array,
                        ptrsize swapchain_count,
                        VkSemaphore* _restrict waitsemaphore_array,
                        ptrsize waitsemaphore_count,
                        VkResult* result_array){
    
    VkPresentInfoKHR present_info = {};
    
    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present_info.pNext = 0;
    present_info.waitSemaphoreCount = waitsemaphore_count;
    present_info.pWaitSemaphores = waitsemaphore_array;
    present_info.swapchainCount = swapchain_count;
    present_info.pSwapchains = swapchain_array;
    present_info.pImageIndices = imageindex_array;
    present_info.pResults = result_array;
    
    _vktest(vkQueuePresentKHR(queue, &present_info));
}

void VQueuePresent(VkQueue queue,u32 image_index,VkSwapchainKHR swapchain,
                   VkSemaphore waitsemaphore){
    
    VkResult res;
    
    VQueuePresentArray(queue,&image_index,&swapchain,1,&waitsemaphore,1,&res);  
    
}

VkFramebuffer VCreateFrameBuffer(const  VDeviceContext* _restrict vdevice,
                                 VkFramebufferCreateFlags flags,
                                 VkRenderPass renderpass,VkImageView* attachment_array,
                                 u32 attachment_count,u32 width,u32 height,u32 layers){
    
    VkFramebuffer framebuffer;
    
    VkFramebufferCreateInfo info = {};
    
    info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    info.flags = flags;
    info.renderPass = renderpass;
    info.attachmentCount = attachment_count;
    info.pAttachments = attachment_array;
    info.width = width;
    info.height = height;
    info.layers = layers;
    
    _vktest(vkCreateFramebuffer(vdevice->device,&info,global_allocator,&framebuffer));
    
    return framebuffer;
}


void VAllocateCommandBufferArray(const  VDeviceContext* _restrict vdevice,VkCommandPool pool,
                                 VkCommandBufferLevel level,
                                 VkCommandBuffer* _restrict commandbuffer_array,
                                 u32 commandbuffer_count){
    
    VkCommandBufferAllocateInfo cmd_info = {};
    
    cmd_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    cmd_info.pNext = 0;
    cmd_info.commandPool = pool;
    cmd_info.level = level;
    cmd_info.commandBufferCount = commandbuffer_count;
    
    
    _vktest(vkAllocateCommandBuffers(vdevice->device, &cmd_info, commandbuffer_array));
}

VkCommandBuffer VAllocateCommandBuffer(const  VDeviceContext* _restrict vdevice,
                                       VkCommandPool pool,VkCommandBufferLevel level){
    
    VkCommandBuffer cmdbuffer;
    
    VAllocateCommandBufferArray(vdevice,pool,level,&cmdbuffer,1);
    
    return cmdbuffer;
}

VkCommandPool VCreateCommandPool(const  VDeviceContext* _restrict vdevice,
                                 VkCommandPoolCreateFlags flags,u32 familyindex){
    
    VkCommandPool pool;
    
    VkCommandPoolCreateInfo pool_info = {};
    pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    pool_info.pNext = 0;
    pool_info.flags = flags;
    pool_info.queueFamilyIndex = familyindex;
    
    _vktest(vkCreateCommandPool(vdevice->device,&pool_info,global_allocator,&pool));
    
    return pool;
}

void VStartRenderpass(VkCommandBuffer commandbuffer,VkSubpassContents contents,
                      VkRenderPass renderpass,VkFramebuffer framebuffer,VkRect2D renderarea,
                      VkClearValue* clearvalue_array,u32 clearvalue_count){
    
    VkRenderPassBeginInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    info.renderPass = renderpass;
    info.framebuffer = framebuffer;
    info.renderArea = renderarea;
    info.clearValueCount = clearvalue_count;
    info.pClearValues = clearvalue_array;
    
    vkCmdBeginRenderPass(commandbuffer,&info,contents);
}

void VStartRenderpass(VkCommandBuffer commandbuffer,VkSubpassContents contents,
                      VkRenderPass renderpass,VkFramebuffer framebuffer,VkRect2D renderarea,
                      VkClearValue clearvalue){
    
    VStartRenderpass(commandbuffer,contents,renderpass,framebuffer,renderarea,&clearvalue,1);
}

void VEndRenderPass(VkCommandBuffer commandbuffer){
    vkCmdEndRenderPass(commandbuffer);
}

VkSemaphore VCreateSemaphore(const  VDeviceContext* _restrict vdevice){
    
    VkSemaphoreCreateInfo semaphore_info = {};
    
    semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    semaphore_info.pNext = 0;
    semaphore_info.flags = 0;
    
    VkSemaphore semaphore = {};
    
    
    _vktest(vkCreateSemaphore(vdevice->device,&semaphore_info,global_allocator,&semaphore));
    
    return semaphore;
}

VBufferContext VCreateUniformBufferContext(const  VDeviceContext* _restrict vdevice,
                                           u32 data_size,logic is_coherrent){
    
    VBufferContext context;
    VkMemoryRequirements memreq;
    
    context.buffer = CreateBuffer(vdevice->device,0,data_size,
                                  VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                  VK_SHARING_MODE_EXCLUSIVE,0,0);
    
    vkGetBufferMemoryRequirements(vdevice->device,context.buffer,&memreq);
    
    u32 flag = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
    
    if(is_coherrent){
        flag |= VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    }
    
    else{
        flag |= VK_MEMORY_PROPERTY_HOST_CACHED_BIT;
    }
    
    
    u32 typeindex = 
        VGetMemoryTypeIndex(*(vdevice->memoryproperties),
                            memreq.memoryTypeBits,flag);
    
    _kill("invalid memory type\n",typeindex == (u32)-1);
    
    context.memory = 
        deviceallocator(vdevice->device,memreq.size,typeindex);
    
    _vktest(vkBindBufferMemory(vdevice->device,context.buffer,context.memory,0));
    
    context.size = memreq.size;
    
    return context;
}

VBufferContext VCreateShaderStorageBufferContext(
const  VDeviceContext* _restrict vdevice,
u32 data_size,logic is_devicelocal,logic is_coherrent){
    VBufferContext context;
    VkMemoryRequirements memreq;
    
    context.buffer = CreateBuffer(vdevice->device,0,data_size,
                                  VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
                                  VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                  VK_SHARING_MODE_EXCLUSIVE,0,0);
    
    vkGetBufferMemoryRequirements(vdevice->device,context.buffer,&memreq);
    
    
    u32 flag = 0;
    
    if(is_devicelocal){
        flag |= VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    }
    
    else{
        
        flag = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
        
        if(is_coherrent){
            flag |= VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
        }
        
        else{
            flag |= VK_MEMORY_PROPERTY_HOST_CACHED_BIT;
        }
        
    }
    
    
    u32 typeindex = 
        VGetMemoryTypeIndex(*(vdevice->memoryproperties),
                            memreq.memoryTypeBits,flag);
    
    _kill("invalid memory type\n",typeindex == (u32)-1);
    
    context.memory = 
        deviceallocator(vdevice->device,memreq.size,typeindex);
    
    _vktest(vkBindBufferMemory(vdevice->device,context.buffer,context.memory,0));
    
    context.size = memreq.size;
    
    return context;
}

void VUpdateUniformBuffer(const  VDeviceContext* _restrict vdevice,VBufferContext context,
                          void* data,u32 data_size){
    
    void* mapped_ptr;
    
    _vktest(vkMapMemory(vdevice->device,context.memory,0,context.size,0,&mapped_ptr));
    
    
    memcpy(mapped_ptr,data,data_size);
    
    
    vkUnmapMemory(vdevice->device,context.memory);
}

VImageMemoryContext VCreateColorImageMemory(
const  VDeviceContext* _restrict vdevice,u32 width,u32 height,u32 usage,
logic is_device_local,logic is_coherent,VkImageTiling tiling,
VkFormat format){
    
    VImageMemoryContext context = {};
    
    u32 memory_property = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    auto layout = VK_IMAGE_LAYOUT_UNDEFINED;
    
    if(!is_device_local){
        layout = VK_IMAGE_LAYOUT_PREINITIALIZED;
        memory_property = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
        
        if(is_coherent){
            memory_property |= VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
        }
        
        else{
            memory_property |= VK_MEMORY_PROPERTY_HOST_CACHED_BIT;
        }
        
    }
    
    
    context.image =
        CreateImage(vdevice->device,0,
                    VK_IMAGE_TYPE_2D,format,
                    {(u32)width,(u32)height,1},
                    1,1,VK_SAMPLE_COUNT_1_BIT,tiling,
                    usage,
                    VK_SHARING_MODE_EXCLUSIVE,0,0,
                    layout,vdevice->physicaldevice);
    
    VkMemoryRequirements memoryreq;
    
    vkGetImageMemoryRequirements(vdevice->device,context.image,&memoryreq);
    
    //MARK:
    u32 typeindex = VGetMemoryTypeIndex(*(vdevice->memoryproperties),
                                        memoryreq.memoryTypeBits,
                                        memory_property);
    
    _kill("invalid memory type\n",typeindex == (u32)-1);
    
    context.memory = 
        deviceallocator(vdevice->device,memoryreq.size,typeindex);
    
    _vktest(vkBindImageMemory(vdevice->device,context.image,context.memory,0));
    
    return context;
}


VImageContext VCreateColorImage(const  VDeviceContext* _restrict vdevice,
                                u32 width,u32 height,u32 usage,logic is_device_local,logic is_coherent,
                                VkImageTiling tiling,VkFormat format){
    
    VImageContext context = {};
    
    auto img_mm =
        VCreateColorImageMemory(vdevice,width,height,usage,is_device_local,is_coherent,tiling,
                                format);
    context.image = img_mm.image;
    context.memory = img_mm.memory;
    
    context.view = 
        CreateImageView(vdevice->device,0,context.image,VK_IMAGE_VIEW_TYPE_2D,
                        format,
                        {VK_COMPONENT_SWIZZLE_R,VK_COMPONENT_SWIZZLE_G,
                        VK_COMPONENT_SWIZZLE_B,VK_COMPONENT_SWIZZLE_A},
                        {VK_IMAGE_ASPECT_COLOR_BIT,0,1,0,1});
    
    return context;
}



//TODO: make this bgra instead
VTextureContext VCreateTextureImage(const  VDeviceContext* _restrict vdevice,void* data,
                                    u32 width,u32 height,VkCommandBuffer commandbuffer,VkQueue queue){
    
    VTextureContext handle;
    VBufferContext src;
    VkMemoryRequirements memoryreq;
    
    u32 data_size = width * height * 4;
    
    src.buffer = CreateBuffer(vdevice->device,0,data_size,
                              VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                              VK_SHARING_MODE_EXCLUSIVE,0,0);
    
    vkGetBufferMemoryRequirements(vdevice->device,src.buffer,
                                  &memoryreq);
    
    src.size = memoryreq.size;
    
    
    
    auto typeindex = VGetMemoryTypeIndex(*(vdevice->memoryproperties),
                                         memoryreq.memoryTypeBits,
                                         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
    
    _kill("invalid memory type\n",typeindex == (u32)-1);
    
    src.memory = 
        deviceallocator(vdevice->device,memoryreq.size,typeindex);
    
    vkBindBufferMemory(vdevice->device,src.buffer,src.memory,0);
    
    
    
    void* mappedmemory_ptr;
    
    vkMapMemory(vdevice->device,src.memory,0,src.size,0,&mappedmemory_ptr);
    
    memcpy(mappedmemory_ptr,data,data_size);
    
    vkUnmapMemory(vdevice->device,src.memory);
    
    handle.image = 
        CreateImage(vdevice->device,0,
                    VK_IMAGE_TYPE_2D,VK_FORMAT_R8G8B8A8_UNORM,
                    {(u32)width,(u32)height,1},
                    1,1,VK_SAMPLE_COUNT_1_BIT,VK_IMAGE_TILING_OPTIMAL,
                    VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                    VK_SHARING_MODE_EXCLUSIVE,0,0,VK_IMAGE_LAYOUT_UNDEFINED,
                    vdevice->physicaldevice);
    
    
    auto layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    
    
    vkGetImageMemoryRequirements(vdevice->device,handle.image,&memoryreq);
    
    typeindex = VGetMemoryTypeIndex(*(vdevice->memoryproperties),
                                    memoryreq.memoryTypeBits,
                                    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    
    _kill("invalid memory type\n",typeindex == (u32)-1);
    
    //printf("image %d\n",typeindex);
    
    handle.memory = 
        deviceallocator(vdevice->device,memoryreq.size,typeindex);
    
    _vktest(vkBindImageMemory(vdevice->device,handle.image,handle.memory,0));
    
    //transfer data and transition image layouts
    {
        
        VkImageMemoryBarrier transfer_imagebarrier = {
            VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            0,
            0,
            VK_ACCESS_TRANSFER_WRITE_BIT,
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            VK_QUEUE_FAMILY_IGNORED,
            VK_QUEUE_FAMILY_IGNORED,
            handle.image,
            {VK_IMAGE_ASPECT_COLOR_BIT,0,1,0,1},
        };
        
        VkImageMemoryBarrier shader_imagebarrier = {
            VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            0,
            VK_ACCESS_TRANSFER_WRITE_BIT,
            VK_ACCESS_SHADER_READ_BIT,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            layout,
            VK_QUEUE_FAMILY_IGNORED,
            VK_QUEUE_FAMILY_IGNORED,
            handle.image,
            {VK_IMAGE_ASPECT_COLOR_BIT,0,1,0,1},
        };
        
        VStartCommandBuffer(commandbuffer,
                            VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
        
        vkCmdPipelineBarrier(commandbuffer,
                             VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                             VK_PIPELINE_STAGE_TRANSFER_BIT,0,0,0,0,0,1,
                             &transfer_imagebarrier);
        
        //copy image
        
        //mip level refers to the current one, not the total
        VkBufferImageCopy copy = {
            0,0,0,{VK_IMAGE_ASPECT_COLOR_BIT,0,0,1},
            {},
            {(u32)width,(u32)height,1},
        };
        
        vkCmdCopyBufferToImage(commandbuffer,src.buffer,handle.image,
                               VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                               1,&copy);
        
        
        vkCmdPipelineBarrier(commandbuffer,
                             VK_PIPELINE_STAGE_TRANSFER_BIT,
                             VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,0,0,0,0,0,1,
                             &shader_imagebarrier);
        
        
        VEndCommandBuffer(commandbuffer);
        
        VSubmitCommandBuffer(queue,commandbuffer);
        
        vkQueueWaitIdle(queue);
        
    }
    
    handle.sampler = 
        CreateSampler(vdevice->device,0,VK_FILTER_LINEAR,VK_FILTER_LINEAR,
                      VK_SAMPLER_MIPMAP_MODE_LINEAR,
                      VK_SAMPLER_ADDRESS_MODE_REPEAT,
                      VK_SAMPLER_ADDRESS_MODE_REPEAT,
                      VK_SAMPLER_ADDRESS_MODE_REPEAT,0.0f,
                      VK_TRUE,8.0f,VK_FALSE,VK_COMPARE_OP_NEVER,0.0f,0.0f,
                      VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE,VK_FALSE);
    
    handle.view = 
        CreateImageView(vdevice->device,0,handle.image,VK_IMAGE_VIEW_TYPE_2D,
                        VK_FORMAT_R8G8B8A8_UNORM,
                        {VK_COMPONENT_SWIZZLE_R,VK_COMPONENT_SWIZZLE_G,
                        VK_COMPONENT_SWIZZLE_B,VK_COMPONENT_SWIZZLE_A},
                        {VK_IMAGE_ASPECT_COLOR_BIT,0,1,0,1});
    
    vkDestroyBuffer(vdevice->device,src.buffer,global_allocator);
    vkFreeMemory(vdevice->device,src.memory,global_allocator);
    
    return handle;
}

//move to vvulkanx
VTextureContext VCreateTextureCache(const  VDeviceContext* _restrict vdevice,u32 width,u32 height,VkFormat format){
    
    VTextureContext handle;
    VkMemoryRequirements memoryreq;
    
    handle.image = 
        CreateImage(vdevice->device,0,
                    VK_IMAGE_TYPE_2D,format,
                    {(u32)width,(u32)height,1},
                    1,1,VK_SAMPLE_COUNT_1_BIT,VK_IMAGE_TILING_OPTIMAL,
                    VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                    VK_SHARING_MODE_EXCLUSIVE,0,0,VK_IMAGE_LAYOUT_UNDEFINED,
                    vdevice->physicaldevice);
    
    
    vkGetImageMemoryRequirements(vdevice->device,handle.image,&memoryreq);
    
    auto typeindex = VGetMemoryTypeIndex(*(vdevice->memoryproperties),
                                         memoryreq.memoryTypeBits,
                                         VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    
    _kill("invalid memory type\n",typeindex == (u32)-1);
    
    handle.memory = 
        deviceallocator(vdevice->device,memoryreq.size,typeindex);
    
    _vktest(vkBindImageMemory(vdevice->device,handle.image,handle.memory,0));
    
    handle.view = 
        CreateImageView(vdevice->device,0,handle.image,VK_IMAGE_VIEW_TYPE_2D,
                        format,
                        {VK_COMPONENT_SWIZZLE_R,VK_COMPONENT_SWIZZLE_G,
                        VK_COMPONENT_SWIZZLE_B,VK_COMPONENT_SWIZZLE_A},
                        {VK_IMAGE_ASPECT_COLOR_BIT,0,1,0,1});
    
    
#if 1
    
    handle.sampler =
        CreateSampler(vdevice->device,0,VK_FILTER_NEAREST,VK_FILTER_NEAREST,
                      VK_SAMPLER_MIPMAP_MODE_NEAREST,
                      VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE  ,
                      VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE ,
                      VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE ,
                      0.0f,
                      VK_FALSE,0.0f,VK_FALSE,VK_COMPARE_OP_NEVER,0.0f,0.0f,
                      VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE,VK_FALSE);
    
#else
    
    
    handle.sampler =
        CreateSampler(vdevice->device,0,VK_FILTER_LINEAR,VK_FILTER_LINEAR,
                      VK_SAMPLER_MIPMAP_MODE_NEAREST,
                      VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE  ,
                      VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE ,
                      VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE ,
                      0.0f,
                      VK_FALSE,0.0f,VK_FALSE,VK_COMPARE_OP_NEVER,0.0f,0.0f,
                      VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE,VK_FALSE);
    
#endif
    
    return handle;
}

void VDestroyPipeline(const  VDeviceContext* _restrict vdevice,VkPipeline pipeline){
    vkDestroyPipeline(vdevice->device,pipeline,global_allocator);
}

void VDestroyBuffer(const  VDeviceContext* _restrict vdevice,VkBuffer buffer){
    vkDestroyBuffer(vdevice->device,buffer,global_allocator);
}

void VDestroyBufferContext(const  VDeviceContext* _restrict vdevice,VBufferContext buffer){
    VDestroyBuffer(vdevice,buffer.buffer);
    VFreeMemory(vdevice,buffer.memory);
}

void VFreeMemory(const  VDeviceContext* _restrict vdevice,VkDeviceMemory memory){
    vkFreeMemory(vdevice->device,memory,global_allocator);
}



//MARK: this doesn't feel like it fits
VTextureContext VCreateTexturePageTable(const  VDeviceContext* _restrict vdevice,
                                        u32 width,u32 height,u32 miplevels){
    
    auto format = VK_FORMAT_R8G8B8A8_UNORM;
    
    VTextureContext context;
    VkMemoryRequirements memoryreq;
    
    auto pwidth = width/128;
    auto pheight = height/128;
    auto arraylayers = 1;
    
    context.image = 
        CreateImage(vdevice->device,0,
                    VK_IMAGE_TYPE_2D,format,
                    {(u32)pwidth,(u32)pheight,1},
                    miplevels,arraylayers,
                    VK_SAMPLE_COUNT_1_BIT,VK_IMAGE_TILING_OPTIMAL,
                    VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                    VK_SHARING_MODE_EXCLUSIVE,
                    0,0,
                    VK_IMAGE_LAYOUT_UNDEFINED,
                    vdevice->physicaldevice);
    
    vkGetImageMemoryRequirements(vdevice->device,context.image,&memoryreq);
    
    auto typeindex = VGetMemoryTypeIndex(*(vdevice->memoryproperties),
                                         memoryreq.memoryTypeBits,
                                         VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    
    _kill("invalid memory type\n",typeindex == (u32)-1);
    
    context.memory = 
        deviceallocator(vdevice->device,memoryreq.size,typeindex);
    
    _vktest(vkBindImageMemory(vdevice->device,context.image,context.memory,0));
    
    context.view = 
        CreateImageView(vdevice->device,0,context.image,VK_IMAGE_VIEW_TYPE_2D,format,
                        {VK_COMPONENT_SWIZZLE_R,VK_COMPONENT_SWIZZLE_G,
                        VK_COMPONENT_SWIZZLE_B,VK_COMPONENT_SWIZZLE_A},
                        {VK_IMAGE_ASPECT_COLOR_BIT,0,miplevels,0,1});
    
    context.sampler = 
        CreateSampler(vdevice->device,0,VK_FILTER_NEAREST,VK_FILTER_NEAREST,
                      VK_SAMPLER_MIPMAP_MODE_NEAREST,
                      VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE ,
                      VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE ,
                      VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE ,
                      0.0f,
                      VK_FALSE,0.0f,VK_FALSE,VK_COMPARE_OP_NEVER,0.0f,
                      (f32)miplevels,VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE,
                      VK_FALSE);
    
    return context;
}

void VSetComputePipelineSpecShader(VComputePipelineSpec* spec,void* shader_data,
                                   u32 shader_size,VkSpecializationInfo specialization){
    
    spec->shader_data = shader_data;
    spec->shader_size = shader_size;
    spec->shader_specialization = specialization;
}

void VGenerateComputePipelineSpec(VComputePipelineSpec* spec,VkPipelineLayout layout,
                                  VkPipelineCreateFlags flags,VkPipeline parent_pipeline,
                                  s32 parentpipeline_index){
    
    spec->flags = flags;
    spec->parent_pipeline = parent_pipeline;
    spec->parentpipeline_index = parentpipeline_index;
    spec->layout = layout;
}

void VCreateComputePipelineArray(const  VDeviceContext* _restrict vdevice,
                                 VkPipelineCache cache,VComputePipelineSpec* spec_array,u32 spec_count,
                                 VkPipeline* pipeline_array){
    
    VkComputePipelineCreateInfo info_array[10];
    VkShaderModule shadermodule_array[10];
    u32 shadermodule_count = 0;
    
    for(u32 i = 0; i < spec_count; i++){
        
        auto spec = &spec_array[i];
        
        shadermodule_array[shadermodule_count] =
            VCreateShaderModule(vdevice->device,spec->shader_data,spec->shader_size,0);
        
        VkSpecializationInfo* specialization = 0;
        
        if(!spec->shader_specialization.mapEntryCount){
            specialization = &spec->shader_specialization;
        }
        
        info_array[i] = {
            VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
            0,
            spec->flags,
            {
                VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                0,0,VK_SHADER_STAGE_COMPUTE_BIT,
                shadermodule_array[shadermodule_count],
                "main",
                specialization,
            },
            spec->layout,
            spec->parent_pipeline,
            spec->parentpipeline_index
        };
        
        shadermodule_count++;
    }
    
    _vktest(vkCreateComputePipelines(vdevice->device,cache,spec_count,info_array,
                                     global_allocator,pipeline_array));
    
    for(u32 i = 0; i < shadermodule_count; i++){
        vkDestroyShaderModule(vdevice->device,shadermodule_array[i],global_allocator);
    }
    
    
}



void VSetFixedViewportGraphicsPipelineSpec(VGraphicsPipelineSpecObj* spec,
                                           VkViewport* viewport,u32 viewport_count,VkRect2D* scissor,
                                           u32 scissor_count){
    
    memcpy(&spec->viewport_array[0],viewport,sizeof(VkViewport) * viewport_count);
    
    memcpy(&spec->scissor_array[0],scissor,sizeof(VkRect2D) * scissor_count);
    
    spec->viewport.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    
    spec->viewport.viewportCount = viewport_count;
    spec->viewport.scissorCount = scissor_count;
}

void VSetFixedViewportGraphicsPipelineSpec(VGraphicsPipelineSpecObj* spec,
                                           u16 width,u16 height){
    
    VkViewport viewport = {0.0f,0.0f,(f32)width,(f32)height,0.0f,1.0f};
    VkRect2D scissor = {{},width,height};
    
    VSetFixedViewportGraphicsPipelineSpec(spec,&viewport,1,&scissor,1);
}


//TODO: VkSampleMask* samplemask is a dependency
void VSetMultisampleGraphicsPipelineSpec(VGraphicsPipelineSpecObj* spec,
                                         VkSampleCountFlagBits samplecount_bits,
                                         VkBool32 is_persample_perfragment,//true = sample,else frag
                                         f32 minsampleshading,
                                         VkSampleMask* samplemask,
                                         VkBool32 enable_alpha_to_coverage,
                                         VkBool32 enable_alpha_to_one){
    spec->multisample.sType =VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    
    spec->multisample.pNext = 0;
    spec->multisample.flags = 0;
    spec->multisample.rasterizationSamples = samplecount_bits;
    spec->multisample.sampleShadingEnable = is_persample_perfragment;
    spec->multisample.minSampleShading = minsampleshading;
    spec->multisample.pSampleMask= samplemask;
    spec->multisample.alphaToCoverageEnable = enable_alpha_to_coverage;
    spec->multisample.alphaToOneEnable = enable_alpha_to_one;
}


void VSetDepthStencilGraphicsPipelineSpec(VGraphicsPipelineSpecObj* spec,
                                          VkBool32 depthtest_enable,
                                          VkBool32 depthwrite_enable,VkCompareOp depthtest_op,
                                          VkBool32 depthboundstest_enable,
                                          f32 min_depthbounds,
                                          f32 max_depthbounds,
                                          VkBool32 stencil_enable,
                                          VkStencilOpState front,
                                          VkStencilOpState back){
    spec->depthstencil.sType =
        VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    
    spec->depthstencil.pNext = 0;
    spec->depthstencil.flags = 0;
    spec->depthstencil.depthTestEnable = depthtest_enable;
    spec->depthstencil.depthWriteEnable = depthwrite_enable;
    spec->depthstencil.depthCompareOp = depthtest_op;
    spec->depthstencil.depthBoundsTestEnable = depthboundstest_enable;
    spec->depthstencil.stencilTestEnable = stencil_enable;
    spec->depthstencil.front = front;
    spec->depthstencil.back = back;
    spec->depthstencil.minDepthBounds = min_depthbounds;
    spec->depthstencil.maxDepthBounds = max_depthbounds;
}

void VSetColorBlend(VGraphicsPipelineSpecObj* spec,
                    VkPipelineColorBlendAttachmentState* attachment_array,u32 attachment_count,
                    VkBool32 logicop_enable,VkLogicOp logic_op,
                    f32 blendconstants[4]){
    
    _kill("we do not support this many color attachments\n",
          attachment_count > _arraycount(spec->colorattachment_array));
    
    memcpy(&spec->colorattachment_array[0],&attachment_array[0],attachment_count * sizeof(VkPipelineColorBlendAttachmentState));
    
    spec->colorblendstate.sType =
        VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    
    spec->colorblendstate.pNext = 0;
    spec->colorblendstate.flags = 0;
    spec->colorblendstate.logicOpEnable = logicop_enable;
    spec->colorblendstate.logicOp = logic_op;
    spec->colorblendstate.attachmentCount = attachment_count;
    
}

void VEnableColorBlendTransparency(VGraphicsPipelineSpecObj* spec,
                                   u32 colorattachment_bitmask,
                                   VkBlendFactor srccolor_blendfactor,
                                   VkBlendFactor dstcolor_blendfactor,
                                   VkBlendOp colorblend_op,
                                   VkBlendFactor srcalpha_blendfactor,
                                   VkBlendFactor dst_alphablendfactor,
                                   VkBlendOp alphablend_op,
                                   VkColorComponentFlags colorWriteMask){
    
    auto count = spec->colorblendstate.attachmentCount;
    
    for(u32 i = 0; i < count; i++){
        
        if((1 << i) & colorattachment_bitmask){
            
            spec->colorattachment_array[i] = {
                VK_TRUE,srccolor_blendfactor,dstcolor_blendfactor,colorblend_op,
                srcalpha_blendfactor,
                dst_alphablendfactor,alphablend_op,colorWriteMask,
            };
        }
    }
}

void VEnableDynamicStateGraphicsPipelineSpec(VGraphicsPipelineSpecObj* spec,
                                             VkDynamicState* dynamic_array,u32 dynamic_count){
    
    _kill("max possible dynamic statues set\n",dynamic_count > 9);
    
    memcpy(&spec->dynamic_array[0],dynamic_array,sizeof(VkDynamicState) * dynamic_count);
    
    spec->dynamicstate.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    spec->dynamicstate.pNext = 0;
    spec->dynamicstate.flags = 0;
    spec->dynamicstate.dynamicStateCount = dynamic_count;
}

void VSetInputAssemblyState(VGraphicsPipelineSpecObj* spec,VkPrimitiveTopology topology,VkBool32 restart){
    
    spec->assembly = {
        VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        0,
        0,
        topology,
        restart//restart assembly of primitives for indexed draw if index(MAX_INT) is reached
    };
}

void VSetRasterState(VGraphicsPipelineSpecObj* spec,VkCullModeFlags cullmode,VkFrontFace frontface,VkBool32 enable_depthclamp,VkPolygonMode polymode,VkBool32 enable_depthbias,f32 depthbias_const,f32 depthbias_clamp,f32 depthbias_slope,f32 linewidth,VkBool32 enable_discard){
    
    spec->raster = {
        
        VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        0,
        0,
        enable_depthclamp,
        enable_discard,
        polymode,
        cullmode,
        frontface,
        enable_depthbias,
        depthbias_const,
        depthbias_clamp,
        depthbias_slope,
        linewidth
    };
}

void VPushBackShaderData(VShaderObj* obj,VkShaderStageFlagBits type,void* data,
                         u32 size,VkSpecializationInfo spec){
    
    _kill("max possible shaders reached\n",obj->shader_count >= 5);
    
    obj->shaderstage_array[obj->shader_count] = type;
    obj->shader_data_array[obj->shader_count] = data;
    obj->spv_size_array[obj->shader_count] = size;
    obj->spec_array[obj->shader_count] = spec;
    obj->shader_count++;
}


void VPushBackVertexAttrib(VShaderObj* obj,u32 binding_no,VkFormat format,u32 attrib_size){
    
    _kill("cannot push more vert attrib\n",obj->vert_attrib_count >= _arraycount(obj->vert_attrib_array));
    
    u32 offset = 0;
    
    for(u32 i = 0; i < obj->vert_attrib_count; i++){
        
        if(binding_no == obj->vert_attrib_array[i].binding){
            offset += obj->vert_attrib_size_array[i];
        }
    }
    
    obj->vert_attrib_array[obj->vert_attrib_count] = {obj->vert_attrib_count,binding_no,format,offset
    };
    
    obj->vert_attrib_size_array[obj->vert_attrib_count] = attrib_size;
    obj->vert_attrib_count++;
    
}

void VPushBackVertexDesc(VShaderObj* obj,u32 binding_no,u32 vert_size,VkVertexInputRate inputrate){
    
    _kill("cannot push more vert desc\n",obj->vert_desc_count >= _arraycount(obj->vert_desc_array));
    
    obj->vert_desc_array[obj->vert_desc_count] = {binding_no,vert_size,inputrate};
    obj->vert_desc_count++;
}

void VPushBackSetElement(VShaderObj::DescSetEntry* set,VkDescriptorType type,u32 bind,u32 array_count){
    
    _kill("max desc set element reached\n",set->element_count >= _arraycount(set->element_array));
    
    set->element_array[set->element_count] = {type,bind,array_count};
    set->element_count++;
    
}

VShaderObj::DescSetEntry* VGetSet(VShaderObj* obj,u32 set_no){
    
    for(u32 i = 0; i < obj->descset_count; i++){
        auto entry = &obj->descset_array[i];
        if(set_no == entry->set_no){
            return entry;
        }
    }
    
    return 0;
}

void VPushBackDescSet(VShaderObj* obj,u32 set_no,u32 shader_stage){
    
    VShaderObj::DescSetEntry* set = VGetSet(obj,set_no);
    
    if(!set){
        
        _kill("max desc set reached\n",obj->descset_count >= _arraycount(obj->descset_array));
        
        set = &obj->descset_array[obj->descset_count];
        obj->descset_count++;
        
        set->set_no = set_no;
    }
    
    set->shader_stage |= shader_stage;
}

void VPushBackPushConstRange(VShaderObj* _restrict obj,VkFormat* format_array,u32 format_count,u32 size,VkShaderStageFlagBits shader_stage){
    
    auto hash = VFormatHash(&format_array[0],format_count);
    
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
        range->size = size;
    }
    
    range->stageFlags |= shader_stage;
}

void VCreateGraphicsPipelineArray(const  VDeviceContext* _restrict vdevice,VGraphicsPipelineSpecObj* spec_array,u32 spec_count,VkPipeline* pipeline_array,VkPipelineCache cache){
    
    VkGraphicsPipelineCreateInfo info_array[16] = {};
    
    _kill("too many specs\n",spec_count > _arraycount(info_array));
    
    for(u32 i = 0; i < spec_count; i++){
        
        auto spec = &spec_array[i];
        
        //bind all the pointers
        
        //vertex descs and attribs
        spec->vertexinput.pVertexBindingDescriptions = &spec->vert_desc_array[0];
        
        spec->vertexinput.pVertexAttributeDescriptions = &spec->vert_attrib_array[0];
        
        //fixed viewport
        spec->viewport.pViewports = &spec->viewport_array[0];
        spec->viewport.pScissors = &spec->scissor_array[0];
        
        //color blend state
        spec->colorblendstate.pAttachments = &spec->colorattachment_array[0];
        
        
        VkPipelineTessellationStateCreateInfo* tessalationstate = 0;
        VkPipelineDynamicStateCreateInfo* dynamicstate = 0;
        
        if(spec->tessalationstate.sType){
            tessalationstate = &spec->tessalationstate;
        }
        
        if(spec->dynamicstate.sType){
            spec->dynamicstate.pDynamicStates = &spec->dynamic_array[0];
            dynamicstate= &spec->dynamicstate;
        }
        
        //shader info
        for(u32 j = 0; j < spec->shadermodule_count; j++){
            
            VkSpecializationInfo* spec_info = 0;
            
            if(spec->spec_array[i].mapEntryCount){
                spec_info = &spec->spec_array[j];
            }
            
            spec->shaderinfo_array[j].pSpecializationInfo = spec_info;
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
    
    _vktest(vkCreateGraphicsPipelines(vdevice->device,cache,spec_count,&info_array[0],global_allocator,
                                      pipeline_array));
    
    //destroy all modules
    for(u32 i = 0; i < spec_count; i++){
        
        auto spec = &spec_array[i];
        
        for(u32 j = 0; j < spec->shadermodule_count; j++){
            vkDestroyShaderModule(vdevice->device,spec->shadermodule_array[j],global_allocator);
        }
    }
    
}

VGraphicsPipelineSpecObj VMakeGraphicsPipelineSpecObj(const  VDeviceContext* vdevice,VShaderObj* obj,VkPipelineLayout layout,
                                                      VkRenderPass renderpass,u32 subpass_index,VSwapchainContext* swap,u32 colorattachment_count,VkPipelineCreateFlags flags,
                                                      VkPipeline parent_pipeline,s32 parentpipeline_index){
    
    VGraphicsPipelineSpecObj spec = {};
    
    memcpy(&spec.spec_array[0],&obj->spec_array[0],sizeof(VkSpecializationInfo) * obj->shader_count);
    
    
    spec.subpass_index = subpass_index;
    spec.flags = flags;
    spec.layout = layout;
    spec.renderpass = renderpass;
    spec.parent_pipeline = parent_pipeline;
    spec.parentpipeline_index = parentpipeline_index;
    
    memcpy(&spec.vert_desc_array[0],&obj->vert_desc_array[0],obj->vert_desc_count * sizeof(VkVertexInputBindingDescription));
    
    memcpy(&spec.vert_attrib_array[0],&obj->vert_attrib_array[0],obj->vert_attrib_count * sizeof(VkVertexInputAttributeDescription));
    
    spec.vertexinput = {
        
        VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        0,
        0,
        obj->vert_desc_count,
        0,
        obj->vert_attrib_count,
        0
    };
    
    VSetInputAssemblyState(&spec);
    
    VSetRasterState(&spec);
    
    if(swap){
        
        VSetFixedViewportGraphicsPipelineSpec(&spec,swap->width,swap->height);
    }
    
    VSetMultisampleGraphicsPipelineSpec(&spec);
    
    
    //MARK: this is disabled by default
    VSetDepthStencilGraphicsPipelineSpec(&spec);
    
    
    _kill("we do not support this many color attachments\n",
          colorattachment_count > _arraycount(spec.colorattachment_array));
    
    VkPipelineColorBlendAttachmentState attachment_array[16] = {};
    
    for(u32 i = 0; i < colorattachment_count; i++){
        
        attachment_array[i] = {
            VK_FALSE,
            VK_BLEND_FACTOR_SRC_ALPHA,
            VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
            VK_BLEND_OP_ADD,
            VK_BLEND_FACTOR_ONE,
            VK_BLEND_FACTOR_ZERO,
            VK_BLEND_OP_ADD,0xf
        };
    }
    
    VSetColorBlend(&spec,&attachment_array[0],colorattachment_count);
    
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
            "main"
        };
    }
    
    return spec;
}



VkPipelineLayout VCreatePipelineLayout(const  VDeviceContext* _restrict vdevice,
                                       VkDescriptorSetLayout* descriptorsetlayout_array,
                                       u32 descriptorsetlayout_count,VkPushConstantRange* pushconstrange_array,
                                       u32 pushconstrange_count){
    
    VkPipelineLayout layout;
    
    VkPipelineLayoutCreateInfo createinfo =
    {
        VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        0,
        0,
        descriptorsetlayout_count,
        descriptorsetlayout_array,
        pushconstrange_count,
        pushconstrange_array
    };
    
    vkCreatePipelineLayout(vdevice->device,&createinfo,global_allocator,&layout);
    
    return layout;
}


VkDescriptorPool VCreateDescriptorPoolX(VDeviceContext* _in_ vdevice,
                                        VDescriptorPoolSpec poolspec,u32 flags){
    
    return VCreateDescriptorPool(vdevice,poolspec,flags,poolspec.desc_count);
}

VkPipelineCache VCreatePipelineCache(const VDeviceContext* _in_ vdevice,void* init_data,ptrsize init_size){
    
    VkPipelineCacheCreateInfo info = {
        VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO,
        0,
        0,
        init_size,
        init_data
    };
    
    VkPipelineCache cache;
    
    _vktest(vkCreatePipelineCache(vdevice->device,&info,global_allocator,&cache));
    
    return cache;
}

void VGetPipelineCacheData(const VDeviceContext* _in_ vdevice,VkPipelineCache cache,void* init_data,ptrsize* init_size){
    
    _vktest(vkGetPipelineCacheData(vdevice->device,cache,(size_t*)init_size,init_data));
}