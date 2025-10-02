#include "conx_lua.h"
#include "conx.h"
#include "conx_math.h"
#include "conx_2d.h"
#include "conx_3d.h"
#include <GL/gl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static ConXLuaState lua_state = {0};
static int original_require_ref = LUA_NOREF;

// File tracking for hot reload
static void add_tracked_file(const char *filepath) {
  if (lua_state.tracked_count >= MAX_TRACKED_FILES) return;
  
  // Check if already tracked
  for (int i = 0; i < lua_state.tracked_count; i++) {
    if (lua_state.tracked_files[i].filepath && 
        strcmp(lua_state.tracked_files[i].filepath, filepath) == 0) {
      return;
    }
  }
  
  struct stat file_stat;
  if (stat(filepath, &file_stat) == 0) {
    lua_state.tracked_files[lua_state.tracked_count].filepath = strdup(filepath);
    lua_state.tracked_files[lua_state.tracked_count].last_modified = file_stat.st_mtime;
    lua_state.tracked_count++;
  }
}

static void clear_tracked_files(void) {
  for (int i = 0; i < lua_state.tracked_count; i++) {
    if (lua_state.tracked_files[i].filepath) {
      free(lua_state.tracked_files[i].filepath);
      lua_state.tracked_files[i].filepath = NULL;
    }
  }
  lua_state.tracked_count = 0;
}

// Custom require function that tracks loaded files
static int lua_tracked_require(lua_State *L) {
  const char *modname = luaL_checkstring(L, 1);
  
  // Call original require using registry reference
  lua_rawgeti(L, LUA_REGISTRYINDEX, original_require_ref);
  lua_pushstring(L, modname);
  lua_call(L, 1, 1);
  
  // Convert module name to file path
  char filepath[512];
  char modname_copy[256];
  strncpy(modname_copy, modname, sizeof(modname_copy) - 1);
  modname_copy[sizeof(modname_copy) - 1] = '\0';
  
  // Replace dots with slashes for module paths
  for (char *p = modname_copy; *p; p++) {
    if (*p == '.') *p = '/';
  }
  
  snprintf(filepath, sizeof(filepath), "lua_scripts/%s.lua", modname_copy);
  
  add_tracked_file(filepath);
  return 1;
}

// Configuration function
static int lua_conx_config(lua_State *L) {
  if (!lua_istable(L, 1)) {
    luaL_error(L, "config must be a table");
    return 0;
  }
  
  // Store config in registry for later retrieval
  lua_pushvalue(L, 1);
  lua_setfield(L, LUA_REGISTRYINDEX, "conx_config");
  
  return 0;
}

// Lua API functions
static int lua_conx_set_clear_color(lua_State *L) {
  float r = (float)luaL_checknumber(L, 1);
  float g = (float)luaL_checknumber(L, 2);
  float b = (float)luaL_checknumber(L, 3);
  float a = (float)luaL_optnumber(L, 4, 1.0);

  conx_set_clear_color(r, g, b, a);
  return 0;
}

static int lua_conx_clear_screen(lua_State *L) {
  conx_clear_screen();
  return 0;
}

static int lua_conx_swap_buffers(lua_State *L) {
  conx_swap_buffers();
  return 0;
}

static int lua_vec3_create(lua_State *L) {
  float x = (float)luaL_checknumber(L, 1);
  float y = (float)luaL_checknumber(L, 2);
  float z = (float)luaL_checknumber(L, 3);

  Vec3 *vec = (Vec3 *)lua_newuserdata(L, sizeof(Vec3));
  *vec = vec3_create(x, y, z);

  luaL_getmetatable(L, "ConX.Vec3");
  lua_setmetatable(L, -2);

  return 1;
}

// Vec3 metatable methods
static int lua_vec3_add(lua_State *L) {
  Vec3 *a = (Vec3 *)luaL_checkudata(L, 1, "ConX.Vec3");
  Vec3 *b = (Vec3 *)luaL_checkudata(L, 2, "ConX.Vec3");

  Vec3 *result = (Vec3 *)lua_newuserdata(L, sizeof(Vec3));
  *result = vec3_add(*a, *b);

  luaL_getmetatable(L, "ConX.Vec3");
  lua_setmetatable(L, -2);

  return 1;
}

static int lua_vec3_tostring(lua_State *L) {
  Vec3 *vec = (Vec3 *)luaL_checkudata(L, 1, "ConX.Vec3");
  lua_pushfstring(L, "Vec3(%f, %f, %f)", vec->x, vec->y, vec->z);
  return 1;
}

// Input functions
static int lua_conx_is_key_pressed(lua_State *L) {
  const Uint8 *keys = (const Uint8*)lua_touserdata(L, 1);
  int scancode = (int)luaL_checknumber(L, 2);
  
  if (keys && scancode >= 0 && scancode < SDL_NUM_SCANCODES) {
    lua_pushboolean(L, keys[scancode]);
  } else {
    lua_pushboolean(L, 0);
  }
  return 1;
}

// 3D functions
static int lua_conx_set_3d_mode(lua_State *L) {
  bool enable = lua_toboolean(L, 1);
  if (enable) {
    conx_3d_init();
    
    // Get actual window size
    ConXEngine *engine = conx_get_engine();
    if (engine && engine->window) {
      int width, height;
      SDL_GetWindowSize((SDL_Window*)engine->window, &width, &height);
      glViewport(0, 0, width, height);
      
      // Update camera aspect ratio
      ConXCamera *camera = conx_3d_get_camera();
      if (camera) {
        camera->aspect = (float)width / (float)height;
      }
    }
    
    glEnable(GL_DEPTH_TEST);
  } else {
    glDisable(GL_DEPTH_TEST);
  }
  return 0;
}

static int lua_conx_draw_cube(lua_State *L) {
  float x = (float)luaL_checknumber(L, 1);
  float y = (float)luaL_checknumber(L, 2);
  float z = (float)luaL_checknumber(L, 3);
  float w = (float)luaL_optnumber(L, 4, 1.0);
  float h = (float)luaL_optnumber(L, 5, 1.0);
  float d = (float)luaL_optnumber(L, 6, 1.0);
  float r = (float)luaL_optnumber(L, 7, 1.0);
  float g = (float)luaL_optnumber(L, 8, 1.0);
  float b = (float)luaL_optnumber(L, 9, 1.0);
  float a = (float)luaL_optnumber(L, 10, 1.0);
  
  Vec3 pos = {x, y, z};
  Vec3 size = {w, h, d};
  Vec4 color = {r, g, b, a};
  conx_draw_cube(pos, size, color);
  return 0;
}

static int lua_conx_draw_sphere(lua_State *L) {
  float x = (float)luaL_checknumber(L, 1);
  float y = (float)luaL_checknumber(L, 2);
  float z = (float)luaL_checknumber(L, 3);
  float radius = (float)luaL_checknumber(L, 4);
  float r = (float)luaL_optnumber(L, 5, 1.0);
  float g = (float)luaL_optnumber(L, 6, 1.0);
  float b = (float)luaL_optnumber(L, 7, 1.0);
  float a = (float)luaL_optnumber(L, 8, 1.0);
  
  Vec3 pos = {x, y, z};
  Vec4 color = {r, g, b, a};
  conx_draw_sphere(pos, radius, color);
  return 0;
}

static int lua_conx_set_camera(lua_State *L) {
  float px = (float)luaL_checknumber(L, 1);
  float py = (float)luaL_checknumber(L, 2);
  float pz = (float)luaL_checknumber(L, 3);
  float tx = (float)luaL_optnumber(L, 4, 0.0);
  float ty = (float)luaL_optnumber(L, 5, 0.0);
  float tz = (float)luaL_optnumber(L, 6, 0.0);
  
  ConXCamera camera = conx_camera_create(
    vec3_create(px, py, pz),
    vec3_create(tx, ty, tz),
    45.0f
  );
  conx_3d_set_camera(&camera);
  return 0;
}

// Drawing functions
static int lua_conx_draw_rect(lua_State *L) {
  float x = (float)luaL_checknumber(L, 1);
  float y = (float)luaL_checknumber(L, 2);
  float w = (float)luaL_checknumber(L, 3);
  float h = (float)luaL_checknumber(L, 4);
  float r = (float)luaL_optnumber(L, 5, 1.0);
  float g = (float)luaL_optnumber(L, 6, 1.0);
  float b = (float)luaL_optnumber(L, 7, 1.0);
  float a = (float)luaL_optnumber(L, 8, 1.0);
  
  Vec2 pos = {x, y};
  Vec2 size = {w, h};
  Vec4 color = {r, g, b, a};
  conx_draw_rect(pos, size, color);
  return 0;
}

static int lua_conx_draw_circle(lua_State *L) {
  float x = (float)luaL_checknumber(L, 1);
  float y = (float)luaL_checknumber(L, 2);
  float radius = (float)luaL_checknumber(L, 3);
  float r = (float)luaL_optnumber(L, 4, 1.0);
  float g = (float)luaL_optnumber(L, 5, 1.0);
  float b = (float)luaL_optnumber(L, 6, 1.0);
  float a = (float)luaL_optnumber(L, 7, 1.0);
  
  Vec2 center = {x, y};
  Vec4 color = {r, g, b, a};
  conx_draw_circle(center, radius, color);
  return 0;
}

static int lua_conx_load_texture(lua_State *L) {
  const char *filepath = luaL_checkstring(L, 1);
  ConXTexture *texture = conx_load_texture(filepath);
  
  if (!texture) {
    lua_pushnil(L);
    return 1;
  }
  
  ConXTexture **userdata = (ConXTexture **)lua_newuserdata(L, sizeof(ConXTexture *));
  *userdata = texture;
  
  luaL_getmetatable(L, "ConX.Texture");
  lua_setmetatable(L, -2);
  
  return 1;
}

static int lua_conx_draw_texture(lua_State *L) {
  ConXTexture **texture = (ConXTexture **)luaL_checkudata(L, 1, "ConX.Texture");
  float x = (float)luaL_checknumber(L, 2);
  float y = (float)luaL_checknumber(L, 3);
  float w = (float)luaL_checknumber(L, 4);
  float h = (float)luaL_checknumber(L, 5);
  
  Vec2 pos = {x, y};
  Vec2 size = {w, h};
  conx_draw_texture(*texture, pos, size);
  return 0;
}

static int lua_texture_gc(lua_State *L) {
  ConXTexture **texture = (ConXTexture **)luaL_checkudata(L, 1, "ConX.Texture");
  if (*texture) {
    conx_free_texture(*texture);
    *texture = NULL;
  }
  return 0;
}

// Register the ConX API with Lua
void conx_lua_register_api(void) {
  lua_State *L = lua_state.L;

  // Hook require function for file tracking
  if (original_require_ref == LUA_NOREF) {
    lua_getglobal(L, "require");
    original_require_ref = luaL_ref(L, LUA_REGISTRYINDEX);
  }
  lua_pushcfunction(L, lua_tracked_require);
  lua_setglobal(L, "require");

  // Create ConX table
  lua_newtable(L);

  // Register functions
  lua_pushcfunction(L, lua_conx_set_clear_color);
  lua_setfield(L, -2, "set_clear_color");

  lua_pushcfunction(L, lua_conx_clear_screen);
  lua_setfield(L, -2, "clear_screen");

  lua_pushcfunction(L, lua_conx_swap_buffers);
  lua_setfield(L, -2, "swap_buffers");

  lua_pushcfunction(L, lua_vec3_create);
  lua_setfield(L, -2, "vec3");
  
  lua_pushcfunction(L, lua_conx_draw_rect);
  lua_setfield(L, -2, "draw_rect");
  
  lua_pushcfunction(L, lua_conx_draw_circle);
  lua_setfield(L, -2, "draw_circle");
  
  lua_pushcfunction(L, lua_conx_load_texture);
  lua_setfield(L, -2, "load_texture");
  
  lua_pushcfunction(L, lua_conx_draw_texture);
  lua_setfield(L, -2, "draw_texture");
  
  lua_pushcfunction(L, lua_conx_config);
  lua_setfield(L, -2, "config");
  
  // 3D functions
  lua_pushcfunction(L, lua_conx_set_3d_mode);
  lua_setfield(L, -2, "set_3d_mode");
  
  lua_pushcfunction(L, lua_conx_draw_cube);
  lua_setfield(L, -2, "draw_cube");
  
  lua_pushcfunction(L, lua_conx_draw_sphere);
  lua_setfield(L, -2, "draw_sphere");
  
  lua_pushcfunction(L, lua_conx_set_camera);
  lua_setfield(L, -2, "set_camera");
  
  lua_pushcfunction(L, lua_conx_is_key_pressed);
  lua_setfield(L, -2, "is_key_pressed");
  
  // Key constants
  lua_pushnumber(L, SDL_SCANCODE_W);
  lua_setfield(L, -2, "KEY_W");
  lua_pushnumber(L, SDL_SCANCODE_A);
  lua_setfield(L, -2, "KEY_A");
  lua_pushnumber(L, SDL_SCANCODE_S);
  lua_setfield(L, -2, "KEY_S");
  lua_pushnumber(L, SDL_SCANCODE_D);
  lua_setfield(L, -2, "KEY_D");
  lua_pushnumber(L, SDL_SCANCODE_Q);
  lua_setfield(L, -2, "KEY_Q");
  lua_pushnumber(L, SDL_SCANCODE_E);
  lua_setfield(L, -2, "KEY_E");

  lua_setglobal(L, "ConX");

  // Create Vec3 metatable
  luaL_newmetatable(L, "ConX.Vec3");

  lua_pushstring(L, "__add");
  lua_pushcfunction(L, lua_vec3_add);
  lua_settable(L, -3);

  lua_pushstring(L, "__tostring");
  lua_pushcfunction(L, lua_vec3_tostring);
  lua_settable(L, -3);

  lua_pop(L, 1); // Pop metatable
  
  // Create Texture metatable
  luaL_newmetatable(L, "ConX.Texture");
  
  lua_pushstring(L, "__gc");
  lua_pushcfunction(L, lua_texture_gc);
  lua_settable(L, -3);
  
  lua_pop(L, 1); // Pop metatable
}

bool conx_lua_init(void) {
  lua_state.L = luaL_newstate();
  if (!lua_state.L) {
    return false;
  }

  // Open standard libraries
  luaL_openlibs(lua_state.L);

  // Register ConX API
  conx_lua_register_api();

  lua_state.initialized = true;
  return true;
}

void conx_lua_shutdown(void) {
  if (lua_state.L) {
    if (original_require_ref != LUA_NOREF) {
      luaL_unref(lua_state.L, LUA_REGISTRYINDEX, original_require_ref);
      original_require_ref = LUA_NOREF;
    }
    lua_close(lua_state.L);
    lua_state.L = NULL;
  }
  if (lua_state.entry_file) {
    free(lua_state.entry_file);
    lua_state.entry_file = NULL;
  }
  clear_tracked_files();
  lua_state.initialized = false;
}

lua_State *conx_lua_get_state(void) { return lua_state.L; }

bool conx_lua_execute_file(const char *filename) {
  if (!lua_state.initialized || !lua_state.L) {
    return false;
  }

  // Store entry file
  if (lua_state.entry_file) {
    free(lua_state.entry_file);
  }
  lua_state.entry_file = strdup(filename);
  
  // Track the entry file
  add_tracked_file(filename);

  if (luaL_dofile(lua_state.L, filename) != LUA_OK) {
    const char *error = lua_tostring(lua_state.L, -1);
    printf("Lua error: %s\n", error);
    lua_pop(lua_state.L, 1);
    return false;
  }

  return true;
}

bool conx_lua_execute_string(const char *code) {
  if (!lua_state.initialized || !lua_state.L) {
    return false;
  }

  if (luaL_dostring(lua_state.L, code) != LUA_OK) {
    const char *error = lua_tostring(lua_state.L, -1);
    printf("Lua error: %s\n", error);
    lua_pop(lua_state.L, 1);
    return false;
  }

  return true;
}

bool conx_lua_check_reload(void) {
  for (int i = 0; i < lua_state.tracked_count; i++) {
    if (!lua_state.tracked_files[i].filepath) continue;
    
    struct stat file_stat;
    if (stat(lua_state.tracked_files[i].filepath, &file_stat) == 0) {
      if (file_stat.st_mtime > lua_state.tracked_files[i].last_modified) {
        return true;
      }
    }
  }
  return false;
}

void conx_lua_reload_current_file(void) {
  if (!lua_state.entry_file) {
    return;
  }

  printf("Hot reload detected\n");
  
  // Update modification times
  for (int i = 0; i < lua_state.tracked_count; i++) {
    if (!lua_state.tracked_files[i].filepath) continue;
    
    struct stat file_stat;
    if (stat(lua_state.tracked_files[i].filepath, &file_stat) == 0) {
      lua_state.tracked_files[i].last_modified = file_stat.st_mtime;
    }
  }
  
  // Clear Lua state and reload
  lua_settop(lua_state.L, 0);
  
  // Clear package.loaded to force module reloading
  lua_getglobal(lua_state.L, "package");
  lua_getfield(lua_state.L, -1, "loaded");
  lua_pushnil(lua_state.L);
  while (lua_next(lua_state.L, -2)) {
    lua_pop(lua_state.L, 1); // Remove value
    lua_pushvalue(lua_state.L, -1); // Copy key
    lua_pushnil(lua_state.L);
    lua_settable(lua_state.L, -4); // Set package.loaded[key] = nil
  }
  lua_pop(lua_state.L, 2); // Pop package.loaded and package
  
  conx_lua_register_api();
  
  if (luaL_dofile(lua_state.L, lua_state.entry_file) != LUA_OK) {
    const char *error = lua_tostring(lua_state.L, -1);
    printf("Lua reload error: %s\n", error);
    lua_pop(lua_state.L, 1);
  }
}

bool conx_lua_get_config(const char *filename, ConXConfig *config) {
  if (!lua_state.initialized || !lua_state.L) {
    return false;
  }
  
  // Execute the Lua file to get configuration
  if (luaL_dofile(lua_state.L, filename) != LUA_OK) {
    const char *error = lua_tostring(lua_state.L, -1);
    printf("Lua config error: %s\n", error);
    lua_pop(lua_state.L, 1);
    return false;
  }
  
  // Get config from registry
  lua_getfield(lua_state.L, LUA_REGISTRYINDEX, "conx_config");
  if (!lua_istable(lua_state.L, -1)) {
    lua_pop(lua_state.L, 1);
    return false;
  }
  
  // Extract configuration values
  lua_getfield(lua_state.L, -1, "width");
  if (lua_isnumber(lua_state.L, -1)) {
    config->window_width = (int)lua_tonumber(lua_state.L, -1);
  }
  lua_pop(lua_state.L, 1);
  
  lua_getfield(lua_state.L, -1, "height");
  if (lua_isnumber(lua_state.L, -1)) {
    config->window_height = (int)lua_tonumber(lua_state.L, -1);
  }
  lua_pop(lua_state.L, 1);
  
  lua_getfield(lua_state.L, -1, "title");
  if (lua_isstring(lua_state.L, -1)) {
    config->window_title = lua_tostring(lua_state.L, -1);
  }
  lua_pop(lua_state.L, 1);
  
  lua_getfield(lua_state.L, -1, "fullscreen");
  if (lua_isboolean(lua_state.L, -1)) {
    config->fullscreen = lua_toboolean(lua_state.L, -1);
  }
  lua_pop(lua_state.L, 1);
  
  lua_getfield(lua_state.L, -1, "vsync");
  if (lua_isboolean(lua_state.L, -1)) {
    config->vsync = lua_toboolean(lua_state.L, -1);
  }
  lua_pop(lua_state.L, 1);
  
  lua_pop(lua_state.L, 1); // Pop config table
  return true;
}
