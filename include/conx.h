#ifndef CONX_H
#define CONX_H

#include <stdbool.h>

// Engine configuration
typedef struct {
  int window_width;
  int window_height;
  const char *window_title;
  bool fullscreen;
  bool vsync;
} ConXConfig;

// Engine state
typedef struct {
  bool running;
  double delta_time;
  void *window;
  void *renderer;
} ConXEngine;

// Engine lifecycle
bool conx_init(const ConXConfig *config);
void conx_shutdown(void);
void conx_run(void);
ConXEngine *conx_get_engine(void);

// Public API
void conx_set_clear_color(float r, float g, float b, float a);
void conx_clear_screen(void);
void conx_swap_buffers(void);

#endif
