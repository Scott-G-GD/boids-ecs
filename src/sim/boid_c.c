//
//  boid_c.c
//  sim
//
//  Created by Scott on 11/10/2022.
//

#include "boid_c.h"
#include <SDL2/SDL.h>
#include <engine.h>
#include <assert.h>

ecsComponentMask boid_component;

float boid_acceleration = 75.f;
float boid_max_velocity = 50.f;

SDL_Texture* boid_texture;

behaviour_t alignment = {
	.range = 10.f,
	.force = .8f
};
behaviour_t separation = {
	.range = 4.f,
	.force = 50.0f
};
behaviour_t cohesion = {
	.range = 100.f,
	.force = 0.5f
};
behaviour_t wall_avoid = {
	.range = 50.f,
	.force = 100.f
};
behaviour_t mouse_interact = {
	.range = 70.f,
	.force = -1000.f
};

SDL_Rect boid_available_area = {0,0,800, 800};

void system_boid_update_position(ecsEntityId* entities, ecsComponentMask* components, size_t count, float delta_time)
{
	boid_c* boid;
	fvec force, velocity;
	float acceleration = boid_acceleration * delta_time;
	int w, h;
	
	for(size_t i = 0; i < count; i++)
	{
		boid = ecsGetComponentPtr(entities[i], boid_component);
		
		force = boid->force;
		vmulf(&force, &force, boid_max_velocity);
		vmax(&force, &force, boid_max_velocity);
		vmovetowards(&boid->velocity, &boid->velocity, &force, acceleration);
		
		assert(!isnan(boid->position.x) && !isnan(boid->position.y));
		assert(!isnan(boid->velocity.x) && !isnan(boid->velocity.y));
		
		boid->force = (fvec){ 0.f, 0.f };
		
		vmulf(&velocity, &boid->velocity, delta_time);
		vadd(&boid->position, &boid->position, &velocity);
		
	}
}

void system_boid_update_near(ecsEntityId* entities, ecsComponentMask* components, size_t count, float delta_time)
{
	float dist;
	boid_c* boid, *other;
	
	float max_range = alignment.range;
	max_range = separation.range > max_range ? separation.range : max_range;
	max_range = cohesion.range > max_range ? cohesion.range : max_range;
	
	size_t hits = 0;
	
	for(size_t i = 0; i < count; ++i)
	{
		boid = ecsGetComponentPtr(entities[i], boid_component);
		hits = 0;
		
		memset(boid->near, noentity, sizeof(boid->near));
		for(size_t j = 0; j < count && hits < BOID_NEAR_COUNT; ++j)
		{
			other = ecsGetComponentPtr(entities[j], boid_component);
			dist = vdist(&boid->position, &other->position);
			
			assert(!isnan(dist));
			if(dist < max_range)
			{
				boid->near[hits++] = entities[j];
			}
		}
	}
}

void system_draw_boids(ecsEntityId* entities, ecsComponentMask* components, size_t count, float delta_time)
{
	boid_c* boid;
	
	int tw, th;
	SDL_QueryTexture(boid_texture, NULL, NULL, &tw, &th);
	SDL_Rect srcrect = {
		.x = 0, .y = 0,
		.w = tw, .h = th
	};
	SDL_FRect dstrect = {
		.w = 5, .h = 5
	};
	int hw = dstrect.w / 2;
	int hh = dstrect.h / 2;
	
	for(size_t i = 0; i < count; ++i)
	{
		boid = ecsGetComponentPtr(entities[i], boid_component);
		
		dstrect.x = boid->position.x - hw;
		dstrect.y = boid->position.y - hh;
		SDL_RenderCopyExF(renderer, boid_texture,
						  &srcrect, &dstrect,
						  (double)vang(&boid->velocity, &VDOWN) * 57.2957795,
						  &(SDL_FPoint){hw,hh}, SDL_FLIP_NONE);
	}
}

void system_boids_cohesion(ecsEntityId* entities, ecsComponentMask* components, size_t count, float delta_time)
{
	boid_c* boid, *other;
	fvec avrg, force, diff;
	float dist;
	size_t hit_count;
	
	for(size_t i = 0; i < count; ++i)
	{
		boid = ecsGetComponentPtr(entities[i], boid_component);
		avrg = (fvec){ 0.f, 0.f };
		hit_count = 0;
		
		for(size_t j = 0; j < BOID_NEAR_COUNT && boid->near[j] != noentity; ++j)
		{
			other = ecsGetComponentPtr(boid->near[j], boid_component);
			dist = vdist(&other->position, &boid->position);
			if(dist < cohesion.range)
			{
				++hit_count;
				vsub(&diff, &other->position, &avrg);
				vmulf(&diff, &diff, 1.f/(hit_count));
				vadd(&avrg, &avrg, &diff);
			}
			assert(!isnan(avrg.x) && !isnan(avrg.y));
		}
		
		vsub(&force, &avrg, &boid->position);
		vmulf(&force, &force, cohesion.force);
		vadd(&boid->force, &boid->force, &force);
		
		assert(!isnan(boid->force.x) && !isnan(boid->force.y));
	}
}

void system_boids_separation(ecsEntityId* entities, ecsComponentMask* components, size_t count, float delta_time)
{
	boid_c* boid, *other;
	fvec avrg, force, diff;
	float dist;
	size_t hit_count;
	
	for(size_t i = 0; i < count; ++i)
	{
		boid = ecsGetComponentPtr(entities[i], boid_component);
		avrg = (fvec){ 0.f, 0.f };
		hit_count = 0;
		
		for(size_t j = 0; j < BOID_NEAR_COUNT && boid->near[j] != noentity; ++j)
		{
			other = ecsGetComponentPtr(boid->near[j], boid_component);
			dist = vdist(&other->position, &boid->position);
			if(dist < separation.range)
			{
				++hit_count;
				vsub(&diff, &other->position, &avrg);
				vmulf(&diff, &diff, 1.f/(hit_count));
				vadd(&avrg, &avrg, &diff);
			}
			assert(!isnan(avrg.x) && !isnan(avrg.y));
		}
		
		vsub(&force, &avrg, &boid->position);
		vmulf(&force, &force, separation.force);
		vsub(&boid->force, &boid->force, &force);
		assert(!isnan(boid->force.x) && !isnan(boid->force.y));
	}
}

void system_boids_alignment(ecsEntityId* entities, ecsComponentMask* components, size_t count, float delta_time)
{
	boid_c* boid, *other;
	fvec avrg, diff;
	float dist;
	size_t hit_count;
	
	for(size_t i = 0; i < count; ++i)
	{
		boid = ecsGetComponentPtr(entities[i], boid_component);
		avrg = (fvec){ 0.f, 0.f };
		hit_count = 0;
		
		for(size_t j = 0; j < BOID_NEAR_COUNT && boid->near[j] != noentity; ++j)
		{
			other = ecsGetComponentPtr(boid->near[j], boid_component);
			dist = vdist(&other->position, &boid->position);
			
			if(dist < alignment.range)
			{
				++hit_count;
				vsub(&diff, &other->velocity, &avrg);
				vmulf(&diff, &diff, 1.f/(hit_count));
				vadd(&avrg, &avrg, &diff);
			}
			assert(!isnan(avrg.x) && !isnan(avrg.y));
		}
		
		vmulf(&avrg, &avrg, alignment.force);
		vadd(&boid->force, &boid->force, &avrg);
		assert(!isnan(boid->force.x) && !isnan(boid->force.y));
	}
}

void system_boid_mouse(ecsEntityId* entities, ecsComponentMask* components, size_t count, float delta_time)
{
	boid_c* boid;
	fvec mouse, diff;
	float m;
	int imx, imy;
	uint32_t mstate = SDL_GetMouseState(&imx, &imy);
	mouse = (fvec){ (float)imx, (float)imy };
	if((mstate & SDL_BUTTON_LEFT) == 0) return;
	
	for(size_t i = 0; i < count; ++i)
	{
		boid = ecsGetComponentPtr(entities[i], boid_component);
		vsub(&diff, &mouse, &boid->position);
		m = vmag(&diff);
		if(m < mouse_interact.range)
		{
			vmulf(&diff, &diff, (1.f/m)*mouse_interact.force);
			vadd(&boid->force, &boid->force, &diff);
		}
	}
}

void system_boids_wrap(ecsEntityId* entities, ecsComponentMask* components, size_t count, float delta_time)
{
	boid_c* boid;
	int iw, ih;
	float w, h;
	SDL_GetRendererOutputSize(renderer, &iw, &ih);
	w = (float)iw; h = (float)ih;
	
	for(size_t i = 0; i < count; ++i)
	{
		boid = ecsGetComponentPtr(entities[i], boid_component);
		if(boid->position.x >= w) boid->position.x -= w;
		if(boid->position.x < 0) boid->position.x += w;
		if(boid->position.y >= h) boid->position.y -= h;
		if(boid->position.y < 0) boid->position.y += w;
	}
}

void system_boids_wall_avoid(ecsEntityId* entities, ecsComponentMask* components, size_t count, float delta_time)
{
	if(wall_avoid.force == 0.f) return;
	
	boid_c* boid;
	fvec force;

	SDL_Rect limits = boid_available_area;
	limits.x += wall_avoid.range;
	limits.w -= wall_avoid.range*2;
	limits.y += wall_avoid.range;
	limits.h -= wall_avoid.range*2;
	
	for(size_t i = 0; i < count; ++i)
	{
		boid = ecsGetComponentPtr(entities[i], boid_component);
		force = (fvec){ 0.f, 0.f };
		
		if(boid->position.x >= limits.x + limits.w)
			force.x = -1.f;
		if(boid->position.x <= limits.x)
			force.x = 1.f;
		if(boid->position.y >= limits.y + limits.h)
			force.y = -1.f;
		if(boid->position.y <= limits.y)
			force.y = 1.f;
		
		vnor(&force, &force);
		vmulf(&force, &force, wall_avoid.force);
		vadd(&boid->force, &boid->force, &force);
	}
}
