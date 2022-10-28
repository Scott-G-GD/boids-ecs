#ifndef _engine_h
#define _engine_h

#include <stdint.h>

typedef struct engine_init_t {
	int window_width, window_height;
	uint32_t window_init_flags;
	uint32_t sdl_init_flags;
	uint32_t renderer_init_flags;
	int renderer_index;
	int target_framerate;
} engine_init_t;

extern short is_render_frame;

extern void default_engine_init_settings(engine_init_t*);

extern void sim_config(engine_init_t*);
extern void sim_init();
extern void sim_quit();

extern struct SDL_Renderer* renderer;

#if defined(DEBUG)
#define E_LOG(...)\
fprintf(stdout, "%s:%d: ", __FILE__, __LINE__);\
fprintf(stdout, __VA_ARGS__);\
fprintf(stdout, "\n")
#elif defined(NDEBUG)
#define E_LOG(...)
#endif

#endif /* !_engine_h */
