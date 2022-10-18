//
//  kdtree.h
//  engine
//
//  Created by Scott on 12/10/2022.
//

#ifndef kdtree_h
#define kdtree_h

#include "vec.h"
#include <stddef.h>

typedef struct kdtree_t {
	fvec location;
	struct kdtree_t* left;
	struct kdtree_t* right;
} kdtree_t;

void kdtree(kdtree_t** tree, fvec* points, size_t count, size_t depth);

#endif /* kdtree_h */
