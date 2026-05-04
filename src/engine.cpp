#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480

#define SCREEN_SIZE (SCREEN_WIDTH * SCREEN_HEIGHT)

#define PROGRAM_NAME "Lobotomy raycast-Engine v0.0.3"

//  std libs
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

// libs SDL
#include <SDL2/SDL.h>
#include <SDL2/SDL_video.h>
#include <SDL2/SDL_surface.h>
#include <SDL2/SDL_image.h>

// dear imGUI libs
#include <imgui.h>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_sdlrenderer2.h>
#include <imgui_internal.h>

//src libs
#include "player.h"
#include "world_map.h"

int main(int argc, char *argv[])
{

    printf(PROGRAM_NAME" <--- version\n\n");
    printf("Engine: Start!\n");

    (void)argc;
    (void)argv;

    // player
    Player player;
    player.posX = 22.0, player.posY = 12.0;
    player.dirX = -1.0, player.dirY = 0.0;
    player.planeX = 0.0, player.planeY = 0.66;

    if(SDL_Init(SDL_INIT_VIDEO) != 0){
        fprintf(stderr, "ERROR: SDL_Init failed! %s\n", SDL_GetError());
        return 1;
    }
    printf("SDL_INIT_VIDEO: OK\n");

    // init window for display
    SDL_Window *window_ptr = SDL_CreateWindow(PROGRAM_NAME,
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        SCREEN_WIDTH, SCREEN_HEIGHT, 0);
    if(!window_ptr){
        fprintf(stderr, "ERROR: SDL_Window failed! %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }
    printf("SDL_Window: OK\n");

    // top bar logo
    SDL_Surface *engine_logo_icon = IMG_Load("assets/images/Engine-logo.png");
    if(!engine_logo_icon){
        fprintf(stderr, "ERROR: Creating SDL_Surface -> IMG_Load Failed %s\n", SDL_GetError());
        return 1;
    }else{
        SDL_SetWindowIcon(window_ptr, engine_logo_icon);
        SDL_FreeSurface(engine_logo_icon);
    }
    printf("logo: OK\n");

    // create a renderer
    SDL_Renderer *renderer_ptr = SDL_CreateRenderer(window_ptr,
        -1,
        SDL_RENDERER_ACCELERATED |
        SDL_RENDERER_PRESENTVSYNC);
    if(!renderer_ptr){
        fprintf(stderr, "ERROR: SDL_CreateRenderer failed! %s\n", SDL_GetError());
        SDL_DestroyWindow(window_ptr);
        IMG_Quit();
        SDL_Quit();
        return 1;
    }
    printf("SDL_Renderer: OK\n");

    // init imgui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void) io;
    printf("ImGui IO: OK\n");

    ImGui::StyleColorsDark();

    ImGui_ImplSDL2_InitForSDLRenderer(
        window_ptr,
        renderer_ptr
    );
    ImGui_ImplSDLRenderer2_Init(renderer_ptr);
    printf("ImGui_SDL2_Renderer_Init: OK\n");


    // 4 bytes per pixels
    SDL_Texture *frame = SDL_CreateTexture(renderer_ptr,
        SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING,
        SCREEN_WIDTH, SCREEN_HEIGHT);
    if(!frame){
        SDL_DestroyRenderer(renderer_ptr);
        SDL_DestroyWindow(window_ptr);
        IMG_Quit();
        SDL_Quit();
        return 1;
    }
    printf("[GPU] SDL_Texture: OK\n");

    // CPU canvas, write in every frame
    uint32_t *pixels = (uint32_t*)malloc(SCREEN_SIZE * sizeof(uint32_t));
    if(!pixels){
        SDL_DestroyTexture(frame);
        SDL_DestroyRenderer(renderer_ptr);
        SDL_DestroyWindow(window_ptr);
        IMG_Quit();
        SDL_Quit();
        return 1;
    }
    printf("[CPU] Pixeles malloc: OK\n");


    printf("\n         Main Loop:         \n");
    printf("----------------------------\n");

    int running = 1;
    Uint32 oldTime = 0;
    Uint32 curTime = SDL_GetTicks();

    // main loop
    while (running) {

        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT) {
                printf("----------------------------\n");
                printf("            Exit:          \n");
                printf("\tQUIT_BUTTON pressed -> exit\n");
                running = 0;
            }

            if (event.type == SDL_KEYDOWN &&
                event.key.keysym.sym == SDLK_ESCAPE) {
                printf("----------------------------\n");
                printf("            Exit:          \n");
                printf("\t\"ESC\" pressed -> exit\n");
                running = 0;
            }
        }

        for (int i = 0; i < SCREEN_SIZE; i++) {
            pixels[i] = 0xFF000000;
        }

        for (int x = 0; x < SCREEN_WIDTH; x++) {

            /* ray direction for this column */
            double cameraX = 2.0 * x / (double)SCREEN_WIDTH - 1.0;
            double rayDirX = player.dirX + player.planeX * cameraX;
            double rayDirY = player.dirY + player.planeY * cameraX;

            /* which map square the ray currently sits in */
            int mapX = (int)player.posX;
            int mapY = (int)player.posY;

            /* how far the ray travels to cross one full grid cell */
            double deltaDistX = (rayDirX == 0) ? 1e30 : fabs(1.0 / rayDirX);
            double deltaDistY = (rayDirY == 0) ? 1e30 : fabs(1.0 / rayDirY);

            int stepX, stepY;
            double sideDistX, sideDistY;

            /* set step direction and initial distance to first grid line */
            if (rayDirX < 0) {
                stepX = -1;
                sideDistX = (player.posX - mapX) * deltaDistX;
            } else {
                stepX = 1;
                sideDistX = (mapX + 1.0 - player.posX) * deltaDistX;
            }
            if (rayDirY < 0) {
                stepY = -1;
                sideDistY = (player.posY - mapY) * deltaDistY;
            } else {
                stepY = 1;
                sideDistY = (mapY + 1.0 - player.posY) * deltaDistY;
            }

            /* DDA: step grid line by grid line until a wall is hit */
            int hit = 0;
            int side = 0;
            while (!hit) {
                if (sideDistX < sideDistY) {
                    sideDistX += deltaDistX;
                    mapX += stepX;
                    side = 0;
                } else {
                    sideDistY += deltaDistY;
                    mapY += stepY;
                    side = 1;
                }
                if (world_map[mapX][mapY] > 0) hit = 1;
            }

            /* perpendicular distance to camera plane (no fisheye) */
            double perpWallDist = (side == 0)
                ? (sideDistX - deltaDistX)
                : (sideDistY - deltaDistY);

            /* height of the wall slice on screen */
            int lineHeight = (int)(SCREEN_HEIGHT / perpWallDist);

            int drawStart = -lineHeight / 2 + SCREEN_HEIGHT / 2;
            if (drawStart < 0) drawStart = 0;
            int drawEnd = lineHeight / 2 + SCREEN_HEIGHT / 2;
            if (drawEnd >= SCREEN_HEIGHT) drawEnd = SCREEN_HEIGHT - 1;

            uint32_t color;
            switch (world_map[mapX][mapY]) {
                case 1:  color = 0xFFFF0000; break; /* red */
                case 2:  color = 0xFF00FF00; break; /* green */
                case 3:  color = 0xFF0000FF; break; /* blue */
                case 4:  color = 0xFFFFFFFF; break; /* white */
                default: color = 0xFFFFFF00; break; /* yellow */
            }
            if (side == 1) color = (color >> 1) & 0xFF7F7F7F;

            /* draw the vertical stripe */
            for (int y = drawStart; y <= drawEnd; y++) {
                pixels[y * SCREEN_WIDTH + x] = color;
            }
        }

        oldTime = curTime;
        curTime = SDL_GetTicks();
        double frameTime = (curTime - oldTime) / 1000.0;
        if (frameTime <= 0) frameTime = 0.001;

        double moveSpeed = frameTime * 5.0;
        double rotSpeed  = frameTime * 3.0;

        const Uint8 *keys = SDL_GetKeyboardState(NULL);

        if (keys[SDL_SCANCODE_UP]) {
            if (world_map[(int)(player.posX + player.dirX * moveSpeed)][(int)player.posY] == 0)
                player.posX += player.dirX * moveSpeed;
            if (world_map[(int)player.posX][(int)(player.posY + player.dirY * moveSpeed)] == 0)
                player.posY += player.dirY * moveSpeed;
            printf("\tPLAYER: forward\n");
        }
        if (keys[SDL_SCANCODE_DOWN]) {
            if (world_map[(int)(player.posX - player.dirX * moveSpeed)][(int)player.posY] == 0)
                player.posX -= player.dirX * moveSpeed;
            if (world_map[(int)player.posX][(int)(player.posY - player.dirY * moveSpeed)] == 0)
                player.posY -= player.dirY * moveSpeed;
            printf("\tPLAYER: back\n");
        }
        if (keys[SDL_SCANCODE_RIGHT]) {
            double oldDirX = player.dirX;
            player.dirX = player.dirX * cos(-rotSpeed) - player.dirY * sin(-rotSpeed);
            player.dirY = oldDirX * sin(-rotSpeed) + player.dirY * cos(-rotSpeed);
            double oldPlaneX = player.planeX;
            player.planeX = player.planeX * cos(-rotSpeed) - player.planeY * sin(-rotSpeed);
            player.planeY = oldPlaneX * sin(-rotSpeed) + player.planeY * cos(-rotSpeed);
            printf("\tCAMERA --> \n");
        }
        if (keys[SDL_SCANCODE_LEFT]) {
            double oldDirX = player.dirX;
            player.dirX = player.dirX * cos(rotSpeed) - player.dirY * sin(rotSpeed);
            player.dirY = oldDirX * sin(rotSpeed) + player.dirY * cos(rotSpeed);
            double oldPlaneX = player.planeX;
            player.planeX = player.planeX * cos(rotSpeed) - player.planeY * sin(rotSpeed);
            player.planeY = oldPlaneX * sin(rotSpeed) + player.planeY * cos(rotSpeed);
            printf("\tCAMERA <-- \n");
        }

        ImGui_ImplSDLRenderer2_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        // ImGui windows goes here
        ImGui::Begin("Debug");
        ImGui::Text(PROGRAM_NAME);
        ImGui::Text("player pos: (%.2f, %.2f)", player.posX, player.posY);
        ImGui::Text("player dir: (%.2f, %.2f)", player.dirX, player.dirY);
        ImGui::Text("FPS: %.1f", io.Framerate);
        ImGui::End();

        
        
        // render
        ImGui::Render();

        // present


        SDL_UpdateTexture(frame, NULL, pixels, SCREEN_WIDTH * sizeof(uint32_t));
        SDL_RenderClear(renderer_ptr);
        SDL_RenderCopy(renderer_ptr, frame, NULL, NULL);
        ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), renderer_ptr);
        SDL_RenderPresent(renderer_ptr);
    }

    // close imGui
    ImGui_ImplSDLRenderer2_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    // close SDL2
    free(pixels);
    SDL_DestroyTexture(frame);
    SDL_DestroyRenderer(renderer_ptr);
    SDL_DestroyWindow(window_ptr);
    IMG_Quit();
    SDL_Quit();
    return 0;
}
