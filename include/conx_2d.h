#ifndef CONX_2D_H
#define CONX_2D_H

#include "conx_math.h"
#include <stdbool.h>
#include <SDL2/SDL.h>

// Texture handle
typedef struct {
  SDL_Texture *texture;
  int width;
  int height;
} ConXTexture;

// 2D Sprite
typedef struct {
  Vec2 position;
  Vec2 size;
  Vec2 scale;
  float rotation;
  Vec4 color;
  ConXTexture *texture;
} ConXSprite;

// 2D subsystem
void conx_2d_init(void);
void conx_2d_shutdown(void);

// Asset loading
ConXTexture *conx_load_texture(const char *filepath);
void conx_free_texture(ConXTexture *texture);

// Drawing functions
void conx_draw_rect(Vec2 position, Vec2 size, Vec4 color);
void conx_draw_circle(Vec2 center, float radius, Vec4 color);
void conx_draw_sprite(ConXSprite *sprite);
void conx_draw_texture(ConXTexture *texture, Vec2 position, Vec2 size);

#endif
