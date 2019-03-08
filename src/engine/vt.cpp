

/*
this is included by assetmanager.cpp
this is strictly to isolate the core variables and vt system 
Do not include functionality that does not involved writing and reading pages
*/



#ifdef DEBUG

_persist VTReadbackPixelFormat* debug_pixels = 0;

#endif

//UTIL

_ainline void InternalPageToPhysCoord(u32 page,u8* x,u8* y){
    //MARK: Hopefully the compiler is smart enough to fold this into a single div
    *y = page / (phys_w/_tpage_side);
    *x = page % (phys_w/_tpage_side);
}

_ainline u32 InternalPhysCoordToPage(u8 x,u8 y){
    return x + (y * (phys_w/_tpage_side));
}

TCoord InternalToCoordAtMipLevel(TCoord src_coord,u32 dst_mip){
    
    TCoord c = {};
    
    c.x = src_coord.x >> (dst_mip - src_coord.mip);
    c.y = src_coord.y >> (dst_mip - src_coord.mip);
    c.mip = dst_mip;
    
#ifdef DEBUG
    
#if  0
    
    printf("a: %d %d %d\n",c.mip,c.x,c.y);
    
#endif
    
#endif
    
    return c;
}

Coord InternalToQuadCoord(TCoord dst_coord,TCoord prevdst_coord){
    
    Coord c = {
        (u8)(dst_coord.x - (prevdst_coord.x << 1)),
        (u8)(dst_coord.y - (prevdst_coord.y << 1))
    };
    
#ifdef DEBUG
    
#if  0
    printf("q: %d %d\n",c.x,c.y);
#endif
    
#endif
    
    return c;
}

//EVICTING PAGES


//MARK: maybe we shouldn't have a list and just directly write to the global freepages_array and return the number of pages freed
void _ainline InternalEvictAllTexturePages(TextureAssetHandle* _restrict handle,FreepageList* list){
    
    auto node_count = InternalGetTotalNodes(handle->max_miplevel +
                                            1) - 1;
    
    auto node_array = handle->pagetree.first;
    
    for(u32 i = 0; i < node_count; i++){
        
        auto node = &node_array[i];
        
        if(node->page_value != (u32)-1){
            
            auto page_value = node->page_value;
            node->page_value = (u32)-1;
            
            list->array[list->count] = page_value;
            list->count++;
            
        }
        
    }
    
    //returns a FreepageList to the global freepage_array
    auto ReturnFreePages = [](FreepageList* list) -> void {
        
        for(u32 i = 0; i < list->count; i++){
            
            vt_freepages_array[vt_freepages_count] 
                = InternalPhysCoordToPage(list->format_array[i].x,list->format_array[i].y);
            vt_freepages_count++;
        }
        
    };
    
    ReturnFreePages(list);
    
    //TODO: we need to have a push evict or something here
    
}


void TestEvictTextureAsset(TextureAssetHandle* _restrict handle){
    
    FreepageList list = {};
    
    InternalEvictAllTexturePages(handle,&list);
    
    exit(0);
    
}

//PAGE MANAGEMENT  (MAY CONTAIN EVICT)

void InternalGetPageCoord(u8* x,u8* y,FreepageList* list){
    
    
    //MARK: lambda becasue I don't want this to be called anywhere
    //IDK if this is the best use of it
    auto GetAvailablePage = [](FreepageList* list) -> u32 {
        
        /*
MARK: I think we depend on this not moving
*/
        if(!vt_freepages_count){
            
            TextureAssetHandle handle_array[_texturehandle_max] = {};
            
            memcpy(handle_array,handle_array,sizeof(TextureAssetHandle) * texturehandle_count);
            
            
            qsort(handle_array,texturehandle_count,
                  sizeof(TextureAssetHandle),
                  [](const void * a, const void* b)->s32 {
                  
                  auto t_a = (TextureAssetHandle*)a;
                  auto t_b = (TextureAssetHandle*)b;
                  
                  return t_a->timestamp - t_b->timestamp;
                  });
            
            for(u32 i = 0; i < texturehandle_count; i++){
                
                auto t = &handle_array[i];
                
                if(global_timestamp != t->timestamp){
                    //evict texture
                    InternalEvictAllTexturePages(t,list);
                }
                
                //MARK: maybe we should evict more than one texture?
                if(list->count){
                    break;
                }
                
            }
            
        }
        
        _kill("can't get any pages\n",!vt_freepages_count);
        
        vt_freepages_count--;
        u32 page = vt_freepages_array[vt_freepages_count];
        
        
        
        return page;
    };
    
    auto page = GetAvailablePage(list);
    InternalPageToPhysCoord(page,x,y);
}


/*
  NOTE: 
  We have 3 sets of coordinates:
  src coord - coord at the src mip level
  dst coord - coord at the dst mip level (derived from src coord)
  quad coord - coord of the tile in the quad (derived from next src coord)
  
   a higher mip can generate coords for a lower mip,but not the other way around
*/
void InternalTraverseMipTree(TPageQuadNode* node,TCoord src_coord,
                             TCoord dst_coord,FetchList* list,FreepageList* freepage_list){
    
    if(node->page_value == (u32)-1){
        
        _kill("not enough fetch space\n",list->count >=
              _arraycount(list->array));
        
        auto fetch = &list->array[list->count];
        
        fetch->src_coord.mip = dst_coord.mip;
        fetch->src_coord.x = dst_coord.x;
        fetch->src_coord.y = dst_coord.y;
        
#if  0
        printf("f %d %d %d %p :: ",dst_coord.mip,dst_coord.x,dst_coord.y,(void*)node);
        printf("incoord %d %d %d\n",src_coord.mip,src_coord.x,src_coord.y);
#endif
        
        InternalGetPageCoord(&fetch->dst_coord.x,&fetch->dst_coord.y,freepage_list);
        list->count++;
        node->page_value = _encode_rgba(fetch->dst_coord.x,fetch->dst_coord.y,0,255);
    }
    
#ifdef DEBUG
    
#if 0
    
    else{
        printf("rejected %d %d %d\n",dst_coord.mip,dst_coord.x,dst_coord.y);
    }
    
#endif
    
#endif
    
    if(dst_coord.mip == src_coord.mip){
        return;
    }
    
    //computing the next dst_coord
    auto next_dst_coord = InternalToCoordAtMipLevel(src_coord,dst_coord.mip - 1);
    
    //computing the coord in quad space
    auto q_coord = InternalToQuadCoord(next_dst_coord,dst_coord);
    
    TPageQuadNode* nextnode = 0;
    
    if(q_coord.y < (1)){
        
        if(q_coord.x < (1)){
            nextnode = node->first;
        }
        else{
            nextnode = node->second;
        }
        
    }
    
    else{
        
        if(q_coord.x < (1)){
            nextnode = node->third;
        }
        else{
            nextnode = node->fourth;
        }
        
    }
    
    
    if(nextnode){
        InternalTraverseMipTree(nextnode,src_coord,next_dst_coord,list,freepage_list);
    }
    
}

void InternalGenerateDependentCoords(TextureAssetHandle* asset,u8 mip,u8 x,u8 y,
                                     FetchList* list,FreepageList* freepage_list){
    
    TCoord src = {};
    
    src.x = x;
    src.y = y;
    src.mip = mip;
    
    auto dst = InternalToCoordAtMipLevel(src,asset->max_miplevel);
    
    auto node = &asset->pagetree;
    
    InternalTraverseMipTree(node,src,dst,list,freepage_list);
}



//MARK: start here (Get vt fetch working again and work on vt evict next)
u32 GenTextureFetchList(TextureAssetHandle* asset,VTReadbackPixelFormat* src_coords,
                        u32 count,FetchList* list,FreepageList* freepage_list){
    
    
    TIMEBLOCK(Purple);
    
    for(u32 i = 0; i < count; i++){
        
        auto a = &src_coords[i];
        
        InternalGenerateDependentCoords(asset,a->mip,a->x,a->y,list,freepage_list);
    }
    
    return list->count;
}


//ACTUAL PAGE TRANSFER
void FetchTextureTiles(ThreadFetchBatch* batch,VkCommandBuffer fetch_cmdbuffer){
    
    //MARK:printf
    printf("async alloc %d\n",(u32)async_transferbuffer.offset);
    
    
    if(batch->fetchlist.count){
        
        _kill("over possible fetch tiles array\n",batch->fetchlist.count >= 21845);
        
        VkBufferImageCopy imagecopy_array[21845];
        
        VkBufferImageCopy pagecopy_array[21845];
        
        auto file = FOpenFile(batch->assetfile,F_FLAG_READONLY);
        
        {
            
            TIMEBLOCK(Purple);
            
            u32 tile_size = (u32)(batch->bpp * _tpage_side * _tpage_side);
            u32 total_size = batch->fetchlist.count * tile_size;
            
            auto transferbuffer = async_transferbuffer.buffer;
            auto transferbuffer_offset =
                AllocateTransferBuffer(&async_transferbuffer,
                                       total_size + (sizeof(u32) * batch->fetchlist.count));
            
            {
                
                TIMEBLOCK(DarkBlue);
                
                s8* mappedmemory_ptr;
                
                VMapMemory(global_device->device,transferbuffer.memory,transferbuffer_offset,
                           total_size + (sizeof(u32) * batch->fetchlist.count),(void**)&mappedmemory_ptr);
                
                //Copy image data here
                for(u32 i = 0; i < batch->fetchlist.count; i++){
                    
                    auto cur = mappedmemory_ptr + (tile_size * i);
                    
                    auto fetch_data = &batch->fetchlist.array[i];
                    
                    auto mip = fetch_data->src_coord.mip;
                    auto x = fetch_data->src_coord.x;
                    auto y = fetch_data->src_coord.y;
                    
#if  0
                    if(batch->assetfile == texturehandle_array[0].assetfile)
                        printf("s %s %d %d %d\n",batch->assetfile,mip,x,y);
#endif
                    
                    
                    GetTileDataTDF(file,batch->w,batch->h,x,y,mip,batch->bpp,cur);
                    
                }
                
                //copy page table data
                for(u32 i = 0; i < batch->fetchlist.count; i++){
                    auto cur = ((u32*)(mappedmemory_ptr + total_size)) + i;
                    auto fetch_data = &batch->fetchlist.array[i];
                    
                    auto x = fetch_data->dst_coord.x;
                    auto y = fetch_data->dst_coord.y;
                    
                    *cur = _encode_rgba(x,y,0,255);
                }
                
                vkUnmapMemory(global_device->device,transferbuffer.memory);
            }
            
            {
                TIMEBLOCK(Salmon);
                
                for(u32 i = 0; i < batch->fetchlist.count; i++){
                    
                    //Transfer image data
                    
                    auto data_copy = &imagecopy_array[i];
                    auto page_copy = &pagecopy_array[i];
                    auto fetch_data = &batch->fetchlist.array[i];
                    
                    auto src_mip = (u32)(fetch_data->src_coord.mip);
                    
                    auto dst_x = (u32)(fetch_data->dst_coord.x);
                    auto dst_y = (u32)(fetch_data->dst_coord.y);
                    
                    auto mip_w = batch->w;
                    auto mip_h = batch->h;
                    
                    for(u32 j = 0; j < src_mip; j++){
                        mip_w >>= 1;
                        mip_h >>= 1;
                    }
                    
                    //copies image data
                    data_copy->bufferOffset = transferbuffer_offset + (tile_size * i);
                    
                    data_copy->bufferRowLength = 0;
                    data_copy->bufferImageHeight = 0;
                    data_copy->imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT,0,0,1};
                    data_copy->imageOffset = {(int32_t)(dst_x * _tpage_side),(int32_t)(dst_y * _tpage_side),0};
                    data_copy->imageExtent = {_tpage_side,_tpage_side,1};
                    
                    _kill("coord exceeds texture bounds\n",fetch_data->src_coord.x >= batch->w ||
                          fetch_data->src_coord.y >= batch->h);
                    
                    //copies page data
                    page_copy->bufferOffset = transferbuffer_offset + total_size + (i * sizeof(u32));
                    page_copy->bufferRowLength = 0;
                    page_copy->bufferImageHeight = 0;
                    page_copy->imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT,src_mip,0,1};
                    page_copy->imageOffset = {(int32_t)(fetch_data->src_coord.x),
                        (int32_t)(fetch_data->src_coord.y),0};
                    page_copy->imageExtent = {1,1,1};
                }
                
            }
            
            vkCmdCopyBufferToImage(fetch_cmdbuffer,transferbuffer.buffer,
                                   global_texturecache.image,VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                   batch->fetchlist.count,imagecopy_array);
            
            vkCmdCopyBufferToImage(fetch_cmdbuffer,transferbuffer.buffer,
                                   batch->pagetable.image,VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                   batch->fetchlist.count,pagecopy_array);
            
            transferbuffer_offset += total_size + (sizeof(u32) * batch->fetchlist.count);
            
        }
        
        FCloseFile(file);
    }
    
}


void PushThreadTextureEvictQueue(TextureAssetHandle* asset,TSemaphore sem){
    //TODO:
}

void PushThreadTextureFetchQueue(ThreadTextureFetchQueue* queue,
                                 TextureAssetHandle* asset,FetchList* list,TSemaphore sem){
    
    _kill("too many entries\n",queue->count >= _FetchqueueSize);
    
#if  0
    {
        static u32 count = 0;
        printf("%d pushed %s\n",count,asset->assetfile);
        count++;
    }
#endif
    
    auto t = &queue->buffer[queue->count];
    
    t->w = asset->w;
    t->h = asset->h;
    t->bpp = asset->bpp;
    t->assetfile = asset->assetfile;
    t->pagetable = asset->pagetable;
    
    t->fetchlist.count = list->count;
    t->total_miplevel = asset->max_miplevel + 1;
    
    memcpy(t->fetchlist.array,list->array,
           sizeof(FetchData) * list->count);
    
    queue->count++;
    
    TSignalSemaphore(sem);
}


//TODO: have a threaded eviction too
void ThreadExecuteTextureFetchQueue(ThreadTextureFetchQueue* queue){
    
    if(queue->index == queue->count){
        return;
    }
    
    u32 tfetch_count = 0;
    
    while(queue->index != queue->count){
        
        auto batch = &queue->buffer[queue->index];
        
        _kill("invalid asset texture\n",!batch->pagetable.image || !batch->assetfile);
        
#if  0
        {
            static u32 count = 0;
            printf("%d fetched %s\n",count,batch->assetfile);
            count++;
        }
        
#endif
        
        VkImageMemoryBarrier transfer_imagebarrier =  {
            VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            0,
            0,
            VK_ACCESS_TRANSFER_WRITE_BIT,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            VK_QUEUE_FAMILY_IGNORED,
            VK_QUEUE_FAMILY_IGNORED,
            batch->pagetable.image,
            {VK_IMAGE_ASPECT_COLOR_BIT,0,(batch->total_miplevel),0,1}  
        };
        
        VkImageMemoryBarrier shader_imagebarrier = {
            VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            0,
            VK_ACCESS_TRANSFER_WRITE_BIT,
            VK_ACCESS_SHADER_READ_BIT,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            VK_QUEUE_FAMILY_IGNORED,
            VK_QUEUE_FAMILY_IGNORED,
            batch->pagetable.image,
            {VK_IMAGE_ASPECT_COLOR_BIT,0,(batch->total_miplevel),0,1}
        };
        
        vkCmdPipelineBarrier(queue->cmdbuffer,
                             VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                             VK_PIPELINE_STAGE_TRANSFER_BIT,0,0,0,0,0,1,
                             &transfer_imagebarrier);
        
        FetchTextureTiles(batch,queue->cmdbuffer);
        
        tfetch_count += batch->fetchlist.count;
        
        vkCmdPipelineBarrier(queue->cmdbuffer,
                             VK_PIPELINE_STAGE_TRANSFER_BIT,
                             VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,0,0,0,0,0,1,&shader_imagebarrier);
        
        queue->index ++;
        
    }
    
    
    queue->fetch_count = tfetch_count;
    
    queue->is_done = true;
    
}

void ThreadExecuteTextureEvictQueue(){
    //TODO: actual perform the clear
}

void ThreadExecuteVTSystem(ThreadTextureFetchQueue* queue){
    ThreadExecuteTextureEvictQueue();
    ThreadExecuteTextureFetchQueue(queue);
}

//we do not do temp page allocations anymore
VkCommandBuffer GenerateTextureFetchRequests(
ThreadTextureFetchQueue* fetchqueue,TSemaphore sem){
    
    TIMEBLOCK(Red);
    
    {
        TIMEBLOCKTAGGED("Flushing readbackbuffer",Green);
        
        VMemoryRangesArray ranges = {};
        
        VPushBackMemoryRanges(&ranges,vt_targetreadbackbuffer.memory,
                              0, VK_WHOLE_SIZE);
        
        VInvalidateMemoryRanges(global_device,&ranges);
    }
    
#if 0
    
    {
        
        u32 total_tiles = vt_readbackbuffer.w * vt_readbackbuffer.h;
        u32 count = 0;
        
        static u32 hit_count = 0;
        
        if(hit_count){
            
            for(u32 i = 0; i < total_tiles; i++){
                
                auto a = &vt_readbackpixels[i];
                
                if(a->texture_id && (a->texture_id - 1) >= _arraycount(texturehandle_array)){
                    
                    a->value = _encode_rgba(255,0,0,255);
                }
                
                else if(a->texture_id){
                    a->value = (u32)-1;
                    
                }
                
                else{
                    a->value = _encode_rgba(0,0,255,255);
                }
                
            }
            
            WriteBMP(vt_readbackpixels,vt_readbackbuffer.w,vt_readbackbuffer.h,
                     "test.bmp");
            
            exit(0);
            
        }
        
        hit_count++;
        
    }
    
    
    
#endif  
    
#ifdef DEBUG 
    
    {
        
        memcpy(debug_pixels,vt_readbackpixels,
               vt_readbackbuffer.w * vt_readbackbuffer.h * 4);
    }
    
#endif
    
    
    u32 total_tiles = vt_readbackbuffer.w * vt_readbackbuffer.h;
    u32 count = 0;
    
    for(u32 i = 0; i < total_tiles; i++){
        
        auto a = &vt_readbackpixels[i];
        
        if(a->texture_id){
            
#ifdef DEBUG
            
            if(a->texture_id >= _arraycount(texturehandle_array)){
                DebugRenderFeedbackBuffer();
            }
            
#endif
            
            a->texture_id--;
            threadtexturefetch_array[count] = *a;
            count++;
            
        }
        
    }
    
    qsort(threadtexturefetch_array,count,sizeof(u32),
          [](const void * a, const void* b)->s32 {
          
          auto a_t = ((VTReadbackPixelFormat*)a)->texture_id;
          auto b_t = ((VTReadbackPixelFormat*)b)->texture_id;
          
          return a_t - b_t;
          });
    
    
    struct TAssetEntry{
        TextureAssetHandle* asset;
        u32 offset;
        u32 count;
    };
    
    TAssetEntry entry_array[16] = {};
    u32 entry_count = 0;
    //this locks in all the textures we are gonna use first and narrows down the a list of assets
    {
        
        TextureAssetHandle* active = 0;
        u32 active_index = 0;
        
        for(u32 i = 0; i < count; i++){
            
            auto tid = threadtexturefetch_array[i].texture_id;
            
#ifdef DEBUG
            
            if(tid >= _arraycount(texturehandle_array)){
                
                printf("got invalid tid %d\n",tid);
                
                DebugRenderFeedbackBuffer();
                
                
            }
            
            
#endif
            
            auto cur = &texturehandle_array[tid];
            
            if(active != cur){
                
                _kill("too many entries\n",entry_count >= _arraycount(entry_array));
                
                if(active){
                    entry_array[entry_count] = {active,active_index,i - active_index};
                    entry_count++;
                }
                
                active = cur;
                active_index = i;
                CommitTexture(active);
            }
            
        }
        
        if(active_index != count){
            entry_array[entry_count] = {active,active_index,count - active_index};
            entry_count++;
        }
        
        
    }
    
    VkCommandBuffer resolve_cmdbuffer = fetchqueue->cmdbuffer;
    
    auto is_done = IsThreadTextureFetchQueueDone(fetchqueue);
    auto tfetch_count = fetchqueue->fetch_count;
    
    if(is_done){
        
        auto  commit_cmdbuffer = fetchcmdbuffer_array[fetchcmdbuffer_count];
        
        fetchcmdbuffer_count++;
        
        if(fetchcmdbuffer_count >= _arraycount(fetchcmdbuffer_array)){
            fetchcmdbuffer_count = 0;  
        }
        
        SetCmdBufferThreadTextureFetchQueue(fetchqueue,commit_cmdbuffer);
        
        VStartCommandBuffer(fetchqueue->cmdbuffer,0,0,0,0,0,0,0);
        
        VkImageMemoryBarrier transfer_tcacheimagebarrier =     {
            VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            0,
            0,
            VK_ACCESS_TRANSFER_WRITE_BIT,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            VK_QUEUE_FAMILY_IGNORED,
            VK_QUEUE_FAMILY_IGNORED,
            GetTextureCache()->image,
            {VK_IMAGE_ASPECT_COLOR_BIT,0,1,0,1}  
        };
        
        vkCmdPipelineBarrier(fetchqueue->cmdbuffer,
                             VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                             VK_PIPELINE_STAGE_TRANSFER_BIT,0,0,0,0,0,1,
                             &transfer_tcacheimagebarrier);
        
        Clear(fetchqueue);
    }
    
    //this begins page allocations
    {
        
        for(u32 i = 0; i < entry_count; i++){
            
            auto entry = &entry_array[i];
            FetchList list = {};
            FreepageList freepage_list = {};
            
            GenTextureFetchList(entry->asset,&threadtexturefetch_array[entry->offset],entry->count,
                                &list,&freepage_list);
            
            
            if(list.count){
                
                //TODO: clear evicted textures too
                PushThreadTextureFetchQueue(fetchqueue,entry->asset,&list,sem);	
            }
            
        }
        
    }
    
    if(is_done && resolve_cmdbuffer){
        
        VkImageMemoryBarrier shader_tcacheimagebarrier =     {
            VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            0,
            VK_ACCESS_TRANSFER_WRITE_BIT,
            VK_ACCESS_SHADER_READ_BIT,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            VK_QUEUE_FAMILY_IGNORED,
            VK_QUEUE_FAMILY_IGNORED,
            GetTextureCache()->image,
            {VK_IMAGE_ASPECT_COLOR_BIT,0,1,0,1}
        };
        
        vkCmdPipelineBarrier(resolve_cmdbuffer,
                             VK_PIPELINE_STAGE_TRANSFER_BIT,
                             VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,0,0,0,0,0,1,
                             &shader_tcacheimagebarrier);
        
        VEndCommandBuffer(resolve_cmdbuffer);
        
        if(tfetch_count){
            return resolve_cmdbuffer;
        }
        
    }
    
    return 0;
}

//INTERNAL INIT
void _ainline InitVTSubsystem(u32 phys_w_tiles,u32 phys_h_tiles,VDeviceContext* _restrict vdevice,VSwapchainContext* swapchain){
    
    {
        
        vt_freepages_count = phys_w_tiles * phys_h_tiles;
        vt_freepages_array = (u16*)alloc(sizeof(u16) * vt_freepages_count);
        
        auto max_index = vt_freepages_count - 1;
        
        for(u32 i = 0; i < vt_freepages_count; i++){
            
            vt_freepages_array[i] = max_index - i;
            
        }
        
        phys_w = phys_w_tiles * _tpage_side;
        phys_h = phys_h_tiles * _tpage_side;
    }
    
#if _usergba
    
    global_texturecache =
        VCreateTextureCache(vdevice,phys_w,phys_h,VK_FORMAT_R8G8B8A8_UNORM);
    
#else
    
    global_texturecache =
        VCreateTextureCache(vdevice,phys_w,phys_h,VK_FORMAT_BC1_RGB_UNORM_BLOCK);
    
#endif
    
    //transition to proper layout
    {
        
        auto queue = VGetQueue(vdevice,VQUEUETYPE_ROOT);
        
        auto pool =
            VCreateCommandPool(vdevice,
                               VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
                               VGetQueueFamilyIndex(VQUEUETYPE_ROOT));
        
        auto cmdbuffer = VAllocateCommandBuffer(vdevice,pool,
                                                VK_COMMAND_BUFFER_LEVEL_PRIMARY);
        
        VStartCommandBuffer(cmdbuffer,
                            VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
        
        VkImageMemoryBarrier shader_barrier = {
            VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            0,
            0,//srcAccessMask
            VK_ACCESS_SHADER_READ_BIT,
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            VK_QUEUE_FAMILY_IGNORED,
            VK_QUEUE_FAMILY_IGNORED,
            global_texturecache.image,
            {
                VK_IMAGE_ASPECT_COLOR_BIT,
                0,
                1,
                0,
                1
            }
        };
        
        vkCmdPipelineBarrier(cmdbuffer,VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                             VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,0,0,0,0,0,1,&shader_barrier);
        
        VEndCommandBuffer(cmdbuffer);
        VSubmitCommandBuffer(queue,cmdbuffer);
        vkQueueWaitIdle(queue);
        vkDestroyCommandPool(vdevice->device,pool,0);
    }
    
    
#ifdef DEBUG
    
#if (_usergba)
    {
        //NOTE:Clear texturecache - Linux doesn't clear gpu memory by default. this is better for
        // debugging
        
        auto queue = VGetQueue(vdevice,VQUEUETYPE_ROOT);
        
        auto pool =
            VCreateCommandPool(vdevice,
                               VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
                               VGetQueueFamilyIndex(VQUEUETYPE_ROOT));
        
        auto cmdbuffer = VAllocateCommandBuffer(vdevice,pool,
                                                VK_COMMAND_BUFFER_LEVEL_PRIMARY);
        
        VStartCommandBuffer(cmdbuffer,
                            VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
        
        VkImageMemoryBarrier transfer_membarrier = {
            VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            0,
            0,//srcAccessMask
            VK_ACCESS_TRANSFER_WRITE_BIT,
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            VK_QUEUE_FAMILY_IGNORED,
            VK_QUEUE_FAMILY_IGNORED,
            global_texturecache.image,
            {
                VK_IMAGE_ASPECT_COLOR_BIT,
                0,
                1,
                0,
                1
            },
        };
        
        vkCmdPipelineBarrier(cmdbuffer,
                             VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                             VK_PIPELINE_STAGE_TRANSFER_BIT,
                             0,
                             0,0,0,0,1,&transfer_membarrier);
        
        VkClearColorValue color = {};
        
        VkImageSubresourceRange range = {
            VK_IMAGE_ASPECT_COLOR_BIT,0,1,0,1
        };
        
        vkCmdClearColorImage(cmdbuffer,global_texturecache.image,
                             VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,&color,1,&range);
        
        
        VEndCommandBuffer(cmdbuffer);
        
        VSubmitCommandBuffer(queue,cmdbuffer);
        
        vkQueueWaitIdle(queue);
        
        vkDestroyCommandPool(vdevice->device,pool,0);
    }
#endif
    
#endif
    
    {
        
        auto w = swapchain->width/_fetch_dim_scale_w;
        auto h = swapchain->height/_fetch_dim_scale_h;
        
        auto img = VCreateColorImage(vdevice,w,h,
                                     VK_IMAGE_USAGE_STORAGE_BIT |
                                     VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                                     VK_IMAGE_USAGE_TRANSFER_SRC_BIT);
        
        vt_readbackbuffer.image = img.image;
        vt_readbackbuffer.view = img.view;
        vt_readbackbuffer.memory = img.memory;
        vt_readbackbuffer.w = w;
        vt_readbackbuffer.h = h;
        
#ifdef DEBUG
        
        debug_pixels = (VTReadbackPixelFormat*)alloc(w * h * 4);
#endif
        
        //(FIXME:)MARK: do we really want to make this cached?
        vt_targetreadbackbuffer = VCreateColorImageMemory(vdevice,w,h,
                                                          VK_IMAGE_USAGE_TRANSFER_DST_BIT,false,VMAPPED_NONE,
                                                          VK_IMAGE_TILING_LINEAR);
        
        VMapMemory(global_device,vt_targetreadbackbuffer.memory,
                   0,w * h * sizeof(VTReadbackPixelFormat),(void**)&vt_readbackpixels);
        
        threadtexturefetch_array = (VTReadbackPixelFormat*)alloc(w * h *
                                                                 sizeof(VTReadbackPixelFormat));
        
        
        //clear the image
#if 0
        {
            auto queue = VGetQueue(vdevice,VQUEUETYPE_ROOT);
            
            auto pool =
                VCreateCommandPool(vdevice,
                                   VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
                                   VGetQueueFamilyIndex(VQUEUETYPE_ROOT));
            
            auto cmdbuffer = VAllocateCommandBuffer(vdevice,pool,
                                                    VK_COMMAND_BUFFER_LEVEL_PRIMARY);
            
            VStartCommandBuffer(cmdbuffer,
                                VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
            
            VkImageMemoryBarrier transfer_membarrier = {
                VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
                0,
                0,//srcAccessMask
                VK_ACCESS_TRANSFER_WRITE_BIT,
                VK_IMAGE_LAYOUT_UNDEFINED,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                VK_QUEUE_FAMILY_IGNORED,
                VK_QUEUE_FAMILY_IGNORED,
                vt_targetreadbackbuffer.image,
                {
                    VK_IMAGE_ASPECT_COLOR_BIT,
                    0,
                    1,
                    0,
                    1
                },
            };
            
            vkCmdPipelineBarrier(cmdbuffer,
                                 VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                                 VK_PIPELINE_STAGE_TRANSFER_BIT,
                                 0,
                                 0,0,0,0,1,&transfer_membarrier);
            
            VkClearColorValue color = {};
            
            VkImageSubresourceRange range = {
                VK_IMAGE_ASPECT_COLOR_BIT,0,1,0,1
            };
            
            vkCmdClearColorImage(cmdbuffer,vt_targetreadbackbuffer.image,
                                 VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,&color,1,&range);
            
            
            VEndCommandBuffer(cmdbuffer);
            
            VSubmitCommandBuffer(queue,cmdbuffer);
            
            
            vkQueueWaitIdle(queue);
            
            vkDestroyCommandPool(vdevice->device,pool,0);
        }
        
#endif
    }
    
    
    
    for(u32 i = 0; i < _arraycount(fetch_pool); i++){
        fetch_pool[i] =
            VCreateCommandPool(vdevice,
                               VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
                               VGetQueueFamilyIndex(VQUEUETYPE_ROOT));
        
        fetchcmdbuffer_array[i] =
            VAllocateCommandBuffer(vdevice,fetch_pool[i],
                                   VK_COMMAND_BUFFER_LEVEL_SECONDARY);
    }
    
}


void SetupQuadTree(AQuadNode*curnode,AQuadNode* space,u32 curlevel,u32 maxlevel){
    
    if(curlevel == maxlevel){
        curnode->children_array = 0;
        return;
    }
    
    curnode->children_array = space;
    space += 4;
    
    for(u32 i = 0; i < 4; i++){
        auto child = &curnode->children_array[i];
        SetupQuadTree(child,space,curlevel + 1,maxlevel);
    }
    
}

void SetupQuadTree(DQuadNode*curnode,DQuadNode* space,u32 curlevel,u32 maxlevel){
    
    if(curlevel == maxlevel){
        curnode->first = 0;
        curnode->second = 0;
        curnode->third = 0;
        curnode->fourth = 0;
        return;
    }
    
    curnode->first = space;
    space++;
    SetupQuadTree(curnode->first,space,curlevel + 1,maxlevel);
    
    curnode->second = space;
    space++;
    SetupQuadTree(curnode->second,space,curlevel + 1,maxlevel);
    
    curnode->third = space;
    space++;
    SetupQuadTree(curnode->third,space,curlevel + 1,maxlevel);
    
    curnode->fourth = space;
    space++;
    
    SetupQuadTree(curnode->fourth,space,curlevel + 1,maxlevel);
    
}

void SetupPageTree(TPageQuadNode*curnode,TPageQuadNode** space,u32 curlevel,
                   u32 maxlevel){
    
    *curnode = {};
    
    curnode->page_value = (u32)-1;
    
    if(curlevel == maxlevel){
        return;
    }
    
    curnode->first = *(space);
    *space = *(space) + 1;
    SetupPageTree(curnode->first,space,curlevel + 1,maxlevel);
    
    curnode->second = *(space);
    *space = *(space) + 1;
    SetupPageTree(curnode->second,space,curlevel + 1,maxlevel);
    
    curnode->third = *(space);
    *space = *(space) + 1;
    SetupPageTree(curnode->third,space,curlevel + 1,maxlevel);
    
    curnode->fourth = *(space);
    *space = *(space) + 1;
    SetupPageTree(curnode->fourth,space,curlevel + 1,maxlevel);
    
}

void DebugTree(TPageQuadNode*curnode,u32 counter = 0,u32 index = 0){
    
    if(curnode){
        // printf("%d\n",curnode->page_value);
        printf("%d(%d : %d) > ",curnode->page_value,index,counter);
        counter ++;
        DebugTree(curnode->first,counter,1);
        DebugTree(curnode->second,counter,2);
        DebugTree(curnode->third,counter,3);
        DebugTree(curnode->fourth,counter,4);
    }
    
    else{
        // printf("\nbreak(%d : %d)\n",index,counter);
        // printf("counter %d\n",counter);
        // printf("------------\n");
    }
    
}


//
