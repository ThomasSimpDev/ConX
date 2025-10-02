#include "conx.h"
#include "conx_lua.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <GL/gl.h>
#include <stdio.h>
#include <stdlib.h>

static ConXEngine *engine = NULL;

bool conx_init(const ConXConfig *config) {
  if (engine) {
    printf("Engine already initialized\n");
    return false;
  }

  // Allocate engine
  engine = (ConXEngine *)malloc(sizeof(ConXEngine));
  if (!engine) {
    printf("Failed to allocate engine\n");
    return false;
  }

  // Initialize SDL
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
    printf("SDL initialization failed: %s\n", SDL_GetError());
    free(engine);
    engine = NULL;
    return false;
  }
  
  // Initialize SDL_image
  int img_flags = IMG_INIT_PNG | IMG_INIT_JPG;
  if (!(IMG_Init(img_flags) & img_flags)) {
    printf("SDL_image initialization failed: %s\n", IMG_GetError());
    SDL_Quit();
    free(engine);
    engine = NULL;
    return false;
  }

  // Set OpenGL attributes
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

  // Create window
  Uint32 flags = SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN;
  if (config->fullscreen) {
    flags |= SDL_WINDOW_FULLSCREEN;
  }

  engine->window = SDL_CreateWindow(
      config->window_title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
      config->window_width, config->window_height, flags);

  if (!engine->window) {
    printf("Window creation failed: %s\n", SDL_GetError());
    SDL_Quit();
    free(engine);
    engine = NULL;
    return false;
  }

  // Create OpenGL context
  SDL_GLContext gl_context = SDL_GL_CreateContext((SDL_Window *)engine->window);
  if (!gl_context) {
    printf("OpenGL context creation failed: %s\n", SDL_GetError());
    SDL_DestroyWindow((SDL_Window *)engine->window);
    SDL_Quit();
    free(engine);
    engine = NULL;
    return false;
  }

  // Create renderer
  engine->renderer =
      SDL_CreateRenderer((SDL_Window *)engine->window, -1,
                         SDL_RENDERER_ACCELERATED |
                             (config->vsync ? SDL_RENDERER_PRESENTVSYNC : 0));

  if (!engine->renderer) {
    printf("Renderer creation failed: %s\n", SDL_GetError());
    SDL_DestroyWindow((SDL_Window *)engine->window);
    SDL_Quit();
    free(engine);
    engine = NULL;
    return false;
  }

  engine->running = true;
  engine->delta_time = 0.0;

  printf("ConX Engine initialized successfully\n");
  return true;
}

void conx_shutdown(void) {
  if (!engine)
    return;

  if (engine->renderer) {
    SDL_DestroyRenderer((SDL_Renderer *)engine->renderer);
  }
  if (engine->window) {
    SDL_DestroyWindow((SDL_Window *)engine->window);
  }

  IMG_Quit();
  SDL_Quit();
  free(engine);
  engine = NULL;

  printf("ConX Engine shutdown\n");
}

void conx_run(void) {
  if (!engine || !engine->running)
    return;

  Uint64 last_time = SDL_GetPerformanceCounter();

  while (engine->running) {
    Uint64 current_time = SDL_GetPerformanceCounter();
    engine->delta_time = (double)((current_time - last_time) * 1000 /
                                  (double)SDL_GetPerformanceFrequency());
    last_time = current_time;

    // Process events
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) {
        engine->running = false;
      }
    }
    
    // Update input state
    const Uint8 *keys = SDL_GetKeyboardState(NULL);
    int mouse_x, mouse_y;
    Uint32 mouse_state = SDL_GetMouseState(&mouse_x, &mouse_y);
    
    // Handle mouse warping for right-click
    static bool was_right_pressed = false;
    static int center_x = 0, center_y = 0;
    bool is_right_pressed = mouse_state & SDL_BUTTON(SDL_BUTTON_RIGHT);
    
    if (is_right_pressed && !was_right_pressed) {
      // Store center position when starting to drag
      center_x = mouse_x;
      center_y = mouse_y;
    }
    
    int delta_x = 0, delta_y = 0;
    if (is_right_pressed) {
      delta_x = mouse_x - center_x;
      delta_y = mouse_y - center_y;
      // Warp mouse back to center position
      SDL_WarpMouseInWindow((SDL_Window*)engine->window, center_x, center_y);
    }
    
    was_right_pressed = is_right_pressed;
    
    // Call Lua input function if it exists
    lua_State *L = conx_lua_get_state();
    if (L) {
      lua_getglobal(L, "handle_input");
      if (lua_isfunction(L, -1)) {
        lua_pushlightuserdata(L, (void*)keys);
        lua_pushnumber(L, delta_x);
        lua_pushnumber(L, delta_y);
        lua_pushboolean(L, is_right_pressed);
        if (lua_pcall(L, 4, 0, 0) != LUA_OK) {
          const char *error = lua_tostring(L, -1);
          printf("Lua input error: %s\n", error);
          lua_pop(L, 1);
        }
      } else {
        lua_pop(L, 1);
      }
    }

    // Check for Lua file changes and reload if needed
    if (conx_lua_check_reload()) {
      conx_lua_reload_current_file();
    }

    // Call Lua update function
    // Reuse L from above
    if (L) {
      lua_getglobal(L, "update");
      if (lua_isfunction(L, -1)) {
        if (lua_pcall(L, 0, 0, 0) != LUA_OK) {
          const char *error = lua_tostring(L, -1);
          printf("Lua update error: %s\n", error);
          lua_pop(L, 1);
        }
      } else {
        lua_pop(L, 1);
      }
    }
  }
}

ConXEngine *conx_get_engine(void) { return engine; }

void conx_set_clear_color(float r, float g, float b, float a) {
  if (!engine)
    return;
  glClearColor(r, g, b, a);
}

void conx_clear_screen(void) {
  if (!engine)
    return;
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void conx_swap_buffers(void) {
  if (!engine || !engine->window)
    return;
  SDL_GL_SwapWindow((SDL_Window *)engine->window);
}
