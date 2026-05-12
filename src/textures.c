#include <textures.h>

uint32_t *load_texture(const char *path, SDL_Renderer *renderer_ptr)
{

    (void)renderer_ptr;     // we dont need renderer here, only IMG_Load()

    SDL_Surface *raw = IMG_Load(path);
    if(!raw){
        fprintf(stderr, "ERROR: IMG_Load(%s): %s\n", path, IMG_GetError());
        return NULL;
    }

    // convert to know format ARGB8888, to not handle every pixle layout
    SDL_Surface *surface = SDL_ConvertSurfaceFormat(raw, SDL_PIXELFORMAT_ARGB8888, 0);
    if(!surface){
        fprintf(stderr, "ERROR: SDL_ConvertSurfaceFormat(%s): %s\n", path, SDL_GetError());
        return NULL;
    }

    // sanity check for expected size 64x64
    if(surface->w != TEX_WIDTH || surface->h != TEX_HEIGHT){
        fprintf(stderr, "%s is %dx%d, expected %dx%d\n",
            path, surface->w, surface->h, TEX_WIDTH, TEX_HEIGHT);
        return NULL;
    }

    uint32_t *TexturePixels = (uint32_t*)malloc(TEX_PIXELS*sizeof(uint32_t));
    if(!TexturePixels){
        SDL_FreeSurface(surface);
        return NULL;
    }

    for (int y = 0; y < TEX_HEIGHT; y++) {
        uint32_t *src = (uint32_t *)((uint8_t *)surface->pixels + y * surface->pitch);
        uint32_t *dst = TexturePixels + y * TEX_WIDTH;
        for (int x = 0; x < TEX_WIDTH; x++) {
            dst[x] = src[x];
        }
    }

    SDL_FreeSurface(surface);
    return TexturePixels;
}

