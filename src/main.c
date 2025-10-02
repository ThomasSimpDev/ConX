#include "conx_all.h"
#include <stdio.h>

int main(int argc, char *argv[]) {
  // Check if Lua script path is provided
  if (argc < 2) {
    printf("Usage: %s <lua_script_path>\n", argv[0]);
    printf("Example: %s lua_scripts/example.lua\n", argv[0]);
    return -1;
  }

  // Initialize Lua first to get configuration
  if (!conx_lua_init()) {
    printf("Failed to initialize Lua\n");
    return -1;
  }

  // Default configuration
  ConXConfig config = {.window_width = 800,
                       .window_height = 600,
                       .window_title = "ConX Engine",
                       .fullscreen = false,
                       .vsync = true};

  // Try to get configuration from Lua
  if (!conx_lua_get_config(argv[1], &config)) {
    printf("Using default configuration\n");
  }

  // Initialize engine with configuration
  if (!conx_init(&config)) {
    printf("Failed to initialize ConX engine\n");
    conx_lua_shutdown();
    return -1;
  }

  // Initialize other subsystems
  conx_2d_init();
  conx_3d_init();

  // Execute the specified Lua script
  if (!conx_lua_execute_file(argv[1])) {
    printf("Failed to execute Lua script: %s\n", argv[1]);
    conx_2d_shutdown();
    conx_shutdown();
    conx_lua_shutdown();
    return -1;
  }

  // Run the engine
  conx_run();

  conx_2d_shutdown();
  conx_3d_shutdown();
  conx_shutdown();
  conx_lua_shutdown();
  return 0;
}
