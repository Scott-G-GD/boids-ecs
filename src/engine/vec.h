//
//  vec.h
//  sim
//
//  Created by Scott on 11/10/2022.
//

#ifndef vec_h
#define vec_h

#include <math.h>
#include <string.h>

typedef struct fvec {
	float x, y;
} fvec;

static inline void vadd(fvec* r, fvec* a, fvec* b)
{ r->x = a->x + b->x; r->y = a->y + b->y; }

static inline void vsub(fvec* r, fvec* a, fvec* b)
{ r->x = a->x - b->x; r->y = a->y - b->y; }

static inline void vmul(fvec* r, fvec* a, fvec* b)
{ r->x = a->x * b->x; r->y = a->y * b->y; }

static inline void vmulf(fvec* r, fvec* a, float b)
{ r->x = a->x * b; r->y = a->y * b; }

static inline void vdiv(fvec* r, fvec* a, fvec* b)
{ r->x = a->x / b->x; r->y = a->y / b->y; }

static inline float vmag(fvec* a)
{ return sqrt(fabsf(a->x*a->x) + fabsf(a->y*a->y)); }

static inline void vnor(fvec* r, fvec* a)
{
	float m = vmag(a);
	if(m == 0)
	{
		*r = (fvec){0.f, 0.f};
	}
	else
	{
		r->x = a->x / m;
		r->y = a->y / m;
	}
}

static inline void vabs(fvec* r, fvec* a)
{
	r->x = fabsf(a->x);
	r->y = fabsf(a->y);
}

static inline float vdist(fvec* a, fvec* b)
{
	fvec diff;
	vsub(&diff, a, b);
	vabs(&diff, &diff);
	float m = vmag(&diff);
	return isnan(m) ? 0 : m;
}

static inline void vmovetowards(fvec* r, fvec* from, fvec* to, float max_delta)
{
	fvec diff, dir;
	
	if(max_delta == 0 || isnan(max_delta))
	{
		memmove(r, from, sizeof(fvec));
		return;
	}
	
	vsub(&diff, to, from);
	
	float diffm = vmag(&diff);
	
	if(diffm == 0 || isnan(diffm))
	{
		memmove(r, from, sizeof(fvec));
		return;
	}
	
	vmulf(&dir, &diff, 1.f/diffm * max_delta);
	
	float dirm = vmag(&dir);
	
	if(dirm >= diffm)
	{
		memmove(r, from, sizeof(fvec));
	}
	else
	{
		vadd(r, from, &dir);
	}
}

static inline void vlerp(fvec* r, fvec* from, fvec* to, float t)
{
	fvec dir;
	vsub(&dir, to, from);
	vnor(&dir, &dir);
	vmulf(&dir, &dir, t);
	vadd(r, from, &dir);
}

static inline void vmin(fvec* r, fvec* a, float min)
{
	float m = vmag(a);
	if(m < min)
	{
		vmulf(r, a, (1.f/m)*min);
	}
	else
	{
		*r = *a;
	}
}

static inline void vmax(fvec* r, fvec* a, float max)
{
	float m = vmag(a);
	if(m > max)
		vmulf(r, a, (1.f/m)*max);
	else
		*r = *a;
}

static inline void vclamp(fvec* r, fvec* a, float min, float max)
{
	float m = vmag(a);
	if(m > max)
		vmulf(r, a, (1.f/m)*max);
	else if(m < min)
		vmulf(r, a, (1.f/m)*min);
	else
		*r = *a;
}

static inline float vdot(fvec* a, fvec* b)
{
	return(a->x * b->x + a->y * b->y);
}

static inline float vang(fvec* a, fvec* b)
{
	return atan2f(a->y, a->x) - atan2f(b->y, b->x);
}

#define VUP ((fvec){0.f, 1.f})
#define VDOWN ((fvec){0.f, -1.f})
#define VLEFT ((fvec){-1.f, 0.f})
#define VRIGHT ((fvec){1.f, 0.f})

#endif /* vec_h */
