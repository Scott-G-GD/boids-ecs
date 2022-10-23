//
//  boid_c.h
//  sim
//
//  Created by Scott on 11/10/2022.
//

#ifndef boid_c_h
#define boid_c_h

#include <ecs.h>
#include <stdint.h>
#include <vec.h>

#ifndef BOID_NEAR_COUNT
#define BOID_NEAR_COUNT (100)
#endif

extern ecsComponentMask boid_component;
typedef struct boid_c {
	fvec position;
	fvec velocity;
	fvec force;
	ecsEntityId near[BOID_NEAR_COUNT];
} boid_c;

typedef struct behaviour_t {
	float force;
	float range;
} behaviour_t;

extern struct SDL_Texture* boid_texture;
extern float boid_acceleration;
extern float boid_max_velocity;

extern behaviour_t alignment;
extern behaviour_t separation;
extern behaviour_t cohesion;
extern behaviour_t wall_avoid;
extern behaviour_t mouse_interact;
extern struct SDL_Rect boid_available_area;

extern void system_boid_update_position(ecsEntityId*, ecsComponentMask*, size_t, float);
extern void system_boids_cohesion(ecsEntityId*, ecsComponentMask*, size_t, float);
extern void system_boids_separation(ecsEntityId*, ecsComponentMask*, size_t, float);
extern void system_boids_alignment(ecsEntityId*, ecsComponentMask*, size_t, float);
extern void system_boids_wall_avoid(ecsEntityId*, ecsComponentMask*, size_t, float);
extern void system_boids_wrap(ecsEntityId*, ecsComponentMask*, size_t, float);
extern void system_draw_boids(ecsEntityId*, ecsComponentMask*, size_t, float);
extern void system_boid_mouse(ecsEntityId*, ecsComponentMask*, size_t, float);
extern void system_boid_update_near(ecsEntityId*, ecsComponentMask*, size_t, float);

#endif /* boid_c_h */
