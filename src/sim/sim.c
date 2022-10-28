#include <SDL2/SDL.h>
#include <ecs.h>
#include <engine.h>
#include <adb.h>
#include <ui.h>

#define BOID_NEAR_COUNT (200)
#include "boid_c.h"

int boid_spawn_num;

void system_draw_gui(ecsEntityId* entities, ecsComponentMask* mask, size_t count, float delta_time)
{
	static int show_sliders = 1;
	
	int ww, wh;
	SDL_GetRendererOutputSize(renderer, &ww, &wh);
	
	boid_available_area.w = ww;
	boid_available_area.h = wh;
	boid_available_area.x = 0;
	
	SDL_Rect rect = {
		0, 0, 500, wh
	};
	
	uiBeginFrame();
	
	if(uiBeginWindow(&rect, &show_sliders))
	{
		// set the area boids will stay in to exclude the area of the ui
		boid_available_area.x = rect.w;
		boid_available_area.w = ww - rect.w;
		
		// render velocity and acceleration sliders
		uiLabelNext("speed", 0.25f);
		uiSlider(&boid_max_velocity, 10.f, 100.f, 5.f);
		uiLabelNext("acceleration", 0.25f);
		uiSlider(&boid_acceleration, 10.f, 100.f, 5.f);
		
		// alignment parameters
		uiHeader("alignment");
		
		uiLabelNext("range", 0.25f);
		uiSlider(&(alignment.range), 0.1f, 100.f, 1.f);
		uiLabelNext("force", 0.25f);
		uiSlider(&(alignment.force), 0.0f, 2.f, 0.01f);
		
		// cohesion parameters
		uiHeader("cohesion");
		
		uiLabelNext("range", 0.25f);
		uiSlider(&(cohesion.range), 0.1f, 100.f, 1.f);
		uiLabelNext("force", 0.25f);
		uiSlider(&(cohesion.force), 0.0f, 2.f, .01f);
		
		// mouse interaction parameters
		uiHeader("mouse");
		
		uiLabelNext("range", 0.25f);
		uiSlider(&(mouse_interact.range), 0.1f, 100.f, 1.f);
		uiLabelNext("force", 0.25f);
		uiSlider(&(mouse_interact.force), -200.f, 200.f, 10.f);
		
		
		// separation range
		uiHeader("separation");

		uiSlider(&(separation.range), 0.1f, 100.f, 1.f);
	}
}

void sim_config(engine_init_t* config)
{
	config->window_width = 1700;
	config->window_height = 1000;
	config->window_init_flags |= SDL_WINDOW_RESIZABLE;
	boid_spawn_num = 500;
	config->target_framerate = 24;
}

void spawn_boids()
{
	int w, h;
	SDL_GetRendererOutputSize(renderer, &w, &h);
	
	fvec position = {9, 0};
	ecsEntityId entity;
	boid_c* boid;
	
	for(int i = 0; i < boid_spawn_num; i++)
	{
		position = (fvec){rand() % w, rand() % h};
		if((entity = ecsCreateEntity(boid_component)) != noentity)
		{
			boid = ecsGetComponentPtr(entity, boid_component);
			(*boid) = (boid_c){
				.position = position,
				.velocity = {0,0},
				.force = {0,0}
			};
		}
		else
		{
			exit(2);
		}
	}
}

void sim_init()
{
	// register boid_c as a component type
	boid_component = ecsRegisterComponent(boid_c);
	
	// enable the functions that make boids boid
	ecsEnableSystem(&system_boid_update_position, boid_component, ECS_QUERY_ALL, 8, 50);
	ecsEnableSystem(&system_boid_update_near, boid_component, ECS_QUERY_ALL, 0, 100);
	ecsEnableSystem(&system_draw_boids, boid_component, ECS_QUERY_ALL, 0, 200);
	ecsEnableSystem(&system_boids_wall_avoid, boid_component, ECS_QUERY_ALL, 8, 300);
	ecsEnableSystem(&system_boids_cohesion, boid_component, ECS_QUERY_ALL, 8, 400);
	ecsEnableSystem(&system_boids_alignment, boid_component, ECS_QUERY_ALL, 8, 410);
	ecsEnableSystem(&system_boids_separation, boid_component, ECS_QUERY_ALL, 8, 420);
	ecsEnableSystem(&system_boid_mouse, boid_component, ECS_QUERY_ALL, 8, 430);
	
	// enable the gui system
	ecsEnableSystem(&system_draw_gui, nocomponent, ECS_NOQUERY, 0, 500);

	int w, h;
	SDL_GetRendererOutputSize(renderer, &w, &h);
	
	// load boid image
	//asset_handle_t blur_asset = load_asset("blur.png");
	asset_handle_t arrow_asset = load_asset("boid.png");
	// set boid texture
	boid_texture = get_asset(arrow_asset);
	
	// load and set font
	asset_handle_t font_asset = load_asset("Inter-Regular.otf");
	uiSetFont(get_asset(font_asset));
	
	// set the initially available area
	boid_available_area = (SDL_Rect){
		.x = 0, .y = 0,
		.w = w, .h = h
	};
	
	// spawn a bunch of boids
	spawn_boids();
}

void sim_quit()
{
}

