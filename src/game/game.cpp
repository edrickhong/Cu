#ifdef _WIN32

#define WIN32DLL 1

#endif

#include "stdio.h"
#include "mode.h"

#include "wwindow.h"

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

#ifndef CPP_PASS

#include "gamecomp_meta.h"

#endif

#include "editor_ui.h"

#define _pos_width 0.345f


/*
TODO:
light
add a color picker
separate sampler and texture

FIXME: models is black when just added it. we do not set the material add at time
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
                
                context->SetObjectMaterial(obj_id,drawobj->material);
                
                
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
                light->radius = 1.0f;
                light->intensity = 1.0f;
            }
            
            if(compname_hash == PHashString("SpotLight")){
                
                auto light = (SpotLight*)obj;
                
                light->R = 1.0f;
                light->G = 1.0f;
                light->B = 1.0f;
                
                
                Vector3 dir = {0.0f,0.0f,1.0f};
                
                dir = RotateVector3(dir,data->orientation.rot[obj_id]);
                
                light->dir_x = dir.x;
                light->dir_y = dir.y;
                light->dir_z = dir.z;
                
                light->radius = 1.0f;
                light->intensity = 1.0f;
                
                light->full_angle = 60.0f;
                light->hard_angle = 50.0f;
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
    
    data->orientation.rot[id] = ConstructQuaternion(Vector3{1,0,0},_radians(0));
    
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
        
        Quaternion rot = ConstructQuaternion(Vector3{0,1,0},data->roty);
        
        Vector3 pos1 = {data->orientation.pos_x[0],data->orientation.pos_y[0],
            data->orientation.pos_z[0]};
        
        Vector3 pos2 = {data->orientation.pos_x[1],data->orientation.pos_y[1],
            data->orientation.pos_z[1]};
        
        Vector3 pos3 = {data->orientation.pos_x[2],data->orientation.pos_y[2],
            data->orientation.pos_z[2]};
        
        auto q1 = rot * data->orientation.rot[0];
        auto q2 = rot * data->orientation.rot[1];
        auto q3 = rot * data->orientation.rot[2];
        
        context->SetObjectOrientation(0,pos1,q1,data->orientation.scale[0]);
        context->SetObjectOrientation(1,pos2,q2,data->orientation.scale[1]);
        context->SetObjectOrientation(2,pos3,q3,data->orientation.scale[2]); 
    }
    
}

#define _enable_read_log 0


void ComponentRead(ComponentStruct* components,SceneContext* context){
    
    if(!FIsFileExists(_COMPFILE)){
        return;
    }
    
    auto file = FOpenFile(_COMPFILE,F_FLAG_READONLY);
    
    u32 metacount;
    
    FRead(file,&metacount,sizeof(metacount));
    
#if _enable_read_log
    
    printf("meta count %d\n",metacount);
    
#endif
    
    for(u32 i = 0; i < metacount; i++){
        
        u32 comp_hash;
        u32 comp_count;
        u32 field_count;
        
        {
            s8 buffer[128] = {};
            
            FRead(file,(void*)&buffer[0],sizeof(buffer));
            
            comp_hash = PHashString(buffer);
        }
        
        
        
        FRead(file,&comp_count,sizeof(comp_count));
        FRead(file,&field_count,sizeof(field_count));
        
#if _enable_read_log
        
        printf("c_hash %d c_count %d c_field_count %d\n",comp_hash,comp_count,field_count);
        
#endif
        
        auto comp = MetaGetCompByNameHash(comp_hash);
        
        MetaDataCompOut out_comp;
        
        if(comp){
            out_comp = GetComponentData(components,*comp);
            (*out_comp.count) = comp_count;
            
#if _enable_read_log
            
            printf("%d name %s\n",i,out_comp.comp_name_string);
            
#endif
        }
        
        else{
            
#if _enable_read_log
            
            
            printf("%d comp name not found\n",i);
            
#endif
        }
        
        for(u32 j = 0; j < comp_count; j++){
            
            for(u32 k = 0; k < field_count; k++){
                
                u32 type_hash;
                
                {
                    s8 buffer[128] = {};
                    
                    FRead(file,(void*)&buffer[0],sizeof(buffer));
                    
                    type_hash = PHashString(buffer);
                }
                
                
                u32 type_size;
                FRead(file,&type_size,sizeof(type_size));
                
                u32 name_hash;
                
                {
                    s8 buffer[128] = {};
                    
                    FRead(file,(void*)&buffer[0],sizeof(buffer));
                    
                    name_hash = PHashString(buffer);
                }
                
                
                u32 arraycount;
                FRead(file,&arraycount,sizeof(arraycount));
                
                
                
#if _enable_read_log
                
                
                printf("t_hash %d t_size %d t_name_hash %d t_count %d\n",type_hash,type_size,name_hash,arraycount);
                
#endif
                
                
                for(u32 a = 0; a < arraycount; a++){
                    
                    s8 buffer[128] = {};
                    
                    FRead(file,&buffer[0],type_size);
                    
#if _enable_read_log
                    
                    
                    printf("read file %d\n",type_size);
                    
#endif
                    
                    if(comp){
                        
                        if(MetaGetTypeByNameHash(name_hash,&out_comp.metadata_table[0],
                                                 out_comp.metadata_count) == type_hash){
                            
                            s8* obj = out_comp.array + (j * out_comp.element_size);
                            
                            
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
            
            Vector3 pos = {data->orientation.pos_x[i],data->orientation.pos_y[i],
                data->orientation.pos_z[i]};
            
            auto scale = data->orientation.scale[i];
            
            auto rot = data->orientation.rot[i];
            
            context->SetObjectOrientation(i,pos,rot,scale);
            
        }
        
        //ambient light
        FRead(file,&data->ambient_color,sizeof(Color));
        FRead(file,&data->ambient_intensity,sizeof(data->ambient_intensity));
        
        context->SetAmbientColor(data->ambient_color,data->ambient_intensity);
        
        u32* dir_count = 0;
        DirLight* dir_array = 0;
        
        ((SceneContext*)context)->GetDirLightList(&dir_array,&dir_count);
        
        FRead(file,dir_count,sizeof(*dir_count));
        
        for(u32 i = 0; i < (*dir_count); i++){
            
            auto k = &dir_array[i];
            
            FRead(file,&k->dir,sizeof(k->dir));
            FRead(file,&k->color,sizeof(k->color));
        }
        
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
        
        auto pos = Vector3{data->orientation.pos_x[light->id],data->orientation.pos_y[light->id],data->orientation.pos_z[light->id]};
        
        Vector4 c = {light->R,light->G,light->B,1.0f};
        c =  c * light->intensity;
        
        context->AddPointLight(pos,Color{c.x,c.y,c.z,1.0f},light->radius);
        
#ifdef DEBUG
        
#if 0
        GUIDrawAxisSphere(pos,4.0f);
#endif
        
#endif
    }
    
    for(u32 i = 0; i < comp->spotlight_count; i++){
        
        auto light = &comp->spotlight_array[i];
        
        auto pos = Vector3{data->orientation.pos_x[light->id],data->orientation.pos_y[light->id],data->orientation.pos_z[light->id]};
        
        auto dir = Vector3{light->dir_x,light->dir_y,light->dir_z};
        
        Vector4 c = {light->R,light->G,light->B,1.0f};
        c =  c * light->intensity;
        
        context->AddSpotLight(pos,dir,Color{c.x,c.y,c.z,1.0f},light->full_angle,light->hard_angle,light->radius);
        
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


u32 GetNumberOfUsedComponents(){
    
    u32 count = 0;
    
    for(u32 i = 0; i < _arraycount(METACOMP_ARRAY); i++){
        
        auto component = &METACOMP_ARRAY[i];
        
        auto out_comp = GetComponentData((ComponentStruct*)data->components,*component);
        
        if((*out_comp.count)){
            count++;
        }
    }
    
    return count;
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
            auto count = GetNumberOfUsedComponents();
            
            FWrite(outfile,&count,sizeof(count));
        }
        
        
        
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
            
            if(!(*out_comp.count)){
                continue;
            }
            
            //comp name string
            
            FWrite(outfile,(void*)&out_comp.comp_name_string[0],sizeof(MetaDataStructEntry::comp_name_string));
            
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
                    
                    //type string
                    FWrite(outfile,(void*)&entry->type_string[0],sizeof(entry->type_string));
                    
                    //size
                    FWrite(outfile,&entry->size,sizeof(entry->size));
                    
                    //name string
                    FWrite(outfile,(void*)&entry->name_string[0],sizeof(entry->name_string));
                    
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
        
        
        //static stuff can go here so we can just append stuff
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
            
            //ambient light
            FWrite(outfile,&data->ambient_color,sizeof(Color));
            FWrite(outfile,&data->ambient_intensity,sizeof(data->ambient_intensity));
            
            //dir light
            
            u32* dir_count = 0;
            DirLight* dir_array = 0;
            
            ((SceneContext*)context)->GetDirLightList(&dir_array,&dir_count);
            
            FWrite(outfile,dir_count,sizeof(*dir_count));
            
            for(u32 i = 0; i < (*dir_count); i++){
                
                auto k = &dir_array[i];
                
                FWrite(outfile,&k->dir,sizeof(k->dir));
                FWrite(outfile,&k->color,sizeof(k->color));
            }
            
        }
        
        FCloseFile(outfile);
    }
    
    
    _dllexport void GameInit(GameInitData* initdata){
        
        data = (GameData*)initdata->memory;
        
        data->running = true;
        
        data->camera_pos = Vector3{0.0f,0.0f,-4.0f};
        data->camera_lookdir = Vector3{0.0f,0.0f,1.0f};
        
        
#ifdef DEBUG
        //set gui state
        
        data->draw_profiler = true;
        
        data->prev_mpos = {};
        data->widget_type = 0;
        data->obj_id = 0;
        data->show_object_list = false;
        data->show_object_editor = false;
        data->pos_control = {-1.0f,1.0f};
        data->dim_control = {GUIDEFAULT_W * 6.8f,GUIDEFAULT_H * 0.22f};
        
        data->write_orientation = true;
        
        data->pos_obj_list = {-0.16f,GUIDEFAULT_Y};
        data->dim_obj_list = {GUIDEFAULT_W * 2.2f,GUIDEFAULT_H};
        
        data->pos_obj_editor = {0.4f,GUIDEFAULT_Y};
        data->dim_obj_list = {GUIDEFAULT_W * 2.2f,GUIDEFAULT_H * 2.5f};
        
        data->dirlight_id = (u32)-1;
        
        data->pos_dirlight = data->pos_obj_editor;
        data->pos_ambient = data->pos_dirlight;
        
        memset(&data->o_buffer[0][0],0,sizeof(data->o_buffer));
        
#endif
        
        data->ambient_color = White;
        data->ambient_intensity = 0.4f;
        
        initdata->context->SetAmbientColor(data->ambient_color,data->ambient_intensity);
        
        
    }
    
    _dllexport void GameUpdateRender(SceneContext* context){
        
        TIMEBLOCK(LimeGreen);
        
#if 0
        ComponentStartInit(context);
        GameComponentWrite();
        exit(0);
#endif
        
#ifdef DEBUG 
#if _enable_gui
        
        EditorGUI(context);
        
#endif
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
        
        //MARK: why is this here?
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


#ifdef DEBUG

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
        
        Vector3 lookdir = data->camera_lookdir;
        
        if(x_len != 0){
            x_angle = atanf(x_len/1.0f);
        }
        
        if(y_len != 0){
            y_angle = atanf(y_len/1.0f);
        }
        
        if(fabsf(x_angle) > fabsf(y_angle)){
            
            lookdir = RotateVector(lookdir,
                                   Vector3{0,x_angle});  
        }
        
        else{
            
            auto k = RotateVector(lookdir,
                                  Vector3{y_angle});  
            
            if((1.0f - Dot(k,Vector3{0.0f,-1.0f,0.0f})) > 0.01f){
                lookdir = k;
            }
        }
        
        lookdir = Normalize(lookdir);
        
        data->camera_lookdir = lookdir;
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
    auto s = Cross(data->camera_lookdir,Vector3{0,-1});
    
    Vector3 dir = {};
    
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
    
    
    if(Magnitude(dir)){
        dir = Normalize(dir);  
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
    
    if(obj_id == (u32)-1){
        return false;
    }
    
    
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
                
                //rotate spot lights. we will not tread dir lights as objects
                
                auto comp = (ComponentStruct*)data->components;
                
                for(u32 i = 0; i < comp->spotlight_count; i++){
                    
                    auto light = &comp->spotlight_array[i];
                    
                    if(obj_id == light->id){
                        
                        Vector3 dir = {0.0f,0.0f,1.0f};
                        
                        dir = RotateVector3(dir,data->orientation.rot[obj_id]);
                        
                        light->dir_x = dir.x;
                        light->dir_y = dir.y;
                        light->dir_z = dir.z;
                        
                        break;
                    }
                }
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
    
    EditorWidget(context,data->obj_id,data->widget_type);
    
    //control panel
    
    {
        
        GUIBeginWindow("Control Panel",&data->pos_control,&data->dim_control);
        
        if(GUIButton("Obj List")){
            data->show_object_list = !data->show_object_list;
        }
        
        if(GUIButton("Obj Editor") && data->obj_id != (u32)-1){
            data->show_object_editor = !data->show_object_editor;  
        }
        
        if(GUIButton("Profiler")){
            data->draw_profiler = !data->draw_profiler;
        }
        
        if(GUIButton("Dir Light")){
            data->show_dir_light_editor = !data->show_dir_light_editor;
            data->dirlight_id = (u32)-1;
        }
        
        if(GUIButton("Ambient Light")){
            
            data->show_ambient_light_editor = !data->show_ambient_light_editor;
        }
        
    }
    
    
    if(data->show_ambient_light_editor){
        
        GUIBeginWindow("Ambient Light",&data->pos_ambient,&data->dim_obj_list);
        
        auto color = &data->ambient_color;
        auto intensity = &data->ambient_intensity;
        
        auto write_values = false;
        
        s8 buffer[128] = {};
        
        sprintf(&buffer[0],"%f",color->R);
        
        if(GUITextField("R",&buffer[0])){
            color->R = (f32)atof(&buffer[0]);
            write_values = true;
        }
        
        memset(&buffer[0],0,sizeof(buffer));
        
        sprintf(&buffer[0],"%f",color->G);
        
        if(GUITextField("G",&buffer[0])){
            color->G = (f32)atof(&buffer[0]);
            write_values = true;
        }
        
        memset(&buffer[0],0,sizeof(buffer));
        
        sprintf(&buffer[0],"%f",color->B);
        
        if(GUITextField("B",&buffer[0])){
            color->B = (f32)atof(&buffer[0]);
            write_values = true;
        }
        
        memset(&buffer[0],0,sizeof(buffer));
        
        sprintf(&buffer[0],"%f",*intensity);
        
        if(GUITextField("intensity",&buffer[0])){
            *intensity = (f32)atof(&buffer[0]);
            write_values = true;
        }
        
        memset(&buffer[0],0,sizeof(buffer));
        
        if(write_values){
            
            context->SetAmbientColor(*color,*intensity);
        }
    }
    
    //directional light list (this should overwrite the regular widgets)
    if(data->show_dir_light_editor){
        
        s8 title_buffer[128] = {};
        
        sprintf(&title_buffer[0],"Directional Lights(%d)",data->dirlight_id);
        
        GUIBeginWindow(&title_buffer[0],&data->pos_dirlight,&data->dim_obj_list);
        
        //MARK: if you think about it, directional lights are constant. maybe we should just have a dir light register instead of using AddDirLight that clears every frame
        
        u32* dir_count = 0;
        DirLight* dir_array = 0;
        
        context->GetDirLightList(&dir_array,&dir_count);
        
        //list lights
        for(u32 i = 0; i < *dir_count; i++){
            
            s8 buffer[128] = {};
            
            sprintf(&buffer[0],"light: %d",i);
            
            if(GUIButton(&buffer[0])){
                data->dirlight_id = i;
            }
        }
        
        if(GUIButton("Add Light")){
            dir_array[*dir_count] = {Vector4{0.0f,0.0f,1.0f,1.0f},White};
            data->dir_light_rot[*dir_count] = ConstructQuaternion(Vector3{0.0f,1.0f,0.0f},0.0f);
            
            data->dir_light_color[*dir_count] = White;
            data->dir_light_intensity[*dir_count] = 1.0f;
            
            data->dirlight_id = (*dir_count);
            (*dir_count)++;
        }
        
        if(GUIButton("Remove Light")){
            (*dir_count)--;
            dir_array[data->dirlight_id] = dir_array[*dir_count];
            
            data->dirlight_id = (*dir_count) - 1;
        }
        
        
        
        if(data->dirlight_id != (u32)-1){
            
            auto light = &dir_array[data->dirlight_id];
            auto rot = &data->dir_light_rot[data->dirlight_id];
            Vector3 dir = Vector3{0.0f,0.0f,1.0f};
            
            auto color = &data->dir_light_color[data->dirlight_id];
            auto intensity = &data->dir_light_intensity[data->dirlight_id];
            
            logic write_values = false;
            
            //dir light fields
            s8 buffer[128] = {};
            
            sprintf(&buffer[0],"%f",color->R);
            
            if(GUITextField("R",&buffer[0])){
                color->R = (f32)atof(&buffer[0]);
                write_values = true;
            }
            
            memset(&buffer[0],0,sizeof(buffer));
            
            sprintf(&buffer[0],"%f",color->G);
            
            if(GUITextField("G",&buffer[0])){
                color->G = (f32)atof(&buffer[0]);
                write_values = true;
            }
            
            memset(&buffer[0],0,sizeof(buffer));
            
            
            sprintf(&buffer[0],"%f",color->B);
            
            if(GUITextField("B",&buffer[0])){
                color->B = (f32)atof(&buffer[0]);
                write_values = true;
            }
            
            memset(&buffer[0],0,sizeof(buffer));
            
            
            sprintf(&buffer[0],"%f",*intensity);
            
            if(GUITextField("intensity",&buffer[0])){
                *intensity = (f32)atof(&buffer[0]);
                write_values = true;
            }
            
            memset(&buffer[0],0,sizeof(buffer));
            
            if(write_values){
                auto c = Vector4{color->R,color->G,color->B,color->A} * (*intensity);
                light->color = Color{c.x,c.y,c.z,1.0f};
            }
            
            //rotation widget and maybe scale to control intensity
            
            auto pos = data->camera_pos + (Normalize(data->camera_lookdir) * 2.0f);
            
            if(GUIRotationGizmo(pos,rot)){
                
                light->dir = ToVec4(RotateVector3(dir,*rot));
            }
        }
        
        
    }
    
    //object list
    if(data->show_object_list){
        
        s8 title_buffer[128] = {};
        
        sprintf(title_buffer,"Object List(%d)",data->obj_id);
        
        GUIBeginWindow(title_buffer,&data->pos_obj_list,&data->dim_obj_list);
        
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
            
            data->obj_id = (u32)-1;
            
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
    if(data->show_object_editor && data->orientation.count){
        
        if(data->obj_id == (u32)-1){
            data->show_object_editor = false;
        }
        
        GUIBeginWindow("Object Editor",&data->pos_obj_editor,&data->dim_obj_list);
        
        //orientation fields
        {
            
            sprintf(&data->o_buffer[0][0],"%f",data->orientation.pos_x[data->obj_id]);
            
            sprintf(&data->o_buffer[1][0],"%f",data->orientation.pos_y[data->obj_id]);
            
            sprintf(&data->o_buffer[2][0],"%f",data->orientation.pos_z[data->obj_id]);
            
            sprintf(&data->o_buffer[3][0],"%f",data->orientation.scale[data->obj_id]);
            
            if(GUITextField("x",&data->o_buffer[0][0],false,_pos_width)){
                
                if(PIsStringFloat(&data->o_buffer[0][0])){
                    
                    auto value = (f32)atof(&data->o_buffer[0][0]);
                    
                    memcpy(&data->orientation.pos_x[data->obj_id],&value,
                           sizeof(data->orientation.pos_x[data->obj_id]));
                    
                    data->write_orientation = true;
                }
            }
            
            
            if(GUITextField("y",&data->o_buffer[1][0],false,_pos_width)){
                
                if(PIsStringFloat(&data->o_buffer[1][0])){
                    
                    auto value = (f32)atof(&data->o_buffer[1][0]);
                    
                    memcpy(&data->orientation.pos_y[data->obj_id],&value,
                           sizeof(data->orientation.pos_y[data->obj_id]));
                    
                    data->write_orientation = true;
                }
            }
            
            if(GUITextField("z",&data->o_buffer[2][0],false,_pos_width)){
                
                if(PIsStringFloat(&data->o_buffer[2][0])){
                    
                    auto value = (f32)atof(&data->o_buffer[2][0]);
                    
                    memcpy(&data->orientation.pos_z[data->obj_id],&value,
                           sizeof(data->orientation.pos_z[data->obj_id]));
                    
                    data->write_orientation = true;
                }
            }
            
            if(GUITextField("scale",&data->o_buffer[3][0])){
                
                if(PIsStringFloat(&data->o_buffer[3][0])){
                    
                    auto value = (f32)atof(&data->o_buffer[3][0]);
                    
                    memcpy(&data->orientation.scale[data->obj_id],&value,
                           sizeof(data->orientation.scale[data->obj_id]));
                    
                    data->write_orientation = true;
                }
            }
            
            if(data->write_orientation){
                
                data->write_orientation = false;
                
                Vector3 pos = {data->orientation.pos_x[data->obj_id],data->orientation.pos_y[data->obj_id],
                    data->orientation.pos_z[data->obj_id]};
                
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
                        
                        if(GUITextField(comp_meta_entry.name_string,cur_buffer)){
                            
                            if(PIsStringFloat(&cur_buffer[0])){
                                
                                auto value = (f32)atof(&cur_buffer[0]);
                                
                                if(IsIntType(comp_meta_entry.type_hash)){
                                    
                                    auto v = (u32)value;
                                    memcpy(&cur_buffer[0],&v,sizeof(v)); 
                                }
                                
                                if(IsFloatType(comp_meta_entry.type_hash)){
                                    memcpy(&cur_buffer[0],&value,sizeof(value)); 
                                }
                                
                                MetaSetValueByName(comp_data_entry,a,&cur_buffer[0],comp_meta_entry.name_string,
                                                   comp_meta.metadata_table,comp_meta.metadata_count);
                            }
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
