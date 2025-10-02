#include "conx_all.h"

bool conx_initialize_all(const ConXConfig *config) {
  if (!conx_init(config)) {
    return false;
  }

  if (!conx_lua_init()) {
    conx_shutdown();
    return false;
  }

  conx_2d_init();
  conx_3d_init();

  return true;
}

void conx_shutdown_all(void) {
  conx_3d_shutdown();
  conx_2d_shutdown();
  conx_lua_shutdown();
  conx_shutdown();
}
