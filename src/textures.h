#ifndef TEXTURES_H
#define TEXTURES_H

#include <stdint.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#define TEX_WIDTH     64
#define TEX_HEIGHT    64
#define TEX_PIXELS    (TEX_WIDTH * TEX_HEIGHT)
#define NUM_TEXTURES  8

#ifdef __cplusplus
extern "C" {
#endif

uint32_t *load_texture(const char *path, SDL_Renderer *renderer_ptr);

#ifdef __cplusplus
}
#endif

#endif