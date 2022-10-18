//
//  adb.c
//  engine
//
//  Created by Scott on 14/10/2022.
//

#include "adb.h"
#include <memory.h>
#include <math.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

typedef struct asset_t {
	char* filename;
	size_t extention_offset;
	void* instance;
	asset_handle_t handle;
} asset_t;

typedef struct file_handler_t {
	uint64_t ext_hash;
	char* extention;
	asset_load_fn load;
	asset_free_fn free;
} file_handler_t;

int adb_init = 0;

asset_t* adb_assets_first;
size_t adb_assets_last;
size_t adb_assets_size;

file_handler_t* adb_handlers_first;
size_t adb_handlers_last;
size_t adb_handlers_size;

static inline uint64_t str_hash(const char* str);
static inline void unload_asset(asset_t* asset);
static inline size_t file_extention_offset(const char* filename);
static inline void free_asset_t(asset_t*);


//
// MODULE LIFETIME
//

int init_asset_database(size_t max_asset_bytes)
{
	assert(adb_init == 0);
	adb_assets_size = max_asset_bytes;
	adb_handlers_size = 5;
	
	adb_assets_last = 0;
	adb_assets_first = malloc(adb_assets_size * sizeof(asset_t));
	
	adb_handlers_last = 0;
	adb_handlers_first = malloc(adb_handlers_size * sizeof(file_handler_t));
	
	adb_init = 1;
	
	return 0;
}

int close_asset_database()
{
	for(size_t i = 0; i < adb_assets_last; ++i)
	{
		free_asset_t(&adb_assets_first[i]);
	}
	for(size_t i = 0; i < adb_handlers_last; ++i)
	{
		free(adb_handlers_first[i].extention);
	}
	free(adb_assets_first);
	free(adb_handlers_first);
	
	return 0;
}


//
//	FILE HANDLERS
//

void sort_file_handlers()
{
	size_t swaps = 0;
	file_handler_t tmp;
	do
	{
		swaps = 0;
		for(size_t i = 1; i < adb_handlers_last; ++i)
		{
			if(adb_handlers_first[i-1].ext_hash > adb_handlers_first[i].ext_hash)
			{
				memcpy(&tmp, &adb_handlers_first[i-1], sizeof(file_handler_t));
				memcpy(&adb_handlers_first[i-1], &adb_handlers_first[i], sizeof(file_handler_t));
				memcpy(&adb_handlers_first[i], &tmp, sizeof(file_handler_t));
				++swaps;
			}
		}
	} while(swaps > 0);
}

void destroy_file_handler(file_handler_t* handler)
{
	free(handler->extention);
	free(handler);
}

file_handler_t* get_file_handler(const char* extention)
{
	if(adb_handlers_last == 0)
		return NULL;
	
	uint64_t hash = str_hash(extention);
	long int l = 0, r = adb_handlers_last-1, m;
	file_handler_t* current;
	while(l <= r)
	{
		m = floorf((float)(l+r)/2.f);
		current = &adb_handlers_first[m];
		if(current->ext_hash < hash)
			l = m + 1;
		else if(current->ext_hash > hash)
			r = m - 1;
		else if(current->ext_hash == hash)
			return current;
	}
	return NULL;
}

int register_file_handler(const char* file_extention, asset_load_fn loadfn, asset_free_fn freefn)
{
	file_handler_t* existing = get_file_handler(file_extention);
	if(existing == NULL)
	{
		if(adb_handlers_last + 1 >= adb_handlers_size)
		{
			adb_handlers_first = realloc(adb_handlers_first, adb_handlers_size * 2);
		}
		file_handler_t* new = (adb_handlers_first+adb_handlers_last);
		new->load = loadfn;
		new->free = freefn;
		new->extention = malloc(strlen(file_extention));
		strcpy(new->extention, file_extention);
		new->ext_hash = str_hash(file_extention);
		++adb_handlers_last;
	}
	else
	{
		if(loadfn != NULL)
			existing->load = loadfn;
		if(loadfn != NULL)
			existing->free = freefn;
	}
	
	sort_file_handlers();
	
	return 0;
}


//
//	ASSETS
//

void sort_assets()
{
	size_t swaps = 0;
	asset_t tmp;
	do
	{
		swaps = 0;
		for(size_t i = 0; i < adb_assets_last; ++i)
		{
			if(adb_assets_first[i-1].handle > adb_assets_first[i].handle)
			{
				memcpy(&tmp, &adb_assets_first[i-1], sizeof(asset_t));
				memcpy(&adb_assets_first[i-1], &adb_assets_first[i], sizeof(asset_t));
				memcpy(&adb_assets_first[i], &tmp, sizeof(asset_t));
				++swaps;
			}
		}
	} while(swaps > 0);
}

asset_t* get_asset_t(asset_handle_t handle)
{
	if(adb_assets_last == 0)
		return NULL;
	
	asset_t* current;
	long int l = 0, r = adb_assets_last-1, m;
	while(l <= r)
	{
		m = floorf((float)(l+r)/2.f);
		current = &adb_assets_first[m];
		
		if(current->handle < handle)
			l = m + 1;
		else if(current->handle > handle)
			r = m - 1;
		else if(current->handle == handle)
			return current;
	}
	return NULL;
}

void* get_asset(asset_handle_t handle)
{
	asset_t* asset = get_asset_t(handle);
	if(asset == NULL)
		return NULL;
	else
		return asset->instance;
}

int try_get_asset(asset_handle_t handle, void** o_ptr)
{
	void* asset = get_asset(handle);
	if(o_ptr != NULL)
		memcpy(o_ptr, &asset, sizeof(void*));
	return asset != NULL;
}

asset_handle_t load_asset(const char* file)
{
	char* filename = realpath(file, NULL);
	if(filename == NULL)
		return 0;
	
	asset_handle_t handle = str_hash(filename);
	
	if(try_get_asset(handle, NULL))
		return handle;
	
	size_t ext_offset = file_extention_offset(filename);
	file_handler_t* handler = get_file_handler(filename + ext_offset);
	
	adb_assets_first[adb_assets_last] = (asset_t){
		.filename = filename,
		.extention_offset = ext_offset,
		.instance = handler->load(filename),
		.handle = handle
	};
	
	++adb_assets_last;
	
	sort_assets();
	return handle;
}

void free_asset_t(asset_t* asset)
{
	file_handler_t* handler = get_file_handler(asset->filename + asset->extention_offset);
	assert(handler != NULL);
	
	handler->free(asset->instance);
	free(asset->filename);
}

void free_asset(asset_handle_t handle)
{
	asset_t* asset = get_asset_t(handle);
	
	if(asset == NULL)
		return;
	
	free_asset_t(asset);
}


//
//	FILES
//

asset_handle_t get_file_handle(const char* filename)
{
	char* real_path = realpath(filename, NULL);
	asset_handle_t handle = str_hash(real_path);
	free(real_path);
	return try_get_asset(handle, NULL) ? handle : 0;
}

size_t file_extention_offset(const char* filename)
{
	const char* cptr = filename + strlen(filename);
	while(cptr != filename)
	{
		if(*cptr == '.')
			return cptr - filename;
		--cptr;
	}
	
	return 0;
}

uint64_t str_hash(const char* str)
{
	static const int shift = (8 * sizeof(uint64_t) - 4);
	static const uint64_t upper4mask = (uint64_t)0xF << shift;
	uint64_t hash = 0;
	uint64_t upperfour;
	size_t len = 0;
	
	for(const char* c = str; *c != '\0'; ++c)
	{
		upperfour = hash & upper4mask;
		hash = ((hash << 4) ^ (*c));
		if(upperfour != 0)
			hash ^= (upperfour >> shift);
		++len;
	}
	
	return hash ^ len;
}
