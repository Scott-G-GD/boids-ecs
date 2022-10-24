#include "engine.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <stdlib.h>
#include <time.h>
#include <ecs.h>
#include "adb.h"
#include "ui.h"

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

void* asset_load_ttf_font(const char* filename)
{
	TTF_Font* font = TTF_OpenFont(filename, 50);
	return font;
}

void asset_free_ttf_font(void* instance)
{
	TTF_Font* font = instance;
	if(instance != NULL)
	{
		TTF_CloseFont(font);
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
	// set the frame start time here to ensure there will be time passed when engine_run is called
	frame_start_time = (float)clock() / CLOCKS_PER_SEC;
	engine_wants_to_quit = 0;
	// init asset database for 100 assets
	init_asset_database(100);

	// register default image file handlers
	register_file_handler(".png", &asset_load_sdl_image, &asset_free_sdl_image);
	register_file_handler(".jpg", &asset_load_sdl_image, &asset_free_sdl_image);
	register_file_handler(".ttf", &asset_load_ttf_font, &asset_free_ttf_font);
	register_file_handler(".otf", &asset_load_ttf_font, &asset_free_ttf_font);

	// load default init settings
	engine_init_t init_settings;
	default_engine_init_settings(&init_settings);
	// allow sim to adjust init settings as needed
	sim_config(&init_settings);

	// init sdl, create window and create renderer from window
	SDL_Init(init_settings.sdl_init_flags);
	TTF_Init();
	IMG_Init(IMG_INIT_JPG | IMG_INIT_PNG);
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

	// initialize runtime object database
	ecsInit();

	// initialize imgui renderer
	uiInit(renderer);

	// enable screen refresh system
	ecsEnableSystem(&system_window_clear, nocomponent, ECS_NOQUERY, 0, -100);
	
	// initialize sim
	sim_init();
	// run created tasks
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
	// quit sim, ui asset database, ecs
	sim_quit();
	uiTerminate();
	close_asset_database();
	ecsTerminate();
	// delete renderer and window, quit sdl
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
}

void system_window_clear(ecsEntityId* entities, ecsComponentMask* components, size_t size, float delta_time)
{
	// swap buffer
	SDL_RenderPresent(renderer);
	// clear screen all black
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

