//
//  adb.h
//  engine
//
//  Created by Scott on 14/10/2022.
//

#ifndef adb_h
#define adb_h

#include <stddef.h>
#include <stdint.h>

typedef uint64_t asset_handle_t;

typedef void* (*asset_load_fn)(const char*);
typedef void (*asset_free_fn)(void*);

typedef struct asset_t asset_t;
typedef struct file_handler_t file_handler_t;

extern int init_asset_database(size_t max_asset);
extern asset_handle_t get_file_handle(const char* filename);
extern int asset_is_loaded(asset_handle_t handle);
extern void* get_asset(asset_handle_t handle);
#define get_asset_as(__TYPE, handle) ((__TYPE*)get_asset(handle))
extern int try_get_asset(asset_handle_t handle, void** o_ptr);
extern asset_handle_t load_asset(const char* file);
extern void free_asset(asset_handle_t handle);
extern size_t set_asset_memory(size_t max_bytes);
extern int close_asset_database();
extern int register_file_handler(const char* file_extention, asset_load_fn loadfn, asset_free_fn freefn);


#endif /* adb_h */
