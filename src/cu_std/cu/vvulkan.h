#pragma once

#ifdef _WIN32

#define VK_USE_PLATFORM_WIN32_KHR

#else

#define VK_USE_PLATFORM_XLIB_KHR

#endif


#include "vulkan/vulkan.h"
#include "wwindow.h"
#include "aanimation.h"

#include "mode.h"
#include "ttype.h"
#include "ccolor.h"


extern void* vkenumerateinstanceextensionproperties;
extern void* vkenumerateinstancelayerproperties;
extern void* vkcreateinstance;
extern void* vkgetinstanceprocaddress;
extern void* vkgetdeviceprocaddress;

extern void* vkenumeratephysicaldevices;
extern void* vkgetphysicaldeviceproperties;
extern void* vkenumeratedevicelayerproperties;
extern void* vkenumeratedeviceextensionproperties;
extern void* vkgetphysicaldevicequeuefamilyproperties;
extern void* vkgetphysicaldevicefeatures;
extern void* vkcreatedevice;
extern void* vkgetphysicaldeviceformatproperties;
extern void* vkgetphysicaldevicememoryproperties;
extern void* vkcmdpipelinebarrier;
extern void* vkcreateshadermodule;
extern void* vkcreatebuffer;
extern void* vkgetbuffermemoryrequirements;
extern void* vkmapmemory;
extern void* vkunmapmemory;
extern void* vkflushmappedmemoryranges;
extern void* vkinvalidatemappedmemoryranges;
extern void* vkbindbuffermemory;
extern void* vkdestroybuffer;
extern void* vkallocatememory;
extern void* vkfreememory;
extern void* vkcreaterenderpass;
extern void* vkcmdbeginrenderpass;
extern void* vkcmdendrenderpass;
extern void* vkcmdnextsubpass;
extern void* vkcmdexecutecommands;
extern void* vkcreateimage;
extern void* vkgetimagememoryrequirements;
extern void* vkcreateimageview;
extern void* vkdestroyimageview;
extern void* vkbindimagememory;
extern void* vkgetimagesubresourcelayout;
extern void* vkcmdcopyimage;
extern void* vkcmdblitimage;
extern void* vkdestroyimage;
extern void* vkcmdclearattachments;
extern void* vkcmdcopybuffer;
extern void* vkcmdcopybuffertoimage;
extern void* vkcreatesampler;
extern void* vkdestroysampler;
extern void* vkcreatesemaphore;
extern void* vkdestroysemaphore;
extern void* vkcreatefence;
extern void* vkdestroyfence;
extern void* vkwaitforfences;
extern void* vkresetfences;
extern void* vkcreatecommandpool;
extern void* vkdestroycommandpool;
extern void* vkallocatecommandbuffers;
extern void* vkbegincommandbuffer;
extern void* vkendcommandbuffer;
extern void* vkgetdevicequeue;
extern void* vkqueuesubmit;
extern void* vkqueuewaitidle;
extern void* vkdevicewaitidle;
extern void* vkcreateframebuffer;
extern void* vkcreatepipelinecache;
extern void* vkcreatepipelinelayout;
extern void* vkcreategraphicspipelines;
extern void* vkcreatecomputepipelines;
extern void* vkcreatedescriptorpool;
extern void* vkcreatedescriptorsetlayout;
extern void* vkallocatedescriptorsets;
extern void* vkupdatedescriptorsets;
extern void* vkcmdbinddescriptorsets;
extern void* vkcmdbindpipeline;
extern void* vkcmdbindvertexbuffers;
extern void* vkcmdbindindexbuffer;
extern void* vkcmdsetviewport;
extern void* vkcmdsetscissor;
extern void* vkcmdsetlinewidth;
extern void* vkcmdsetdepthbias;
extern void* vkcmdpushconstants;
extern void* vkcmddrawindexed;
extern void* vkcmddraw;
extern void* vkcmddrawindexedindirect;
extern void* vkcmddrawindirect;
extern void* vkcmddispatch;
extern void* vkdestroypipeline;
extern void* vkdestroypipelinelayout;
extern void* vkdestroydescriptorsetlayout;
extern void* vkdestroydevice;
extern void* vkdestroyinstance;
extern void* vkdestroydescriptorpool;
extern void* vkfreecommandbuffers;
extern void* vkdestroyrenderpass;
extern void* vkdestroyframebuffer;
extern void* vkdestroyshadermodule;
extern void* vkdestroypipelinecache;
extern void* vkcreatequerypool;
extern void* vkdestroyquerypool;
extern void* vkgetquerypoolresults;
extern void* vkcmdbeginquery;
extern void* vkcmdendquery;
extern void* vkcmdresetquerypool;
extern void* vkcmdcopyquerypoolresults;
extern void* vkcreate_xlib_wayland_win32surfacekhr;
extern void* vkdestroysurfacekhr;
extern void* vkcmdfillbuffer;
extern void* vkacquirenextimagekhr;
extern void* vkgetfencestatus;
extern void* vkcreateswapchainkhr;
extern void* vkgetswapchainimageskhr;
extern void* vkqueuepresentkhr;
extern void* vkgetphysicaldevice_xlib_wayland_win32_presentationsupportkhr;
extern void* vkgetphysicaldevicesurfacesupportkhr;
extern void* vkcmdclearcolorimage;
extern void* vkgetphysicaldeviceimageformatproperties;
extern void* vkcmdcopyimagetobuffer;
extern void* vkgetpipelinecachedata;

//vulkan 1.1
extern void* vkenumeratephysicaldevicegroups;

//defines
#define vkEnumerateInstanceExtensionProperties ((PFN_vkEnumerateInstanceExtensionProperties)(vkenumerateinstanceextensionproperties))
#define vkEnumerateInstanceLayerProperties ((PFN_vkEnumerateInstanceLayerProperties)(vkenumerateinstancelayerproperties))
#define vkCreateInstance ((PFN_vkCreateInstance)(vkcreateinstance))
#define vkGetInstanceProcAddr ((PFN_vkGetInstanceProcAddr)(vkgetinstanceprocaddress))
#define vkGetDeviceProcAddr ((PFN_vkGetDeviceProcAddr)(vkgetdeviceprocaddress))

#define vkEnumeratePhysicalDevices ((PFN_vkEnumeratePhysicalDevices)(vkenumeratephysicaldevices))
#define vkGetPhysicalDeviceProperties ((PFN_vkGetPhysicalDeviceProperties)(vkgetphysicaldeviceproperties))
#define vkEnumerateDeviceLayerProperties ((PFN_vkEnumerateDeviceLayerProperties)(vkenumeratedevicelayerproperties))
#define vkEnumerateDeviceExtensionProperties ((PFN_vkEnumerateDeviceExtensionProperties)(vkenumeratedeviceextensionproperties))
#define vkGetPhysicalDeviceQueueFamilyProperties ((PFN_vkGetPhysicalDeviceQueueFamilyProperties)(vkgetphysicaldevicequeuefamilyproperties))
#define vkGetPhysicalDeviceFeatures ((PFN_vkGetPhysicalDeviceFeatures)(vkgetphysicaldevicefeatures))
#define vkCreateDevice ((PFN_vkCreateDevice)(vkcreatedevice))
#define vkGetPhysicalDeviceFormatProperties ((PFN_vkGetPhysicalDeviceFormatProperties)(vkgetphysicaldeviceformatproperties))
#define vkGetPhysicalDeviceMemoryProperties ((PFN_vkGetPhysicalDeviceMemoryProperties)(vkgetphysicaldevicememoryproperties))
#define vkCmdPipelineBarrier ((PFN_vkCmdPipelineBarrier)(vkcmdpipelinebarrier))
#define vkCreateShaderModule ((PFN_vkCreateShaderModule)(vkcreateshadermodule))
#define vkCreateBuffer ((PFN_vkCreateBuffer)(vkcreatebuffer))
#define vkGetBufferMemoryRequirements ((PFN_vkGetBufferMemoryRequirements)(vkgetbuffermemoryrequirements))
#define vkMapMemory ((PFN_vkMapMemory)(vkmapmemory))
#define vkUnmapMemory ((PFN_vkUnmapMemory)(vkunmapmemory))
#define vkFlushMappedMemoryRanges ((PFN_vkFlushMappedMemoryRanges)(vkflushmappedmemoryranges))
#define vkInvalidateMappedMemoryRanges ((PFN_vkInvalidateMappedMemoryRanges)(vkinvalidatemappedmemoryranges))
#define vkBindBufferMemory ((PFN_vkBindBufferMemory)(vkbindbuffermemory))
#define vkDestroyBuffer ((PFN_vkDestroyBuffer)(vkdestroybuffer))
#define vkAllocateMemory ((PFN_vkAllocateMemory)(vkallocatememory))
#define vkFreeMemory ((PFN_vkFreeMemory)(vkfreememory))
#define vkCreateRenderPass ((PFN_vkCreateRenderPass)(vkcreaterenderpass))
#define vkCmdBeginRenderPass ((PFN_vkCmdBeginRenderPass)(vkcmdbeginrenderpass))
#define vkCmdEndRenderPass ((PFN_vkCmdEndRenderPass)(vkcmdendrenderpass))
#define vkCmdNextSubpass ((PFN_vkCmdNextSubpass)(vkcmdnextsubpass))
#define vkCmdExecuteCommands ((PFN_vkCmdExecuteCommands)(vkcmdexecutecommands))
#define vkCreateImage ((PFN_vkCreateImage)(vkcreateimage))
#define vkGetImageMemoryRequirements ((PFN_vkGetImageMemoryRequirements)(vkgetimagememoryrequirements))
#define vkCreateImageView ((PFN_vkCreateImageView)(vkcreateimageview))
#define vkDestroyImageView ((PFN_vkDestroyImageView)(vkdestroyimageview))
#define vkBindImageMemory ((PFN_vkBindImageMemory)(vkbindimagememory))
#define vkGetImageSubresourceLayout ((PFN_vkGetImageSubresourceLayout)(vkgetimagesubresourcelayout))
#define vkCmdCopyImage ((PFN_vkCmdCopyImage)(vkcmdcopyimage))
#define vkCmdBlitImage ((PFN_vkCmdBlitImage)(vkcmdblitimage))
#define vkDestroyImage ((PFN_vkDestroyImage)(vkdestroyimage))
#define vkCmdClearAttachments ((PFN_vkCmdClearAttachments)(vkcmdclearattachments))
#define vkCmdCopyBuffer ((PFN_vkCmdCopyBuffer)(vkcmdcopybuffer))
#define vkCmdCopyBufferToImage ((PFN_vkCmdCopyBufferToImage)(vkcmdcopybuffertoimage))
#define vkCreateSampler ((PFN_vkCreateSampler)(vkcreatesampler))
#define vkDestroySampler ((PFN_vkDestroySampler)(vkdestroysampler))
#define vkCreateSemaphore ((PFN_vkCreateSemaphore)(vkcreatesemaphore))
#define vkDestroySemaphore ((PFN_vkDestroySemaphore)(vkdestroysemaphore))
#define vkCreateFence ((PFN_vkCreateFence)(vkcreatefence))
#define vkDestroyFence ((PFN_vkDestroyFence)(vkdestroyfence))
#define vkWaitForFences ((PFN_vkWaitForFences)(vkwaitforfences))
#define vkResetFences ((PFN_vkResetFences)(vkresetfences))
#define vkCreateCommandPool ((PFN_vkCreateCommandPool)(vkcreatecommandpool))
#define vkDestroyCommandPool ((PFN_vkDestroyCommandPool)(vkdestroycommandpool))
#define vkAllocateCommandBuffers ((PFN_vkAllocateCommandBuffers)(vkallocatecommandbuffers))
#define vkBeginCommandBuffer ((PFN_vkBeginCommandBuffer)(vkbegincommandbuffer))
#define vkEndCommandBuffer ((PFN_vkEndCommandBuffer)(vkendcommandbuffer))
#define vkGetDeviceQueue ((PFN_vkGetDeviceQueue)(vkgetdevicequeue))
#define vkQueueSubmit ((PFN_vkQueueSubmit)(vkqueuesubmit))
#define vkQueueWaitIdle ((PFN_vkQueueWaitIdle)(vkqueuewaitidle))
#define vkDeviceWaitIdle ((PFN_vkDeviceWaitIdle)(vkdevicewaitidle))
#define vkCreateFramebuffer ((PFN_vkCreateFramebuffer)(vkcreateframebuffer))
#define vkCreatePipelineCache ((PFN_vkCreatePipelineCache)(vkcreatepipelinecache))
#define vkCreatePipelineLayout ((PFN_vkCreatePipelineLayout)(vkcreatepipelinelayout))
#define vkCreateGraphicsPipelines ((PFN_vkCreateGraphicsPipelines)(vkcreategraphicspipelines))
#define vkCreateComputePipelines ((PFN_vkCreateComputePipelines)(vkcreatecomputepipelines))
#define vkCreateDescriptorPool ((PFN_vkCreateDescriptorPool)(vkcreatedescriptorpool))
#define vkCreateDescriptorSetLayout ((PFN_vkCreateDescriptorSetLayout)(vkcreatedescriptorsetlayout))
#define vkAllocateDescriptorSets ((PFN_vkAllocateDescriptorSets)(vkallocatedescriptorsets))
#define vkUpdateDescriptorSets ((PFN_vkUpdateDescriptorSets)(vkupdatedescriptorsets))
#define vkCmdBindDescriptorSets ((PFN_vkCmdBindDescriptorSets)(vkcmdbinddescriptorsets))
#define vkCmdBindPipeline ((PFN_vkCmdBindPipeline)(vkcmdbindpipeline))
#define vkCmdBindVertexBuffers ((PFN_vkCmdBindVertexBuffers)(vkcmdbindvertexbuffers))
#define vkCmdBindIndexBuffer ((PFN_vkCmdBindIndexBuffer)(vkcmdbindindexbuffer))
#define vkCmdSetViewport ((PFN_vkCmdSetViewport)(vkcmdsetviewport))
#define vkCmdSetScissor ((PFN_vkCmdSetScissor)(vkcmdsetscissor))
#define vkCmdSetLineWidth ((PFN_vkCmdSetLineWidth)(vkcmdsetlinewidth))
#define vkCmdSetDepthBias ((PFN_vkCmdSetDepthBias)(vkcmdsetdepthbias))
#define vkCmdPushConstants ((PFN_vkCmdPushConstants)(vkcmdpushconstants))
#define vkCmdDrawIndexed ((PFN_vkCmdDrawIndexed)(vkcmddrawindexed))
#define vkCmdDraw ((PFN_vkCmdDraw)(vkcmddraw))
#define vkCmdDrawIndexedIndirect ((PFN_vkCmdDrawIndexedIndirect)(vkcmddrawindexedindirect))
#define vkCmdDrawIndirect ((PFN_vkCmdDrawIndirect)(vkcmddrawindirect))
#define vkCmdDispatch ((PFN_vkCmdDispatch)(vkcmddispatch))
#define vkDestroyPipeline ((PFN_vkDestroyPipeline)(vkdestroypipeline))
#define vkDestroyPipelineLayout ((PFN_vkDestroyPipelineLayout)(vkdestroypipelinelayout))
#define vkDestroyDescriptorSetLayout ((PFN_vkDestroyDescriptorSetLayout)(vkdestroydescriptorsetlayout))
#define vkDestroyDevice ((PFN_vkDestroyDevice)(vkdestroydevice))
#define vkDestroyInstance ((PFN_vkDestroyInstance)(vkdestroyinstance))
#define vkDestroyDescriptorPool ((PFN_vkDestroyDescriptorPool)(vkdestroydescriptorpool))
#define vkFreeCommandBuffers ((PFN_vkFreeCommandBuffers)(vkfreecommandbuffers))
#define vkDestroyRenderPass ((PFN_vkDestroyRenderPass)(vkdestroyrenderpass))
#define vkDestroyFramebuffer ((PFN_vkDestroyFramebuffer)(vkdestroyframebuffer))
#define vkDestroyShaderModule ((PFN_vkDestroyShaderModule)(vkdestroyshadermodule))
#define vkDestroyPipelineCache ((PFN_vkDestroyPipelineCache)(vkdestroypipelinecache))
#define vkCreateQueryPool ((PFN_vkCreateQueryPool)(vkcreatequerypool))
#define vkDestroyQueryPool ((PFN_vkDestroyQueryPool)(vkdestroyquerypool))
#define vkGetQueryPoolResults ((PFN_vkGetQueryPoolResults)(vkgetquerypoolresults))
#define vkCmdBeginQuery ((PFN_vkCmdBeginQuery)(vkcmdbeginquery))
#define vkCmdEndQuery ((PFN_vkCmdEndQuery)(vkcmdendquery))
#define vkCmdResetQueryPool ((PFN_vkCmdResetQueryPool)(vkcmdresetquerypool))
#define vkCmdCopyQueryPoolResults ((PFN_vkCmdCopyQueryPoolResults)(vkcmdcopyquerypoolresults))
#define vkCreateXlibSurfaceKHR ((PFN_vkCreateXlibSurfaceKHR)(vkcreate_xlib_wayland_win32surfacekhr))
#define vkCreateWaylandSurfaceKHR ((PFN_vkCreateWaylandSurfaceKHR)(vkcreate_xlib_wayland_win32surfacekhr))


#define vkCreateWin32SurfaceKHR ((PFN_vkCreateWin32SurfaceKHR)(vkcreate_xlib_wayland_win32surfacekhr))


#define vkDestroySurfaceKHR ((PFN_vkDestroySurfaceKHR)(vkdestroysurfacekhr))

#define vkCmdFillBuffer ((PFN_vkCmdFillBuffer)vkcmdfillbuffer)
#define vkAcquireNextImageKHR ((PFN_vkAcquireNextImageKHR)vkacquirenextimagekhr)
#define vkGetFenceStatus ((PFN_vkGetFenceStatus)vkgetfencestatus)
#define vkCreateSwapchainKHR ((PFN_vkCreateSwapchainKHR)vkcreateswapchainkhr)
#define vkGetSwapchainImagesKHR ((PFN_vkGetSwapchainImagesKHR)vkgetswapchainimageskhr)
#define vkQueuePresentKHR ((PFN_vkQueuePresentKHR)vkqueuepresentkhr)

#define vkGetPhysicalDeviceXlibPresentationSupportKHR ((PFN_vkGetPhysicalDeviceXlibPresentationSupportKHR)vkgetphysicaldevice_xlib_wayland_win32_presentationsupportkhr)

#define vkGetPhysicalDeviceWaylandPresentationSupportKHR ((PFN_vkGetPhysicalDeviceWaylandPresentationSupportKHR)vkgetphysicaldevice_xlib_wayland_win32_presentationsupportkhr)

#define vkGetPhysicalDeviceWin32PresentationSupportKHR ((PFN_vkGetPhysicalDeviceWin32PresentationSupportKHR)vkgetphysicaldevice_xlib_wayland_win32_presentationsupportkhr)

#define vkGetPhysicalDeviceSurfaceSupportKHR ((PFN_vkGetPhysicalDeviceSurfaceSupportKHR)vkgetphysicaldevicesurfacesupportkhr)

#define vkCmdClearColorImage ((PFN_vkCmdClearColorImage)vkcmdclearcolorimage)
#define vkGetPhysicalDeviceImageFormatProperties ((PFN_vkGetPhysicalDeviceImageFormatProperties)vkgetphysicaldeviceimageformatproperties)

#define vkCmdCopyImageToBuffer ((PFN_vkCmdCopyImageToBuffer)vkcmdcopyimagetobuffer)

#define vkGetPipelineCacheData ((PFN_vkGetPipelineCacheData)vkgetpipelinecachedata)

//vulkan 1.1
#define vkEnumeratePhysicalDeviceGroups ((PFN_vkEnumeratePhysicalDeviceGroups)vkenumeratephysicaldevicegroups)


#if _debug
#define _vk_inject_cmdbuffers 1
#else
#define _vk_inject_cmdbuffers 0
#endif

#if 0

#define _vthreaddump(string, ...) _dprint(string, __VA_ARGS__)

#else

#define _vthreaddump(string, ...)

#endif


/* 

   NOTE: This will be designed based on the metal api
   
*/


struct VSwapchainContext{
    
    struct DepthStencil{
        VkImage image;
        VkImageView view;
        VkFormat format;
        VkDeviceMemory memory;
    };
    
    
    struct PresentImageResource{
        VkImage image;
        VkImageView view;
        VkFramebuffer framebuffer;
    };
    
    struct InternalData{
        DepthStencil depthstencil;
        VkSurfaceKHR surface;
        VkFormat format;
    };
    
    u16 image_index;
    u16 image_count;
    VkSwapchainKHR swap;
    
    
    InternalData* internal;
    
    u16 width;
    u16 height;
    PresentImageResource* presentresource_array;
};


struct VBufferContext{
    VkBuffer buffer;
    VkDeviceMemory memory;
    u32 size;
    
    //unique attrib according to buffer type. binding no on vertex buffer/instance,
    //count on index buffer
    u32 attrib;
    
#if (_debug && 0)
    u32 max_inst = 0; // for instance buffer checking only
#endif
};

//TODO: fold all the texture/image functions

struct VImageMemoryContext{
    VkImage image;
    VkDeviceMemory memory;
};

struct VImageContext : VImageMemoryContext{
    VkImageView view;
};

struct VTextureContext : VImageContext{
    VkSampler sampler;
};

#define VCREATEQUEUEBIT_ALL (u32)-1
#define VCREATEQUEUEBIT_ROOT 1
#define VCREATEQUEUEBIT_COMPUTE 2
#define VCREATEQUEUEBIT_TRANSFER 4

enum VQueueType{
    VQUEUETYPE_ROOT = 0,
    VQUEUETYPE_COMPUTE = 1,
    VQUEUETYPE_TRANSFER = 2
};


struct VDeviceContext{
    VkPhysicalDevice physicaldevice;
    VkDevice device;
    VkPhysicalDeviceMemoryProperties* memoryproperties;
};

struct VDeviceGroupContext{
    
    struct VPhysicalDeviceGroup{
        
        VkPhysicalDevice physicaldevice_array[4];
        //MARK: these devices will be similar. we should use the minimum set
        VkPhysicalDeviceMemoryProperties* memoryproperties[4];
        u32 physicaldevice_count;
    };
    
    VPhysicalDeviceGroup devicegroup;
    VkDevice device;
};

struct VModel{
    VBufferContext vertexbuffercontext;
    VBufferContext indexbuffercontext;
};

//can contain many meshes with animations
struct VSkeletalModel{
    VBufferContext vertexbuffercontext;
    VBufferContext indexbuffercontext;
    
    ALinearBone* rootbone;
    AAnimationSet* animationset_array;
    
    u16 bone_count;
    u16 animationset_count;
};

struct VDescriptorPoolSpec{
    VkDescriptorPoolSize container[16];
    u32 count = 0;
    u32 desc_count = 0;
};

struct VDescriptorBindingSpec{
    VkDescriptorSetLayoutBinding container[20];
    u32 count = 0;
};

struct VDescriptorWriteSpec{
    VkWriteDescriptorSet container[20];
    u32 count = 0;
};

struct VDescriptorCopySpec{
    VkCopyDescriptorSet container[20];
    u32 count = 0;
};

struct VAttachmentSpec{
    VkAttachmentDescription des_container[10];
    VkAttachmentReference ref_container[10];
    u32 count = 0;
};

struct VSubpassDescriptionSpec{
    VkSubpassDescription container[10];
    u32 count = 0;
};

struct VSubpassDependencySpec{
    VkSubpassDependency container[20];
    u32 count = 0;
};

struct VSubmitBatch{
    VkSubmitInfo container[10];
    u32 count;
};





void VDescPushBackPoolSpec(VDescriptorPoolSpec* poolspec,VkDescriptorType type,u32 count);

void VDescPushBackBindingSpec(VDescriptorBindingSpec* bindingspec,
                              VkDescriptorType type,u32 count,VkShaderStageFlags stage_flags,
                              VkSampler* immutable_samplers = 0);

void VDescPushBackWriteSpecImage(VDescriptorWriteSpec* spec,VkDescriptorSet dst_set,
                                 u32 dst_binding,u32 dst_startindex,u32 count,VkDescriptorType type,
                                 VkDescriptorImageInfo* imageinfo_array);

void VDescPushBackWriteSpecBuffer(VDescriptorWriteSpec* spec,VkDescriptorSet dst_set,
                                  u32 dst_binding,u32 dst_startindex,u32 count,VkDescriptorType type,
                                  VkDescriptorBufferInfo* bufferinfo_array);

void VDescPushBackWriteSpecView(VDescriptorWriteSpec* spec,VkDescriptorSet dst_set,
                                u32 dst_binding,u32 dst_startindex,u32 count,VkDescriptorType type,
                                VkBufferView* bufferview_array);

void VPushBackAttachmentSpec(VAttachmentSpec* spec,
                             VkAttachmentDescriptionFlags flags,VkFormat format,
                             VkSampleCountFlagBits samples,VkAttachmentLoadOp loadop,
                             VkAttachmentStoreOp storeop,VkAttachmentLoadOp stencil_loadop,
                             VkAttachmentStoreOp stencil_storeop,VkImageLayout initial,VkImageLayout final);

void VPushBackSubpassDescriptionSpec(VSubpassDescriptionSpec* spec,
                                     VkSubpassDescriptionFlags flags,VkPipelineBindPoint bindpoint,
                                     u32 inputattachment_count,
                                     const VkAttachmentReference* inputattachment_array,
                                     u32 colorattachment_count,
                                     const VkAttachmentReference* colorattachment_array,
                                     const VkAttachmentReference* resolveattachment_array,
                                     const VkAttachmentReference* depthstencilattachment_array,
                                     u32 preserveattachment_count,
                                     const u32* preserveattachment_array);

void VPushBackSubpassDependencySpec(VSubpassDependencySpec* spec,
                                    u32 srcsubpass_index,u32 dstsubpass_index,
                                    VkPipelineStageFlags src_stagemask,
                                    VkPipelineStageFlags dst_stagemask,
                                    VkAccessFlags src_accessmask,VkAccessFlags dst_accessmask,
                                    VkDependencyFlags dependencyflags);

enum V_Instance_Flags{
    V_INSTANCE_FLAGS_NONE = 0,
    V_INSTANCE_FLAGS_SINGLE_VKDEVICE = 1,
    V_INSTANCE_FLAGS_API_VERSION_OPTIONAL = 2,
};

u32 VCreateInstance(const s8* applicationname_string,logic validation_enable,u32 api_version,u32 v_inst_flags = V_INSTANCE_FLAGS_NONE);

VDeviceContext VCreateDeviceContext(WWindowContext* window = 0,
                                    u32 createqueue_bits = VCREATEQUEUEBIT_ALL,
                                    u32 physicaldevice_index = 0);

VkQueue VGetQueue(const VDeviceContext* _in_ vdevice,VQueueType type);

u32  VGetQueueFamilyIndex(VQueueType type);

enum VPresentSyncType{
    VSYNC_NONE = VK_PRESENT_MODE_IMMEDIATE_KHR,
    VSYNC_NORMAL = VK_PRESENT_MODE_FIFO_KHR,
    VSYNC_LAZY = VK_PRESENT_MODE_FIFO_RELAXED_KHR,
    VSYNC_FAST = VK_PRESENT_MODE_MAILBOX_KHR,
};

VSwapchainContext VCreateSwapchainContext(const VDeviceContext* _in_ vdevice,
                                          u32 swapcount,
                                          WWindowContext windowcontext,
                                          VPresentSyncType sync_type = VSYNC_NONE,
                                          VSwapchainContext* oldswapchain = 0);

struct VPhysicalDevice_Index{
    VkPhysicalDevice physicaldevice;
    u32 index;
};

struct VPhysicalDeviceGroups{};

void VEnumeratedPhysicalDevices(VPhysicalDevice_Index* array,u32* count,WWindowContext* window = 0);

void VEnumeratePhysicalDeviceGroups(VPhysicalDeviceGroups* array,u32* count,WWindowContext* window = 0);

VkFence VCreateFence(VDeviceContext* _in_ vdevice,VkFenceCreateFlags flags);

VkDescriptorPool VCreateDescriptorPool(VDeviceContext* _in_ vdevice,
                                       VDescriptorPoolSpec poolspec,u32 flags,u32 max_sets);

VkDescriptorSetLayout VCreateDescriptorSetLayout(
const  VDeviceContext* _restrict vdevice,
VDescriptorBindingSpec bindingspec);

void VAllocDescriptorSetArray(const  VDeviceContext* _restrict vdevice,
                              VkDescriptorPool pool,u32 count,VkDescriptorSetLayout* layout_array,
                              VkDescriptorSet* set_array);

void _ainline VUpdateDescriptorSets(const  VDeviceContext* _restrict vdevice,
                                    VDescriptorWriteSpec writespec,VDescriptorCopySpec copyspec = {}){
    
    vkUpdateDescriptorSets(vdevice->device,writespec.count,writespec.container,
                           copyspec.count,copyspec.container);
}

VkRenderPass VCreateRenderPass(const  VDeviceContext* _restrict vdevice,
                               VkRenderPassCreateFlags flags,VAttachmentSpec attachmentspec,
                               VSubpassDescriptionSpec subpassdescspec,
                               VSubpassDependencySpec subpassdepspec);


VkPipelineLayout VCreatePipelineLayout(const  VDeviceContext* _restrict vdevice,
                                       VkDescriptorSetLayout* descriptorsetlayout_array,
                                       u32 descriptorsetlayout_count,
                                       VkPushConstantRange* pushconstrange_array,
                                       u32 pushconstrange_count);

void VDestroyPipeline(const  VDeviceContext* _restrict vdevice,VkPipeline pipeline);

void VSubmitCommandBuffer(VkQueue queue,VkCommandBuffer commandbuffer,
                          VkSemaphore* wait_semaphore = 0,u32 wait_count = 0,
                          VkSemaphore* signal_semaphore = 0,u32 signal_count = 0,
                          VkPipelineStageFlags* wait_dstmask = 0,VkFence fence = 0);

void VSubmitCommandBuffer(VkQueue queue,VkCommandBuffer commandbuffer,
                          VkSemaphore wait_semaphore,
                          VkSemaphore signal_semaphore,VkPipelineStageFlags wait_dstmask,
                          VkFence fence = 0);

void VSubmitCommandBufferArray(VkQueue queue,VkCommandBuffer* commandbuffer,
                               u32 buffer_count,VkSemaphore* wait_semaphore = 0,u32 wait_count = 0,
                               VkSemaphore* signal_semaphore = 0,u32 signal_count = 0,
                               VkPipelineStageFlags* wait_dstmask = 0,VkFence fence = 0);

void VSubmitCommandBufferArray(VkQueue queue,VkCommandBuffer* commandbuffer,
                               u32 buffer_count,VkSemaphore wait_semaphore,
                               VkSemaphore signal_semaphore,VkPipelineStageFlags wait_dstmask,
                               VkFence fence);

void VSubmitCommandBufferBatch(VkQueue queue,VSubmitBatch batch,VkFence fence);


VTextureContext VCreateTextureImage(const  VDeviceContext* _restrict vdevice,void* data,
                                    u32 width,u32 height,VkCommandBuffer commandbuffer,VkQueue queue);

VTextureContext VCreateTextureImage(const  VDeviceContext* _restrict vdevice,
                                    const s8* filepath,VkCommandBuffer commandbuffer,VkQueue queue);

VImageContext VCreateColorImage(const  VDeviceContext* _restrict vdevice,
                                u32 width,u32 height,u32 usage,logic is_device_local = true,
                                logic is_coherent = false,VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL,
                                VkFormat format = VK_FORMAT_R8G8B8A8_UNORM);

VImageMemoryContext VCreateColorImageMemory(const  VDeviceContext* _restrict vdevice,
                                            u32 width,u32 height,u32 usage,logic is_device_local = true,
                                            logic is_coherent = false,
                                            VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL,
                                            VkFormat format = VK_FORMAT_R8G8B8A8_UNORM);

void VQueuePresentArray(VkQueue queue,u32* imageindex_array,
                        VkSwapchainKHR* swapchain_array,
                        ptrsize swapchain_count,
                        VkSemaphore* _restrict waitsemaphore_array,
                        ptrsize waitsemaphore_count,
                        VkResult* result_array);

void VQueuePresent(VkQueue queue,u32 image_index,VkSwapchainKHR swapchain,
                   VkSemaphore waitsemaphore);

void inline VDrawIndex(VBufferContext vertex_buffer,VBufferContext index_buffer,
                       VkCommandBuffer commandbuffer,VkDeviceSize offset){
    
    vkCmdBindVertexBuffers(commandbuffer,vertex_buffer.attrib,1,
                           &vertex_buffer.buffer,
                           &offset);
    
    vkCmdBindIndexBuffer(commandbuffer,index_buffer.buffer,
                         0,VK_INDEX_TYPE_UINT32);
    
    vkCmdDrawIndexed(commandbuffer,index_buffer.attrib,1,0,0,0);
    
}

void inline VDrawIndexInstanced(VBufferContext vertex_buffer,VBufferContext index_buffer,
                                VBufferContext instance_buffer,u32 instance_count,
                                VkCommandBuffer commandbuffer,VkDeviceSize offset){
    
    vkCmdBindVertexBuffers(commandbuffer,vertex_buffer.attrib,1,
                           &vertex_buffer.buffer,
                           &offset);
    
    vkCmdBindVertexBuffers(commandbuffer,instance_buffer.attrib,1,
                           &instance_buffer.buffer,
                           &offset);
    
    vkCmdBindIndexBuffer(commandbuffer,index_buffer.buffer,
                         0,VK_INDEX_TYPE_UINT32);
    
    vkCmdDrawIndexed(commandbuffer,index_buffer.attrib,instance_count,0,0,0);
    
}


VkFramebuffer VCreateFrameBuffer(const  VDeviceContext* _restrict vdevice,
                                 VkFramebufferCreateFlags flags,VkRenderPass renderpass,
                                 VkImageView* attachment_array,u32 attachment_count,u32 width,u32 height,
                                 u32 layers);

void VStartCommandBuffer(VkCommandBuffer cmdbuffer,
                         VkCommandBufferUsageFlags flags = 0);

void VStartCommandBuffer(VkCommandBuffer cmdbuffer,
                         VkCommandBufferUsageFlags flags,
                         VkRenderPass renderpass,u32 subpass,
                         VkFramebuffer framebuffer,
                         VkBool32 occlusion_enable,
                         VkQueryControlFlags queryflags,
                         VkQueryPipelineStatisticFlags querypipelineflags);

void VEndCommandBuffer(VkCommandBuffer cmdbuffer);

void VAllocateCommandBufferArray(const  VDeviceContext* _restrict vdevice,VkCommandPool pool,
                                 VkCommandBufferLevel level,
                                 VkCommandBuffer* _restrict commandbuffer_array,
                                 u32 commandbuffer_count);

VkCommandBuffer VAllocateCommandBuffer(const  VDeviceContext* _restrict vdevice,
                                       VkCommandPool pool,VkCommandBufferLevel level);

VkCommandPool VCreateCommandPool(const  VDeviceContext* _restrict vdevice,
                                 VkCommandPoolCreateFlags flags,u32 familyindex);

void VStartRenderpass(VkCommandBuffer commandbuffer,VkSubpassContents contents,
                      VkRenderPass renderpass,VkFramebuffer framebuffer,VkRect2D renderarea,
                      VkClearValue* clearvalue_array,u32 clearvalue_count);

void VStartRenderpass(VkCommandBuffer commandbuffer,VkSubpassContents contents,
                      VkRenderPass renderpass,VkFramebuffer framebuffer,VkRect2D renderarea,
                      VkClearValue clearvalue);

void VEndRenderPass(VkCommandBuffer commandbuffer);

VkSemaphore VCreateSemaphore(const  VDeviceContext* _restrict vdevice);

VBufferContext VCreateUniformBufferContext(const  VDeviceContext* _restrict vdevice,
                                           u32 data_size,logic is_coherrent = true);

void VUpdateUniformBuffer(const  VDeviceContext* _restrict vdevice,
                          VBufferContext context,void* data,u32 data_size);

void VSetDriverAllocator(VkAllocationCallbacks allocator);
void VSetDeviceAllocator(VkDeviceMemory (*allocator)(VkDevice,VkDeviceSize,u32,
                                                     VkAllocationCallbacks*));

#define V_AMD_DEVICE_HOST_VISIBLE (VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)

VBufferContext VCreateTransferBuffer(const  VDeviceContext* _restrict vdevice,
                                     ptrsize data_size,u32 add_flags =
                                     VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);


struct _cachealign CacheAlignedCommandbuffer{
    VkCommandBuffer cmdbuffer;
};

struct VThreadCommandbufferList{
    CacheAlignedCommandbuffer* container;
    volatile u32 count;
};

void _ainline VPushThreadCommandbufferList(VThreadCommandbufferList* list,
                                           VkCommandBuffer cmdbuffer){
    
    u32 index;
    u32 actual_index;
    
    do{
        
        index = list->count;
        
        actual_index = LockedCmpXchg(&list->count,index,index + 1);
        
    }while(actual_index != index);
    
    //FIXME: we sometimes get null cmdbuffers
    _kill("submitted null cmdbuffer\n",!cmdbuffer);
    
    list->container[index].cmdbuffer = cmdbuffer;
    
    _vthreaddump("submit %p index %d\n",(void*)cmdbuffer,index);
    
    
}



void VDestroyBuffer(const  VDeviceContext* _restrict vdevice,VkBuffer buffer);

void VDestroyBufferContext(const  VDeviceContext* _restrict vdevice,
                           VBufferContext buffer);

void VFreeMemory(const  VDeviceContext* _restrict vdevice,VkDeviceMemory memory);


VkDeviceMemory  
VRawDeviceAlloc(VkDevice device,VkDeviceSize alloc_size,u32 memorytype_index);

u32 VGetMemoryTypeIndex(VkPhysicalDeviceMemoryProperties properties,
                        u32 typebits,u32 flags);

VkShaderModule VCreateShaderModule(VkDevice device,void* data,
                                   ptrsize size,
                                   VkShaderModuleCreateFlags flags = 0);

enum VFilter{
    VFILTER_NONE,
    VFILTER_BILINEAR,
    VFILTER_TRILINEAR,
    VFILTER_ANISO,
};

VkSampler VCreateSampler(u32 filtering);

VTextureContext VCreateTextureCache(const  VDeviceContext* _restrict vdevice,u32 width,
                                    u32 height,VkFormat format);

void VUpdatePages();


VTextureContext VCreateTexturePageTable(const  VDeviceContext* _restrict vdevice,
                                        u32 width,u32 height,u32 miplevels);


struct VComputePipelineSpec{
    
    void* shader_data;
    u32 shader_size;
    VkSpecializationInfo shader_specialization;
    
    VkPipelineLayout layout;
    VkPipelineCreateFlags flags;//controls if pipeline has parent for now
    VkPipeline parent_pipeline;
    s32 parentpipeline_index;
};



void VSetComputePipelineSpecShader(VComputePipelineSpec* spec,void* shader_data,
                                   u32 shader_size,VkSpecializationInfo specialization = {});

void VGenerateComputePipelineSpec(VComputePipelineSpec* spec,VkPipelineLayout layout,
                                  VkPipelineCreateFlags flags = 0,VkPipeline parent_pipeline = 0,
                                  s32 parentpipeline_index = -1);

void VCreateComputePipelineArray(const  VDeviceContext* _restrict vdevice,
                                 VkPipelineCache cache,VComputePipelineSpec* spec_array,u32 spec_count,
                                 VkPipeline* pipeline_array);


VBufferContext VCreateShaderStorageBufferContext(
const  VDeviceContext* _restrict vdevice,
u32 data_size,logic is_devicelocal,logic is_coherrent = true);

VkDescriptorBufferInfo _ainline VGetBufferInfo(const VBufferContext* buffer,
                                               VkDeviceSize offset = 0,
                                               VkDeviceSize range = VK_WHOLE_SIZE){
    
    _kill("Offset too large\n",(offset + range) > buffer->size);
    
    return {buffer->buffer,offset,range};
}


VkBuffer VRawCreateBuffer(const  VDeviceContext* _restrict vdevice,
                          VkBufferCreateFlags flags,
                          VkDeviceSize size,VkBufferUsageFlags usage,
                          VkSharingMode sharingmode = VK_SHARING_MODE_EXCLUSIVE,
                          u32* queuefamilyindex_array = 0 ,
                          ptrsize queuefamilyindex_count = 0);


VBufferContext VCreateStaticVertexBuffer(const  VDeviceContext* _restrict vdevice,
                                         VkCommandBuffer commandbuffer,
                                         VkDeviceMemory memory,
                                         VkDeviceSize offset,
                                         VBufferContext src,VkDeviceSize src_offset,void* data,
                                         ptrsize data_size,u32 bindingno);

VBufferContext VCreateStaticIndexBuffer(const  VDeviceContext* _restrict vdevice,
                                        VkCommandBuffer commandbuffer,
                                        VkDeviceMemory memory,
                                        VkDeviceSize offset,
                                        VBufferContext src,VkDeviceSize src_offset,void* data,
                                        ptrsize data_size);

VBufferContext VCreateStaticVertexBuffer(const  VDeviceContext* _restrict vdevice,
                                         ptrsize data_size,u32 bindingno,logic isdevice_local = true);

VBufferContext VCreateStaticIndexBuffer(const  VDeviceContext* _restrict vdevice,
                                        ptrsize data_size,logic isdevice_local = true);


u32 _ainline VFormatHash(VkFormat* format_array,u32 count){
    
    u32 hash = 0;
    
    for(u32 i = 0; i < count; i++){
        hash += ((format_array[i] * (i + 1)) ^ hash) * 31;
    }
    
    return hash;
}





//

struct VShaderObj{
    
#if _debug
    u64 vert_hash;
#endif
    
    u8 vert_desc_count = 0;
    u8 vert_attrib_count = 0;
    u8 descset_count;
    u8 range_count = 0;
    u32 shader_count = 0;
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

struct VGraphicsPipelineSpecObj{
    
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

u32 VGetDescriptorSetLayoutHash(VShaderObj* obj,u32 descset_no);

VGraphicsPipelineSpecObj VMakeGraphicsPipelineSpecObj(const  VDeviceContext* vdevice,VShaderObj* obj,VkPipelineLayout layout,VkRenderPass renderpass,u32 subpass_index = 0,VSwapchainContext* swap = 0,u32 colorattachment_count = 1,VkPipelineCreateFlags flags = 0,
                                                      VkPipeline parent_pipeline = 0,s32 parentpipeline_index = -1);

void VCreateGraphicsPipelineArray(const  VDeviceContext* _restrict vdevice,VGraphicsPipelineSpecObj* spec_array,u32 spec_count,VkPipeline* pipeline_array,VkPipelineCache cache = 0);



void VSetFixedViewportGraphicsPipelineSpec(VGraphicsPipelineSpecObj* spec,
                                           VkViewport* viewport,u32 viewport_count,VkRect2D* scissor,
                                           u32 scissor_count);

void VSetFixedViewportGraphicsPipelineSpec(VGraphicsPipelineSpecObj* spec,
                                           u16 width,u16 height);

void VSetMultisampleGraphicsPipelineSpec(VGraphicsPipelineSpecObj* spec,
                                         VkSampleCountFlagBits samplecount_bits = VK_SAMPLE_COUNT_1_BIT,
                                         VkBool32 is_persample_perfragment = VK_FALSE,//true = sample,else frag
                                         f32 minsampleshading = 1.0f,
                                         VkSampleMask* samplemask = 0,
                                         VkBool32 enable_alpha_to_coverage = VK_FALSE,
                                         VkBool32 enable_alpha_to_one = VK_FALSE);


void VSetDepthStencilGraphicsPipelineSpec(VGraphicsPipelineSpecObj* spec,
                                          VkBool32 depthtest_enable = VK_FALSE,
                                          VkBool32 depthwrite_enable = VK_FALSE,VkCompareOp depthtest_op = VK_COMPARE_OP_NEVER,
                                          VkBool32 depthboundstest_enable = VK_FALSE,
                                          f32 min_depthbounds = 0.0f,
                                          f32 max_depthbounds = 1.0f,
                                          VkBool32 stencil_enable = false,
                                          VkStencilOpState front = {},
                                          VkStencilOpState back = {});

void VSetColorBlend(VGraphicsPipelineSpecObj* spec,
                    VkPipelineColorBlendAttachmentState* attachment_array,u32 attachment_count,
                    VkBool32 logicop_enable = VK_FALSE,VkLogicOp logic_op = VK_LOGIC_OP_CLEAR,
                    f32 blendconstants[4] = {});

void VEnableColorBlendTransparency(VGraphicsPipelineSpecObj* spec,
                                   u32 colorattachment_bitmask = 1,
                                   VkBlendFactor srccolor_blendfactor = VK_BLEND_FACTOR_SRC_ALPHA,
                                   VkBlendFactor dstcolor_blendfactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
                                   VkBlendOp colorblend_op = VK_BLEND_OP_ADD,
                                   VkBlendFactor srcalpha_blendfactor = VK_BLEND_FACTOR_ONE,
                                   VkBlendFactor dst_alphablendfactor = VK_BLEND_FACTOR_ZERO,
                                   VkBlendOp alphablend_op = VK_BLEND_OP_ADD,
                                   VkColorComponentFlags colorWriteMask = 0xf);

void VEnableDynamicStateGraphicsPipelineSpec(VGraphicsPipelineSpecObj* spec,
                                             VkDynamicState* dynamic_array,u32 dynamic_count);


void VPushBackShaderData(VShaderObj* obj,VkShaderStageFlagBits type,void* data,
                         u32 size,VkSpecializationInfo spec = {});



void VSetInputAssemblyState(VGraphicsPipelineSpecObj* spec,VkPrimitiveTopology topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,VkBool32 restart = false);

void VSetRasterState(VGraphicsPipelineSpecObj* spec,VkCullModeFlags cullmode = VK_CULL_MODE_BACK_BIT,VkFrontFace frontface = VK_FRONT_FACE_CLOCKWISE,VkBool32 enable_depthclamp = false,VkPolygonMode polymode = VK_POLYGON_MODE_FILL,VkBool32 enable_depthbias = false,f32 depthbias_const = 0.0f,f32 depthbias_clamp = 0.0f,f32 depthbias_slope = 0.0f,f32 linewidth = 1.0f,VkBool32 enable_discard = false);

void VPushBackVertexAttrib(VShaderObj* obj,u32 binding_no,VkFormat format,u32 attrib_size);

void VPushBackVertexDesc(VShaderObj* obj,u32 binding_no,u32 vert_size,VkVertexInputRate inputrate);

void VPushBackSetElement(VShaderObj::DescSetEntry* set,VkDescriptorType type,u32 bind,u32 array_count);

VShaderObj::DescSetEntry* VGetSet(VShaderObj* obj,u32 set_no);

void VPushBackDescSet(VShaderObj* obj,u32 set_no,u32 shader_stage);

void VPushBackPushConstRange(VShaderObj* _restrict obj,VkFormat* format_array,u32 format_count,u32 size,VkShaderStageFlagBits shader_stage);

VkPipelineCache VCreatePipelineCache(const VDeviceContext* _in_ vdevice,void* init_data = 0,ptrsize init_size = 0);

void VGetPipelineCacheData(const VDeviceContext* _in_ vdevice,VkPipelineCache cache,void* init_data,ptrsize* init_size);