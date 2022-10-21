#include "engine.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <ecs.h>
#include "adb.h"
#include "ui.h"
#include <stdlib.h>

SDL_Window* window;
SDL_Renderer* renderer;
int engine_wants_to_quit;
float frame_start_time;
float last_frame_time;

void system_window_clear(ecsEntityId* entities, ecsComponentMask* components, size_t size, float delta_time);

void engine_init();
void engine_run();
void engine_handle_event(SDL_Event* event);
void engine_clean();

void* asset_load_sdl_image(const char* filename)
{
	SDL_Surface* surf = IMG_Load(filename);
	SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surf);
	SDL_FreeSurface(surf);
	return tex;
}

void asset_free_sdl_image(void* instance)
{
	SDL_Texture* tex = instance;
	if(tex != NULL)
	{
		SDL_DestroyTexture(tex);
	}
}

int main(int argc, char* argv[])
{
	engine_init();
	engine_run();
	engine_clean();
}

void engine_init()
{
	frame_start_time = (float)clock() / CLOCKS_PER_SEC;
	engine_wants_to_quit = 0;
	engine_init_t init_settings;
	default_engine_init_settings(&init_settings);
	init_asset_database(100);

	register_file_handler(".png", &asset_load_sdl_image, &asset_free_sdl_image);
	register_file_handler(".jpg", &asset_load_sdl_image, &asset_free_sdl_image);

	sim_config(&init_settings);

	SDL_Init(init_settings.sdl_init_flags);
	window = SDL_CreateWindow("BOIDS!!",
							  SDL_WINDOWPOS_CENTERED,
							  SDL_WINDOWPOS_CENTERED,
							  init_settings.window_width,
							  init_settings.window_height,
							  init_settings.window_init_flags);
	if(window == NULL)
	{
		exit(1);
	}
	
	renderer = SDL_CreateRenderer(window, init_settings.renderer_index,
								  init_settings.renderer_init_flags);
	
	if(renderer == NULL)
	{
		exit(2);
	}

	ecsInit();

	uiInit(renderer);

	ecsEnableSystem(&system_window_clear, nocomponent, ECS_NOQUERY, 0, -100);
	
	sim_init();
	
	ecsRunTasks();
}

void engine_run()
{
	SDL_Event evt;
	float actual_time;
	
	while(!engine_wants_to_quit)
	{
		last_frame_time = frame_start_time;
		frame_start_time = (float)clock() / CLOCKS_PER_SEC;
		
		ecsRunSystems(frame_start_time - last_frame_time);
		
		while(SDL_PollEvent(&evt))
		{
			engine_handle_event(&evt);
		}
	}
}

void engine_handle_event(SDL_Event* event)
{
	switch(event->type)
	{
	default: break;
	case SDL_QUIT:
		engine_wants_to_quit = 1;
		break;
	}
}

void engine_clean()
{
	sim_quit();
	uiTerminate();
	close_asset_database();
	ecsTerminate();
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
}

void system_window_clear(ecsEntityId* entities, ecsComponentMask* components, size_t size, float delta_time)
{
	static float render_time;
	render_time += delta_time;
	//if(render_time < 1.f/20.f) return;
	
	render_time = 0;
	SDL_RenderPresent(renderer);
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderClear(renderer);
}

void default_engine_init_settings(engine_init_t* init_settings)
{
	(*init_settings) = (engine_init_t){
		.window_width = 640, .window_height = 420,
		.window_init_flags = SDL_WINDOW_SHOWN,
		.sdl_init_flags = SDL_INIT_VIDEO,
		.renderer_init_flags = SDL_RENDERER_ACCELERATED,
		.renderer_index = -1
	};
}

