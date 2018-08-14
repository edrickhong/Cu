#include "mode.h"
#include "ffileio.h"
#include "pparse.h"
#include "vulkan/vulkan.h"
#include "main.h"

/*
  Things to capture:
  vert layout
  pushconst layout
  descriptorlayout
  
  TODO: We do not support input attachments yet
  
  Subpass Inputs
  --------------
  
  Within a rendering pass, a subpass can write results to an output target
  that can then be read by the next subpass as an input subpass.  The
  "Subpass Input" feature regards the ability to read an output target.
  
  Subpasses are read through a new set of types, available only
  to fragment shaders:
  
  subpassInput
  subpassInputMS
  isubpassInput
  isubpassInputMS
  usubpassInput
  usubpassInputMS
  
  Unlike sampler and image objects, subpass inputs are implicitly addressed
  by the fragment's (x, y, layer) coordinate.
  
  A subpass input is selected by using a new layout qualifier identifier
  'input_attachment_index'.  For example:
  
  layout(input_attachment_index = i, ...) uniform subpassInput t;
  
  An input_attachment_index of i selects the ith entry in the input pass
  list. (See API specification for more information.)
  
  These objects support reading the subpass input through the following
  functions:
  
  gvec4 subpassLoad(gsubpassInput   subpass);
  gvec4 subpassLoad(gsubpassInputMS subpass, int sample);
  
  
  we will have to save a struct array that our layouts can use to lookup within the file
  
  
  We should keep in mind of the data layout (std140 for uniforms, std430 for storage: https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#interfaces-resources-layout)
  
  TODO: add a way to query for struct/block data layouts (according to std140 or std430)
  
  
  NOTE: 
GLSLTypes are tagged with TAG_CTYPE
If dim_array_count is 0, it is not an array and should be treated as an array element of 1 (same for cparser)

*/

s32 main(s32 argc,s8** argv){
    
    if(argc < 3){
        printf("please provide an in and out file\n");
        
        return -1;
    }
    
    s8* infile = 0;
    s8* outfile = 0;
    
    u32 cur = 0;
    
    for(u32 i = 1; i < (u32)argc; i++){
        
        auto arg_s = argv[i];
        
        //MARK: we don't actually use this anymore (we could support it)
        if(arg_s[0] == '-' && arg_s[1] == 'D'){
            macro_array[macro_count] = PHashString(&arg_s[2]);
            macro_count++;
            continue;
        }
        
        if(!cur){
            infile = arg_s;
            cur++;
        }
        
        else{
            outfile = arg_s;
        }
        
    }
    
    //TODO: add a way to query layouts
    
    auto type = GetShaderType(infile);
    
    auto struct_array = (GenericStruct*)alloc(sizeof(GenericStruct) * 1024);
    u32 struct_count = 0;
    
    
    
    PushConstLayout pushconstlayout = {};
    VertexLayout vertexlayout = {};
    VertexLayout instancelayout = {};
    DescLayout desclayout = {};
    
    {
        
        VertexLayoutEx vertexlayoutex = {};
        VertexLayoutEx instancelayoutex = {};
        
        auto file = FOpenFile(infile,F_FLAG_READONLY);
        
        ptrsize size;
        auto buffer = FReadFileToBuffer(file,&size);
        
        InternalParseSource(type,buffer,size,struct_array,&struct_count,&vertexlayoutex,&instancelayoutex,&pushconstlayout,&desclayout);
        
        FCloseFile(file);
        
        unalloc(buffer);
        unalloc(struct_array);
        
        
        qsort(vertexlayoutex.entry_array,vertexlayoutex.entry_count,sizeof(VertexEntryEx),
              
              [](const void * a, const void* b)->s32 {
              
              auto v_a = (VertexEntryEx*)a;
              auto v_b = (VertexEntryEx*)b;
              
              return v_a->index - v_b->index;
              });
        
        qsort(instancelayoutex.entry_array,instancelayoutex.entry_count,sizeof(VertexEntryEx),
              
              [](const void * a, const void* b)->s32 {
              
              auto v_a = (VertexEntryEx*)a;
              auto v_b = (VertexEntryEx*)b;
              
              return v_a->index - v_b->index;
              });
        
        
#if _log_string
        InternalDebugPrintVertexLayout(&vertexlayoutex);
        InternalDebugPrintInstanceLayout(&instancelayoutex);
        InternalDebugPrintDescLayout(&desclayout);
        InternalDebugPrintPushConstLayout(&pushconstlayout);
#endif
        
        
        vertexlayout.size = vertexlayoutex.size;
        vertexlayout.entry_count = vertexlayoutex.entry_count;
        
        for(u32 i = 0; i < vertexlayoutex.entry_count; i++){
            
            auto entry = &vertexlayoutex.entry_array[i];
            
            vertexlayout.entry_array[i] = {entry->format,entry->size};
        }
        
        
        
        instancelayout.size = instancelayoutex.size;
        instancelayout.entry_count = instancelayoutex.entry_count;
        
        for(u32 i = 0; i < instancelayoutex.entry_count; i++){
            
            auto entry = &instancelayoutex.entry_array[i];
            
            instancelayout.entry_array[i] = {entry->format,entry->size};
        }
        
    }
    
    
    WriteSPXFile(type,outfile,pushconstlayout,
                 vertexlayout,
                 instancelayout,
                 desclayout);
    
    return 0;
}


// printf("%s:\n",argv[1]);
// auto file_string = "../src/shaders/resolve_tfetch.comp";

// GenerateShaderTable((s8*)file_string,(s8*)"temp.spv");
