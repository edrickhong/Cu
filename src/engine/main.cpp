#include "main.h"

_compile_kill(sizeof(SkelUBO) > _kilobytes(64));
_compile_kill(sizeof(LightUBO) > _kilobytes(64));
_compile_kill(sizeof(PushConst) > 128);
_compile_kill(VK_INDEX_TYPE_UINT16 != 0);
_compile_kill(VK_INDEX_TYPE_UINT32 != 1);

REFDOC(Settings,ParseSettings,{
		\documentclass[12pt]{}
		\begin{document}
		Hello World!
		\end{document}
		});



#if 0
void InitParticles(){


	tdata.emitter_sbo = VCreateShaderStorageBufferContext(&pdata->vdevice,sizeof(Emitters),VBLOCK_READWRITE);
	tdata.particle_sbo =  VCreateShaderStorageBufferContext(&pdata->vdevice,sizeof(Particles),VBLOCK_DEVICE);

	tdata.particle_vbo = TCreateStaticVertexBuffer(&pdata->vdevice,sizeof(Vec4) * _max_particles * 4,0,VBLOCK_DEVICE);
	tdata.particle_ibo = TCreateStaticIndexBuffer(&pdata->vdevice,sizeof(u32) * _max_particles * 6,sizeof(u32),VBLOCK_DEVICE);

	auto ptr = (Emitters*)VGetReadWriteBlockPtr(&tdata.emitter_sbo);
	memset(ptr,0,sizeof(Emitters));

	ptr->emitters[0] = {{0.0f,-8.0f,0.0f},0.0f,100.0f};
	ptr->emitter_use[0] = true;

	//create compute pipeline
	SPXData comp_shader[] = {
		LoadSPX(SHADER_PATH(generate_particles.comp.spx)),
	};
	auto shader_obj = VMakeShaderObjSPX(comp_shader,1);

	VDescriptorPoolSpec poolspec;

	VDescPushBackPoolSpec(&poolspec,&shader_obj);
	auto pool = VCreateDescriptorPoolX(&pdata->vdevice,poolspec);
	VkDescriptorSetLayout layouts[] = {
		VCreateDescriptorSetLayout(&pdata->vdevice,&shader_obj,0),
		VCreateDescriptorSetLayout(&pdata->vdevice,&shader_obj,1),
	};

	VAllocDescriptorSetArray(&pdata->vdevice,pool,_arraycount(layouts),layouts,tdata.sets);

	tdata.layout = VCreatePipelineLayout(&pdata->vdevice,layouts,_arraycount(layouts),&shader_obj);

	auto emitter_binfo = VGetBufferInfo(&tdata.emitter_sbo,0,tdata.emitter_sbo.size);
	auto particle_binfo = VGetBufferInfo(&tdata.particle_sbo,0,tdata.particle_sbo.size);

	auto vbo_binfo = VGetBufferInfo(&tdata.particle_vbo,0,tdata.particle_vbo.size);
	auto ibo_binfo = VGetBufferInfo(&tdata.particle_ibo,0,tdata.particle_ibo.size);

	VDescriptorWriteSpec writespec;
	VDescPushBackWriteSpecBuffer(&writespec,tdata.sets[0],0,0,1,VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,&emitter_binfo);
	VDescPushBackWriteSpecBuffer(&writespec,tdata.sets[0],1,0,1,VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,&particle_binfo);


	VDescPushBackWriteSpecBuffer(&writespec,tdata.sets[1],0,0,1,VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,&vbo_binfo);
	VDescPushBackWriteSpecBuffer(&writespec,tdata.sets[1],1,0,1,VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,&ibo_binfo);

	VUpdateDescriptorSets(&pdata->vdevice,writespec);

	VComputePipelineSpec spec = {};
	VGenerateComputePipelineSpec(&spec,tdata.layout);
	VSetComputePipelineSpecShader(&spec,comp_shader[0].spv,comp_shader[0].spv_size);
	VCreateComputePipelineArray(&pdata->vdevice,0,&spec,1,&tdata.pipeline);

	VkCommandBuffer cmdbuffer = pdata->rendercmdbuffer_array[0].buffer;

	VStartCommandBuffer(cmdbuffer,VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

	vkCmdFillBuffer(cmdbuffer,tdata.particle_sbo.buffer,0,tdata.particle_sbo.size,0);
	vkCmdFillBuffer(cmdbuffer,tdata.particle_vbo.buffer,0,tdata.particle_vbo.size,0);
	vkCmdFillBuffer(cmdbuffer,tdata.particle_ibo.buffer,0,tdata.particle_ibo.size,0);

	VEndCommandBuffer(cmdbuffer);

	VkQueue queue = pdata->root_queue;
	VSubmitCommandBuffer(queue,cmdbuffer);

	vkQueueWaitIdle(queue);

	//TODO:create draw pipeline
	{
		SPXData shader_data[] = {
			LoadSPX(SHADER_PATH(particles.vert.spx)),
			LoadSPX(SHADER_PATH(particles.frag.spx))
		};
		auto shader_obj = VMakeShaderObjSPX(shader_data,_arraycount(shader_data));
		tdata.draw_layout = VCreatePipelineLayout(&pdata->vdevice,0,0,&shader_obj);

		auto spec = VMakeGraphicsPipelineSpecObj(&pdata->vdevice,&shader_obj,tdata.draw_layout,pdata->renderpass,0,&pdata->swapchain);
		VSetDepthStencilGraphicsPipelineSpec(&spec,
				VK_TRUE,
				VK_TRUE,VK_COMPARE_OP_LESS_OR_EQUAL,
				VK_TRUE);

		VCreateGraphicsPipelineArray(&pdata->vdevice,&spec,1,&tdata.draw_pipeline,0);
	}

}


struct MouseKey{
	Vec3 pos;
	TimeSpec timestamp;
};

_global MouseKey mousekey_array[1024 * 1024] = {};
_global u32 mousekey_count = 0;


void CreateDirVectors(){
	auto ptr = (Emitters*)VGetReadWriteBlockPtr(&tdata.emitter_sbo);
	u32 count = mousekey_count - 1;
	auto keys = ptr->keys_array;
	ptr->keys_count = count;

	f32 elapsed_time = 0.0f;

	for(u32 i = 0; i < count; i++){
		auto k = &keys[i];

		auto m1 = mousekey_array[i];
		auto m2 = mousekey_array[i + 1];

		auto timediff = GetTimeDifferenceMS(m1.timestamp,m2.timestamp); 
		elapsed_time += timediff;
		auto dir = (m2.pos - m1.pos)/timediff;

		k->dir = Vec3ToDir4(dir);
		k->time = elapsed_time;

		//printf("%f %f %f | %f (%f)\n",
				//(f64)dir.x,(f64)dir.y,(f64)dir.z,(f64)elapsed_time,(f64)timediff);
	}

	//exit(0);

}


void ParticleFunction(Mat4 viewproj){

	//particles stuff
	
	if(IsMouseDown(&pdata->mousestate,MOUSEBUTTON_LEFT)){
#if 1

		auto key = &mousekey_array[mousekey_count];
		mousekey_count ++;


		auto mpos = PixelCoordToNDC({(f32)pdata->mousestate.x,(f32)pdata->mousestate.y},
				{(f32)pdata->swapchain.width,(f32)pdata->swapchain.height});

		auto mforward = PixelCoordToNDC({(f32)pdata->mousestate.x,(f32)pdata->mousestate.y,1.0f},
				{(f32)pdata->swapchain.width,(f32)pdata->swapchain.height});

		auto pos = ClipSpaceToWorldSpaceVec3(mpos,viewproj);
		auto forward = ClipSpaceToWorldSpaceVec3(mforward,viewproj);

		forward = NormalizeVec3(forward - pos);

		auto ray = ConstructRay3(pos,forward);
		IntersectOutRay3Plane(ray,ConstructPlaneD({0,0,-1},0.0f),&key->pos);
		GetTime(&key->timestamp);


#endif
	}
	

	if(OnMouseUp(&pdata->mousestate,MOUSEBUTTON_LEFT)){
		CreateDirVectors();
		mousekey_count = 0;
		printf("Created dir function\n");
	}

	//draw positions
	for(u32 i = 0; i < mousekey_count; i++){
		auto p = mousekey_array[i].pos;
		GUIDrawPosMarker(p,Green);
	}

}

#endif


#define _grid_dim 16

struct Nome{
	struct Coord{
		u16 x,y;
	};

	Coord pos;
	Coord path[_grid_dim * _grid_dim];
	u32 count;
	u32 i;
	Coord target;
	b32 is_dead;
	f32 time;
	f32 move_time;
};

enum TileState : u8{
	FREE_TILE = 0,
	WALL_TILE = 1,
	CAMPA_TILE = 'a',
	CAMPB_TILE = 'b',
	A_TILE,
	B_TILE,
	RESERVED_TILE,
};

_global Nome nomes_a[8] = {};
_global Nome nomes_b[8] = {};

_global Nome::Coord camp_a = {};
_global Nome::Coord camp_b = {};

_global u8 grid[_grid_dim][_grid_dim] = {
	{0,0,0,0,1,0,0,0,0,0,0,1,1,1,1,'a'},
	{0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,1},
	{0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1},
	{0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0},
	{0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,0},
	{0,0,0,0,1,0,0,1,1,1,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0},
	{0,0,0,0,1,0,0,1,1,0,0,0,0,0,0,0},
	{0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,0},
	{0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,1,'b',0,0,0,0,0,0,0,0,0,0},
};


_global Vec3 origin = {-5,-5,5};
_global f32 w = 2;


struct Path{
	Nome::Coord pos;
	Path* parent = 0;
};

Vec3 CoordToVec3(Nome::Coord coord){
	auto x = coord.x;
	auto y = coord.y;
	return origin + (Vec3{(f32)x,(f32)y} * w);
}


#define _rd_map(map,x,y)  map[x + ((y) * _grid_dim)]

void AStarGetNeighbors(Nome::Coord tile,Path* neighbors,u32* count,u8* map){
	//up down left right
	u32 c = 0;

	if(tile.x + 1 < _grid_dim && ! _rd_map(map,tile.x + 1,tile.y)){
		neighbors[c] = Path{(u16)(tile.x + 1),tile.y};
		c++;
	}


	if(tile.x != 0 && !_rd_map(map,tile.x - 1,tile.y)){
		neighbors[c] = {(u16)(tile.x - 1),tile.y};
		c++;
	}

	if(tile.y + 1 < _grid_dim && !_rd_map(map,tile.x,tile.y + 1)){
		neighbors[c] = {tile.x,(u16)(tile.y + 1)};
		c++;
	}

	if(tile.y != 0 && !_rd_map(map,tile.x,tile.y - 1)){
		neighbors[c] = {tile.x,(u16)(tile.y - 1)};
		c++;
	}

	*count = c;
}

f32 GetCost(Nome::Coord tile,Nome::Coord start,Nome::Coord goal){
	f32 dist1 = sqrtf((tile.x - goal.x) * (tile.x - goal.x) + (tile.y - goal.y) * (tile.y - goal.y));
	f32 dist2 = sqrtf((tile.x - start.x) * (tile.x - start.x) + (tile.y - start.y) * (tile.y - start.y));
	return dist1 + dist2;
}

void PrintMap(u8* map,Nome::Coord goal){
	for(int y = 0; y < _grid_dim; y++){
		for(int x = 0; x < _grid_dim; x++){
			if(x == goal.x && y == goal.y){
				printf("g ");
			}
			else{
				printf("%d ",_rd_map(map,x,y));
			}
		}
		printf("\n");
	}
}

void PrintTile(Nome::Coord tile){
	printf("%d %d\n",(u32)tile.x,(u32)tile.y);
}

void AddPath(Path* path,Nome::Coord* coords,u32* count){

	if(path->parent){
		AddPath(path->parent,coords,count);
	}
	coords[*count] = path->pos;
	(*count)++;
	//printf("%d\n",*count);
}


void AStarPath(Nome::Coord start,Nome::Coord goal,Nome::Coord* out,u32* out_count){
	auto map = TAlloc(u8,sizeof(grid));
	memcpy(map,grid,sizeof(grid));
	_rd_map(map,start.x,start.y) = 2;
	_rd_map(map,goal.x,goal.y) = 0;

	//auto activetiles_array = TAlloc(Path, _grid_dim * _grid_dim);
	Path activetiles_array[_grid_dim * _grid_dim] = {};
	activetiles_array [0] = {start};
	u32 activetiles_count = 1;

	while(true){
		//PrintMap(map,goal);
		//_breakpoint();
		for(u32 i = activetiles_count - 1; i != (u32)-1; i--){
			const auto tile = &activetiles_array[i];

			Path neighbors[4] = {};
			f32 costs[4] = {};
			u32 count = 0;

			AStarGetNeighbors(tile->pos,neighbors,&count,map);

			if(!count){
				continue;
			}

			for(u32 j = 0; j < count; j++){
				costs[j] = GetCost(neighbors[j].pos,start,goal);
			}

			f32 min = 1024.0f * 1024.0f;
			u32 min_index = (u32)-1;

			for(u32 j = 0; j < count; j++){
				auto n = neighbors[j];
				if(n.pos.x == goal.x && n.pos.y == goal.y){
					//reach goal!
					min_index = j;
					break;
				}

				if(costs[j] < min){
					min = costs[j];
					min_index = j;
				}
			}

			auto n_tile = neighbors[min_index];
			n_tile.parent = tile;

			if(n_tile.pos.x == goal.x && n_tile.pos.y == goal.y){
				//found path
				AddPath(&n_tile,out,out_count);
				return;
			}

			activetiles_array[activetiles_count] = n_tile;
			activetiles_count++;

			_rd_map(map,n_tile.pos.x,n_tile.pos.y) = RESERVED_TILE;

			//printf("added tile:"); PrintTile(n_tile);

		}
	}


	_kill("should never reach here\n",1);
}

void UpdatePath(){

	for(u32 i = 0; i < _arraycount(nomes_a); i++){
		auto nome = &nomes_a[i];
		AStarPath(nome->pos,camp_b,nome->path,&nome->count);
	}

	for(u32 i = 0; i < _arraycount(nomes_b); i++){
		auto nome = &nomes_b[i];
		AStarPath(nome->pos,camp_a,nome->path,&nome->count);
	}
}

f32 NRand(){
	return ((f32)rand() / (f32)RAND_MAX);
}

enum NomeState{
	DEFAULT_NOME_STATE,
	SAD_NOME_STATE,
	ANGRY_NOME_STATE,
	AFRAID_NOME_STATE,
};


void AddWall(u8 x,u8 y){
	grid[y][x] = WALL_TILE;
	UpdatePath();
}
void MakeAngry(NomeState* state){
	*state = ANGRY_NOME_STATE;
}
void MakeSad(NomeState* state){}
void MakeAfraid(NomeState* state){}


#define _default_time 1000.0f

void InitSim(){

	for(u32 y = 0; y < _grid_dim; y++){
		for(u32 x = 0; x < _grid_dim; x++){
			if(grid[y][x] == 'a'){
				camp_a = {(u16)x,(u16)y};
				for(u32 i = 0; i < _arraycount(nomes_a); i++){
					nomes_a[i].pos = {(u16)x,(u16)y};
					nomes_a[i].time = NRand() * _default_time * 1.1f;
					nomes_a[i].move_time = _default_time - (NRand() * _default_time/2.0f);

					//printf("a %d %f %f\n",i,(f64)nomes_a[i].time,(f64)nomes_a[i].move_time);
				}
			}

			else if(grid[y][x] == 'b'){
				camp_b = {(u16)x,(u16)y};
				for(u32 i = 0; i < _arraycount(nomes_b); i++){
					nomes_b[i].pos = {(u16)x,(u16)y};
					nomes_b[i].time = NRand() * _default_time * 1.1f;
					nomes_b[i].move_time = _default_time - (NRand() * _default_time/2.0f);

					//printf("b %d %f %f\n",i,(f64)nomes_b[i].time,(f64)nomes_b[i].move_time);
				}
			}
		}
	}


	UpdatePath();
}

#define _a_color Green
#define _b_color Coral



void Sim(f32 delta){

	static u32 a_dead_count = 0;
	static u32 b_dead_count = 0;


	static NomeState a_state = ANGRY_NOME_STATE;
	static NomeState b_state = DEFAULT_NOME_STATE;



	auto update_nome = [](Nome* nome,Color4 color,f32 delta,NomeState state,u32* count) -> void {
		if(!nome->is_dead){

			nome->time += delta;

			auto move_time = state == SAD_NOME_STATE ? nome->move_time * 1.8f : nome->move_time;

			if(nome->time > nome->move_time){
				nome->time = 0.0f;

				auto move = [](Nome* n,Nome::Coord dst,Color4 color) ->void {

					auto prev_pos = n->pos;
					n->pos = dst;

					grid[prev_pos.y][prev_pos.x] = 
						grid[prev_pos.y][prev_pos.x] == A_TILE || grid[prev_pos.y][prev_pos.x] == B_TILE ?
						0 : grid[prev_pos.y][prev_pos.x];

					grid[n->pos.y][n->pos.x] = (color == _a_color) ? A_TILE : B_TILE;
				};

				switch(state){
					case ANGRY_NOME_STATE:{
								      //get neighbors
								      auto tile = nome->pos;
								      auto enemy = (color == _a_color) ? B_TILE : A_TILE;
								      auto list = (color == _a_color) ? nomes_b : nomes_a;
								      auto count = (color == _a_color) ? &b_dead_count : &a_dead_count;
								      auto camp = (color == _a_color) ? camp_a : camp_b;

								      auto kill_enemy = [](Nome::Coord tile,Nome* list,u32* count) -> b32 {

									      for(u32 i = 0; i < _arraycount(nomes_a); i++){
										      auto n = &list[i];
										      if(tile.x == n->pos.x && tile.y == n->pos.y && !n->is_dead){
											      n->is_dead = true;
											      (*count)++;
											      return true;
										      }

									      }
									      return false;
								      };

								      if(tile.x + 1 < _grid_dim && grid[tile.y][tile.x + 1] == enemy){
									      auto t = Nome::Coord{(u16)(tile.x + 1),tile.y};
									      if(kill_enemy(t,list,count)){
										      move(nome,t,color);
										      AStarPath(nome->pos,camp,nome->path,&nome->count);
									      }

								      }
								      else if(tile.x != 0 && grid[tile.y][tile.x - 1] == enemy){
									      auto t = Nome::Coord{(u16)(tile.x - 1),tile.y};
									      if(kill_enemy(t,list,count)){
										      move(nome,t,color);
										      AStarPath(nome->pos,camp,nome->path,&nome->count);
									      }
								      }
								      else if(tile.y + 1 < _grid_dim && grid[tile.y + 1][tile.x] == enemy){
									      auto t = Nome::Coord{tile.x,(u16)(tile.y + 1)};
									      if(kill_enemy(t,list,count)){
										      move(nome,t,color);
										      AStarPath(nome->pos,camp,nome->path,&nome->count);
									      }
								      }
								      else if(tile.y != 0 && grid[tile.y - 1][tile.x] == enemy){
									      auto t = Nome::Coord{tile.x,(u16)(tile.y - 1)};
									      if(kill_enemy(t,list,count)){
										      move(nome,t,color);
										      AStarPath(nome->pos,camp,nome->path,&nome->count);
									      }
								      }

								      else{
									      nome->i++;
									      move(nome,nome->path[nome->i],color);
									      nome->is_dead = nome->i >= nome->count;

									      *count = nome->is_dead ? (*count) + 1 : *count;
								      }

							      }break;
					case AFRAID_NOME_STATE:{
							      }break;
					default:{
							nome->i++;
							move(nome,nome->path[nome->i],color);
							nome->is_dead = nome->i >= nome->count;

							*count = nome->is_dead ? (*count) + 1 : *count;
						}break;
				}

			}

			GUIDrawPosRect(CoordToVec3(nome->pos),color);
		}

		else{
			GUIDrawPosRect(CoordToVec3(nome->pos),White);
		}
	};


	for(u32 y = 0; y < _grid_dim; y++){
		for(u32 x = 0; x < _grid_dim; x++){

			if(grid[y][x] == A_TILE || grid[y][x] == B_TILE){
				continue;
			}

			auto color = grid[y][x] ? Red : White;
			color = camp_a.x == x && camp_a.y == y ?  _a_color : color;
			color = camp_b.x == x && camp_b.y == y ?  _b_color : color;
			GUIDrawPosMarkerX(origin + (Vec3{(f32)x,(f32)y} * w),color);
		}
	}


	for(u32 i = 0; i < _arraycount(nomes_a); i++){
		update_nome(nomes_a + i,_a_color,delta,a_state,&a_dead_count);
	}


	for(u32 i = 0; i < _arraycount(nomes_b); i++){
		update_nome(nomes_b + i,_b_color,delta,b_state,&b_dead_count);
	}

	if(a_dead_count == _arraycount(nomes_a) || b_dead_count == _arraycount(nomes_b)){
		printf("Game Ended\n");
		exit(0);
	}
}

s32 main(s32 argc, s8** argv) {

#if 0
	TestSW();

	{
		for (u32 i = 0; i < 200; i++) {
			printf("Hello world\n");
		}
	}

	return 0;

#endif

	{
		TimeSpec s;
		GetTime(&s);
		srand(TimeSpecAsInt(s));
	}



	InitAllSystems();
	InitSim();

	//InitParticles();

#ifdef DEBUG

#if 0
	GetExecFileAssetData();
#endif

#endif

	void (*reload)(GameReloadData*) = 0;

	while (gdata->running) {
		{
			pdata->objupdate_count = 0;
			TSingleEntryUnlock(&gui_draw_is_locked);

			Clear(&pdata->rendercontext);
			ClearLightList();


#if _enable_gui

			s8 ascii_buffer[256] = {};
			u32 ascii_count = 0;

			if (GUIIsAnyElementActive()) {
				for (u32 i = 0;
				     i < _arraycount(
					     pdata->keyboardstate.curkeystate);
				     i++) {
					if (IsKeyPressed(&pdata->keyboardstate,
							 i)) {
						auto c = WKeyCodeToASCII(i);

						if (PIsVisibleChar(c)) {
							ascii_buffer
							    [ascii_count] = c;
							ascii_count++;
						}
					}
				}
			}

			GUIUpdate(&pdata->swapchain, &pdata->keyboardstate,
				  ascii_buffer, ascii_count, &pdata->mousestate,
				  pdata->view, pdata->proj);

			GUIBegin();

			Sim(pdata->deltatime);
			//ParticleFunction(pdata->proj * pdata->view);
#endif

			BUILDGUIGRAPH(gdata->draw_profiler);

			TimeSpec start, end;

			MASTERTIMEBLOCKSTART(Aqua);

			GetTime(&start);

			{
				EXECTIMEBLOCK(Black);

				// MARK: we should move this to
				// buildrendercmdbuffer
				// FIXME: turning on vsync has frame hitches
				{
					TIMEBLOCKTAGGED("AcquireImage", Orange);
					vkAcquireNextImageKHR(
							pdata->vdevice.device,
							pdata->swapchain.swap,
							0xFFFFFFFFFFFFFFFF,
							pdata->waitacquireimage_semaphore,
							0,
							(u32*)&pdata->swapchain
							.image_index);
				}

				UpdateAllocatorTimeStamp();
				Clear(&pdata->threadqueue);

				ProcessEvents(
				    &pdata->window, &pdata->keyboardstate,
				    &pdata->mousestate, &gdata->running);

				{
					TIMEBLOCKTAGGED("RELOADLIB", Firebrick);
					pdata->lib = ReloadGameLibrary(
					    pdata->lib, (void**)&reload,
					    &pdata->scenecontext);
				}

				if (reload) {
					// FIXME(WIN32): we are reloading every
					// frame. seems like a problem with
					// FFileChanged

					auto context = &pdata->scenecontext;

					GameReloadData reloaddata = {
					    gdata,
					    pdata->vdevice,
					    pdata->renderpass,
					    &pdata->window,
					    context,
					    GetGUIContext(),
					    GetAAllocatorContext(),
					    DEBUGTIMERGETCONTEXT()};

					vkDeviceWaitIdle(pdata->vdevice.device);

					reload(&reloaddata);
					reload = 0;

					CompileAllPipelines(pdata);
				}

				u32 audio_count = 0;
				EntityAudioData* audio_data;

				if (pdata->lib.updaterender) {
					auto context = &pdata->scenecontext;
					context->prev_frametime =
					    pdata->deltatime;
					context->keyboardstate =
					    &pdata->keyboardstate;
					context->mousestate =
					    &pdata->mousestate;
					context->audiocontext = &audio_data;
					context->audiocontext_count =
					    &audio_count;

					auto ptr = (void (*)(SceneContext*))
						       pdata->lib.updaterender;

					ptr(context);
				}

				auto frames =
				    AAudioDeviceWriteAvailable(&pdata->audio);

				if (frames >=
				    pdata->submit_audiobuffer.size_frames) {
					auto args = TAlloc(MixAudioLayout, 1);

					*args = {&pdata->submit_audiobuffer,
						 audio_data, audio_count,
						 pdata->resample_scale};

					PushThreadWorkQueue(
					    &pdata->threadqueue, MixAudio,
					    (void*)args, pdata->worker_sem);
				}

				ProcessDrawList();

				MainThreadDoWorkQueue(&pdata->threadqueue, 0);

#if _enable_gui

				GUIEnd();

#endif
				// we can thread this
				BuildRenderCommandBuffer(pdata);

				PresentBuffer(pdata);
			}

			GetTime(&end);

			auto diff = GetTimeDifferenceMS(start, end);
			f32 sleeptime = (_targetframerate - diff);

			if (sleeptime > 0) {
				TIMEBLOCKTAGGED("Sleep", DarkGray);
				SleepMS(sleeptime);
			}

			GetTime(&end);

			pdata->deltatime = GetTimeDifferenceMS(start, end);
			//auto ptr = (Emitters*)VGetReadWriteBlockPtr(&tdata.emitter_sbo);
			//ptr->emitters[0].timer += pdata->deltatime;
			//ptr->time = pdata->deltatime;

			//printf("delta time %f\n",(f64)ptr->time);

		}
	}

	WritePipelineCache();

	return 0;
}

// add cpp's here to limit scope
#include "aassetmanager.cpp"
#include "aassettools.cpp"
#include "vvulkanx.cpp"

#ifndef CPP_PASS

#include "engine_meta.cpp"

#endif

/*

 **VK_KHR_16bit_storage                : extension revision  1
 **VK_KHR_8bit_storage                 : extension revision  1
 **VK_KHR_driver_properties            : extension revision  1
 **VK_KHR_relaxed_block_layout         : extension revision  1
 **VK_KHR_uniform_buffer_standard_layout: extension revision  1
 **VK_KHR_variable_pointers            : extension revision  1
 **VK_KHR_vulkan_memory_model          : extension revision  3

 libpapi-dev
 libdbus-1-dev
 wayland-protocols
 libvulkan-dev
 vulkan-validationlayers
 zenity

TODO:

Make keyboardstate and mousestate a bit array

Compile all assets into an adb file (asset data base). We will build a function
a constexpr function at compile time which translates filepaths to indices and
an adb file w raw data. We will keep the adb file open at all times and just
read from the offset as required.

Beef up our profiler
We can profile the cmdbuffers using this. We should inject and record this too

vkCmdWriteTimestamp();


memory protect our allocations
Support drawing multiple objects off a single buffer(use vert & index buffer
offsets into buffer) implement quaternion double cover Implement Dual quaternion
blending in MDF

multi gpu functions

void vkCmdSetDeviceMask(
VkCommandBuffer commandBuffer, uint32_t deviceMask);

VkResult vkAcquireNextImage2KHR(
VkDevice device, const VkAcquireNextImageInfoKHR* pAcquireInfo, uint32_t*
pImageIndex); typedef struct VkAcquireNextImageInfoKHR { VkStructureType sType;
const void* pNext; VkSwapchainKHR swapchain; uint64_t timeout; VkSemaphore
semaphore; VkFence fence; uint32_t deviceMask; } VkAcquireNextImageInfoKHR;


typedef struct VkDeviceGroupPresentInfoKHR {
VkStructureType sType; const void* pNext;
uint32_t swapchainCount;
const uint32_t* pDeviceMasks;
VkDeviceGroupPresentModeFlagBitsKHR mode;
} VkDeviceGroupPresentInfoKHR;

typedef struct VkDeviceGroupCommandBufferBeginInfo {
VkStructureType sType; const void* pNext;
uint32_t deviceMask;
} VkDeviceGroupCommandBufferBeginInfo;

enum VkDeviceGroupPresentModeFlagBitsKHR:
VK_DEVICE_GROUP_PRESENT_MODE_X_BIT_KHR where X is
LOCAL,
REMOTE,
SUM,
LOCAL_MULTI_DEVICE
typedef struct VkDeviceGroupRenderPassBeginInfo {
	VkStructureType sType; const void* pNext;
	uint32_t deviceMask;
	uint32_t deviceRenderAreaCount;
	const VkRect2D* pDeviceRenderAreas; P.15
} VkDeviceGroupRenderPassBeginInfo;
typedef struct VkDeviceGroupSubmitInfo {
	VkStructureType sType; const void* pNext;
	uint32_t waitSemaphoreCount;
	const uint32_t* pWaitSemaphoreDeviceIndices;
	uint32_t commandBufferCount;
	const uint32_t* pCommandBufferDeviceMasks;
	uint32_t signalSemaphoreCount;
	const uint32_t* pSignalSemaphoreDeviceIndices;
} VkDeviceGroupSubmitInfo;


typedef struct VkMemoryAllocateFlagsInfo {
	VkStructureType sType; const void* pNext;
	VkMemoryAllocateFlags flags;
	uint32_t deviceMask;
} VkMemoryAllocateFlagsInfo;



// libC dependencies

free
pthread_create
__errno_location
stdout
strcpy
cosf
qsort
abs
atof
clock_gettime
write
opendir
strlen
mmap
pthread_setaffinity_np
printf
nanosleep
lseek
__assert_fail
sinf
sem_timedwait
memset
close
read
__libc_start_main
pthread_attr_init
ceilf
acosf
sem_wait
tanf
fprintf
__gmon_start__
memcpy
sqrtf
__xstat
readdir
pthread_attr_setdetachstate
dlopen
malloc
fflush
sem_post
dlclose
pthread_attr_setstacksize
realloc
atan2f
munmap
poll
memmove
pthread_self
sem_init
open
floorf
pthread_attr_destroy
atoi
sprintf
exit
sem_destroy
dlsym
fmodf
dlerror
stderr
*/
