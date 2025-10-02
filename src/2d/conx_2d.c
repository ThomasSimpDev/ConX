#include "conx_2d.h"
#include "conx.h"
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

void conx_2d_init(void) {
  // 2D subsystem initialization
}

void conx_2d_shutdown(void) {
  // 2D subsystem cleanup
}

ConXTexture *conx_load_texture(const char *filepath) {
  ConXEngine *engine = conx_get_engine();
  if (!engine || !engine->renderer) {
    printf("Engine not initialized\n");
    return NULL;
  }

  SDL_Surface *surface = IMG_Load(filepath);
  if (!surface) {
    printf("Failed to load image %s: %s\n", filepath, IMG_GetError());
    return NULL;
  }

  SDL_Texture *sdl_texture = SDL_CreateTextureFromSurface((SDL_Renderer *)engine->renderer, surface);
  if (!sdl_texture) {
    printf("Failed to create texture from %s: %s\n", filepath, SDL_GetError());
    SDL_FreeSurface(surface);
    return NULL;
  }

  ConXTexture *texture = (ConXTexture *)malloc(sizeof(ConXTexture));
  texture->texture = sdl_texture;
  texture->width = surface->w;
  texture->height = surface->h;

  SDL_FreeSurface(surface);
  printf("Loaded texture: %s (%dx%d)\n", filepath, texture->width, texture->height);
  return texture;
}

void conx_free_texture(ConXTexture *texture) {
  if (texture) {
    if (texture->texture) {
      SDL_DestroyTexture(texture->texture);
    }
    free(texture);
  }
}

void conx_draw_rect(Vec2 position, Vec2 size, Vec4 color) {
  ConXEngine *engine = conx_get_engine();
  if (!engine || !engine->renderer) return;

  SDL_Renderer *renderer = (SDL_Renderer *)engine->renderer;
  SDL_SetRenderDrawColor(renderer, (Uint8)(color.x * 255), (Uint8)(color.y * 255), 
                        (Uint8)(color.z * 255), (Uint8)(color.w * 255));
  
  SDL_Rect rect = {(int)position.x, (int)position.y, (int)size.x, (int)size.y};
  SDL_RenderFillRect(renderer, &rect);
}

void conx_draw_circle(Vec2 center, float radius, Vec4 color) {
  ConXEngine *engine = conx_get_engine();
  if (!engine || !engine->renderer) return;

  SDL_Renderer *renderer = (SDL_Renderer *)engine->renderer;
  SDL_SetRenderDrawColor(renderer, (Uint8)(color.x * 255), (Uint8)(color.y * 255), 
                        (Uint8)(color.z * 255), (Uint8)(color.w * 255));
  
  int x = (int)center.x;
  int y = (int)center.y;
  int r = (int)radius;
  
  for (int dy = -r; dy <= r; dy++) {
    for (int dx = -r; dx <= r; dx++) {
      if (dx*dx + dy*dy <= r*r) {
        SDL_RenderDrawPoint(renderer, x + dx, y + dy);
      }
    }
  }
}

void conx_draw_texture(ConXTexture *texture, Vec2 position, Vec2 size) {
  if (!texture || !texture->texture) return;
  
  ConXEngine *engine = conx_get_engine();
  if (!engine || !engine->renderer) return;

  SDL_Renderer *renderer = (SDL_Renderer *)engine->renderer;
  SDL_Rect dest = {(int)position.x, (int)position.y, (int)size.x, (int)size.y};
  SDL_RenderCopy(renderer, texture->texture, NULL, &dest);
}

void conx_draw_sprite(ConXSprite *sprite) {
  if (!sprite) return;
  
  Vec2 scaled_size = {sprite->size.x * sprite->scale.x, sprite->size.y * sprite->scale.y};
  
  if (sprite->texture) {
    ConXEngine *engine = conx_get_engine();
    if (!engine || !engine->renderer) return;
    
    SDL_Renderer *renderer = (SDL_Renderer *)engine->renderer;
    SDL_SetTextureColorMod(sprite->texture->texture, 
                          (Uint8)(sprite->color.x * 255),
                          (Uint8)(sprite->color.y * 255), 
                          (Uint8)(sprite->color.z * 255));
    SDL_SetTextureAlphaMod(sprite->texture->texture, (Uint8)(sprite->color.w * 255));
    
    SDL_Rect dest = {(int)sprite->position.x, (int)sprite->position.y, 
                     (int)scaled_size.x, (int)scaled_size.y};
    
    if (sprite->rotation != 0.0f) {
      SDL_Point center = {(int)(scaled_size.x / 2), (int)(scaled_size.y / 2)};
      SDL_RenderCopyEx(renderer, sprite->texture->texture, NULL, &dest, 
                       sprite->rotation * 180.0f / M_PI, &center, SDL_FLIP_NONE);
    } else {
      SDL_RenderCopy(renderer, sprite->texture->texture, NULL, &dest);
    }
  } else {
    conx_draw_rect(sprite->position, scaled_size, sprite->color);
  }
}
