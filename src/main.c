#define SDL_MAIN_HANDLED
#include <stdio.h>
#include <SDL.h>
#include <SDL_image.h>
#include "fnae_core.h"
#include "visuals.h"

int main(int argc, char *argv[]) {
    (void)argc;
    (void)argv;

    printf("FNaE STARTED\n");
    fflush(stdout);

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_EVENTS) != 0) {
        printf("SDL INIT FAILED: %s\n", SDL_GetError());
        fflush(stdout);
        return 1;
    }

    printf("SDL INIT OK\n");
    fflush(stdout);

    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        printf("SDL_IMAGE INIT FAILED: %s\n", IMG_GetError());
        fflush(stdout);
        SDL_Quit();
        return 1;
    }

    printf("SDL_IMAGE INIT OK\n");
    fflush(stdout);

    SDL_Window *w = SDL_CreateWindow(
        "Five Nights at Edward's",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        1280,
        720,
        SDL_WINDOW_SHOWN
    );

    if (!w) {
        printf("WINDOW CREATION FAILED: %s\n", SDL_GetError());
        fflush(stdout);
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    printf("WINDOW OK\n");
    fflush(stdout);

    SDL_Renderer *r = SDL_CreateRenderer(
        w,
        -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
    );

    if (!r) {
        printf("RENDERER CREATION FAILED: %s\n", SDL_GetError());
        fflush(stdout);
        SDL_DestroyWindow(w);
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    printf("RENDERER OK\n");
    fflush(stdout);

    FnaeVisuals visuals;

    printf("LOADING VISUALS...\n");
    fflush(stdout);

    if (visuals_init(&visuals, r) != 0) {
        printf("VISUALS INIT FAILED\n");
        fflush(stdout);

        SDL_DestroyRenderer(r);
        SDL_DestroyWindow(w);
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    printf("VISUALS INIT OK\n");
    fflush(stdout);

    FnaeGame game;
    fnae_init(&game);

    printf("GAME INIT OK\n");
    printf("Starting frame: %d\n", game.frame);
    printf("Starting night: %d\n", game.night);
    fflush(stdout);

    Uint64 last = SDL_GetPerformanceCounter();

    while (game.running) {
        Uint64 now = SDL_GetPerformanceCounter();

        float dt = (float)(
            (double)(now - last) /
            (double)SDL_GetPerformanceFrequency()
        );

        last = now;

        if (dt > 0.1f)
            dt = 0.1f;

        SDL_Event e;

        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                game.running = 0;
            }
            else if (e.type == SDL_KEYDOWN && !e.key.repeat) {
                fnae_key(&game, e.key.keysym.sym);
            }
        }

        fnae_update(&game, dt);

        visuals_render(
            &visuals,
            r,
            (int)game.frame,
            game.camera,
            game.cam_anim == CAM_UP,
            game.night,
            game.time_of_day,
            game.hidden_power,
            game.left_door,
            game.right_door,
            game.mask_anim == MASK_DOWN
        );

        SDL_RenderPresent(r);
    }

    printf("GAME EXITING\n");
    fflush(stdout);

    visuals_free(&visuals);

    SDL_DestroyRenderer(r);
    SDL_DestroyWindow(w);

    IMG_Quit();
    SDL_Quit();

    return 0;
}