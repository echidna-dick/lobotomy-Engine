#define GAME_WIDTH 640
#define GAME_HEIGHT 480

#define EDITOR_WIDTH   1290
#define EDITOR_HEIGHT  720

#define GAME_SCREEN_SIZE (GAME_WIDTH * GAME_HEIGHT)

#define MINIMAP_CELL_SIZE   8
#define MINIMAP_WIDTH       (MAP_WIDTH * MINIMAP_CELL_SIZE)
#define MINIMAP_HEIGHT      (MAP_HEIGHT * MINIMAP_CELL_SIZE)
#define MINIMAP_SIZE        (MINIMAP_WIDTH * MINIMAP_HEIGHT)

#define PROGRAM_NAME "Lobotomy raycast-Engine v0.1.32"

/* std libs */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

/* SDL libs */
#include <SDL2/SDL.h>
#include <SDL2/SDL_video.h>
#include <SDL2/SDL_surface.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>


/* Dear ImGui libs */
#include <imgui.h>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_sdlrenderer2.h>
#include <imgui_internal.h>

/* engine src */
#include "player.h"
#include "world_map.h"
#include "minimap.h"
#include "textures.h"

int main(int argc, char *argv[])
{
    (void)argc; (void)argv;

    printf("\n\nhello, this is a raycast game engine made by @0x45, you can see my github here: https://github.com/echidna-dick \n\n\n");
    printf("\t" PROGRAM_NAME "\n\n");
    printf("Engine: Start!\n");

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "ERROR: SDL_INIT_VIDEO: %s\n", SDL_GetError());
        return 1;
    }
    printf("SDL_INIT_VIDEO:\t\tOK\n");

    Mix_Init(MIX_INIT_MP3);
    printf("Mix_Init:\t\tOK\n");

    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 1024);

    Mix_Chunk *welcome_sound = Mix_LoadWAV("assets/sound/engine_sound/welcome.wav");
    if(!welcome_sound){
        fprintf(stderr, "sound ERROR(non-fatal): %s\n", Mix_GetError());
    }
    printf("welcome_sound:\t\tOK\n");
    Mix_Chunk *hub_welcome_sound = Mix_LoadWAV("assets/sound/engine_sound/hub_welcome.wav");
    if(!hub_welcome_sound){
        fprintf(stderr, "sound ERROR(non-fatal): %s\n", Mix_GetError());
    }
    printf("hub_sound:\t\tOK\n");
    Mix_Chunk *exit_sound = Mix_LoadWAV("assets/sound/engine_sound/exit.wav");
    if(!exit_sound){
        fprintf(stderr, "sound ERROR(non-fatal): %s\n", Mix_GetError());
    }
    printf("exit_sound:\t\tOK\n");

    Mix_PlayChannel(-1, hub_welcome_sound, 0);

    SDL_Window *window_ptr = SDL_CreateWindow(
        PROGRAM_NAME,
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        EDITOR_WIDTH, EDITOR_HEIGHT,
        SDL_WINDOW_RESIZABLE);
    if (!window_ptr) {
        fprintf(stderr, "ERROR: SDL_WindowCreate: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }
    printf("SDL_WindowCreate:\tOK\n");

    SDL_Surface *engine_logo_icon = IMG_Load("assets/images/Engine-logo.png");
    if (engine_logo_icon) {
        SDL_SetWindowIcon(window_ptr, engine_logo_icon);
        SDL_FreeSurface(engine_logo_icon);
        printf("logo icon:\t\tOK\n");
    } else {
        fprintf(stderr, "WARN: icon load failed (non-fatal): %s\n", IMG_GetError());
    }

    SDL_Renderer *renderer_ptr = SDL_CreateRenderer(
        window_ptr, -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer_ptr) {
        fprintf(stderr, "ERROR: SDL_CreateRenderer: %s\n", SDL_GetError());
        SDL_DestroyWindow(window_ptr);
        IMG_Quit();
        SDL_Quit();
        return 1;
    }
    printf("renderer:\t\tOK\n");

    SDL_Texture *gameFrame = SDL_CreateTexture(
        renderer_ptr,
        SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING,
        GAME_WIDTH, GAME_HEIGHT);
    if (!gameFrame) {
        fprintf(stderr, "ERROR: gameFrame: %s\n", SDL_GetError());
        SDL_DestroyRenderer(renderer_ptr);
        SDL_DestroyWindow(window_ptr);
        IMG_Quit();
        SDL_Quit();
        return 1;
    }
    printf("game texture:\t\tOK\n");

    IMG_Animation *startup_anim = IMG_LoadAnimation("assets/images/lobotomy-mambo.gif");
    SDL_Texture **mambo_frames = NULL;
    if(!startup_anim){
        fprintf(stderr, "IMG_LoadAnimation: %s\n", IMG_GetError());
        SDL_DestroyWindow(window_ptr);
        SDL_Quit();
        return 1;
    }
    if(startup_anim){
        mambo_frames = (SDL_Texture** )malloc(startup_anim->count*sizeof(SDL_Texture*));
        for(int i = 0; i < startup_anim->count; i++){
            mambo_frames[i] = SDL_CreateTextureFromSurface(renderer_ptr, 
                                                            startup_anim->frames[i]);
            printf("mambo.gif: %d frames\n", startup_anim->count);
        }
    }else{
        printf("startup anim Failed");
    }

    SDL_Texture *minimapFrame = SDL_CreateTexture(
        renderer_ptr,
        SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING,
        MINIMAP_WIDTH, MINIMAP_HEIGHT);
    if (!minimapFrame) {
        fprintf(stderr, "ERROR: minimapFrame: %s\n", SDL_GetError());
        SDL_DestroyTexture(gameFrame);
        SDL_DestroyRenderer(renderer_ptr);
        SDL_DestroyWindow(window_ptr);
        IMG_Quit();
        SDL_Quit();
        return 1;
    }
    printf("minimap texture:\tOK\n");

    uint32_t *pixels = (uint32_t*)malloc(GAME_SCREEN_SIZE * sizeof(uint32_t));
    if (!pixels) {
        fprintf(stderr, "ERROR: pixels malloc failed\n");
        SDL_DestroyTexture(minimapFrame);
        SDL_DestroyTexture(gameFrame);
        SDL_DestroyRenderer(renderer_ptr);
        SDL_DestroyWindow(window_ptr);
        IMG_Quit();
        SDL_Quit();
        return 1;
    }
    printf("Pixel buffer:\t\tOK\n");

    uint32_t *minimapPixels = (uint32_t*)malloc(MINIMAP_SIZE * sizeof(uint32_t));
    if (!minimapPixels) {
        fprintf(stderr, "ERROR: minimapPixels malloc failed\n");
        free(pixels);
        SDL_DestroyTexture(minimapFrame);
        SDL_DestroyTexture(gameFrame);
        SDL_DestroyRenderer(renderer_ptr);
        SDL_DestroyWindow(window_ptr);
        IMG_Quit();
        SDL_Quit();
        return 1;
    }
    printf("minimap pixels:\t\tOK\n");

    uint32_t *TexturePixels = (uint32_t*)malloc(TEX_PIXELS*sizeof(uint32_t));
    uint32_t *textures[NUM_TEXTURES] = {NULL};
    const char *texture_names[NUM_TEXTURES] = {
        "eagle",
        "redbrick",
        "purplestone",
        "greystone",
        "bluestone",
        "mossy",
        "wood",
        "colorstone"
    };
    const char *texture_paths[NUM_TEXTURES] = {
        "assets/textures/eagle.png",
        "assets/textures/redbrick.png",
        "assets/textures/purplestone.png",
        "assets/textures/greystone.png",
        "assets/textures/bluestone.png",
        "assets/textures/mossy.png",
        "assets/textures/wood.png",
        "assets/textures/colorstone.png"
    };

    for (int i = 0; i < NUM_TEXTURES; i++) {
        textures[i] = load_texture(texture_paths[i], renderer_ptr);
        if (!textures[i]) {
            fprintf(stderr, "FATAL: Failed to load texture %s\n", texture_paths[i]);
            /* clean up textures we already loaded */
            for (int j = 0; j < i; j++) free(textures[j]);
            /* clean up everything else */
            free(minimapPixels);
            free(TexturePixels);
            SDL_DestroyTexture(minimapFrame);
            SDL_DestroyTexture(gameFrame);
            SDL_DestroyRenderer(renderer_ptr);
            SDL_DestroyWindow(window_ptr);
            IMG_Quit();
            SDL_Quit();
            return 1;
        }
        printf("texture %d: %-12s OK\n", i, texture_names[i]);
    }

    int gif_current_frame = 0;
    Uint32 gif_frame_started_at = SDL_GetTicks();

    /* === ImGui init === */
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    printf("imGui version:\t\t%s\n", IMGUI_VERSION);
    printf("ImGui IO:\t\tOK\n");

    ImGui::StyleColorsDark();

    ImGui_ImplSDL2_InitForSDLRenderer(window_ptr, renderer_ptr);
    ImGui_ImplSDLRenderer2_Init(renderer_ptr);
    printf("ImGui (docking):\tOK\n");


    /* === Player === */
    Player player;
    player.posX = 22.0; player.posY = 12.0;
    player.dirX = -1.0; player.dirY = 0.0;
    player.planeX = 0.0; player.planeY = 0.66;

    Uint32 oldTime = 0;
    Uint32 curTime = SDL_GetTicks();

    bool show_demo = false;
    bool game_pannel_focused = false;

    int running = 1;
    while (running) {

        /* === events === */
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT) {
                
                printf("----------------------------\n");
                printf("            Exit:           \n");
                printf("QUIT_BUTTON pressed -> exit\n");
                printf("----------------------------\n");
                running = 0;
            }
            if (event.type == SDL_KEYDOWN &&
                event.key.keysym.sym == SDLK_ESCAPE) {
            
                printf("------------------------\n");
                printf("         Exit:          \n");
                printf(" \"ESC\" pressed -> exit\n");
                printf("------------------------\n");
                running = 0;
            }
        }

        /* === clear game pixel buffer to black === */
        for (int i = 0; i < GAME_SCREEN_SIZE; i++) {
            pixels[i] = 0xFF000000;
        }

        /* === raycast every column === */
        for (int x = 0; x < GAME_WIDTH; x++) {

            double cameraX = 2.0 * x / (double)GAME_WIDTH - 1.0;
            double rayDirX = player.dirX + player.planeX * cameraX;
            double rayDirY = player.dirY + player.planeY * cameraX;

            int mapX = (int)player.posX;
            int mapY = (int)player.posY;

            double deltaDistX = (rayDirX == 0) ? 1e30 : fabs(1.0 / rayDirX);
            double deltaDistY = (rayDirY == 0) ? 1e30 : fabs(1.0 / rayDirY);

            int stepX, stepY;
            double sideDistX, sideDistY;

            if (rayDirX < 0) { stepX = -1; sideDistX = (player.posX - mapX) * deltaDistX; }
            else             { stepX =  1; sideDistX = (mapX + 1.0 - player.posX) * deltaDistX; }
            if (rayDirY < 0) { stepY = -1; sideDistY = (player.posY - mapY) * deltaDistY; }
            else             { stepY =  1; sideDistY = (mapY + 1.0 - player.posY) * deltaDistY; }

            int hit = 0;
            int side = 0;
            while (!hit) {
                if (sideDistX < sideDistY) {
                    sideDistX += deltaDistX; mapX += stepX; side = 0;
                } else {
                    sideDistY += deltaDistY; mapY += stepY; side = 1;
                }
                if (world_map[mapX][mapY] > 0) hit = 1;
            }

            double perpWallDist = (side == 0)
                ? (sideDistX - deltaDistX)
                : (sideDistY - deltaDistY);

            int lineHeight = (int)(GAME_HEIGHT / perpWallDist);
            int drawStart = -lineHeight / 2 + GAME_HEIGHT / 2;
            if (drawStart < 0) drawStart = 0;
            int drawEnd = lineHeight / 2 + GAME_HEIGHT / 2;
            if (drawEnd >= GAME_HEIGHT) drawEnd = GAME_HEIGHT - 1;

            /* === Texturing === */

            /* Which texture to use — wall type 1 maps to texture 0, etc. */
            int texNum = world_map[mapX][mapY] - 1;
            if (texNum < 0) texNum = 0;
            if (texNum >= NUM_TEXTURES) texNum = NUM_TEXTURES - 1;

            /* Where on the wall did we hit (0.0 to 1.0)? */
            double wallX;
            if (side == 0) wallX = player.posY + perpWallDist * rayDirY;
            else           wallX = player.posX + perpWallDist * rayDirX;
            wallX -= floor(wallX);

            /* Convert to texture column index */
            int texX = (int)(wallX * (double)TEX_WIDTH);
            if (side == 0 && rayDirX > 0) texX = TEX_WIDTH - texX - 1;
            if (side == 1 && rayDirY < 0) texX = TEX_WIDTH - texX - 1;

            /* How fast to step through the texture vertically */
            double step = 1.0 * TEX_HEIGHT / lineHeight;
            double texPos = (drawStart - GAME_HEIGHT / 2 + lineHeight / 2) * step;

            /* Draw the stripe, sampling one texture pixel per screen pixel */
            for (int y = drawStart; y <= drawEnd; y++) {
                int texY = (int)texPos & (TEX_HEIGHT - 1);
                texPos += step;

                uint32_t color = textures[texNum][TEX_WIDTH * texY + texX];
                if (side == 1) color = (color >> 1) & 0xFF7F7F7F;

                pixels[y * GAME_WIDTH + x] = color;
            }
        }

        /* upload game pixels to GPU */
        SDL_UpdateTexture(gameFrame, NULL, pixels, GAME_WIDTH * sizeof(uint32_t));

        /* === Render the minimap === */

        /* 1. Clear minimap to dark gray */
        for (int i = 0; i < MINIMAP_SIZE; i++) {
            minimapPixels[i] = 0xFF202020;
        }

        /* 2. Draw map cells */
        for (int my = 0; my < MAP_HEIGHT; my++) {
            for (int mx = 0; mx < MAP_WIDTH; mx++) {
                int cell = world_map[mx][my];
                uint32_t mcolor;
                if (cell == 0) {
                    mcolor = 0xFF404040;  /* empty: lighter gray */
                } else {
                    switch (cell) {
                        case 1:  mcolor = 0xFFFF0000; break;
                        case 2:  mcolor = 0xFF00FF00; break;
                        case 3:  mcolor = 0xFF0000FF; break;
                        case 4:  mcolor = 0xFFFFFFFF; break;
                        default: mcolor = 0xFFFFFF00; break;
                    }
                }
                fill_rect(minimapPixels, MINIMAP_WIDTH, MINIMAP_HEIGHT,
                          mx * MINIMAP_CELL_SIZE, my * MINIMAP_CELL_SIZE,
                          MINIMAP_CELL_SIZE, MINIMAP_CELL_SIZE,
                          mcolor);
            }
        }

        /* 3. Draw player position as a 4x4 green dot */
        int player_pixel_x = (int)(player.posX * MINIMAP_CELL_SIZE);
        int player_pixel_y = (int)(player.posY * MINIMAP_CELL_SIZE);
        fill_rect(minimapPixels, MINIMAP_WIDTH, MINIMAP_HEIGHT,
                  player_pixel_x - 2, player_pixel_y - 2, 4, 4,
                  0xFF00FF00);

        /* 4. Draw direction line */
        int dir_end_x = (int)(player_pixel_x + player.dirX * MINIMAP_CELL_SIZE * 2);
        int dir_end_y = (int)(player_pixel_y + player.dirY * MINIMAP_CELL_SIZE * 2);
        draw_line(minimapPixels, MINIMAP_WIDTH, MINIMAP_HEIGHT,
                  player_pixel_x, player_pixel_y,
                  dir_end_x, dir_end_y,
                  0xFFFFFF00);

        /* 5. Upload minimap to GPU */
        SDL_UpdateTexture(minimapFrame, NULL, minimapPixels,
                          MINIMAP_WIDTH * sizeof(uint32_t));

        /* === frame timing === */
        oldTime = curTime;
        curTime = SDL_GetTicks();
        double frameTime = (curTime - oldTime) / 1000.0;
        if (frameTime <= 0) frameTime = 0.001;

        double moveSpeed = frameTime * 5.0;
        double rotSpeed  = frameTime * 3.0;

        /* === input — only when game panel focused === */
        if (game_pannel_focused) {
            const Uint8 *keys = SDL_GetKeyboardState(NULL);

            if (keys[SDL_SCANCODE_UP]) {
                if (world_map[(int)(player.posX + player.dirX * moveSpeed)][(int)player.posY] == 0)
                    player.posX += player.dirX * moveSpeed;
                if (world_map[(int)player.posX][(int)(player.posY + player.dirY * moveSpeed)] == 0)
                    player.posY += player.dirY * moveSpeed;
            }
            if (keys[SDL_SCANCODE_DOWN]) {
                if (world_map[(int)(player.posX - player.dirX * moveSpeed)][(int)player.posY] == 0)
                    player.posX -= player.dirX * moveSpeed;
                if (world_map[(int)player.posX][(int)(player.posY - player.dirY * moveSpeed)] == 0)
                    player.posY -= player.dirY * moveSpeed;
            }
            if (keys[SDL_SCANCODE_RIGHT]) {
                double oldDirX = player.dirX;
                player.dirX = player.dirX * cos(-rotSpeed) - player.dirY * sin(-rotSpeed);
                player.dirY = oldDirX * sin(-rotSpeed) + player.dirY * cos(-rotSpeed);
                double oldPlaneX = player.planeX;
                player.planeX = player.planeX * cos(-rotSpeed) - player.planeY * sin(-rotSpeed);
                player.planeY = oldPlaneX * sin(-rotSpeed) + player.planeY * cos(-rotSpeed);
            }
            if (keys[SDL_SCANCODE_LEFT]) {
                double oldDirX = player.dirX;
                player.dirX = player.dirX * cos(rotSpeed) - player.dirY * sin(rotSpeed);
                player.dirY = oldDirX * sin(rotSpeed) + player.dirY * cos(rotSpeed);
                double oldPlaneX = player.planeX;
                player.planeX = player.planeX * cos(rotSpeed) - player.planeY * sin(rotSpeed);
                player.planeY = oldPlaneX * sin(rotSpeed) + player.planeY * cos(rotSpeed);
            }
        }

        if(startup_anim){
            Uint32 now = SDL_GetTicks();
            if (now - gif_frame_started_at >= (Uint32)startup_anim->delays[gif_current_frame]) {
                gif_current_frame = (gif_current_frame + 1) % startup_anim->count;
                gif_frame_started_at = now;
            }
        }

        /* === ImGui frame === */
        ImGui_ImplSDLRenderer2_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport());

        if (startup_anim) {
            ImGui::Begin("Mambo");
            ImGui::Image((ImTextureID)(intptr_t)mambo_frames[gif_current_frame],
                        ImVec2(startup_anim->w, startup_anim->h));
            ImGui::End();
        }

        /* Game View */
        ImGui::Begin("Game View");
        game_pannel_focused = ImGui::IsWindowFocused();
        ImGui::Image((ImTextureID)(intptr_t)gameFrame,
                     ImVec2(GAME_WIDTH, GAME_HEIGHT));
        ImGui::End();

        /* Inspector */
        ImGui::Begin("Inspector");
        ImGui::Text("%s", PROGRAM_NAME);
        ImGui::Text("FPS: %.1f", io.Framerate);
        ImGui::Separator();
        ImGui::Text("Player");
        ImGui::Text("   pos:    (%.2f, %.2f)", player.posX, player.posY);
        ImGui::Text("   dir:    (%.2f, %.2f)", player.dirX, player.dirY);
        ImGui::Text("   plane:  (%.2f, %.2f)", player.planeX, player.planeY);
        ImGui::Separator();
        if (ImGui::Button("reset position")) {
            player.posX = 22.0; player.posY = 12.0;
            player.dirX = -1.0; player.dirY = 0.0;
            player.planeX = 0.0; player.planeY = 0.66;
            printf("player: position reset\n");
        }
        static bool show_map_window = false;
        if (ImGui::Button("open map")) {
            show_map_window = true;
        }
        ImGui::End();

        /* Map editor placeholder */
        if (show_map_window) {
            if (ImGui::Begin("Map window", &show_map_window)) {
                ImGui::Text("map");
                if (ImGui::Button("Close")) {
                    show_map_window = false;
                }
            }
            ImGui::End();
        }

        /* Minimap */
        ImGui::Begin("Minimap");
        ImGui::Image((ImTextureID)(intptr_t)minimapFrame,
                     ImVec2(MINIMAP_WIDTH * 2, MINIMAP_HEIGHT * 2));
        ImGui::End();

        /* Console */
        ImGui::Begin("Console");
        ImGui::TextWrapped(
            "Click the Game View panel to give it focus, then use Arrow keys to move.\n\n"
            "Drag panel tabs to rearrange. Drop one panel onto another to\n"
            "merge them as tabs. ESC quits the editor.");
        ImGui::Checkbox("show ImGui Demo", &show_demo);
        ImGui::End();

        if (show_demo) {
            ImGui::ShowDemoWindow(&show_demo);
        }
/*
        ImGui::Begin("Texture explorer");
        ImGui::
*/
        ImGui::Render();

        /* === present === */
        SDL_SetRenderDrawColor(renderer_ptr, 30, 30, 30, 255);
        SDL_RenderClear(renderer_ptr);
        ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), renderer_ptr);
        SDL_RenderPresent(renderer_ptr);
    }

    if (startup_anim) {
        for (int i = 0; i < startup_anim->count; i++) {
            SDL_DestroyTexture(mambo_frames[i]);
        }
        free(mambo_frames);
        IMG_FreeAnimation(startup_anim);
    }

    if (exit_sound) {
        Mix_PlayChannel(-1, exit_sound, 0);
        printf("Exit sound playing...\n");
    }

    /* === cleanup === */
    printf("\n\tClose ImGUI:\n");
    ImGui_ImplSDLRenderer2_Shutdown();   printf("imGui SDL2 render:\tOK\n");
    ImGui_ImplSDL2_Shutdown();           printf("imGui SDL2:\t\tOK\n");
    ImGui::DestroyContext();             printf("imGui context:\t\tOK\n");

    printf("\n\ttextures:\n");
    for (int i = 0; i < NUM_TEXTURES; i++) {
        free(textures[i]);
    }
    printf("free textures:\t\tOK\n");

    printf("\n\tminimap:\n");
    free(minimapPixels);                 printf("free minimap:\t\tOK\n");
    free(TexturePixels);                 printf("free texture:\t\tOK\n");
    SDL_DestroyTexture(minimapFrame);    printf("destroy minimap tex:\tOK\n");

    printf("\n\tClose SDL2:\n");
    free(pixels);                        printf("free pixels:\t\tOK\n");
    SDL_DestroyTexture(gameFrame);       printf("destroy game tex:\tOK\n");
    SDL_DestroyRenderer(renderer_ptr);   printf("destroy render:\t\tOK\n");
    SDL_DestroyWindow(window_ptr);       printf("destroy window:\t\tOK\n");

    printf("window closed, audio still playing...\n");
    while(Mix_Playing(-1) > 0){
        SDL_Delay(50);
    }
    printf("Audio:\t\tDONE\n");

    if (exit_sound){                      Mix_FreeChunk(exit_sound);             printf("free chunk\t\t\"exit_sound\"\n");}
    if (welcome_sound){                   Mix_FreeChunk(welcome_sound);          printf("free chunk\t\t\"welcome_sound\"\n");}
    if (hub_welcome_sound){               Mix_FreeChunk(hub_welcome_sound);      printf("free chunk\t\t\"hub_welcome_sound\"\n");}
    Mix_CloseAudio();                    printf("Mix_CloseAudio:\tOK\n");
    IMG_Quit();                          printf("IMG_Quit:\t\tOK\n");
    SDL_Quit();                          printf("SDL_Quit:\t\tOK\n");
    printf("\nexit program.\n");
    printf("\n:3\n\n");
    return 0;
}