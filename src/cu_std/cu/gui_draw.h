#pragma once

#include "ttype.h"
#include "vvulkan.h"
#include "mmath.h"
#include "debugtimer.h"

struct KeyboardState;
struct MouseState;


struct GUIVertex{
    f32 pos[3];
    f32 uv[2];
    f32 color[4];
};

struct GUIBoundingRect{
    u16 x;
    u16 y;
    u16 width;
    u16 height;
};

typedef Vector2 GUIVec2;
typedef Vector3 GUIVec3;

struct GUIDim2{
    f32 w;
    f32 h;
};

struct GUIFont{
    VTextureContext texture;
    u16 width;
    u16 height;
    VkDescriptorSet descset;
};

enum GUIRenderMode{
    GUI_RENDER_SOLID = 0,
    GUI_RENDER_LINE = 1,
    GUI_RENDER_TEXT = 2,
};

enum GUICameraMode{
    GUI_CAMERA_NONE = 0,
    GUI_CAMERA_PERSPECTIVE = 1,
};

GUIFont GUICreateFontFromFile(const s8* filepath,VkCommandBuffer cmdbuffer,
                              VDeviceContext* vdevice,VkQueue queue);

void GUIInit(VDeviceContext* vdevice,VSwapchainContext* swap,
             VkRenderPass renderpass,VkQueue queue,VkCommandBuffer cmdbuffer,VkPipelineCache cache = 0,
             u32 vertexbinding_no = 0,
             GUIFont* fonthandle = 0);

void GUIUpdate(WWindowContext* window,KeyboardState* keyboardstate,
               MouseState* mousestate,Matrix4b4 view,Matrix4b4 proj);

void GUIDraw(VkCommandBuffer cmdbuffer);

void GUISetRenderMode(GUIRenderMode rendermode);

void GUISetCameraMode(GUICameraMode cameramode);

void GUISetBackColor(Color color);
void GUISetFrontColor(Color color);
void GUISetTitleColor(Color color);
void GUISetTextColor(Color color);

enum GUIWindowBehavoir{
    GUIWINDOW_NONE = 0,
    GUIWINDOW_NORESIZE = 1,
    GUIWINDOW_NOMOVE = 2,
};

void GUIBegin(const s8* title = 0,GUIVec2* pos = 0,GUIDim2* dim = 0);
void GUIEnd();

void GUIBeginWindow(const s8* title,GUIVec2* pos = 0,GUIDim2* dim = 0);

void GUIString(const s8* string);

logic GUITextBox(const s8* label,const s8* buffer,logic fill_w = true,GUIDim2 dim = {0.5f,0.5f});

logic GUITextField(const s8* label,const s8* buffer,logic fill_w = true,f32 w = 0.5f);

logic GUIButton(const s8* title);

logic GUIComboBox(const s8* label,const s8** options_array,u32 options_count,u32* index,
                  logic fill_w = true);

void GUISlider();

void GUI3DTranslate(f32* x,f32* y,f32* z);
void GUI3DScale(f32* x);
void GUI3DRotate(f32* x,f32* y,f32* z);

logic GUIHistogram(const s8* label_x,const s8* label_y,GUIVec2* data_array,u32 data_count,
                   u32* out_entry_index,f32* max = 0,GUIDim2 dim = {0.5f,0.5f},u32* highlight_index = 0);



logic GUIProfileView(const s8* profilename,const DebugTable* table,
                     GUIDim2 dim = {0.5f,0.5f});

logic GUIIsElementActive(const s8* element_name);

logic GUIIsAnyElementActive();

void GUILineGraph();

#define GUIDEFAULT_X -0.7f
#define GUIDEFAULT_Y 0.5f

#define GUIDEFAULT_W 0.25f
#define GUIDEFAULT_H 0.25f

logic GUITranslateGizmo(GUIVec3* world_pos);

//TODO: make a vec3 version
logic GUIScaleGizmo(GUIVec3 world_pos,f32* scale);

//TODO: make a vec3 version
logic GUIRotationGizmo(GUIVec3 world_pos,Quaternion* rot);

void GUIDrawPosMarker(GUIVec3 world_pos,Color color);

GUIVec2 GUIMouseCoordToScreenCoord();


struct GUIContext;

GUIContext* GetGUIContext();
void SetGUIContext(GUIContext* context);


void GUIGenFontFile(const s8* filepath,const s8* writepath,f32 fontsize);

void GUIDebugGetCurrentHolder();

void GUIDrawAxisSphere(Vector3 obj_w,f32 radius,Color x = White,Color y = White,Color z = White);

//TODO: implement a color picker (https://en.wikipedia.org/wiki/HSL_and_HSV)