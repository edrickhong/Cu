#include "main.h"


#define _targetframerate 16.0f//33.33f
#define _ms2s(ms)  ((f32)ms/1000.0f)

_persist auto ustring = "patchthisvalueinatassetpacktime";


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

  pdata->window = WCreateWindow("Cu",W_CREATE_NORESIZE,100,100,1280,720);
    
  TInitTimer();
  INIT_DEBUG_TIMER();
    
  VCreateInstance("eengine",true,1,0,0);
    
  pdata->vdevice = VCreateDeviceContext(&pdata->window);
    
  pdata->swapchain = VCreateSwapchainContext(&pdata->vdevice,2,pdata->window,
					     VSYNC_NORMAL);

  InitAssetAllocator(_gigabytes(1),_megabytes(500),&pdata->vdevice,
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

  pdata->skel_ubo = VCreateUniformBufferContext(&pdata->vdevice,sizeof(SkelUBO[64]),false);
      
  
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
    memset(info,sizeof(Threadinfo),0);
    
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

  GUIInit(&pdata->vdevice,&pdata->window,pdata->renderpass,pdata->transfer_queue,
	  transfercmdbuffer);

  GameInitData initdata = {
    &pdata->scenecontext,gdata,&pdata->window,pdata->vdevice,pdata->renderpass,
    transfercmdbuffer,pdata->transfer_queue
  };

    
  void (*gameinit_funptr)(GameInitData*);
  pdata->lib = InitGameLibrary((void**)(&gameinit_funptr));
    
    
  gameinit_funptr(&initdata);
    
  pdata->deltatime = 0;
    
  ResetTransferBuffer();


  //MARK: keep the obj buffer permanently mapped
  vkMapMemory(pdata->vdevice.device,pdata->skel_ubo.memory,
	      0,pdata->skel_ubo.size,0,(void**)&pdata->objupdate_ptr);
    
  while(gdata->running){
        
    {
      
      pdata->objupdate_count = 0;
      gui_draw_is_locked = 0;

      Clear(&pdata->rendercontext);

      GUIUpdate(&pdata->window,&pdata->keyboardstate,&pdata->mousestate,
		pdata->view,pdata->proj);

      GUIBegin();

      BUILDGUIGRAPH(gdata->draw_profiler);
            
      TimeSpec start,end;

      MASTERTIMEBLOCKSTART(Aqua);
            
      GetTime(&start);
            
      {
	      
	EXECTIMEBLOCK(Black);

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

	ProcessDrawList();
                
	auto frames = AAudioDeviceWriteAvailable(pdata->audio);
                
	if(frames >= pdata->submit_audiobuffer.size_frames){
                    
	  MixAudioLayout args = {&pdata->submit_audiobuffer,audio_data,audio_count};
                    
	  PushThreadWorkQueue(&pdata->threadqueue,
			      MixAudio,(void*)&args,pdata->worker_sem);
	  
	}

	GUIEnd();
                
	BuildRenderCommandBuffer(pdata);
                
	MainThreadDoWorkQueue(&pdata->threadqueue,0);

	//FIXME: if there is corruption, it is either because: 1. Flush is too slow,
	//2. MainThreadDoWorkQueue is not synced
	//3. Someone is writing into another person's data
	ProcessObjUpdateList();
                
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
    
  return 0;
    
}


/*
  TODO: 

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

  FIXME:
  TAlloc calls. I think there are some corruption issues caused by these calls

  main:
  src/engine/main.h:  auto pushconst = TAlloc(PushConst,1);
  src/engine/main.h:  auto cmdbuffers = TAlloc(VkCommandBuffer,count);
  src/engine/main.h:      auto batch = TAlloc(RenderBatch,1);
  src/engine/main.h:      auto batch = TAlloc(RenderBatch,1);
  src/engine/main.h:    TAlloc(VkMappedMemoryRange,pdata->objupdate_count);
  src/engine/main.h:  auto blend = TAlloc(ThreadLinearBlendRes,1);
  src/engine/main.h:  blend->result = TAlloc(Matrix4b4,anim_handle->bone_count);
  
  main(from game):  
  src/engine/main.h:  auto orientation = TAlloc(Matrix4b4,1);



WARNING: Raw ptr, no bounds checking can be done for passed ptr : c:\users\user\desktop\cu\src\engine\main.h SetObjectOrientation 1282
WARNING: Raw ptr, no bounds checking can be done for passed ptr : c:\users\user\desktop\cu\src\engine\main.h DispatchSkelLinearBlend 1592
WARNING: Raw ptr, no bounds checking can be done for passed ptr : c:\users\user\desktop\cu\src\engine\main.h DispatchSkelLinearBlend 1591
WARNING: Raw ptr, no bounds checking can be done for passed ptr : c:\users\user\desktop\cu\src\engine\main.h BuildRenderCommandBuffer 624
WARNING: Raw ptr, no bounds checking can be done for passed ptr : c:\users\user\desktop\cu\src\engine\main.h DispatchRenderContext 418
WARNING: Raw ptr, no bounds checking can be done for passed ptr : c:\users\user\desktop\cu\src\engine\main.h GetCmdBufferArray 453
WARNING: Raw ptr, no bounds checking can be done for passed ptr : c:\users\user\desktop\cu\src\engine\main.h ProcessObjUpdateList 968
  
*/
