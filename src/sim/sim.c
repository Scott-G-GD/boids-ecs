#include <SDL2/SDL.h>
#include <ecs.h>
#include <engine.h>
#include <adb.h>
#include <ui.h>

#define BOID_NEAR_COUNT (200)
#include "boid_c.h"

void sim_config(engine_init_t* config)
{
	config->window_width = 1000;
	config->window_height = 800;
	config->window_init_flags |= SDL_WINDOW_RESIZABLE;
}

void system_draw_gui(ecsEntityId* entities, ecsComponentMask* mask, size_t count, float deltaTime)
{
	static int showSliders = 1;
	uiBeginFrame();
	
	int ww, wh;
	SDL_GetRendererOutputSize(renderer, &ww, &wh);
	
	SDL_Rect rect = {
		0, 0, 300, wh
	};
	
	boid_available_area.w = ww;
	boid_available_area.h = wh;
	boid_available_area.x = 0;
	if(uiBeginWindow(&rect, &showSliders))
	{
		// set the area boids will stay in to exclude the area of the ui
		boid_available_area.x = rect.w;
		boid_available_area.w = ww - rect.w;
		
		// render velocity and acceleration sliders
		uiSameLine(2);
		uiSlider(&boid_max_velocity, 10.f, 100.f, 5.f);
		uiSlider(&boid_acceleration, 10.f, 100.f, 5.f);
		
		// alignment parameters
		uiSameLine(2);
		uiSlider(&(alignment.range), 0.1f, 100.f, 1.f);
		uiSlider(&(alignment.force), 0.0f, 2.f, 0.01f);
		
		// separation range
		uiSlider(&(separation.range), 0.1f, 100.f, 1.f);
		
		// cohesion parameters
		uiSameLine(2);
		uiSlider(&(cohesion.range), 0.1f, 100.f, 1.f);
		uiSlider(&(cohesion.force), 0.0f, 2.f, .01f);
		
		// mouse interaction parameters
		uiSameLine(2);
		uiSlider(&(mouse_interact.range), 0.1f, 100.f, 1.f);
		uiSlider(&(mouse_interact.force), -200.f, 200.f, 10.f);
	}
}

void sim_init()
{
	boid_component = ecsRegisterComponent(boid_c);
	
	ecsEnableSystem(&system_boid_update,		boid_component,	ECS_QUERY_ALL,	0, 100);
	ecsEnableSystem(&system_draw_boids,			boid_component,	ECS_QUERY_ALL,	0, 200);
	ecsEnableSystem(&system_boids_wall_avoid,	boid_component,	ECS_QUERY_ALL,	8, 300);
	ecsEnableSystem(&system_boids_cohesion,		boid_component,	ECS_QUERY_ALL,	8, 400);
	ecsEnableSystem(&system_boids_alignment,	boid_component,	ECS_QUERY_ALL,	8, 410);
	ecsEnableSystem(&system_boids_separation,	boid_component,	ECS_QUERY_ALL,	8, 420);
	ecsEnableSystem(&system_boid_mouse,			boid_component,	ECS_QUERY_ALL,	8, 430);
	ecsEnableSystem(&system_draw_gui,			nocomponent,	ECS_NOQUERY,	0, 500);
	
	int w, h;
	SDL_GetRendererOutputSize(renderer, &w, &h);
	
	asset_handle_t blur_asset = load_asset("blur.png");
	asset_handle_t arrow_asset = load_asset("boid.png");
	boid_texture = get_asset(arrow_asset);
	boid_available_area = (SDL_Rect){
		.x = 0, .y = 0,
		.w = w, .h = h
	};
	
	const int boids = 200;
	fvec position = {9, 0};
	ecsEntityId entity;
	boid_c* boid;
	
	for(int i = 0; i < boids; i++)
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

void sim_quit()
{
}
