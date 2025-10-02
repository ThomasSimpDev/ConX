#ifndef CONX_LUA_H
#define CONX_LUA_H

#include <stdbool.h>
#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>
#include <sys/stat.h>
#include <time.h>
#include "conx.h"

#define MAX_TRACKED_FILES 64

typedef struct {
  char *filepath;
  time_t last_modified;
} TrackedFile;

typedef struct {
  lua_State *L;
  bool initialized;
  char *entry_file;
  TrackedFile tracked_files[MAX_TRACKED_FILES];
  int tracked_count;
} ConXLuaState;

// Lua state management
bool conx_lua_init(void);
void conx_lua_shutdown(void);
lua_State *conx_lua_get_state(void);

// Script execution
bool conx_lua_execute_file(const char *filename);
bool conx_lua_execute_string(const char *code);

// API registration
void conx_lua_register_api(void);

// Configuration
bool conx_lua_get_config(const char *filename, ConXConfig *config);

// Hot reload functionality
bool conx_lua_check_reload(void);
void conx_lua_reload_current_file(void);

#endif
