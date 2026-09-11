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
    
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_EVENTS) != 0)
        return 1;
    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        SDL_Quit();
        return 1;
    }

    SDL_Window *w = SDL_CreateWindow(
        "Five Nights at Edward's",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        1280, 720, SDL_WINDOW_SHOWN);

    if (!w) {
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    SDL_Renderer *r = SDL_CreateRenderer(
        w, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    if (!r) {
        SDL_DestroyWindow(w);
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    FnaeVisuals visuals;
    if (visuals_init(&visuals, r) != 0) {
        SDL_DestroyRenderer(r);
        SDL_DestroyWindow(w);
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    FnaeGame game;
    fnae_init(&game);

    Uint64 last = SDL_GetPerformanceCounter();

    while (game.running) {
        Uint64 now = SDL_GetPerformanceCounter();
        float dt = (float)((double)(now-last) /
                           (double)SDL_GetPerformanceFrequency());
        last = now;
        if (dt > 0.1f) dt = 0.1f;

        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                game.running = 0;
            } else if (e.type == SDL_KEYDOWN && !e.key.repeat) {
                fnae_key(&game, e.key.keysym.sym);
            }
        }

        fnae_update(&game, dt);

        visuals_render(
            &visuals, r,
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

    visuals_free(&visuals);
    SDL_DestroyRenderer(r);
    SDL_DestroyWindow(w);
    IMG_Quit();
    SDL_Quit();
    return 0;
}
