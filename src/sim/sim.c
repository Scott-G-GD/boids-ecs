#include <SDL2/SDL.h>
#include <ecs.h>
#include <engine.h>
#include <adb.h>

#define BOID_NEAR_COUNT (200)
#include "boid_c.h"

void sim_config(engine_init_t* config)
{
	config->window_width = 1000;
	config->window_height = 800;
	config->window_init_flags |= SDL_WINDOW_RESIZABLE;
}

void sim_init()
{
	boid_component = ecsRegisterComponent(boid_c);
	
	ecsEnableSystem(&system_boid_update,		boid_component, ECS_QUERY_ALL, 0, 100);
	ecsEnableSystem(&system_draw_boids,			boid_component, ECS_QUERY_ALL, 0, 200);
	ecsEnableSystem(&system_boids_wall_avoid,	boid_component, ECS_QUERY_ALL, 8, 300);
	//ecsEnableSystem(&system_boids_wrap,			boid_component, ECS_QUERY_ALL, 3);
	ecsEnableSystem(&system_boids_cohesion,		boid_component, ECS_QUERY_ALL, 8, 400);
	ecsEnableSystem(&system_boids_alignment,	boid_component, ECS_QUERY_ALL, 8, 410);
	ecsEnableSystem(&system_boids_separation,	boid_component, ECS_QUERY_ALL, 8, 420);
	ecsEnableSystem(&system_boid_mouse,			boid_component, ECS_QUERY_ALL, 8, 430);

	int w, h;
	SDL_GetRendererOutputSize(renderer, &w, &h);
	
	asset_handle_t blur_asset = load_asset("blur.png");
	asset_handle_t arrow_asset = load_asset("boid.png");
	boid_texture = get_asset(arrow_asset);
	
	const int boids = 600;
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
