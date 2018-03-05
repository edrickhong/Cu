#ifdef _WIN32

#define WIN32DLL 1

#endif

#include "stdio.h"
#include "mode.h"
#include "game.h"

#ifdef _WIN32

#undef TIMEBLOCK
#undef TIMEBLOCKTAGGED

#define TIMEBLOCK(COLOR)
#define TIMEBLOCKTAGGED(NAME,COLOR)

#else

#include "debugtimer.h"

#endif

#include "ccolor.h"

#include "kkeycode.h"

#include "pparse.h"

#include "gamecomp_meta.h"

#include "editor_ui.h"

#define _pos_width 0.345f


/*
TODO:
light
add a color picker
separate sampler and texture
*/


GameData* data;

s8* AddComponent(u32 compname_hash,u32 obj_id,SceneContext* context){
    
    auto components = (ComponentStruct*)data->components;
    
    for(u32 i = 0; i < _arraycount(METACOMP_ARRAY); i++){
        
        auto metacomp = &METACOMP_ARRAY[i];
        
        if(metacomp->comp_name_hash == compname_hash){
            
            auto outmetacomp = GetComponentData(components,*metacomp);
            
            auto count = (*outmetacomp.count);
            
            auto obj = outmetacomp.array + (count * outmetacomp.element_size);
            
            memset(obj,0,outmetacomp.element_size);
            
            (*(u32*)obj) = obj_id;
            
            (*outmetacomp.count)++;
            
            
            //just for drawables
            if(compname_hash == PHashString("EntityDrawData")){
                
                auto drawobj = (EntityDrawData*)obj;
                
                auto model = &context->modelasset_array[drawobj->model];
                
                
                if(model->vert_component == 7){
                    auto comp =
                        (EntityAnimationData*)AddComponent(PHashString("EntityAnimationData"),obj_id,
                                                           context);
                    
                    comp->animdata_id = model->animation_id;
                    comp->speed = 1.0f;
                    
                    drawobj->group = 0;
                }
                
                else{
                    drawobj->group = 1;
                }
                
            }
            
            if(compname_hash == PHashString("PointLight")){
                auto light = (PointLight*)obj;
                
                light->R = 1.0f;
                light->G = 1.0f;
                light->B = 1.0f;
                light->intensity = 0.2f;
            }
            
            return obj;
            
        }
        
    }
    
    return 0;
}

void RemoveComponent(u32 compname_hash,u32 obj_id,SceneContext* context){
    
    auto components = (ComponentStruct*)data->components;
    
    for(u32 i = 0; i < _arraycount(METACOMP_ARRAY); i++){
        
        auto metacomp = &METACOMP_ARRAY[i];
        
        if(metacomp->comp_name_hash == compname_hash){
            
            auto outmetacomp = GetComponentData(components,*metacomp);
            
            auto count = (*outmetacomp.count);
            
            for(u32 j = 0; j < count; j++){
                
                auto obj = outmetacomp.array + (j * outmetacomp.element_size);
                
                if((*(u32*)obj) == obj_id){
                    
                    if(compname_hash == PHashString("EntityDrawData")){
                        
                        auto drawobj = (EntityDrawData*)obj;
                        auto model = &context->modelasset_array[drawobj->model];
                        
                        if(model->vert_component == 7){
                            RemoveComponent(PHashString("EntityAnimationData"),obj_id,context);
                        }
                        
                    }
                    
                    (*outmetacomp.count)--;
                    
                    auto lastobj = outmetacomp.array + ((*outmetacomp.count) * outmetacomp.element_size);
                    
                    memcpy(obj,lastobj,outmetacomp.element_size);
                    
                    return;
                }
                
            }
            
            break;
        }
        
    }
    
}

u32 AddObject(SceneContext* context){
    
    auto pos = data->camera_pos + (data->camera_lookdir * 4.0f);
    
    u32 id = (u32)-1;
    
    for(u32 i = 0; i < data->orientation.count; i++){
        
        if(data->orientation.skip_array[i]){
            id = i;
            break;
        }
        
    }
    
    if(id == (u32)-1){
        id = data->orientation.count;
        data->orientation.count++;
    }
    
    
    
    data->orientation.pos_x[id] = pos.x;
    data->orientation.pos_y[id] = pos.y;
    data->orientation.pos_z[id] = pos.z;
    
    data->orientation.rot[id] = ConstructQuaternion({1,0,0,0},_radians(-90));
    
    data->orientation.scale[id] = 1.0f;
    
    data->orientation.skip_array[id] = false;
    
    context->SetObjectOrientation(id,pos,data->orientation.rot[id],data->orientation.scale[id]);
    
    return id;
}

void RemoveObject(u32 obj_id,SceneContext* context){
    
    data->orientation.skip_array[obj_id] = true;
    
    for(u32 i = 0; i < _arraycount(METACOMP_ARRAY); i++){
        
        auto metacomp = &METACOMP_ARRAY[i];
        
        RemoveComponent(metacomp->comp_name_hash,obj_id,context);
    }
    
}

void KeyboardInput(SceneContext* context){
    
    KeyboardState* keyboardstate = context->keyboardstate;
    //f32 delta_time = context->prev_frametime;
    
    if(IsKeyDown(keyboardstate,KCODE_KEY_ESC)){
        data->running = false;
    }
    
    if(IsKeyDown(keyboardstate,KCODE_KEY_SPACE)){
        
        data->roty += 0.0001f * 100;
        
        Quaternion rot = ConstructQuaternion({0,1,0,0},data->roty);
        
        Vector4 pos1 = {data->orientation.pos_x[0],data->orientation.pos_y[0],
            data->orientation.pos_z[0],1.0f};
        
        Vector4 pos2 = {data->orientation.pos_x[1],data->orientation.pos_y[1],
            data->orientation.pos_z[1],1.0f};
        
        Vector4 pos3 = {data->orientation.pos_x[2],data->orientation.pos_y[2],
            data->orientation.pos_z[2],1.0f};
        
        auto q1 = rot * data->orientation.rot[0];
        auto q2 = rot * data->orientation.rot[1];
        auto q3 = rot * data->orientation.rot[2];
        
        context->SetObjectOrientation(0,pos1,q1,data->orientation.scale[0]);
        context->SetObjectOrientation(1,pos2,q2,data->orientation.scale[1]);
        context->SetObjectOrientation(2,pos3,q3,data->orientation.scale[2]); 
    }
    
}

void ComponentRead(ComponentStruct* components,SceneContext* context){
    
    if(!FIsFileExists(_COMPFILE)){
        return;
    }
    
    auto file = FOpenFile(_COMPFILE,F_FLAG_READONLY);
    
    {
        
        FRead(file,&data->orientation.count,sizeof(data->orientation.count));
        
        auto count = data->orientation.count;
        
        for(u32 i = 0; i < count; i++){
            FRead(file,&data->orientation.pos_x[i],sizeof(data->orientation.pos_x[i]));
        }
        
        for(u32 i = 0; i < count; i++){
            FRead(file,&data->orientation.pos_y[i],sizeof(data->orientation.pos_y[i]));
        }
        
        for(u32 i = 0; i < count; i++){
            FRead(file,&data->orientation.pos_z[i],sizeof(data->orientation.pos_z[i]));
        }
        
        for(u32 i = 0; i < count; i++){
            FRead(file,&data->orientation.rot[i],sizeof(data->orientation.rot[i]));
        }
        
        for(u32 i = 0; i < count; i++){
            FRead(file,&data->orientation.scale[i],sizeof(data->orientation.scale[i]));
        }
        
        //skip array
        for(u32 i = 0; i < count; i++){
            FRead(file,&data->orientation.skip_array[i],sizeof(data->orientation.skip_array[i]));
        }
        
        for(u32 i = 0; i < count; i++){
            
            Vector4 pos = {data->orientation.pos_x[i],data->orientation.pos_y[i],
                data->orientation.pos_z[i],1.0f};
            
            auto scale = data->orientation.scale[i];
            
            auto rot = data->orientation.rot[i];
            
            context->SetObjectOrientation(i,pos,rot,scale);
            
        }
        
    }
    
    u32 metacount;
    
    FRead(file,&metacount,sizeof(metacount));
    
    for(u32 i = 0; i < metacount; i++){
        
        u32 comp_hash;
        u32 comp_count;
        u32 field_count;
        
        FRead(file,&comp_hash,sizeof(comp_hash));
        FRead(file,&comp_count,sizeof(comp_count));
        FRead(file,&field_count,sizeof(field_count));
        
        auto comp = MetaGetCompByNameHash(comp_hash);
        
        MetaDataCompOut out_comp;
        
        if(comp){
            out_comp = GetComponentData(components,*comp);
            (*out_comp.count) = comp_count;
        }
        
        for(u32 j = 0; j < comp_count; j++){
            
            s8* obj;
            
            if(comp){
                obj = out_comp.array + (j * out_comp.element_size);
            }
            
            for(u32 k = 0; k < field_count; k++){
                
                u32 type_hash;
                u32 type_size;
                u32 name_hash;
                u32 arraycount;
                
                
                FRead(file,&type_hash,sizeof(type_hash));
                FRead(file,&type_size,sizeof(type_size));
                FRead(file,&name_hash,sizeof(name_hash));
                FRead(file,&arraycount,sizeof(arraycount));
                
                for(u32 a = 0; a < arraycount; a++){
                    
                    s8 buffer[128] = {};
                    
                    FRead(file,&buffer[0],type_size);
                    
                    if(comp){
                        if(MetaGetTypeByNameHash(name_hash,&out_comp.metadata_table[0],
                                                 out_comp.metadata_count) == type_hash){
                            MetaSetValueByNameHash(obj,a,&buffer[0],name_hash,
                                                   &out_comp.metadata_table[0],out_comp.metadata_count);
                        } 
                    }
                }
                
            }
            
        }
        
    }
    
    
    for(u32 i = 0; i < components->entitydrawdata_count; i++){
        
        auto obj_id = components->entitydrawdata_array[i].id;
        auto mat_id = components->entitydrawdata_array[i].material;
        
        context->SetObjectMaterial(obj_id,mat_id);
    }
    
    FCloseFile(file);
}


typedef u64 AudioToken;

void _ainline UpdateAnimationDataList(SceneContext* context){
    
    auto comp = (ComponentStruct*)data->components;
    
    context->animationdata_array = comp->entityanimationdata_array;
    context->animationdata_count = comp->entityanimationdata_count;
}

void _ainline UpdateDrawList(SceneContext* context){
    
    auto comp = (ComponentStruct*)data->components;
    
    context->draw_array = comp->entitydrawdata_array;
    context->draw_count = comp->entitydrawdata_count;
}

void UpdateLightList(SceneContext* context){
    
    auto comp = (ComponentStruct*)data->components;
    
    for(u32 i = 0; i < comp->pointlight_count; i++){
        
        auto light = &comp->pointlight_array[i];
        
        auto pos = Vector3{data->orientation.pos_x[light->id],data->orientation.pos_y[light->id],data->orientation.pos_z[light->id],1.0f};
        
        context->AddPointLight(pos,Color{light->R,light->G,light->B,1.0f},light->intensity,light->linear,light->quadratic);
    }
}



//TODO: we should move all this to core engine

AudioToken QueueAudio(SceneContext* context,const s8* filepath,u16 looping = true,
                      u32 orientation_id = -1){
    
    auto audio =
        &((ComponentStruct*)data->components)->entityaudiodata_array[((ComponentStruct*)data->components)->entityaudiodata_count];
    
    audio->id = orientation_id;
    audio->audioasset = context->AllocateAssetAudio(filepath);
    audio->islooping = looping;
    audio->toremove = false;
    
    ((ComponentStruct*)data->components)->entityaudiodata_count++;
    
    return (u64)(audio->audioasset.assetfile)  | audio->id;
}

void RemoveAudio(SceneContext* context,EntityAudioData* audio){
    
    _kill("passed null\n",!audio);
    
    ((ComponentStruct*)data->components)->entityaudiodata_count--;
    
    auto last =
        &((ComponentStruct*)data->components)->entityaudiodata_array[((ComponentStruct*)data->components)->entityaudiodata_count];
    
    //Deallocate asset
    context->UnallocateAsset((AssetHandle*)&audio->audioasset);
    *audio = *last;
}

void RemoveAudio(SceneContext* context,AudioToken token){
    
    EntityAudioData* audio = 0;
    
    for(u32 i = 0; i < ((ComponentStruct*)data->components)->entityaudiodata_count; i++){
        
        auto entry = &((ComponentStruct*)data->components)->entityaudiodata_array[i];
        
        if(((u64)(entry->audioasset.assetfile) | entry->id) == token){
            audio = entry;
        }
        
    }
    
    RemoveAudio(context,audio);
}

void RemoveAllAudio(SceneContext* context){
    
    for(u32 i = 0; i < ((ComponentStruct*)data->components)->entityaudiodata_count; i++){
        auto audio = &((ComponentStruct*)data->components)->entityaudiodata_array[i];
        context->UnallocateAsset((AssetHandle*)&audio->audioasset);
    }
    
    ((ComponentStruct*)data->components)->entityaudiodata_count = 0;
    
}

u32 IsPlaying(u64 audiotoken){
    
    if(audiotoken == (u64)-1){
        return 0;
    }
    
    for(u32 i = 0; i < ((ComponentStruct*)data->components)->entityaudiodata_count; i++){
        
        auto audio = &((ComponentStruct*)data->components)->entityaudiodata_array[i];
        
        if(((u64)(audio->audioasset.assetfile)  | audio->id) == audiotoken){
            return true;      
        }
        
    }
    
    return 0;
};

void ProcessAudio(SceneContext* context){
    
    auto audiocontext = context->audiocontext;
    auto audio_count = context->audiocontext_count;
    
    TIMEBLOCK(DimGray);
    
    for(u32 i = 0; i < ((ComponentStruct*)data->components)->entityaudiodata_count; i++){
        
        auto audio = &((ComponentStruct*)data->components)->entityaudiodata_array[i];
        
        if(audio->toremove > 1){
            RemoveAudio(context,audio);
        }
        
        *audiocontext = &((ComponentStruct*)data->components)->entityaudiodata_array[0];
        *audio_count = ((ComponentStruct*)data->components)->entityaudiodata_count;  
    }
    
}

extern "C" {
    
    _dllexport void GameComponentWrite(void* context){
        
        if(!data->components){
            return;
        }
        
        RemoveAllAudio((SceneContext*)context);
        
        auto outfile = FOpenFile(_COMPFILE,F_FLAG_READWRITE |
                                 F_FLAG_TRUNCATE | F_FLAG_CREATE);
        
        {
            
            FWrite(outfile,&data->orientation.count,sizeof(data->orientation.count));
            
            auto count = data->orientation.count;
            
            for(u32 i = 0; i < count; i++){
                auto entry = data->orientation.pos_x[i];
                FWrite(outfile,&entry,sizeof(entry));
            }
            
            for(u32 i = 0; i < count; i++){
                auto entry = data->orientation.pos_y[i];
                FWrite(outfile,&entry,sizeof(entry));
            }
            
            for(u32 i = 0; i < count; i++){
                auto entry = data->orientation.pos_z[i];
                FWrite(outfile,&entry,sizeof(entry));
            }
            
            for(u32 i = 0; i < count; i++){
                auto entry = data->orientation.rot[i];
                FWrite(outfile,&entry,sizeof(entry));
            }
            
            for(u32 i = 0; i < count; i++){
                auto entry = data->orientation.scale[i];
                FWrite(outfile,&entry,sizeof(entry));
            }
            
            //skip array
            for(u32 i = 0; i < count; i++){
                auto entry = data->orientation.skip_array[i];
                FWrite(outfile,&entry,sizeof(entry));
            }
            
        }
        
        u32 metacount = _arraycount(METACOMP_ARRAY);
        
        FWrite(outfile,&metacount,sizeof(metacount));
        
        for(u32 i = 0; i < _arraycount(METACOMP_ARRAY); i++){
            
            auto component = &METACOMP_ARRAY[i];
            
            auto out_comp = GetComponentData((ComponentStruct*)data->components,*component);
            
            qsort(out_comp.array,(*out_comp.count),
                  out_comp.element_size,
                  [](const void * a, const void* b)->s32 {
                  
                  auto id_a = (u32*)a;
                  auto id_b = (u32*)b;
                  
                  return (*id_a) - (*id_b);
                  });
            
            //comp name hash
            FWrite(outfile,&out_comp.comp_name_hash,sizeof(out_comp.comp_name_hash));
            
            //comp count
            FWrite(outfile,out_comp.count,sizeof(u32));
            
            //field count
            FWrite(outfile,&out_comp.metadata_count,sizeof(out_comp.metadata_count));
            
            //actual comp data
            auto count = (*out_comp.count);
            
            for(u32 j = 0; j < count; j++){
                
                auto obj = out_comp.array + (j * out_comp.element_size);
                
                for(u32 k = 0; k < out_comp.metadata_count; k++){
                    
                    auto entry = &out_comp.metadata_table[k];
                    
                    //type hash
                    FWrite(outfile,&entry->type_hash,sizeof(entry->type_hash));
                    //size
                    FWrite(outfile,&entry->size,sizeof(entry->size));
                    //name hash
                    FWrite(outfile,&entry->name_hash,sizeof(entry->name_hash));	
                    
                    //array count
                    FWrite(outfile,&entry->arraycount,sizeof(entry->arraycount));
                    
                    for(u32 a = 0; a < entry->arraycount; a++){
                        
                        s8 buffer[128] = {};
                        
                        MetaGetValueByNameHash(obj,a,&buffer[0],entry->name_hash,
                                               &out_comp.metadata_table[0],out_comp.metadata_count);
                        
                        //value
                        FWrite(outfile,&buffer[0],entry->size);
                    }
                }
                
                
            }
            
        }
        
        FCloseFile(outfile);
    }
    
    _dllexport void GameInit(GameInitData* initdata){
        
        data = (GameData*)initdata->memory;
        
        data->running = true;
        data->draw_profiler = true;
        
        data->camera_pos = Vector4{0.0f,0.0f,-4.0f,1.0f};
        data->camera_lookdir = Vector4{0.0f,0.0f,1.0f,0.0f};
        
        
        //set gui state
        data->prev_mpos = {};
        data->widget_type = 0;
        data->obj_id = 2;
        data->show_object_list = false;
        data->show_object_editor = false;
        data->pos_1 = {-1.0f,1.0f};
        data->dim_1 = {GUIDEFAULT_W * 2.8f,GUIDEFAULT_H * 0.22f};
        data->write_orientation = true;
        data->pos_2 = {-0.16f,GUIDEFAULT_Y};
        data->dim_2 = {GUIDEFAULT_W * 2.2f,GUIDEFAULT_H};
        
        data->w_pos = {0.4f,GUIDEFAULT_Y};
        data->w_dim = {GUIDEFAULT_W * 2.2f,GUIDEFAULT_H * 2.5f};
        
        memset(&data->o_buffer[0][0],0,sizeof(data->o_buffer));
        memset(&data->in_buffer[0],0,sizeof(data->in_buffer));
        
        
    }
    
    _dllexport void GameUpdateRender(SceneContext* context){
        
        TIMEBLOCK(LimeGreen);
        
#if 0
        ComponentStartInit(context);
        GameComponentWrite();
        exit(0);
#endif
        
#if _debug
        EditorGUI(context);
#endif
        
        KeyboardInput(context);
        
        context->SetActiveCameraOrientation(data->camera_pos,data->camera_lookdir);
        
        ProcessAudio(context);
        
        UpdateAnimationDataList(context);
        UpdateDrawList(context);
        UpdateLightList(context);
    }
    
    
    _dllexport void GameReload(GameReloadData* reloaddata){
        
        SetGUIContext(reloaddata->guicontext);
        SetAAllocatorContext(reloaddata->allocatorcontext);
        
        data = (GameData*)reloaddata->memory;
        
        if(data->components){
            unalloc(data->components);
        }
        
        data->components = alloc(sizeof(ComponentStruct));
        memset(data->components,0,sizeof(ComponentStruct));
        
        data->roty = 0.0f;
        
        ComponentRead((ComponentStruct*)data->components,reloaddata->context);
        
        QueueAudio(reloaddata->context,
                   AUDIO_PATH(The_Entertainer_Scott_Joplin.adf),true,-1);
    }
    
}


#ifdef _WIN32

BOOL WINAPI DllMain(HINSTANCE hinstDLL,DWORD fdwReason,LPVOID lpvReserved){
    
    return(TRUE);
}

#endif


#if _debug

logic InspectorIgnoreFields(u32 hash){
    return hash == PHashString("id") || hash == (u32)PHashString("animationcomponent_id")
        || hash == (u32)PHashString("animdata_id");
}

logic InspectorIsOpaqueType(u32 hash){
    return hash == PHashString("AnimationID") || hash == PHashString("ModelID")
        || hash == PHashString("TextureID") || hash == PHashString("RenderGroupIndex") ||
        hash == PHashString("MaterialID");
}




s8* InspectorGetMainAssetString(s8* string){
    
    auto len = strlen(string);
    
    for(u32 i = (len - 1) ; i < (u32)(-1); i--){
        
        if(string[i] == _FileSlash){
            return &string[i + 1];
        }
        
    }
    
    return string;
}

//TODO:
u32 InspectorHandleOpaqueTypes(u32 type_hash,u32 selected_id,u32 obj_id,
                               SceneContext* context){
    
    EntityDrawData* ref_drawdata = 0;
    auto original_id = selected_id;
    
    for(u32 i = 0; i < ((ComponentStruct*)data->components)->entitydrawdata_count; i++){
        
        auto drawdata = &(((ComponentStruct*)data->components)->entitydrawdata_array[i]);
        
        if(obj_id == drawdata->id){
            ref_drawdata = drawdata;
            break;
        }
        
    }
    
    if(!ref_drawdata){
        return (u32)-1;
    }
    
    if(selected_id == (u32)-1){
        return -1;
    }
    
    switch(type_hash){
        
        case PHashString("ModelID"):{
            
            auto model_count = context->modelasset_count;
            auto model_array = &(context->modelasset_array[0]);
            
            s8* model_options[16] = {};
            
            
            for(u32 i = 0; i < model_count; i++){
                
                auto model = &model_array[i];
                
                auto string = InspectorGetMainAssetString((s8*)model->assetfile);
                
                model_options[i] = string;
            }
            
            if(GUIComboBox("ModelID",(const s8**)&model_options,model_count,&selected_id)){
                
                auto prev_model = &model_array[original_id];
                auto cur_model = &model_array[selected_id];
                
                if(cur_model->vert_component == 7){
                    
                    if(prev_model->vert_component == 7){
                        RemoveComponent(PHashString("EntityAnimationData"),obj_id,context); 
                    }
                    
                    auto comp =
                        (EntityAnimationData*)AddComponent(PHashString("EntityAnimationData"),obj_id,
                                                           context);
                    
                    comp->animdata_id = cur_model->animation_id;
                    comp->speed = 1.0f;
                    
                    ref_drawdata->group = 0;
                }
                
                else{
                    
                    if(prev_model->vert_component == 7){
                        
                        RemoveComponent(PHashString("EntityAnimationData"),obj_id,context); 
                    }
                    
                    ref_drawdata->group = 1;
                }
                
                return selected_id;      
            }
            
            
        }break;
        
        case PHashString("MaterialID"):{
            
            auto mat_count = context->materialasset_count;
            s8* mat_options[16] = {};
            
            // s8 mat_buffer[16 * 16] = {};
            auto mat_buffer = TAlloc(s8,16 * 16);
            memset(&mat_buffer[0],0,16 * 16 * sizeof(s8));
            
            for(u32 i = 0; i < mat_count; i++){
                sprintf(&mat_buffer[i * 16],"%d",i);
                mat_options[i] = &mat_buffer[i * 16];
            }
            
            if(GUIComboBox("MaterialID",(const s8**)(s8**)&mat_options[0],mat_count,&selected_id)){
                
                context->SetObjectMaterial(obj_id,selected_id);
                
                return selected_id;  
            }
            
        }break;
        
        case PHashString("RenderGroupIndex"):{
            
            s8* all_rendergroup_options[] = {
                (s8*)"skel",
                (s8*)"static",
            };
            
            u32 vert_comp[] = {
                7,
                3,
            };
            
            s8* rendergroup_options[32] = {};
            u32 rendergroup_indices[32] = {};
            u32 rendergroup_options_count = 0;
            
            
            auto ref_vertcomp = context->modelasset_array[ref_drawdata->model].vert_component;
            
            //FIXME: this is bogus as hell
            for(u32 i = 0; i < _arraycount(rendergroup_options); i++){
                
                //MARK: hack
                if(i == 2){
                    break;
                }
                
                if(selected_id == i){
                    selected_id = rendergroup_options_count;
                }
                
                if(ref_vertcomp == vert_comp[i]){
                    rendergroup_options[rendergroup_options_count] = all_rendergroup_options[i];
                    rendergroup_indices[rendergroup_options_count] = i;
                    rendergroup_options_count++;
                }
                
            }
            
            if(GUIComboBox("RenderGroupIndex",(const s8**)(s8**)&rendergroup_options,
                           rendergroup_options_count,&selected_id)){
                return rendergroup_indices[selected_id];
            }
            
        }break;
        
    }
    
    return (u32)-1;
}


logic FindHash(u32 hash,u32* hash_array,u32 hash_count){
    
    for(u32 i = 0; i < hash_count; i++){
        if(hash == hash_array[i]){
            return true;
        }
    }
    
    return false;
}

void EditorKeyboard(SceneContext* context,u32* widget_type){
    
    auto mousestate = context->mousestate;
    
    auto cur_mpos = GUIMouseCoordToScreenCoord();
    cur_mpos.y *= -1.0f;
    
    auto x_len = cur_mpos.x - data->prev_mpos.x;
    auto y_len = cur_mpos.y - data->prev_mpos.y;
    
    if(IsKeyDown(mousestate,MOUSEBUTTON_RIGHT)){
        
        auto x_angle = 0.0f;
        auto y_angle = 0.0f;
        
        if(x_len != 0){
            x_angle = atanf(x_len/1.0f);
        }
        
        if(y_len != 0){
            y_angle = atanf(y_len/1.0f);
        }
        
        if(fabsf(x_angle) > fabsf(y_angle)){
            data->camera_lookdir = RotateVector(data->camera_lookdir,
                                                {0,x_angle});  
        }
        
        else{
            data->camera_lookdir = RotateVector(data->camera_lookdir,
                                                {y_angle});  
        }
        
        
        data->camera_lookdir = Vec3::Normalize(data->camera_lookdir);
    }
    
    data->prev_mpos = cur_mpos;
    
    KeyboardState* keyboardstate = context->keyboardstate;
    f32 delta_time = context->prev_frametime;
    
    
    if(IsKeyPressed(keyboardstate,KCODE_KEY_1)){
        
        if(*widget_type == 1){
            *widget_type = 0;
        }
        else{
            *widget_type = 1;  
        }
        
    }
    
    if(IsKeyPressed(keyboardstate,KCODE_KEY_2)){
        
        if(*widget_type == 2){
            *widget_type = 0;
        }
        
        else{
            *widget_type = 2;  
        }
        
    }
    
    if(IsKeyPressed(keyboardstate,KCODE_KEY_3)){
        
        if(*widget_type == 3){
            *widget_type = 0;
        }
        
        else{
            *widget_type = 3;  
        }
    }
    
#define _speed 0.004f
    
    auto f = data->camera_lookdir;
    auto s = Vec3::Cross(data->camera_lookdir,Vector3{0,-1});
    
    Vector4 dir = {};
    
    if(IsKeyDown(keyboardstate,KCODE_KEY_W)){
        dir = dir + (f * _speed * delta_time);
    }
    
    if(IsKeyDown(keyboardstate,KCODE_KEY_A)){
        dir = dir - (s * _speed * delta_time);
    }
    
    if(IsKeyDown(keyboardstate,KCODE_KEY_S)){
        dir = dir - (f * _speed * delta_time);
    }
    
    if(IsKeyDown(keyboardstate,KCODE_KEY_D)){
        dir = dir + (s * _speed * delta_time);
    }
    
    
    if(Vec3::Magnitude(dir)){
        dir = Vec3::Normalize(dir);  
    }
    
    if(IsKeyDown(keyboardstate,KCODE_KEY_ESC)){
        data->running = false;
    }
    
    data->camera_pos = data->camera_pos + dir;
    
}

logic EditorWidget(SceneContext* context,u32 obj_id,u32 widget_type){
    
    //Mark out obj
    for(u32 i = 0; i < data->orientation.count; i++){
        
        if(i != obj_id && !data->orientation.skip_array[i]){
            
            Vector3 pos = {
                data->orientation.pos_x[i],
                data->orientation.pos_y[i],
                data->orientation.pos_z[i],
            };
            
            
            GUIDrawPosMarker(pos,White);
        }
    }
    
    logic to_update = false;
    
    Vector3 pos = {
        data->orientation.pos_x[obj_id],
        data->orientation.pos_y[obj_id],
        data->orientation.pos_z[obj_id],
    };
    
    auto scale = data->orientation.scale[obj_id];
    
    auto rot = data->orientation.rot[obj_id];
    
    switch(widget_type){
        
        case 1:{
            
            if(GUITranslateGizmo(&pos)){
                data->orientation.pos_x[obj_id] = pos.x;
                data->orientation.pos_y[obj_id] = pos.y;
                data->orientation.pos_z[obj_id] = pos.z;
                to_update = true;
            }
            
        }break;
        
        case 2:{
            if(GUIScaleGizmo(pos,&scale)){
                data->orientation.scale[obj_id] = scale;
                to_update = true;
            }
            GUIDrawPosMarker(pos,Red);
        }break;
        
        case 3:{
            if(GUIRotationGizmo(pos,&rot)){
                data->orientation.rot[obj_id] = rot;
                to_update = true;
            }  
        }break;
        
        default:{
            GUIDrawPosMarker(pos,Red);
        }break;
        
    }
    
    if(to_update){
        context->SetObjectOrientation(obj_id,pos,rot,scale);
    }
    
    return to_update;
}

void EditorGUI(SceneContext* context){
    
    auto to_update_view = EditorWidget(context,data->obj_id,data->widget_type);
    
    //control panel
    
    {
        
        GUIBeginWindow("Control Panel",&data->pos_1,&data->dim_1);
        
        if(GUIButton("Obj List")){
            data->show_object_list = !data->show_object_list;
        }
        
        if(GUIButton("Obj Editor")){
            data->show_object_editor = !data->show_object_editor;  
        }
        
        if(GUIButton("Profiler")){
            data->draw_profiler = !data->draw_profiler;
        }
        
    }
    
    //object list
    if(data->show_object_list){
        
        s8 title_buffer[128] = {};
        
        sprintf(title_buffer,"Object List(%d)",data->obj_id);
        
        GUIBeginWindow(title_buffer,&data->pos_2,&data->dim_2);
        
        for(u32 i = 0; i < data->orientation.count; i++){
            
            if(data->orientation.skip_array[i]){
                continue;
            }
            
            s8 buffer[128] = {};
            
            sprintf(buffer,"obj_id:%d",i);
            
            if(GUIButton(&buffer[0])){
                data->write_orientation = true;
                data->obj_id = i;
            }
            
        }
        
        if(GUIButton("Add Object")){
            data->obj_id = AddObject(context);
        }
        
        if(GUIButton("Remove Object")){
            
            RemoveObject(data->obj_id,context);
            
            auto tid = data->obj_id;
            
            data->obj_id = 0;
            
            if(tid > (data->orientation.count >> 1)){
                for(u32 j = data->orientation.count - 1; j != (u32)-1 ; j--){
                    
                    if(!data->orientation.skip_array[j]){
                        data->obj_id = j;
                        break;
                    }
                    
                }	
            }
            
            else{
                
                for(u32 j = 0; j < data->orientation.count ; j++){
                    
                    if(!data->orientation.skip_array[j]){
                        data->obj_id = j;
                        break;
                    }
                    
                }	
            }
            
        }
        
    }
    
    
    
    //component editor view
    if(data->show_object_editor){
        
        
        
        GUIBeginWindow("Object Editor",&data->w_pos,&data->w_dim);
        
        //orientation fields
        {
            
            //TODO: I will admit I was lazy, I'll do it individually at some point
            auto init_orientation_field = [](s8* b1,s8* b2,s8* b3,s8* b4)->void {
                sprintf(b1,"%f",data->orientation.pos_x[data->obj_id]);
                sprintf(b2,"%f",data->orientation.pos_y[data->obj_id]);
                sprintf(b3,"%f",data->orientation.pos_z[data->obj_id]);
                sprintf(b4,"%f",data->orientation.scale[data->obj_id]);
            };
            
            
            
            if(data->write_orientation || to_update_view){
                init_orientation_field(&data->o_buffer[0][0],&data->o_buffer[1][0],&data->o_buffer[2][0],&data->o_buffer[3][0]);
                data->write_orientation = false;
            }
            
            if(GUITextField("x",&data->o_buffer[0][0],false,_pos_width)){
                
                if(PIsStringFloat(&data->o_buffer[0][0])){
                    
                    auto value = (f32)atof(&data->o_buffer[0][0]);
                    
                    memcpy(&data->orientation.pos_x[data->obj_id],&value,
                           sizeof(data->orientation.pos_x[data->obj_id]));
                    
                    data->write_orientation = true;
                }
                
                init_orientation_field(&data->o_buffer[0][0],&data->o_buffer[1][0],&data->o_buffer[2][0],&data->o_buffer[3][0]);
            }
            
            
            if(GUITextField("y",&data->o_buffer[1][0],false,_pos_width)){
                
                if(PIsStringFloat(&data->o_buffer[1][0])){
                    
                    auto value = (f32)atof(&data->o_buffer[1][0]);
                    
                    memcpy(&data->orientation.pos_y[data->obj_id],&value,
                           sizeof(data->orientation.pos_y[data->obj_id]));
                    
                    data->write_orientation = true;
                }
                init_orientation_field(&data->o_buffer[0][0],&data->o_buffer[1][0],&data->o_buffer[2][0],&data->o_buffer[3][0]);
            }
            
            if(GUITextField("z",&data->o_buffer[2][0],false,_pos_width)){
                
                if(PIsStringFloat(&data->o_buffer[2][0])){
                    
                    auto value = (f32)atof(&data->o_buffer[2][0]);
                    
                    memcpy(&data->orientation.pos_z[data->obj_id],&value,
                           sizeof(data->orientation.pos_z[data->obj_id]));
                    
                    data->write_orientation = true;
                }
                init_orientation_field(&data->o_buffer[0][0],&data->o_buffer[1][0],&data->o_buffer[2][0],&data->o_buffer[3][0]);
            }
            
            if(GUITextField("scale",&data->o_buffer[3][0])){
                
                if(PIsStringFloat(&data->o_buffer[3][0])){
                    
                    auto value = (f32)atof(&data->o_buffer[3][0]);
                    
                    memcpy(&data->orientation.scale[data->obj_id],&value,
                           sizeof(data->orientation.scale[data->obj_id]));
                    
                    data->write_orientation = true;
                }
                init_orientation_field(&data->o_buffer[0][0],&data->o_buffer[1][0],&data->o_buffer[2][0],&data->o_buffer[3][0]);
            }
            
            if(data->write_orientation){
                
                Vector4 pos = {data->orientation.pos_x[data->obj_id],data->orientation.pos_y[data->obj_id],
                    data->orientation.pos_z[data->obj_id],1.0f};
                
                auto scale = data->orientation.scale[data->obj_id];
                
                auto rot = data->orientation.rot[data->obj_id];
                
                context->SetObjectOrientation(data->obj_id,pos,rot,scale);
            } 
            
        }
        
        u32 comphash_array[128];
        u32 comphash_count = 0;
        
        for(u32 i = 0; i < _arraycount(METACOMP_ARRAY); i++){
            
            auto entry = METACOMP_ARRAY[i];
            auto comp_meta = GetComponentData((ComponentStruct*)data->components,entry);
            
            auto data_array = comp_meta.array;
            auto data_count = *comp_meta.count;
            
            for(u32 j = 0; j < data_count; j++){
                
                auto comp_data_entry = data_array + (j * comp_meta.element_size);
                
                //check object id
                u32 id;
                
                MetaGetValueByName(comp_data_entry,0,&id,"id",comp_meta.metadata_table,
                                   comp_meta.metadata_count);
                
                if(id != data->obj_id){
                    continue;
                }
                
                
                comphash_array[comphash_count] = comp_meta.comp_name_hash;
                comphash_count++;
                
                GUIString(comp_meta.comp_name_string);
                
                if(comp_meta.comp_name_hash != PHashString("EntityAnimationData")){
                    
                    if(GUIButton("X")){
                        RemoveComponent(comp_meta.comp_name_hash,data->obj_id,context);
                    }
                    
                }
                
                for(u32 k = 0; k < comp_meta.metadata_count; k++){
                    
                    auto comp_meta_entry = comp_meta.metadata_table[k];
                    
                    if(InspectorIgnoreFields(comp_meta_entry.name_hash)){
                        continue;
                    }
                    
                    for(u32 a = 0; a < comp_meta_entry.arraycount; a++){
                        
                        s8 buffer[256] = {};
                        
                        MetaGetValueByName(comp_data_entry,a,&buffer[0],comp_meta_entry.name_string,
                                           comp_meta.metadata_table,comp_meta.metadata_count);
                        
                        if(IsIntType(comp_meta_entry.type_hash)){
                            sprintf(&buffer[0],"%d",*((u32*)(&buffer[0])));
                        }
                        
                        else if(IsFloatType(comp_meta_entry.type_hash)){
                            sprintf(&buffer[0],"%f",*((f32*)(&buffer[0])));
                        }
                        
                        else if(InspectorIsOpaqueType(comp_meta_entry.type_hash)){
                            
                            u32 ret_id =
                                InspectorHandleOpaqueTypes(comp_meta_entry.type_hash,*((u32*)(&buffer[0])),
                                                           data->obj_id,context);
                            
                            if(ret_id != (u32)-1){
                                
                                MetaSetValueByName(comp_data_entry,a,&ret_id,comp_meta_entry.name_string,
                                                   comp_meta.metadata_table,comp_meta.metadata_count);
                            }
                            
                            continue;
                        }
                        
                        auto cur_buffer = &buffer[0];
                        
                        if(GUIIsElementActive(comp_meta_entry.name_string)){
                            cur_buffer = &data->in_buffer[0];
                        }
                        
                        if(GUITextField(comp_meta_entry.name_string,cur_buffer)){
                            
                            if(PIsStringFloat(&data->in_buffer[0])){
                                
                                auto value = (f32)atof(&data->in_buffer[0]);
                                
                                if(IsIntType(comp_meta_entry.type_hash)){
                                    
                                    auto v = (u32)value;
                                    memcpy(&data->in_buffer[0],&v,sizeof(v)); 
                                }
                                
                                if(IsFloatType(comp_meta_entry.type_hash)){
                                    memcpy(&data->in_buffer[0],&value,sizeof(value)); 
                                }
                                
                                MetaSetValueByName(comp_data_entry,a,&data->in_buffer[0],comp_meta_entry.name_string,
                                                   comp_meta.metadata_table,comp_meta.metadata_count);
                            }
                            
                            memset(&data->in_buffer[0],0,sizeof(data->in_buffer));
                        }
                        
                    }
                    
                }
                
            }
            
        }
        
        s8* entry_array[_arraycount(METACOMP_ARRAY) + 1] = {
            (s8*)"Add Component",
        };
        
        u32 entry_count = 1;
        
        for(u32 i = 0; i < _arraycount(METACOMP_ARRAY); i++){
            
            auto metacomp = &METACOMP_ARRAY[i];
            
            if(!(FindHash(metacomp->comp_name_hash,comphash_array,comphash_count) ||
                 (metacomp->comp_name_hash == PHashString("EntityAnimationData")) ||
                 (metacomp->comp_name_hash == PHashString("EntityAudioData"))
                 
                 )){
                entry_array[entry_count] = &metacomp->comp_name_string[0];
                entry_count++;
            }
            
        }
        
        u32 id = 0;
        
        if(GUIComboBox("Comp",(const s8**)&entry_array[0],entry_count,&id)){
            
            if(id){
                AddComponent(PHashString(entry_array[id]),data->obj_id,context);
            }
            
        }
        
    }
    
    
#if 0
    {
        GUIDebugGetCurrentHolder();
    }
#endif
    
    if(!GUIIsAnyElementActive()){
        EditorKeyboard(context,&data->widget_type);  
    }
    
}

#endif
