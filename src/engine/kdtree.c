//
//  kdtree.c
//  engine
//
//  Created by Scott on 12/10/2022.
//

#include "kdtree.h"
#include <stdlib.h>

#define K (2)

void swap(void* a, void* b, size_t size)
{
	void* t = malloc(size);
	memcpy(t, b, size);
	memcpy(b, a, size);
	memcpy(a, t, size);
	free(t);
}

void sort_points_by_x(fvec* points, size_t count)
{
	int swaps = 0;
	do {
		for(size_t i = 1; i < count; ++i)
		{
			if(points[i-1].x > points[i].x)
			{
				swap(points+i, points+i-1, sizeof(fvec));
				++swaps;
			}
		}
	} while(swaps > 0);
}

void sort_points_by_y(fvec* points, size_t count)
{
	int swaps = 0;
	do {
		for(size_t i = 1; i < count; ++i)
		{
			if(points[i-1].y > points[i].y)
			{
				swap(points+i, points+i-1, sizeof(fvec));
				++swaps;
			}
		}
	} while(swaps > 0);
}

void kdtree(kdtree_t** tree, fvec* points, size_t count, size_t depth)
{
	int axis = depth % K;
	size_t med;
	
	if(axis == 1)
		sort_points_by_x(points, count);
	else
		sort_points_by_y(points, count);
	
	med = count / 2;
	
	kdtree_t* ntree = *tree = malloc(sizeof(kdtree_t));
	
	ntree->location = points[med];
	kdtree(&ntree->right, points + med + 1, med-1, depth+1);
	kdtree(&ntree->right, points, med, depth+1);
}
