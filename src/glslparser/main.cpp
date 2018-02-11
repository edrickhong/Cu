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

*/


//we don't need to parse the structs

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

  GenerateShaderTable((s8*)infile,(s8*)outfile);
  
  return 0;
}


  // printf("%s:\n",argv[1]);
  // auto file_string = "../src/shaders/resolve_tfetch.comp";

  // GenerateShaderTable((s8*)file_string,(s8*)"temp.spv");
