

#include "gui_draw.h"

#include "debugtimer.h"

#include "gui_bin.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb/stb_truetype.h"

#include "kkeycode.h"
#include "pparse.h"

#include "wwindow.h"
#include "ffileio.h"

struct InternalGUIFontHeader{
    u32 width;
    u32 height;
    u32 offset;
};

struct InternalGUIFontTable{
    InternalGUIFontHeader* header_array;
    s8* buffer;
    u32 start_baseline;
    u32 alphabet_descend;
    u32 punctuation_descend;
    void* entry_ptr;
};

struct InternalGUIFontImage{
    u32 width;
    u32 height;
    s8* buffer;
};

void InternalGUIDestroyFontTable(InternalGUIFontTable table){
    unalloc(table.entry_ptr);
}


s8* InternalGUIGenFontData(const s8* filepath,f32 fontsize,ptrsize* datasize){
    
    auto file = FOpenFile(filepath,F_FLAG_READONLY);
    ptrsize fontfile_buffersize;
    
    auto fontfile_buffer = (const u8*)FReadFileToBuffer(file,&fontfile_buffersize);
    
    stbtt_fontinfo fontinfo = {};
    
    stbtt_InitFont(&fontinfo,
                   fontfile_buffer,stbtt_GetFontOffsetForIndex(fontfile_buffer,0));
    
    s8* buffer = (s8*)alloc(10000000);
    
    auto header_array = (InternalGUIFontHeader*)alloc(sizeof(InternalGUIFontHeader) * 94);
    
    u32 offset = 0;
    
    u32 finwidth = 0, finheight = 0;
    
    s32 ascent;
    s32 descent;
    s32 linegap;
    
    
    u32 start_baseline = 0;
    u32 alphabet_descent;
    u32 punctuation_descent;
    
    stbtt_GetFontVMetrics(&fontinfo,&ascent,&descent,&linegap);
    
    descent *= -1;
    
    f32 descentfactor = ((f32)descent)/((f32)(ascent + descent));
    
    f32 scale = stbtt_ScaleForPixelHeight(&fontinfo,fontsize);
    
    for(u32 point = 33; point < 127; point++){
        
        s32 width;
        s32 height;
        s32 xoff;
        s32 yoff;
        
        
        u8* singlebitmap =
            (u8*)stbtt_GetCodepointBitmap(&fontinfo, 0,scale,point,&width, &height, &xoff,
                                          &yoff);
        
        u32 roundedwidth = (u32)((f32)width/4.0f + 0.5f);
        u32 roundedheight = (u32)((f32)height/4.0f + 0.5f);
        
        if( (point > 64 && point < 123) && !(point > 90 && point < 97) &&
           !(point == 'j' || point == 'p' || point == 'q' ||
             point == 'y' || point == '@' || point == 'g' ||
             point == 'Q') &&
           (roundedheight > start_baseline)){
            
            start_baseline = roundedheight;
        }
        
        if(point == 'j'){
            alphabet_descent = (u32)((descentfactor * ((f32)roundedheight)) + 1.2f);
        }
        
        if(point == ','){
            
            punctuation_descent = (u32)((descentfactor * ((f32)roundedheight)) + 2.5f);
            
        }
        
        header_array[point - 33] = {roundedwidth,roundedheight,offset};
        
        void* fontbitmap_buffer = alloc(roundedwidth * roundedheight * 4);
        
        //does packing
        for(u32 y = 0; y < roundedheight;y++){
            
            s32* curfontbitmap_ptr = (s32*)(fontbitmap_buffer) + (y * roundedwidth);
            s32* cursinglebitmap_ptr = (s32*)(singlebitmap) + (y * width);
            
            for(u32 x = 0; x < roundedwidth;x++){
                
                *(curfontbitmap_ptr + x) = *(cursinglebitmap_ptr + x);
                
            }
        }
        
        s8* curstore_ptr = buffer + offset;
        
        memcpy(curstore_ptr,fontbitmap_buffer,roundedwidth * roundedheight * 4);
        
        offset += roundedwidth * roundedheight * 4;
        
        finwidth = roundedwidth;
        finheight = roundedheight;
        
        unalloc(fontbitmap_buffer);
        stbtt_FreeBitmap(singlebitmap, 0);
        
    }
    
    //save this to a file and clean up
    {
        u32 write_size = (sizeof(u32) * 3) + (sizeof(InternalGUIFontHeader) * 94) +
            ((finwidth * finheight * 4) + offset);
        
        s8* write_buffer = (s8*)alloc(write_size);
        
        s8* curwrite_ptr =  write_buffer;
        
        
        // void* kern = (void*)(fontfile_buffer + fontinfo.kern);
        // u32 kern_size = fontfile_buffersize - fontinfo.kern;
        
        //TODO:Write in the kerning table
        
        
        
        
        memcpy(curwrite_ptr,&start_baseline,sizeof(u32));
        
        curwrite_ptr += sizeof(u32);
        
        
        memcpy(curwrite_ptr,&alphabet_descent,sizeof(u32));
        
        curwrite_ptr += sizeof(u32);
        
        
        memcpy(curwrite_ptr,&punctuation_descent,sizeof(u32));
        
        curwrite_ptr += sizeof(u32);
        
        
        memcpy(curwrite_ptr,header_array,(sizeof(InternalGUIFontHeader) * 94));
        
        curwrite_ptr += (sizeof(InternalGUIFontHeader) * 94);
        
        
        //We can do red filtering here.
        memcpy(curwrite_ptr,buffer,((finwidth * finheight * 4) + offset));
        
        unalloc(buffer);
        unalloc(header_array);
        
        if(datasize){
            *datasize = (sizeof(u32) * 3) + (sizeof(InternalGUIFontHeader) * 94) +
                ((finwidth * finheight * 4) + offset);
        }
        
        return write_buffer;
    }
    
    
    
    
    
    unalloc((void*)fontfile_buffer);
    
    FCloseFile(file);
}

InternalGUIFontTable InternalGUICreatFontTable(s8* fontdata_buffer){
    
    InternalGUIFontTable table = {};
    
    
    table.entry_ptr = fontdata_buffer;
    
    s8* curfontdata_ptr = fontdata_buffer;
    
    {
        table.start_baseline = *((u32*)curfontdata_ptr);
        
        curfontdata_ptr += sizeof(u32);
        
        table.alphabet_descend = *((u32*)curfontdata_ptr);
        
        curfontdata_ptr += sizeof(u32);
        
        
        table.punctuation_descend = *((u32*)curfontdata_ptr);
        
        curfontdata_ptr += sizeof(u32);
    }
    
    table.header_array = (InternalGUIFontHeader*)curfontdata_ptr;
    
    curfontdata_ptr += sizeof(InternalGUIFontHeader) * 94;
    
    table.buffer = curfontdata_ptr;
    
    return table;
}



InternalGUIFontImage InternalGUIGetFontPoint(InternalGUIFontTable fonttable,
                                             u32 codepoint){
    
    InternalGUIFontImage image = {};
    
    u32 index = codepoint - 33;
    
    image.width = fonttable.header_array[index].width;
    image.height = fonttable.header_array[index].height;
    image.buffer = fonttable.buffer + fonttable.header_array[index].offset; 
    
    return image;
}

s32 InternalGUICharacterAscendFactor(InternalGUIFontTable table,u32 codepoint){
    
    auto font_image = InternalGUIGetFontPoint(table,codepoint);
    
    s32 offsety = 0;
    
    if(codepoint == 'j' || codepoint == 'p' || codepoint == 'q' || codepoint == 'y' ||
       codepoint == 'g' || codepoint == '|'){
        offsety -= table.alphabet_descend;
    }
    
    //the heightoffset for ; is the same as ,
    if(codepoint == ',' || codepoint == ';' || codepoint == '@' || codepoint == '/'
       || codepoint == '\\'){
        offsety -= table.punctuation_descend;
    }
    
    
    //this should be the height of the tallest character that does not have a descent
    if(codepoint == '^' || codepoint == '*' || codepoint == '\'' || codepoint == '"'
       || codepoint == '(' || codepoint == ')'  || codepoint == '{' || codepoint == '}'
       || codepoint == '[' || codepoint == ']' || codepoint == '#' || codepoint == '`'){
        
        offsety += (table.start_baseline - font_image.height - 1);
    }
    
    
    //|| codepoint == 'B' ||  
    if(codepoint == 'c' || codepoint == 'e' || codepoint == 'C' || codepoint == 'o'
       || codepoint == 'O' || codepoint == 'a' || codepoint == 's' || codepoint == 'S'
       || codepoint == 'b' || codepoint == 'd'){
        offsety --;
    }
    
    if(codepoint == '-' || codepoint == '+' || codepoint == '~' || codepoint == '='){
        offsety += (table.start_baseline/4);
    }
    
    return offsety;
}


void GUIGenFontFile(const s8* filepath,const s8* writepath,f32 fontsize){
    
    ptrsize datasize;
    
    s8* fontdata = InternalGUIGenFontData(filepath,fontsize,&datasize);
    
    auto table = InternalGUICreatFontTable(fontdata);
    
    s32 travelup = 0;
    s32 traveldown = 0;
    u32 width = 0;
    
    
    for(u32 i = 0; i < 94;i++){
        
        auto header = table.header_array[i];
        
        s32 offsety = InternalGUICharacterAscendFactor(table,i + 33);
        
        if((s32)(offsety + header.height) > travelup){
            travelup = offsety + header.height;
        }//furthest travel up
        
        if(offsety < 0 && (s32)(offsety) < traveldown){
            traveldown = offsety;
        }//furthest travel down
        
        if(header.width > width){
            width = header.width;
        }
        
    }
    
    printf("%d %d\n",travelup,traveldown);
    
    width *=95;
    
    //total height = max up + aka - (traveldown)
    auto height = travelup - traveldown;
    
    u32 cell_width = width/95;
    
    u32 cell_height = height;
    
    auto fontimage_data = (s8*)alloc(sizeof(u32) * width * height);
    
    memset(fontimage_data,0,sizeof(u32) * width * height);
    
    u32* celldata = (u32*)fontimage_data;
    
#if 1
    
    for(u32 i = 0; i <width * height; i++){
        celldata[i] = 255;
    }
    
#endif
    
    
    //draw letters
    for(u32 i = 0; i < 94;i++){
        
        auto image = InternalGUIGetFontPoint(table,i + 33);
        
        auto ascent = InternalGUICharacterAscendFactor(table,i + 33);
        
        u32 offsety = travelup - image.height - ascent;//travel up is the baseline
        
        for(u32 y = 0; y < image.height;y++ ){
            
            for(u32 x = 0; x < image.width;x++ ){
                
                *(celldata + x + ((y + offsety) * width)) = *(((u32*)image.buffer) + x + (y * image.width));
            }
            
        }
        
        celldata += cell_width;
        
    }
    
    //draw line
    for(u32 y = 0; y < cell_height;y++){
        
        for(u32 x = 0; x < cell_width;x++){
            *(celldata + x + (y * width)) = -1;
        }
        
    }
    
    //TODO: We still have to do spacing by character width and kerning
    
    auto writefile = FOpenFile(writepath,F_FLAG_CREATE | F_FLAG_WRITEONLY |
                               F_FLAG_TRUNCATE);
    
    FWrite(writefile,(void*)&width,sizeof(width));
    FWrite(writefile,(void*)&height,sizeof(height));
    
    FWrite(writefile,(void*)fontimage_data,sizeof(u32) * width * height);
    
#if 0
    
    WriteBMP(fontimage_data,width,height,"font_render.bmp");
    
#endif
    
    FCloseFile(writefile);
    
}

//Core GUI stuff

struct InternalGUISubmission{
    u16 to_make_window;
    u16 ind_offset = 0;
    GUIRenderMode rendermode = GUI_RENDER_SOLID;
    GUICameraMode cameramode = GUI_CAMERA_PERSPECTIVE;
    VkViewport viewport;
    VkRect2D scissor;
};

enum GUIType{
    GUITYPE_NONE = 0,
    GUITYPE_HISTOGRAM,
    GUITYPE_PROFILER,
};

struct GUIContext{
    
    GUIFont* default_font;
    
    //TODO:this should be created by the font
    VkDescriptorPool pool;
    VkDescriptorSetLayout desclayout;
    
    VkPipelineLayout pipelinelayout;
    
    VkPipeline pipeline_array[3];
    
    
    VBufferContext vert_buffer;
    VBufferContext ind_buffer;
    
    GUIVertex* vert_mptr;
    u32* ind_mptr;
    
    u16 vert_offset;
    u16 ind_offset;
    
    u16 internal_width;//of the screen
    u16 internal_height;
    
    InternalGUISubmission submit_array[1024];
    u32 submit_count = 0;
    
    VkDevice internal_device = 0;
    
    GUIRenderMode cur_rendermode;
    GUICameraMode cur_cameramode;
    
    Color back_color;
    Color front_color;
    Color title_color;
    Color text_color;
    Color textbox_color;
    Color graph_back_color;
    Color graph_bar_color;
    Color graph_bar_select_color;
    Color graph_bar_highlight_color;
    
    Color axis_x_color;
    Color axis_y_color;
    Color axis_z_color;
    
    f32 default_font_size;
    
    Matrix4b4 view_matrix;
    Matrix4b4 proj_matrix;
    
    //window as in draw bounds
    InternalGUISubmission* cur_window;
    InternalGUISubmission* t_window;
    
    GUIBoundingRect bounds_array[256];
    u32 bounds_count;
    
    f32 put_x;
    f32 put_y;
    
    f32 padding_x;
    f32 padding_y;
    
    f32 aspect_ratio;
    
    u64 internal_active_state;
    
#if _debug
    
    const s8* internal_state_string;
    
#endif
    
    f32 internal_last_h;
    
    u16 internal_mouse_x;
    u16 internal_mouse_y;
    
    u16 internal_prev_mouse_x;
    u16 internal_prev_mouse_y;
    
    u16 internal_mouse_curleft;
    u16 internal_mouse_prevleft;
    
    s8 internal_prevkeystate[256];
    s8 internal_curkeystate[256];
    logic to_restore_window;
    
    u32 combobox_options_count;
    const s8* combobox_options_array[128];
    u32 combobox_index;
    GUIVec2 combobox_pos;
    GUIDim2 combobox_dim;
    InternalGUISubmission* combobox_window;
    
    
    GUIType graph_hover;
    
    union{
        
        // profiler
        struct{
            u32 thread_index;
            DebugRecord p_record;
        };
        
        //histogram
        struct{
            const s8* graph_string_x;
            const s8* graph_string_y;
            f32 graph_value_x;
            f32 graph_value_y;
        };
        
        //scale
        struct{
            f32 start_mouse_pos_len;
            f32 start_scale;
        };
        
        //translate
        struct{
            GUIVec3 trans_dir;
            GUIVec2 prev_mouse_pos;  
        };
        
        //rotate
        struct{
            u32 rot_selected;
            Point3 rot_intersection_point;
            Quaternion start_rot;
        };
        
    };
    
};

#define _reserve_count 1024 * 4

GUIContext* gui = 0;

GUIContext* GetGUIContext(){
    return gui;
}

void SetGUIContext(GUIContext* context){
    gui = context;
}

/*
  TODO: Fix the font table. image is slightly corrupted
  Instead of using normalized coords, use unit coords
*/

logic GUIMouseClickL(){
    return gui->internal_mouse_curleft & !gui->internal_mouse_prevleft;
}

logic GUIMouseUnclickL(){
    return !gui->internal_mouse_curleft & gui->internal_mouse_prevleft;
}

logic GUIMouseDownL(){
    return gui->internal_mouse_curleft;
}

logic GUIMouseUpL(){
    return !gui->internal_mouse_curleft;
}



_ainline
void InternalPixelDimToNormalizedDim(f32 r_w,f32 r_h,f32 p_w,f32 p_h,
                                     f32* w,f32* h){
    
    auto h_width = r_w/2.0f;
    auto h_height = r_h/2.0f;
    
    *w = p_w/h_width;
    *h = p_h/h_height;
    
}

_ainline
void InternalNormalizedDimToPixelDim(f32 r_w,f32 r_h,f32 n_w,f32 n_h,
                                     f32* w,f32* h){
    
    auto h_width = r_w/2.0f;
    auto h_height = r_h/2.0f;
    
    *w = n_w * h_width;
    *h = n_h * h_height;
}

_ainline
void InternalNormalizedCoordToPixelCoord(f32 r_w,f32 r_h,f32 n_x,f32 n_y,
                                         f32* x,f32* y){
    
    auto h_width = r_w/2.0f;
    auto h_height = r_h/2.0f;
    
    *x = (n_x * h_width) + h_width;
    *y = h_height - (n_y * h_height);
}

_ainline
void InternalPixelCoordToNormalizedCoord(f32 r_w,f32 r_h,f32 p_x,f32 p_y,
                                         f32* x,f32* y){
    
    auto h_width = r_w/2.0f;
    auto h_height = r_h/2.0f;
    
    *x = (p_x - h_width)/h_width;
    *y = (h_height - p_y)/h_height;
}


void InternalSetActiveState(const s8* string){
    
    u64 token = PHashString(string);
    
#if _debug
    
    gui->internal_state_string = string;
    
#endif
    
    if((!gui->combobox_options_count) &&
       gui->internal_active_state != PHashString("GUI3DTranslate")){
        
        gui->internal_active_state = token;  
    }
    
}

void _ainline InternalGetTextDim(GUIFont* font,f32* w,f32* h,f32 scale,const s8* string = 0){
    
    auto w_width = gui->cur_window->viewport.width;
    auto w_height = gui->cur_window->viewport.height;
    
    if(w){
        
        f32 len = 1.0f;
        
        if(string){
            len = strlen(string);
        }
        
        *w = (((((f32)font->width)/95.0f)/w_width) * scale) * len;
    }
    
    if(h){
        *h = (((f32)font->height)/w_height) * scale;
    }
    
}

//NOTE: our padding is based on the font height at 1.0f
f32 _ainline InternalGetPaddingX(){
    
    auto w_width = gui->cur_window->viewport.width;
    
    auto font = gui->default_font;
    auto scale = 1.0f;
    
    return ((((f32)font->height)/w_width) * scale) * gui->padding_x;
}

f32 _ainline InternalGetPaddingY(){
    
    auto w_height = gui->cur_window->viewport.height;
    
    auto font = gui->default_font;
    auto scale = 1.0f;
    
    return ((((f32)font->height)/w_height) * scale) * gui->padding_y;
}

enum GUILayoutBehavior : u32{
    GUILAYOUT_NONE = 0,
    GUILAYOUT_STARTNEWLINE = 1,
    GUILAYOUT_NEXTNEWLINE = 2,
    GUILAYOUT_FILLWIDTH = 8,
    GUILAYOUT_OCCUPY_WIDTH = 16,
};
void _ainline InternalResetPutPos(){
    gui->put_x = -1.0f + InternalGetPaddingX();
    gui->put_y = 1.0f - InternalGetPaddingY();
    gui->internal_last_h = 0.0f;
}

void _ainline InternalNextLinePutPos(){
    gui->put_x = -1.0f + InternalGetPaddingX();
    gui->put_y -= (gui->internal_last_h + InternalGetPaddingY());
    
    gui->internal_last_h = 0.0f;
}

GUIVec2 InternalLayoutPos(GUIDim2* dim,u32 behavior = GUILAYOUT_NONE){
    
    if(behavior == GUILAYOUT_OCCUPY_WIDTH){
        
        if(gui->put_x != (-1.0f + InternalGetPaddingX())){
            InternalNextLinePutPos();
        }
        
        GUIVec2 ret = {gui->put_x - InternalGetPaddingX(),
            gui->put_y + InternalGetPaddingY()};
        
        gui->put_y -= dim->h;
        
        return ret;
    }
    
    if((behavior & GUILAYOUT_STARTNEWLINE) || ((gui->put_x + dim->w) >= 1.0f)){
        InternalNextLinePutPos();
    }
    
    f32 x = gui->put_x;
    f32 y = gui->put_y;
    
    if(behavior & GUILAYOUT_FILLWIDTH){
        dim->w = 1.0f - x - InternalGetPaddingX();
    }
    
    if(behavior & GUILAYOUT_NEXTNEWLINE){
        InternalNextLinePutPos();
    }
    
    
    if(dim->h > gui->internal_last_h){
        gui->internal_last_h = dim->h;
    }
    
    gui->put_x += dim->w + InternalGetPaddingX();
    
    return {x,y};
}
logic InternalIsWithinBounds(const GUIBoundingRect* rect){
    
    u32 mouse_x = gui->internal_mouse_x;
    u32 mouse_y = gui->internal_mouse_y;
    
    u32 start_x = rect->x;
    u32 start_y = rect->y;
    
    u32 end_x = rect->x + rect->width;
    u32 end_y = rect->y + rect->height;
    
    auto state = (mouse_x >= start_x && mouse_x <= end_x) &&
        (mouse_y >= start_y && mouse_y <= end_y);
    
    return state;
}

GUIBoundingRect* InternalPushBounds(f32 x,f32 y,f32 width,f32 height){
    
    y *= -1;
    
    width /= 2.0f;
    height /= 2.0f;
    
    const auto last_window = gui->cur_window;
    
    _kill("too many bounding rects\n",gui->bounds_count >= _arraycount(gui->bounds_array));
    
    auto rect = &gui->bounds_array[gui->bounds_count];
    gui->bounds_count++;
    
    auto half_window_width = (u16)(last_window->viewport.width/2.0f);
    auto half_window_height = (u16)(last_window->viewport.height/2.0f);
    
    auto rect_x = (u16)((x * half_window_width) + last_window->viewport.x + half_window_width);
    auto rect_y = (u16)((y * half_window_height) + last_window->viewport.y + half_window_height);
    
    auto rect_width = (u16)(width * last_window->viewport.width);
    auto rect_height = (u16)(height * last_window->viewport.height);
    
    *rect = {rect_x,rect_y,rect_width,rect_height};
    
    return rect;
}

void _ainline InternalReadBuffer(void* dst,s8** src_data_ptr,u32 size){
    memcpy(dst,*src_data_ptr,size);
    (*src_data_ptr) += size;
}

GUIFont GUICreateFont(void* src_data,VkCommandBuffer cmdbuffer,
                      VDeviceContext* vdevice,
                      VkQueue queue){
    
    s8* data = (s8*)src_data;
    
    GUIFont font = {};
    
    u32 t_width,t_height;
    
    InternalReadBuffer(&t_width,&data,sizeof(t_width));
    InternalReadBuffer(&t_height,&data,sizeof(t_height));
    
    font.width = (u16)t_width;
    font.height = (u16)t_height;
    
    auto image_data = data;
    
    //Create texture
    font.texture =
        VCreateTextureImage(vdevice,image_data,font.width,font.height,cmdbuffer,queue);
    
    if(!gui->desclayout){
        
        VDescriptorPoolSpec poolspec;
        
        VDescPushBackPoolSpec(&poolspec,
                              VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,4);
        
        gui->pool = VCreateDescriptorPool(vdevice,poolspec,0,4);
        
        VDescriptorBindingSpec bindingspec;
        
        VDescPushBackBindingSpec(&bindingspec,
                                 VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,1,
                                 VK_SHADER_STAGE_FRAGMENT_BIT);
        
        
        gui->desclayout = VCreateDescriptorSetLayout(vdevice,bindingspec);
        
    }
    
    VAllocDescriptorSetArray(vdevice,gui->pool,1,&gui->desclayout,&font.descset);
    
    VDescriptorWriteSpec writespec;
    
    VkDescriptorImageInfo image_info =
    {
        font.texture.sampler,
        font.texture.view,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
    };
    
    VDescPushBackWriteSpecImage(&writespec,font.descset,0,0,1,
                                VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,&image_info);
    
    VUpdateDescriptorSets(vdevice,writespec);
    
    return font;
}


GUIFont GUICreateFontFromFile(const s8* filepath,VkCommandBuffer cmdbuffer,
                              VDeviceContext* vdevice,VkQueue queue){
    
    GUIFont font = {};
    
    auto file = FOpenFile(filepath,F_FLAG_READONLY);
    
    u32 t_width,t_height;
    
    FRead(file,(s8*)&t_width,sizeof(t_width));
    FRead(file,(s8*)&t_height,sizeof(t_height));
    
    font.width = (u16)t_width;
    font.height = (u16)t_height;
    
    auto image_data = (s8*)alloc(sizeof(u32) * font.width * font.height);
    
    FRead(file,image_data,sizeof(u32) * font.width * font.height);
    
    //Create texture
    font.texture =
        VCreateTextureImage(vdevice,image_data,font.width,font.height,cmdbuffer,queue);
    
    unalloc(image_data);
    
    if(!gui->desclayout){
        
        VDescriptorPoolSpec poolspec;
        
        VDescPushBackPoolSpec(&poolspec,
                              VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,4);
        
        gui->pool = VCreateDescriptorPool(vdevice,poolspec,0,4);
        
        VDescriptorBindingSpec bindingspec;
        
        VDescPushBackBindingSpec(&bindingspec,
                                 VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,1,
                                 VK_SHADER_STAGE_FRAGMENT_BIT);
        
        
        gui->desclayout = VCreateDescriptorSetLayout(vdevice,bindingspec);
        
    }
    
    VAllocDescriptorSetArray(vdevice,gui->pool,1,&gui->desclayout,&font.descset);
    
    VDescriptorWriteSpec writespec;
    
    VkDescriptorImageInfo image_info =
    {
        font.texture.sampler,
        font.texture.view,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
    };
    
    VDescPushBackWriteSpecImage(&writespec,font.descset,0,0,1,
                                VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,&image_info);
    
    VUpdateDescriptorSets(vdevice,writespec);
    
    FCloseFile(file);
    
    return font;
}

void InitInternalComponents(VDeviceContext* vdevice,WWindowContext* window,
                            VkRenderPass renderpass,u32 vertexbinding_no){
    
    if(gui->pipeline_array[0]){
        return;
    }
    
    VkPushConstantRange range = {VK_SHADER_STAGE_VERTEX_BIT,0,sizeof(Matrix4b4)};
    
    gui->pipelinelayout = VCreatePipelineLayout(vdevice,&gui->desclayout,1,&range,1);
    
    VkDynamicState dynamicstate_array[] = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };
    
    //solid
    {
        VGraphicsPipelineSpec pipelinespec;
        
        VPushBackShaderPipelineSpec(&pipelinespec,&m_gui_vert_spv[0],
                                    sizeof(m_gui_vert_spv),VK_SHADER_STAGE_VERTEX_BIT);
        
        VPushBackShaderPipelineSpec(&pipelinespec,&m_gui_frag_spv[0],
                                    sizeof(m_gui_frag_spv),VK_SHADER_STAGE_FRAGMENT_BIT);
        
        VPushBackVertexSpecDesc(&pipelinespec,vertexbinding_no,sizeof(GUIVertex),
                                VK_VERTEX_INPUT_RATE_VERTEX);
        
        VPushBackVertexSpecAttrib(&pipelinespec,vertexbinding_no,VK_FORMAT_R32G32B32_SFLOAT,
                                  sizeof(GUIVertex::pos));
        
        VPushBackVertexSpecAttrib(&pipelinespec,vertexbinding_no,VK_FORMAT_R32G32_SFLOAT,
                                  sizeof(GUIVertex::uv));
        
        VPushBackVertexSpecAttrib(&pipelinespec,vertexbinding_no,VK_FORMAT_R32G32B32A32_SFLOAT,
                                  sizeof(GUIVertex::color));
        
        VGenerateGraphicsPipelineSpec(&pipelinespec,
                                      VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST ,
                                      VK_POLYGON_MODE_FILL,
                                      VK_CULL_MODE_NONE,VK_FRONT_FACE_CLOCKWISE,
                                      window->width,window->height,gui->pipelinelayout,renderpass);
        
        VEnableDynamicStateGraphicsPipelineSpec(&pipelinespec,
                                                &dynamicstate_array[0],_arraycount(dynamicstate_array));
        
        VCreateGraphicsPipelineArray(vdevice,0,&pipelinespec,1,
                                     &gui->pipeline_array[GUI_RENDER_SOLID]);  
    }
    
    //line
    {
        VGraphicsPipelineSpec pipelinespec;
        
        VPushBackShaderPipelineSpec(&pipelinespec,&m_gui_vert_spv[0],
                                    sizeof(m_gui_vert_spv),VK_SHADER_STAGE_VERTEX_BIT);
        
        VPushBackShaderPipelineSpec(&pipelinespec,&m_gui_frag_spv[0],
                                    sizeof(m_gui_frag_spv),VK_SHADER_STAGE_FRAGMENT_BIT);
        
        VPushBackVertexSpecDesc(&pipelinespec,vertexbinding_no,sizeof(GUIVertex),
                                VK_VERTEX_INPUT_RATE_VERTEX);
        
        VPushBackVertexSpecAttrib(&pipelinespec,vertexbinding_no,VK_FORMAT_R32G32B32_SFLOAT,
                                  sizeof(GUIVertex::pos));
        
        VPushBackVertexSpecAttrib(&pipelinespec,vertexbinding_no,VK_FORMAT_R32G32_SFLOAT,
                                  sizeof(GUIVertex::uv));
        
        VPushBackVertexSpecAttrib(&pipelinespec,vertexbinding_no,VK_FORMAT_R32G32B32A32_SFLOAT,
                                  sizeof(GUIVertex::color));
        
        
        VGenerateGraphicsPipelineSpec(&pipelinespec,
                                      VK_PRIMITIVE_TOPOLOGY_LINE_LIST ,
                                      VK_POLYGON_MODE_FILL,
                                      VK_CULL_MODE_NONE,VK_FRONT_FACE_CLOCKWISE,
                                      window->width,window->height,gui->pipelinelayout,renderpass);
        
        VEnableDynamicStateGraphicsPipelineSpec(&pipelinespec,
                                                &dynamicstate_array[0],_arraycount(dynamicstate_array));
        
        VCreateGraphicsPipelineArray(vdevice,0,&pipelinespec,1,
                                     &gui->pipeline_array[GUI_RENDER_LINE]);  
    }
    
    //font
    {
        VGraphicsPipelineSpec pipelinespec;
        
        VPushBackShaderPipelineSpec(&pipelinespec,&m_gui_vert_spv[0],
                                    sizeof(m_gui_vert_spv),VK_SHADER_STAGE_VERTEX_BIT);
        
        VPushBackShaderPipelineSpec(&pipelinespec,&m_gui_tex_frag_spv[0],
                                    sizeof(m_gui_tex_frag_spv),VK_SHADER_STAGE_FRAGMENT_BIT);
        
        VPushBackVertexSpecDesc(&pipelinespec,vertexbinding_no,sizeof(GUIVertex),
                                VK_VERTEX_INPUT_RATE_VERTEX);
        
        VPushBackVertexSpecAttrib(&pipelinespec,vertexbinding_no,VK_FORMAT_R32G32B32_SFLOAT,
                                  sizeof(GUIVertex::pos));
        
        VPushBackVertexSpecAttrib(&pipelinespec,vertexbinding_no,VK_FORMAT_R32G32_SFLOAT,
                                  sizeof(GUIVertex::uv));
        
        VPushBackVertexSpecAttrib(&pipelinespec,vertexbinding_no,VK_FORMAT_R32G32B32A32_SFLOAT,
                                  sizeof(GUIVertex::color));
        
        VGenerateGraphicsPipelineSpec(&pipelinespec,
                                      VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST ,
                                      VK_POLYGON_MODE_FILL,
                                      VK_CULL_MODE_NONE,VK_FRONT_FACE_CLOCKWISE,
                                      window->width,window->height,gui->pipelinelayout,renderpass);
        
        VEnableColorBlendTransparency(&pipelinespec);
        
        VEnableDynamicStateGraphicsPipelineSpec(&pipelinespec,
                                                &dynamicstate_array[0],_arraycount(dynamicstate_array));
        
        VCreateGraphicsPipelineArray(vdevice,0,&pipelinespec,1,
                                     &gui->pipeline_array[GUI_RENDER_TEXT]);  
    }
    
}

void GUIInit(VDeviceContext* vdevice,WWindowContext* window,
             VkRenderPass renderpass,VkQueue queue,VkCommandBuffer cmdbuffer,
             u32 vertexbinding_no,GUIFont* fonthandle){
    
    _kill("GUI already init\n",gui);
    
    gui = (GUIContext*)alloc(sizeof(GUIContext));
    
    memset(gui,0,sizeof(GUIContext));
    
    gui->back_color = DarkSlateGray;
    gui->front_color = Red;
    gui->title_color = Red;
    gui->text_color = DarkRed;
    gui->textbox_color = White;
    gui->graph_back_color = DarkGray;
    gui->graph_bar_color = Yellow;
    gui->graph_bar_select_color = White;
    gui->graph_bar_highlight_color = Red;
    gui->default_font_size = 1.0f;
    gui->put_x = -1.0f;
    gui->put_y = 1.0f;
    gui->padding_x = 0.5f;
    gui->padding_y = 0.8f;
    
    gui-> axis_x_color = Red;
    gui-> axis_y_color = Green;
    gui-> axis_z_color = Yellow;
    
    
    gui->internal_device = vdevice->device;
    
    gui->internal_width = window->width;
    gui->internal_height = window->height;
    
    if(!fonthandle){
        
        if(!gui->default_font){
            gui->default_font = (GUIFont*)alloc(sizeof(GUIFont));
            *gui->default_font = GUICreateFont(&Ubuntu_B_fbmp[0],cmdbuffer,vdevice,queue);
        }
        fonthandle = gui->default_font;
    }
    
    InitInternalComponents(vdevice,window,renderpass,vertexbinding_no);
    
    
    gui->vert_buffer =
        VCreateStaticVertexBuffer(vdevice,sizeof(GUIVertex) * _reserve_count,0,false);
    
    gui->ind_buffer = VCreateStaticIndexBuffer(vdevice,sizeof(u32) * _reserve_count,false);
}


_ainline InternalGUISubmission* InternalGetLastSubmission(){
    
    return &gui->submit_array[gui->submit_count - 1];
}

enum WindowClipState : u16{
    WINDOWSTATE_NONE = 0,
    WINDOWSTATE_MAINWINDOW = 1,
    WINDOWSTATE_SUBWINDOW = 3,
};

void _ainline GUIInternalSubWindow(GUIVec2* pos,GUIDim2* dim){
    
    auto x = pos->x;
    auto y = pos->y;
    
    auto width = dim->w;
    auto height = dim->h;
    
    auto c_x = gui->cur_window->viewport.x;
    auto c_y = gui->cur_window->viewport.y;
    auto c_width = gui->cur_window->viewport.width;
    auto c_height = gui->cur_window->viewport.height;
    
    auto h_c_width = c_width/2.0f;
    auto h_c_height = c_height/2.0f;
    
    InternalPixelCoordToNormalizedCoord(gui->internal_width,gui->internal_height,
                                        c_x + h_c_width + (x * h_c_width),
                                        c_y + (h_c_height - (y * h_c_height)),
                                        &pos->x,&pos->y);
    
    InternalPixelDimToNormalizedDim(gui->internal_width,gui->internal_height,
                                    width * h_c_width,height * h_c_height,&dim->w,&dim->h);
}

void  GUIInternalMakeSubmission(
WindowClipState to_make_window = WINDOWSTATE_NONE,
GUIVec2 pos = {},GUIDim2 dim = {}){
    
    
    f32 x,y,width,height;
    
    if(gui->to_restore_window && to_make_window != WINDOWSTATE_SUBWINDOW){
        gui->cur_window = gui->t_window;
        x = gui->cur_window->viewport.x;
        y = gui->cur_window->viewport.y;
        width = gui->cur_window->viewport.width;
        height = gui->cur_window->viewport.height;
        gui->to_restore_window = false;
        to_make_window = WINDOWSTATE_SUBWINDOW;
        goto  skip_translation;
    }
    
    if(to_make_window == WINDOWSTATE_SUBWINDOW){
        
        gui->to_restore_window = true;
        GUIInternalSubWindow(&pos,&dim);
        
        gui->t_window = gui->cur_window;
    }
    InternalNormalizedCoordToPixelCoord((f32)gui->internal_width,(f32)gui->internal_height,
                                        pos.x,pos.y,&x,&y);
    
    InternalNormalizedDimToPixelDim((f32)gui->internal_width,(f32)gui->internal_height,
                                    dim.w,dim.h,&width,&height);
    skip_translation:
    
    gui->submit_array[gui->submit_count] =
    {to_make_window,gui->ind_offset,gui->cur_rendermode,gui->cur_cameramode,
        {x,y,width,height,0,1},
        {{(s32)x,(s32)y},{(u32)width,(u32)height}}
    };
    
    gui->submit_count++;
    
    if(to_make_window & WINDOWSTATE_MAINWINDOW){
        
        _kill("wrong input\n",(u32)height == 0 || (u32)width == 0);
        
        gui->cur_window = InternalGetLastSubmission();
    }
    
}

#define _cellwidth 1.0f/95.0f

void InternalGUIDrawRect(f32 x,f32 y,f32 width,f32 height,Color color){
    
#define _blanktexcoord 94.0f/95.0f
    
    y *= -1;
    
    u32 curvert = gui->vert_offset;
    u32 curindex = gui->ind_offset;
    
    gui->vert_mptr[curvert] =
    {{x,y},{_blanktexcoord,-1.0f}, { color.R, color.G, color.B,color.A }};
    
    curvert++;
    
    gui->vert_mptr[curvert] =
    {{x,y + height},{_blanktexcoord, 0.0f}, { color.R, color.G, color.B,color.A }};
    
    curvert++;
    
    gui->vert_mptr[curvert] =
    {{x + width,y + height},{_blanktexcoord + _cellwidth, 0.0f}, { color.R, color.G, color.B,color.A }};
    
    curvert++;
    
    gui->vert_mptr[curvert] =
    {{x + width,y},{_blanktexcoord + _cellwidth, -1.0f}, { color.R, color.G, color.B,color.A }};
    
    curvert++;
    
    gui->ind_mptr[curindex] = curvert - 4;
    curindex++;
    
    gui->ind_mptr[curindex] = curvert - 3;
    curindex++;
    
    gui->ind_mptr[curindex] = curvert - 2;
    curindex++;
    
    gui->ind_mptr[curindex] = curvert - 2;
    curindex++;
    
    gui->ind_mptr[curindex] = curvert - 1;
    curindex++;
    
    gui->ind_mptr[curindex] = curvert - 4;
    curindex++;
    
    gui->vert_offset = curvert;
    gui->ind_offset = curindex;
    
    _kill("gui vertex overflow", curvert > _reserve_count);
    _kill("gui index overflow",curindex > _reserve_count);
    
#undef _blanktexcoord
}

void _ainline InternalGetWindowTextDim(GUIFont* font,f32* w,f32* h,f32 scale,
                                       const s8* string = 0){
    
    auto w_width = (f32)gui->internal_width;
    auto w_height = (f32)gui->internal_height;
    
    if(w){
        
        f32 len = 1.0f;
        
        if(string){
            len = strlen(string);
        }
        
        *w = (((((f32)font->width)/95.0f)/w_width) * scale) * len;
    }
    
    if(h){
        *h = (((f32)font->height)/w_height) * scale;
    }  
}

void InternalDrawString(const s8* string,f32 x,f32 y,f32 scale,GUIFont* font,Color color){
    
    f32 width;
    f32 height;
    
    InternalGetTextDim(font,&width,&height,scale);
    
    auto o_x = x;
    
    for(u32 i = 0; i < strlen(string);i++){
        
        s8 code = string[i];
        
        if(code == ' ' || code == '\t'){
            x += width;
            continue;
        }
        
        if(code == '\n'){
            x = o_x;
            y -= height;
            continue;
        }
        
        code -= 33;
        
        InternalGUIDrawRect(x,y,width,height,color);
        
        auto count = gui->vert_offset;
        
        gui->vert_mptr[count - 4].uv[0] = code * _cellwidth;
        gui->vert_mptr[count - 4].uv[1] = -1.0f;
        
        gui->vert_mptr[count - 3].uv[0] = code * _cellwidth;
        gui->vert_mptr[count - 3].uv[1] = 0.0f;
        
        gui->vert_mptr[count - 2].uv[0] = code * _cellwidth + _cellwidth;
        gui->vert_mptr[count - 2].uv[1] = 0.0f;
        
        gui->vert_mptr[count - 1].uv[0] = code * _cellwidth + _cellwidth;
        gui->vert_mptr[count - 1].uv[1] = -1.0f;
        
        
        x += width;
    }
    
}

void GUIUpdate(WWindowContext* window,KeyboardState* keyboardstate,
               MouseState* mousestate,Matrix4b4 view,Matrix4b4 proj){
    
    gui->internal_width = window->width;
    gui->internal_height = window->height;
    
    gui->aspect_ratio = (f32)gui->internal_width/(f32)gui->internal_height;
    
    gui->internal_prev_mouse_x = gui->internal_mouse_x;
    gui->internal_prev_mouse_y = gui->internal_mouse_y;
    
    gui->internal_mouse_x = mousestate->x;
    gui->internal_mouse_y = mousestate->y;
    
    gui->internal_mouse_curleft = mousestate->curstate[MOUSEBUTTON_LEFT];
    gui->internal_mouse_prevleft = mousestate->prevstate[MOUSEBUTTON_LEFT];
    
    memcpy(gui->internal_prevkeystate,keyboardstate->prevkeystate,
           sizeof(gui->internal_prevkeystate));
    
    memcpy(gui->internal_curkeystate,keyboardstate->curkeystate,
           sizeof(gui->internal_curkeystate));
    
    gui->view_matrix = view;
    gui->proj_matrix = proj;
    
}

logic GUIButton(const s8* string){
    
    GUISetRenderMode(GUI_RENDER_SOLID);
    GUISetCameraMode(GUI_CAMERA_NONE);
    
    GUIInternalMakeSubmission();
    
    f32 string_width;
    f32 string_height;
    
    InternalGetTextDim(gui->default_font,&string_width,&string_height,gui->default_font_size,
                       string);
    
    GUIDim2 dim = {string_width,string_height};
    
    auto text_padding = dim.h * 0.125f;
    
    dim.w += (text_padding * 2.0f);
    
    auto pos = InternalLayoutPos(&dim);
    
    InternalGUIDrawRect(pos.x,pos.y,dim.w,dim.h,gui->front_color);
    
    GUISetRenderMode(GUI_RENDER_TEXT);
    
    GUIInternalMakeSubmission();
    
    InternalDrawString(string,pos.x + text_padding,pos.y,gui->default_font_size,
                       gui->default_font,gui->text_color);
    
    return InternalIsWithinBounds(InternalPushBounds(pos.x,pos.y,dim.w,dim.h)) &&
        GUIMouseClickL();
}

void GUIString(const s8* string,u32 behavior){
    
    GUISetRenderMode(GUI_RENDER_TEXT);
    GUISetCameraMode(GUI_CAMERA_NONE);
    
    GUIInternalMakeSubmission();
    
    f32 string_width;
    f32 string_height;
    
    InternalGetTextDim(gui->default_font,&string_width,&string_height,gui->default_font_size,
                       string);
    
    GUIDim2 dim = {string_width,string_height};
    
    auto pos = InternalLayoutPos(&dim,behavior);  
    
    InternalDrawString(string,pos.x,pos.y,gui->default_font_size,gui->default_font,gui->text_color);
}

void GUIString(const s8* string){
    GUIString(string,GUILAYOUT_NONE);
}

void GUIInternalEndWindow(){
    
#if 0
    
    auto sub = &gui->submit_array[gui->submit_count - 1];
    
#else
    
    //    _kill("TODO: implement this\n",1);
    
#endif
    
    //TODO: resize as needed
}

void GUISetRenderMode(GUIRenderMode rendermode){
    gui->cur_rendermode = rendermode;
}

void GUISetCameraMode(GUICameraMode cameramode){
    gui->cur_cameramode = cameramode;
}

GUIVec2 InternalNormalizedClamp(GUIVec2 vec){
    
    if(vec.x > 1.0f){
        vec.x = 1.0f;
    }
    
    if(vec.x < -1.0f){
        vec.x = -1.0f;
    }
    
    if(vec.y > 1.0f){
        vec.y = 1.0f;
    }
    
    if(vec.y < -1.0f){
        vec.y = -1.0f;
    }
    
    return vec;
}

void GUIInternalBeginWindow(const s8* title,GUIVec2* pos,GUIDim2* dim){
    
    gui->to_restore_window = false;
    
    auto token = PHashString(title);
    
    //we have a default width, the height changes as we add stuff
    GUIDim2 d_dim = {GUIDEFAULT_W,GUIDEFAULT_H};
    GUIVec2 d_pos = {GUIDEFAULT_X,GUIDEFAULT_Y};
    
    GUIDim2* dim_ptr = &d_dim;
    GUIVec2* pos_ptr = &d_pos;
    
    if(dim){
        dim_ptr = dim;
    }
    
    if(pos){
        pos_ptr = pos;
    }
    
    GUIDim2 tdim = *dim_ptr;
    tdim.h *= gui->aspect_ratio;
    
    //pushback window
    GUISetRenderMode(GUI_RENDER_SOLID);
    GUISetCameraMode(GUI_CAMERA_NONE);
    
    GUIInternalMakeSubmission(WINDOWSTATE_MAINWINDOW,*pos_ptr,tdim);
    
    InternalGUIDrawRect(-1.0f,1.0f,2.0f,2.0f,gui->back_color);
    
    f32 titlebar_width;
    f32 titlebar_height;
    
    InternalGetTextDim(gui->default_font,&titlebar_width,&titlebar_height,
                       gui->default_font_size,title);
    
    tdim = {0.1f,titlebar_height};
    
    InternalResetPutPos();
    
    InternalLayoutPos(&tdim,GUILAYOUT_OCCUPY_WIDTH);
    
    GUIVec2 title_pos = {-1.0f,1.0f};
    
    InternalGUIDrawRect(title_pos.x,title_pos.y,2.0f,titlebar_height,gui->title_color);
    
    //Window dragging
    {
        
        if(InternalIsWithinBounds(InternalPushBounds(-1.0f,1.0f,2.0f,titlebar_height)) &&
           GUIMouseDownL()){
            InternalSetActiveState(title);
            
        }
        
        if(GUIMouseUnclickL() && gui->internal_active_state == token){
            gui->internal_active_state = false;
        }
        
        if(gui->internal_active_state == token){
            
            Vector2 prev_mouse_pos =
            {(f32)gui->internal_prev_mouse_x,(f32)gui->internal_prev_mouse_y};
            
            Vector2 mouse_pos = {(f32)gui->internal_mouse_x,(f32)gui->internal_mouse_y};
            
            InternalPixelCoordToNormalizedCoord((f32)gui->internal_width,(f32)gui->internal_height,
                                                gui->internal_prev_mouse_x,gui->internal_prev_mouse_y,
                                                &prev_mouse_pos.x,&prev_mouse_pos.y);
            
            InternalPixelCoordToNormalizedCoord((f32)gui->internal_width,(f32)gui->internal_height,
                                                mouse_pos.x,mouse_pos.y,
                                                &mouse_pos.x,&mouse_pos.y);
            
            auto dir = mouse_pos - prev_mouse_pos;
            
            if(Magnitude(dir) > 0.0f){
                
                pos_ptr->x += dir.x;
                pos_ptr->y += dir.y;
                
                *pos_ptr = InternalNormalizedClamp(*pos_ptr);
            }
        }
        
    }
    
    //TODO: Do window resize
    {
        
    }
    
    GUISetRenderMode(GUI_RENDER_TEXT);
    
    GUIInternalMakeSubmission();
    
    titlebar_width = -1.0f + ((2.0f - titlebar_width)/2.0f);
    
    InternalDrawString(title,titlebar_width,1.0f,gui->default_font_size,gui->default_font,
                       gui->text_color);
    
}

void GUIBeginWindow(const s8* title,GUIVec2* pos,GUIDim2* dim){
    GUIInternalEndWindow();
    GUIInternalBeginWindow(title,pos,dim);
}

void GUIBegin(const s8* title,GUIVec2* pos,GUIDim2* dim){
    
    gui->internal_last_h = 0.0f;
    gui->vert_offset = 0;
    gui->ind_offset = 0;
    gui->submit_count = 0;
    gui->bounds_count = 0;
    
    vkMapMemory(gui->internal_device,gui->vert_buffer.memory,0,gui->vert_buffer.size,0,
                (void**)&gui->vert_mptr);
    
    vkMapMemory(gui->internal_device,gui->ind_buffer.memory,0,gui->ind_buffer.size,0,
                (void**)&gui->ind_mptr);
    
    if(title){
        GUIInternalBeginWindow(title,pos,dim);  
    }
    
}

void InternalGUIActiveComboBox(){
    
    if(gui->internal_active_state && (gui->combobox_options_count > 1)){
        
        gui->cur_window = gui->combobox_window;
        
        GUISetRenderMode(GUI_RENDER_SOLID);
        GUISetCameraMode(GUI_CAMERA_NONE);
        
        GUIInternalMakeSubmission();
        
        auto pos = gui->combobox_pos;
        auto dim = gui->combobox_dim;
        
        pos.y -= gui->combobox_dim.h;
        dim.h = gui->combobox_dim.h * (gui->combobox_options_count - 1);
        
        InternalGUIDrawRect(pos.x,pos.y,dim.w,dim.h,gui->textbox_color);
        
        GUISetRenderMode(GUI_RENDER_TEXT);
        GUIInternalMakeSubmission(WINDOWSTATE_SUBWINDOW,pos,dim);
        
        f32 t_y = 1.0f;
        f32 t_h;
        
        InternalGetTextDim(gui->default_font,0,&t_h,gui->default_font_size);
        
        for(u32 i = 0; i < gui->combobox_options_count; i++){
            
            if(i != gui->combobox_index){
                InternalDrawString(gui->combobox_options_array[i],-1.0f,t_y,gui->default_font_size,
                                   gui->default_font,gui->text_color);
                t_y -= t_h;	
            }
            
            
        }
        
    }
    
}


GUIVec2 GUIMouseCoordToScreenCoord(){
    
    GUIVec2 coord;
    
    InternalPixelCoordToNormalizedCoord((f32)gui->internal_width,(f32)gui->internal_height,
                                        (f32)gui->internal_mouse_x,(f32)gui->internal_mouse_y,&coord.x,&coord.y);
    
    return coord;
}

void InternalGUIActiveHistogram(){
    
    if(gui->graph_hover == GUITYPE_HISTOGRAM){
        
        gui->to_restore_window = false;
        
        s8 string1[256] = {};
        s8 string2[256] = {};
        
        sprintf(&string1[0]," %s : %f",gui->graph_string_x,gui->graph_value_x);
        sprintf(&string2[0]," %s : %f",gui->graph_string_y,gui->graph_value_y);
        
        s8* longest_string = &string1[0];
        
        if(strlen(string2) > strlen(string1)){
            longest_string = &string2[0];
        }
        
        GUIDim2 dim;
        GUIVec2 pos = GUIMouseCoordToScreenCoord();
        
        InternalGetWindowTextDim(gui->default_font,&dim.w,&dim.h,gui->default_font_size,
                                 longest_string);
        
        dim.h *= 2.0f;
        
        GUISetRenderMode(GUI_RENDER_SOLID);
        GUISetCameraMode(GUI_CAMERA_NONE);
        
        GUIInternalMakeSubmission(WINDOWSTATE_MAINWINDOW,pos,dim);
        
        InternalGUIDrawRect(-1.0f,1.0f,2.0f,2.0f,gui->back_color);
        InternalGUIDrawRect(-1.0f,1.0f,2.0f,2.0f,gui->back_color);
        
        GUISetRenderMode(GUI_RENDER_TEXT);
        GUIInternalMakeSubmission();
        
        InternalDrawString(&string1[0],-1.0f,1.0f,gui->default_font_size,gui->default_font,
                           gui->text_color);
        InternalDrawString(&string2[0],-1.0f,0.0f,gui->default_font_size,gui->default_font,
                           gui->text_color);
    }
    
}

void InternalGUIActiveProfiler(){
    
    if(gui->graph_hover == GUITYPE_PROFILER){
        
        gui->to_restore_window = false;
        
        s8 string_array[6][256] = {};
        
        s8* longest_string = 0;
        u32 longest_len = 0;
        
        sprintf(&string_array[0][0],"  THREAD %d",gui->thread_index);
        sprintf(&string_array[1][0],"  FILE %s",gui->p_record.file);
        sprintf(&string_array[2][0],"  FUNCTION %s",gui->p_record.function);
        sprintf(&string_array[3][0],"  LINE %d",gui->p_record.line);
        sprintf(&string_array[4][0],"  MS %f",gui->p_record.timelen);
        sprintf(&string_array[5][0],"  CYCLES %llu",gui->p_record.cyclelen);
        
        for(u32 i = 0; i < 6; i++){
            
            s8* string = &string_array[i][0];
            u32 len = strlen(string);
            
            if(len > longest_len){
                longest_len = len;
                longest_string = string;
            }
        }
        
        GUIDim2 dim;
        GUIVec2 pos = GUIMouseCoordToScreenCoord();
        
        InternalGetWindowTextDim(gui->default_font,&dim.w,&dim.h,gui->default_font_size,
                                 longest_string);
        
        dim.h *= 6.0f;
        
        GUISetRenderMode(GUI_RENDER_SOLID);
        GUISetCameraMode(GUI_CAMERA_NONE);
        
        GUIInternalMakeSubmission(WINDOWSTATE_MAINWINDOW,pos,dim);
        
        InternalGUIDrawRect(-1.0f,1.0f,2.0f,2.0f,gui->back_color);
        
        GUISetRenderMode(GUI_RENDER_TEXT);
        GUIInternalMakeSubmission();
        
        InternalGetTextDim(gui->default_font,&dim.w,&dim.h,gui->default_font_size);
        
        f32 start_y = 1.0f;
        
        for(u32 i = 0; i < 6; i++){
            
            InternalDrawString(&string_array[i][0],-1.0f,start_y,gui->default_font_size,gui->default_font,
                               gui->text_color);
            
            start_y -= dim.h;
        }
        
        
    }
    
}

void GUIEnd(){
    
    if(gui->put_y < -1.0f){
        //TODO: resize the window
    }
    
    InternalGUIActiveComboBox();
    InternalGUIActiveHistogram();
    InternalGUIActiveProfiler();
    
    GUIInternalEndWindow();
    vkUnmapMemory(gui->internal_device,gui->vert_buffer.memory);
    vkUnmapMemory(gui->internal_device,gui->ind_buffer.memory);
}


void GUIDraw(VkCommandBuffer cmdbuffer){
    
    TIMEBLOCK(Lime);
    
    vkCmdBindDescriptorSets(cmdbuffer,VK_PIPELINE_BIND_POINT_GRAPHICS,
                            gui->pipelinelayout,0,1,&gui->default_font->descset,0,0);
    
    VkDeviceSize offset = 0;
    
    vkCmdBindVertexBuffers(cmdbuffer,gui->vert_buffer.attrib,1,&gui->vert_buffer.buffer,
                           &offset);
    
    vkCmdBindIndexBuffer(cmdbuffer,gui->ind_buffer.buffer,0,VK_INDEX_TYPE_UINT32);
    
    VkViewport viewport = {};
    VkRect2D scissor = {};
    
    for(u32 i = 0; i < gui->submit_count; i++){
        
        auto sub = &gui->submit_array[i];
        
        if(sub->to_make_window & WINDOWSTATE_MAINWINDOW){
            viewport = sub->viewport;
            scissor = sub->scissor;
        }
        
        Matrix4b4 camera;camera = IdentityMatrix4b4();
        
        if(sub->cameramode == GUI_CAMERA_NONE){
            camera = IdentityMatrix4b4();
        }
        
        else{
            
            camera =
                Transpose(gui->proj_matrix * gui->view_matrix);
        }
        
        if(sub->rendermode == GUI_RENDER_LINE ||
           sub->cameramode == GUI_CAMERA_PERSPECTIVE){
            
            viewport = {0.0f,0.0f,(f32)gui->internal_width,(f32)gui->internal_height,0.0f,1.0f};
            scissor = {{},{gui->internal_width,gui->internal_height}};
        }
        
        
        vkCmdBindPipeline(cmdbuffer,VK_PIPELINE_BIND_POINT_GRAPHICS,
                          gui->pipeline_array[sub->rendermode]);
        
        vkCmdSetViewport(cmdbuffer,0,1,&viewport);
        vkCmdSetScissor(cmdbuffer,0,1,&scissor);
        
        
        vkCmdPushConstants(cmdbuffer,gui->pipelinelayout,VK_SHADER_STAGE_VERTEX_BIT,0,
                           sizeof(camera),&camera);
        
        auto index_offset = sub->ind_offset;
        
        u32 index_count;
        
        if((i + 1) < gui->submit_count){
            auto next_sub = &gui->submit_array[i + 1];
            index_count = next_sub->ind_offset - index_offset;
        }
        
        else{
            index_count = gui->ind_offset - index_offset;
        }
        
        vkCmdDrawIndexed(cmdbuffer,index_count,1,index_offset,0,0);  
        
    }
    
}

logic GUITextBox(const s8* label,const s8* buffer,logic fill_w,GUIDim2 dim){
    
    u32 options = GUILAYOUT_NONE;
    
    if(fill_w){
        options |= GUILAYOUT_STARTNEWLINE;
    }
    
    GUIString(label,options);
    
    auto in_buffer = (s8*)buffer;
    
    GUISetRenderMode(GUI_RENDER_SOLID);
    GUISetCameraMode(GUI_CAMERA_NONE);
    
    GUIInternalMakeSubmission();
    
    if((s32)dim.h == -1){
        InternalGetTextDim(gui->default_font,0,&dim.h,gui->default_font_size);
    }
    
    auto token = PHashString(label);
    
    options = GUILAYOUT_NONE;
    
    if(fill_w){
        options |= GUILAYOUT_FILLWIDTH;
    }
    
    auto pos = InternalLayoutPos(&dim,options);
    
    auto w = dim.w;
    auto h = dim.h;
    
    InternalGUIDrawRect(pos.x,pos.y,w,h,gui->textbox_color);
    
    if(gui->internal_active_state == token){
        
        if((GUIMouseClickL() || gui->internal_curkeystate[KCODE_KEY_ENTER]) &&
           gui->internal_active_state == token){
            gui->internal_active_state = false;
            return true;  
        }
        
        u32 len = strlen(buffer);
        
        KeyboardState state;
        
        memcpy(state.prevkeystate,gui->internal_prevkeystate,sizeof(gui->internal_prevkeystate));
        memcpy(state.curkeystate,gui->internal_curkeystate,sizeof(gui->internal_curkeystate));
        
        //TODO: handle holding down space and backspace
        
        if(IsKeyPressed(&state,KCODE_KEY_SPACE)){
            in_buffer[len] = 32;
            len++;
            return false;
        }
        
        if(IsKeyPressed(&state,KCODE_KEY_BACKSPACE) && len){
            len--;
            in_buffer[len] = 0;
            return false;
        }
        
        for(u32 i = 0; i < _arraycount(gui->internal_curkeystate); i++){
            
            if(IsKeyPressed(&state,i)){
                
                auto c = WKeyCodeToASCII(i);
                
                if(PIsVisibleChar(c)){
                    in_buffer[len] = c;
                    len++;
                }
                
            }
            
        }
        
    }
    
    if(InternalIsWithinBounds(InternalPushBounds(pos.x,pos.y,w,h)) && GUIMouseClickL()){
        
        InternalSetActiveState(label);
        
        //temp behavior
        auto len = strlen(buffer);
        memset(&in_buffer[0],0,len);
    }
    
    
    GUISetRenderMode(GUI_RENDER_TEXT);
    
    GUIInternalMakeSubmission(WINDOWSTATE_SUBWINDOW,pos,{w,h});
    
    InternalDrawString(buffer,-1.0f,1.0f,gui->default_font_size,gui->default_font,gui->text_color);
    return false;
}


logic GUITextField(const s8* label,const s8* buffer,logic fill_w,f32 w){
    
    return GUITextBox(label,buffer,fill_w,{w,-1.0f});
}


logic GUIComboBox(const s8* label,const s8** options_array,u32 options_count,u32* index,
                  logic fill_w){
    
    u32 options = GUILAYOUT_NONE;
    
    if(fill_w){
        options |= GUILAYOUT_STARTNEWLINE;
    }
    
    GUIString(label,options);
    
    GUISetRenderMode(GUI_RENDER_SOLID);
    GUISetCameraMode(GUI_CAMERA_NONE);
    
    GUIInternalMakeSubmission();  
    
    auto token = PHashString(label);
    
    const s8* longest_string = 0;
    u32 len = 0;
    
    for(u32 i = 0; i < options_count; i++){
        
        u32 s_len = strlen(options_array[i]);
        
        if(s_len > len){
            len = s_len;
            longest_string = options_array[i];
        }
        
    }
    
    f32 w;
    f32 h;
    
    InternalGetTextDim(gui->default_font,&w,&h,gui->default_font_size,longest_string);
    
    GUIDim2 dim = {w,h};
    
    options = GUILAYOUT_NONE;
    
    if(fill_w){
        options |= GUILAYOUT_FILLWIDTH;
    }
    
    auto pos = InternalLayoutPos(&dim,options);
    
    w = dim.w;
    h = dim.h;
    
    InternalGUIDrawRect(pos.x,pos.y,w,h,gui->textbox_color);
    
    if(GUIMouseClickL()){
        
        if(gui->internal_active_state == token){
            
            gui->combobox_options_count = 0;
            gui->internal_active_state = 0;
            
            //just adds the bounding box logic for the options, doesn't actually draw them
            
            auto tpos = pos;
            
            for(u32 i = 0; i < options_count; i++){
                
                if(*index == i){
                    continue;
                }
                
                tpos.y -= h;
                
                if(InternalIsWithinBounds(InternalPushBounds(tpos.x,tpos.y,w,h))){
                    *index = i;
                    return true;
                }
                
            }
        }
        
        else if(InternalIsWithinBounds(InternalPushBounds(pos.x,pos.y,w,h))){
            
            gui->combobox_index = *index;
            gui->combobox_pos = pos;
            gui->combobox_dim = {w,h};
            gui->combobox_window = gui->cur_window;
            
            InternalSetActiveState(label);
            gui->combobox_options_count = options_count;
            memcpy(&gui->combobox_options_array[0],&options_array[0],
                   options_count * sizeof(const s8*));
            
        }
        
    }
    
    
    
    GUISetRenderMode(GUI_RENDER_TEXT);
    
    GUIInternalMakeSubmission(WINDOWSTATE_SUBWINDOW,pos,{w,h});
    
    InternalDrawString(options_array[*index],-1.0f,1.0f,gui->default_font_size,gui->default_font,
                       gui->text_color);
    
    return false;
}


logic GUIHistogram(const s8* label_x,const s8* label_y,GUIVec2* data_array,u32 data_count,
                   u32* out_entry_index,f32* max,GUIDim2 dim,u32* highlight_index){
    
    logic ret = false;
    
    GUISetRenderMode(GUI_RENDER_SOLID);
    GUISetCameraMode(GUI_CAMERA_NONE);
    
    GUIInternalMakeSubmission();
    
    //auto token = PHashString(label_x) ^ PHashString(label_y);
    auto token = PHashString("GUIHistogram");
    
    auto pos = InternalLayoutPos(&dim,GUILAYOUT_STARTNEWLINE | GUILAYOUT_FILLWIDTH);
    
    auto w = dim.w;
    auto h = dim.h;
    
    InternalGUIDrawRect(pos.x,pos.y,w,h,gui->graph_back_color);
    
    GUIInternalMakeSubmission(WINDOWSTATE_SUBWINDOW,pos,{w,h});
    
    f32 highest_value = 0.0f;
    
    if(max){
        highest_value = *max;
    }
    
    logic set_active = false;
    
    for(u32 i = 0; i < data_count; i++){
        
        if(highest_value < data_array[i].y){
            highest_value = data_array[i].y;
        }
    }
    
    f32 bar_width = 2.0f/(f32)(data_count);
    
    auto x = -1.0f;
    
    gui->graph_hover = GUITYPE_NONE;
    
    for(u32 i = 0; i < data_count; i++){
        
        f32 bar_height = data_array[i].y/highest_value;
        
        Color barcolor = gui->graph_bar_color;
        
        if(highlight_index){
            
            if(i == *highlight_index){
                barcolor = gui->graph_bar_highlight_color;
            }
            
        }
        
        if(InternalIsWithinBounds(InternalPushBounds(x,(-1.0f + bar_height),bar_width,bar_height))){
            
            set_active = true;
            barcolor = gui->graph_bar_select_color;
            
            if(gui->internal_active_state == token){
                gui->graph_hover = GUITYPE_HISTOGRAM;
                gui->graph_string_x = label_x;
                gui->graph_string_y = label_y;
                gui->graph_value_x = data_array[i].x;
                gui->graph_value_y = data_array[i].y;
            }
            
            
            if(GUIMouseClickL()){
                gui->graph_hover = GUITYPE_NONE;
                *out_entry_index = i;
                ret = true;
            }
        }
        
        InternalGUIDrawRect(x,(-1.0f + bar_height),bar_width,bar_height,barcolor);
        
        x += bar_width;
    }
    
    if(max){
        *max = highest_value;
    }
    
    if(set_active){
        InternalSetActiveState("GUIHistogram");
    }
    
    else if(token == gui->internal_active_state){
        gui->internal_active_state = 0;
    }
    
    return ret;
}

struct StackEntry{
    TimeSpec start_stamp;
    f32 timelen;
    f32 y;
};

f32 InternalGUIGetStackPositionY(StackEntry* array,u32* count,
                                 const DebugRecord* record,TimeSpec initial_timestamp,
                                 f32 start_y,f32 bar_height){
    
    auto tcount = *count;
    
    for(u32 i = tcount - 1; i != (u32)-1; i--){
        
        const auto entry = &array[i];
        
        auto entry_soffset  = GetTimeDifferenceMS(initial_timestamp,entry->start_stamp);
        auto record_soffset  = GetTimeDifferenceMS(initial_timestamp,record->start_stamp);
        
        
        //Child record if is true
        if(record_soffset >= entry_soffset &&
           (record_soffset + record->timelen) <= (entry_soffset + entry->timelen)){
            
            u32 child_index = i + 1;
            
            if(child_index >= (*count)){
                (*count) += 1;
            }
            
            array[child_index] = {record->start_stamp,record->timelen,
                entry->y - bar_height};
            
            return (entry->y - bar_height);
        }
        
    }
    
    array[0] = {record->start_stamp,record->timelen,start_y};
    
    if(1 > (*count)){
        (*count) += 1;  
    }
    
    return start_y;
}

logic GUIProfileView(const s8* profilename,const DebugTable* table,GUIDim2 dim){
    
    gui->graph_hover = GUITYPE_NONE;
    
#define _upperlimit 16.0f
    
    logic ret = false;
    
    GUISetRenderMode(GUI_RENDER_SOLID);
    GUISetCameraMode(GUI_CAMERA_NONE);
    
    GUIInternalMakeSubmission();
    
    auto token = PHashString(profilename);
    
    auto pos = InternalLayoutPos(&dim,GUILAYOUT_FILLWIDTH | GUILAYOUT_STARTNEWLINE);
    
    auto w = dim.w;
    auto h = dim.h;
    
    InternalGUIDrawRect(pos.x,pos.y,w,h,gui->graph_back_color);
    
    GUIInternalMakeSubmission(WINDOWSTATE_SUBWINDOW,pos,{w,h});
    
    u32 all_count = table->thread_count * 6;
    
    f32 bar_height = ((2.0f)/(f32)(all_count));
    
    f32 start_y = 1.0f - bar_height;
    
    auto start_time = table->timestamp;
    
    logic set_active = false;
    
    for(u32 i = 0; i < table->thread_count; i++){
        
        StackEntry stack_array[16];
        u32 stack_count = 0;
        
        auto record_count = table->recordcount_array[i];
        
        if(!record_count){
            continue;
        }
        
        for(u32 j = record_count -1; j != (u32)-1; j--){
            
            const auto record = &table->record_array[i].array[j];
            
            auto y =
                InternalGUIGetStackPositionY(stack_array,&stack_count,record,start_time,start_y,
                                             bar_height);   
            
            auto start_x =
                -1.0f + (GetTimeDifferenceMS(start_time, record->start_stamp)/_upperlimit);
            
            auto bar_width = (record->timelen/_upperlimit);
            
            Color barcolor = record->color;
            
            if(InternalIsWithinBounds(InternalPushBounds(start_x,y,bar_width,bar_height))){
                
                set_active = true;
                
                if(gui->internal_active_state == token){
                    gui->graph_hover = GUITYPE_PROFILER;
                    gui->thread_index = i;
                    gui->p_record = *record;
                    barcolor = gui->graph_bar_select_color;
                }
            }
            
            InternalGUIDrawRect(start_x,y,bar_width,bar_height,barcolor);
            
        }
        
        start_y -= (bar_height * stack_count);
    }
    
    if(set_active){
        InternalSetActiveState(profilename);  
    }
    
    else if(token == gui->internal_active_state){
        gui->internal_active_state = 0;
    }
    
    
    
    return ret;
}

logic GUIIsElementActive(const s8* element_name){
    return gui->internal_active_state == PHashString(element_name);
}

logic GUIIsAnyElementActive(){
    return (logic)gui->internal_active_state;
}


//3D stuff

void InternalGUIDrawLine(GUIVec3 a,GUIVec3 b,Color color = White){
    
    u32 curvert = gui->vert_offset;
    u32 curindex = gui->ind_offset;
    
    gui->vert_mptr[curvert] = {{a.x,a.y,a.z},{},{ color.R, color.G, color.B,color.A }};
    curvert++;
    
    gui->vert_mptr[curvert] = {{b.x,b.y,b.z},{},{ color.R, color.G, color.B,color.A }};
    curvert++;
    
    gui->ind_mptr[curindex] = curvert - 2;
    curindex++;
    
    gui->ind_mptr[curindex] = curvert - 1;
    curindex++;
    
    gui->vert_offset = curvert;
    gui->ind_offset = curindex;
    
    _kill("gui vertex overflow", curvert > _reserve_count);
    _kill("gui index overflow",curindex > _reserve_count);
}

void InternalGUIDrawLine(GUIVec2 a,GUIVec2 b,Color color = White){
    InternalGUIDrawLine({a.x,a.y,0},{b.x,b.y,0},color);
}

logic GUITranslateGizmo(GUIVec3* world_pos){
    
    auto token = PHashString("GUI3DTranslate");
    
    GUISetRenderMode(GUI_RENDER_LINE);
    GUISetCameraMode(GUI_CAMERA_NONE);
    
    GUIInternalMakeSubmission();
    
    auto viewproj = gui->proj_matrix * gui->view_matrix;
    
    auto obj_w = *world_pos;
    auto obj_c = WorldSpaceToClipSpace(obj_w,viewproj);
    
    auto x_c = WorldSpaceToClipSpace(obj_w + Vector4{1,0,0,0},viewproj);
    auto y_c = WorldSpaceToClipSpace(obj_w + Vector4{0,1,0,0},viewproj);
    auto z_c = WorldSpaceToClipSpace(obj_w + Vector4{0,0,1,0},viewproj);
    
    Vector2 mouse_c =GUIMouseCoordToScreenCoord();
    mouse_c.y *= -1.0f;
    
    
    
    InternalGUIDrawLine(obj_c,x_c,gui->axis_x_color);
    InternalGUIDrawLine(obj_c,y_c,gui->axis_y_color);
    InternalGUIDrawLine(obj_c,z_c,gui->axis_z_color);
    
    auto mouse_dir = mouse_c - obj_c.vec2[0];
    auto mouse_ndir = Normalize(mouse_dir);
    
    struct DirDotPair{
        GUIVec3 dir;
        f32 dot;
        f32 len;
    };
    
    DirDotPair pair_array[3] = {
        
        {Vec3::Normalize(x_c - obj_c),
            Dot(mouse_ndir,Normalize(x_c.vec2[0] - obj_c.vec2[0])),
            Magnitude(x_c.vec2[0] - obj_c.vec2[0])},
        
        {Vec3::Normalize(y_c - obj_c),
            Dot(mouse_ndir,Normalize(y_c.vec2[0] - obj_c.vec2[0])),
            Magnitude(y_c.vec2[0] - obj_c.vec2[0])},
        
        {Vec3::Normalize(z_c - obj_c),
            Dot(mouse_ndir,Normalize(z_c.vec2[0] - obj_c.vec2[0])),
            Magnitude(z_c.vec2[0] - obj_c.vec2[0])},
    };
    
    DirDotPair max = {};
    
    for(u32 i = 0; i < _arraycount(pair_array); i++){
        
        auto pair = pair_array[i];
        
        if(pair.dot > max.dot){
            max = pair;
        }
        
    }
    
    if(gui->internal_active_state == token){
        
        if(GUIMouseDownL()){
            auto ext_dir = gui->trans_dir.vec2[0] * 100.0f;
            InternalGUIDrawLine(obj_c.vec2[0] - ext_dir,obj_c.vec2[0] + ext_dir,White);
            
            auto mdir = mouse_c - gui->prev_mouse_pos;
            gui->prev_mouse_pos = mouse_c;
            
            *world_pos = 
                ClipSpaceToWorldSpace(obj_c +
                                      Vec3::ProjectOnto(Vector4{mdir.x,mdir.y,0,0},gui->trans_dir),viewproj);
            
            return true;
        }
        
        else{
            gui->internal_active_state = false;
        }
        
    }
    
    else if(((u32)(max.dot * 100.0f) > 98) && GUIMouseDownL() && 
            !(u32)(((Magnitude(mouse_dir) - max.len)) * 10.0f)){
        
        gui->trans_dir = max.dir;
        gui->prev_mouse_pos = mouse_c;
        InternalSetActiveState("GUI3DTranslate");
        
    }
    
    return false;
}


logic GUIScaleGizmo(GUIVec3 world_pos,f32* scale){
    
    auto token = PHashString("GUI3DScale");
    
    GUISetRenderMode(GUI_RENDER_LINE);
    GUISetCameraMode(GUI_CAMERA_NONE);
    
    GUIInternalMakeSubmission();
    
    auto viewproj = gui->proj_matrix * gui->view_matrix;
    
    auto obj_c = WorldSpaceToClipSpace(world_pos,viewproj).vec2[0];
    
    Vector2 mouse_c =GUIMouseCoordToScreenCoord();
    mouse_c.y *= -1.0f;
    
    auto mouse_dir =  mouse_c - obj_c;
    auto mouse_ndir =  Normalize(mouse_dir);
    
    if(GUIMouseDownL() && gui->internal_active_state != token){
        
        InternalSetActiveState("GUI3DScale");
        
        gui->start_mouse_pos_len = Magnitude(mouse_dir);
        gui->start_scale = *scale;
    }
    
    if(gui->internal_active_state == token){
        
        InternalGUIDrawLine(obj_c,mouse_c,gui->axis_x_color);
        
        InternalGUIDrawLine(obj_c,(mouse_ndir * gui->start_mouse_pos_len) + obj_c,
                            gui->axis_z_color);
        
        *scale = (Magnitude(mouse_dir)/gui->start_mouse_pos_len) * gui->start_scale;
        
        if(GUIMouseUpL()){
            gui->internal_active_state = false;
        }
        
        return true;
        
    }
    
    
    
    return false;
}

void GUIDrawCube(){
    
}

void InternalGUIDrawRotAxis(Vector3 obj_w,Matrix4b4 viewproj,u32 selected_id,
                            logic draw_unselected){
    
#define _granularity 32
    
    struct DrawRotAxisData{
        u32 axis_index;
        Vector3 dir;
        Color color;
    };
    
    DrawRotAxisData rotaxis[] = {
        {0,{0,1},gui->axis_x_color},//x
        {1,{1},gui->axis_y_color},//y
        {2,{1},gui->axis_z_color},//z
    };
    
    //draw circles
    
    Vector3 points[3][_granularity] = {};
    
    for(u32 i = 0; i < 3; i++){
        
        auto axis = rotaxis[i];
        
        for(u32 j = 0; j < _granularity; j++){
            
            auto dir = axis.dir;
            Vector3 rot_axis = {};
            
            rot_axis.floats[axis.axis_index] = j * ((_twopi)/(f32)_granularity);
            
            
            auto p = Vec3::Normalize(RotateVector(dir,rot_axis));
            
            points[i][j] = WorldSpaceToClipSpace(p + obj_w,viewproj);
        }
        
    }
    
    for(u32 i = 0; i < 3; i++){
        
        auto color = rotaxis[i].color;
        
        if(i == selected_id){
            color = White;
        }
        
        else if(!draw_unselected){
            continue;
        }
        
        for(u32 j = 0; j < _granularity; j++){
            
            auto a = points[i][j];
            
            auto next = j + 1;
            
            if(next > _granularity - 1){
                next = 0;
            }
            
            auto b = points[i][next];
            
            
            InternalGUIDrawLine(a,b,color);
        }
        
    }
    
}


logic GUIRotationGizmo(GUIVec3 world_pos,Quaternion* rot){
    
    auto ret = false;
    
    auto token = PHashString("GUI3DRotate");
    
    GUISetRenderMode(GUI_RENDER_LINE);
    GUISetCameraMode(GUI_CAMERA_NONE);
    
    GUIInternalMakeSubmission();
    
    auto viewproj = gui->proj_matrix * gui->view_matrix;
    
    auto obj_w = world_pos;
    auto obj_c = WorldSpaceToClipSpace(obj_w,viewproj);
    
    Vector3 mouse_c = {};
    mouse_c.vec2[0] = GUIMouseCoordToScreenCoord();
    mouse_c.z = 0;
    mouse_c.w = 1.0f;
    mouse_c.y *= -1.0f;
    
    auto mouse_w = ClipSpaceToWorldSpace(mouse_c,viewproj);
    
    mouse_c.z = 1;
    
    auto mouse_dir = ClipSpaceToWorldSpace(mouse_c,viewproj) - mouse_w;
    
    Line3 line = {
        mouse_w,
        mouse_dir
    };
    
    Plane rot_planes[] = {
        {obj_w,{1,0,0}},
        {obj_w,{0,1,0}},
        {obj_w,{0,0,1}},
    };
    
    u32 selected = (u32)-1;
    f32 diff = 100.0f;
    Point3 retpi = {};
    Point3 curpi = {};
    
    logic draw_unselected = true;
    
    for(u32 i = 0; i < 3; i++){
        
        Point3 pi;
        
        if(Vec3::Intersect(line,rot_planes[i],&pi)){
            
            if(gui->rot_selected == i){
                curpi = pi;	
            }
            
            
            auto k = fabsf(1.0f - Vec3::Magnitude(pi - obj_w));
            
            if(k < diff && k < 0.3f){
                diff = k;
                selected = i;
                retpi = pi;
            }
            
        }
        
    }
    
    if(gui->internal_active_state == token){
        
        ret = true;
        draw_unselected = false;
        selected = gui->rot_selected;
        
        auto d1 = Vec3::Normalize(curpi - obj_w);
        auto d2 = Vec3::Normalize(gui->rot_intersection_point - obj_w);
        
        auto p1 = obj_w + d1;
        auto p2 = obj_w + d2;
        
        InternalGUIDrawLine(obj_c,WorldSpaceToClipSpace(p1,viewproj),Yellow);
        InternalGUIDrawLine(obj_c,WorldSpaceToClipSpace(p2,viewproj),White);
        
        f32 angle = 0.0f;
        
        switch(selected){
            
            case 0:{
                angle = atan2f(d2.y,d2.z) - atan2f(d1.y,d1.z);
            }break;
            
            case 1:{
                angle = atan2f(d2.z,d2.x) - atan2f(d1.z,d1.x);
            }break;
            
            default:{
                angle = atan2f(d1.y,d1.x) - atan2f(d2.y,d2.x);
            }break;
            
        }
        
        *rot = ConstructQuaternion(rot_planes[selected].norm,angle) * gui->start_rot;
        
        if(GUIMouseUpL() && gui->internal_active_state == token){
            gui->internal_active_state = false; 
        }
        
    }
    
    if(GUIMouseDownL() && gui->internal_active_state != token && selected != (u32)-1){
        gui->rot_selected = selected;
        gui->rot_intersection_point = retpi;
        gui->start_rot = *rot;
        InternalSetActiveState("GUI3DRotate"); 
    }
    
    InternalGUIDrawRotAxis(obj_w,viewproj,selected,draw_unselected);
    
    return ret;
}


void GUIDebugGetCurrentHolder(){
    printf("GUI current holder %s\n",gui->internal_state_string);
}