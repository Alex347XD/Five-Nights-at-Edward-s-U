#define SDL_MAIN_HANDLED

#include <stdio.h>
#include <stdlib.h>
#include <SDL.h>
#include <SDL_image.h>

#include "fnae_core.h"
#include "headless.h"
#include "visuals.h"

#include <string.h>

static void print_usage(const char *prog) {
    printf("Usage: %s [--headless] [--frames N] [--screenshot PATH] [--script PATH]\n", prog);
    printf("  --headless          run without a visible window (hidden window,\n");
    printf("                      software renderer fallback, fixed 1/60 dt)\n");
    printf("  --frames N          headless frame count (default 600)\n");
    printf("  --screenshot PATH   save final frame (.png or .bmp,\n");
    printf("                      default screenshots/headless.png)\n");
    printf("  --script PATH       headless input script (key/keyup/click/shot\n");
    printf("                      events by frame; see scripts/headless/example.txt)\n");
    printf("Screenshots land under screenshots/; clean them with:\n");
    printf("  cmake --build build --target clean-screenshots\n");
}

int main(int argc, char *argv[]) {
    int headless = 0;
    HeadlessOptions hopt;
    hopt.frames = 600;
    hopt.screenshot = "screenshots/headless.png";
    const char *script_path = NULL;

    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--headless") == 0) {
            headless = 1;
        } else if (strcmp(argv[i], "--frames") == 0 && i + 1 < argc) {
            hopt.frames = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--screenshot") == 0 && i + 1 < argc) {
            hopt.screenshot = argv[++i];
        } else if (strcmp(argv[i], "--script") == 0 && i + 1 < argc) {
            script_path = argv[++i];
        } else if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
            print_usage(argv[0]);
            return 0;
        } else {
            fprintf(stderr, "Unknown argument: %s\n", argv[i]);
            print_usage(argv[0]);
            return 2;
        }
    }
    if (hopt.frames < 1) hopt.frames = 1;

    printf("FNaE STARTED\n");
    if (headless)
        printf("HEADLESS frames=%d screenshot=%s script=%s\n",
            hopt.frames, hopt.screenshot, script_path ? script_path : "(built-in)");
    fflush(stdout);

    HeadlessScript script;
    memset(&script, 0, sizeof script);
    if (script_path && !headless) {
        fprintf(stderr, "Warning: --script ignored without --headless\n");
        script_path = NULL;
    }
    if (script_path && headless_load_script(script_path, &script) != 0)
        return 1;

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_EVENTS) != 0) {
        headless_free_script(&script);
        return 1;
    }
    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        headless_free_script(&script);
        SDL_Quit();
        return 1;
    }

    SDL_Window *w = SDL_CreateWindow(
        "Five Nights at Edward's",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        1280, 720, headless ? SDL_WINDOW_HIDDEN : SDL_WINDOW_SHOWN);

    if (!w) {
        fprintf(stderr, "SDL_CreateWindow failed: %s\n", SDL_GetError());
        headless_free_script(&script);
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    SDL_Renderer *r = SDL_CreateRenderer(
        w, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!r) {
        /* Dummy video driver (headless/CI) has no accelerated renderer. */
        r = SDL_CreateRenderer(w, -1, SDL_RENDERER_SOFTWARE);
    }

    if (!r) {
        fprintf(stderr, "SDL_CreateRenderer failed: %s\n", SDL_GetError());
        headless_free_script(&script);
        SDL_DestroyWindow(w);
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    FnaeVisuals visuals;
    if (visuals_init(&visuals, r) != 0) {
        headless_free_script(&script);
        SDL_DestroyRenderer(r);
        SDL_DestroyWindow(w);
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    FnaeGame game;
    fnae_init(&game);

    if (headless) {
        int rc = headless_run(r, &visuals, &game, &hopt,
                              script_path ? &script : NULL);
        headless_free_script(&script);
        visuals_free(&visuals);
        SDL_DestroyRenderer(r);
        SDL_DestroyWindow(w);
        IMG_Quit();
        SDL_Quit();
        return rc;
    }

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
            } else if (e.type == SDL_KEYUP && !e.key.repeat) {
                fnae_key_up(&game, e.key.keysym.sym);
            } else if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT) {
                fnae_click(&game, e.button.x, e.button.y);
            }
        }

        fnae_update(&game, dt);
        fnae_static_tick(&game);

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
            game.mask_anim == MASK_DOWN,
            game.arrow,
            game.progress,
            game.static_frame,
            game.static_alpha
        );

        SDL_RenderPresent(r);
    }

    headless_free_script(&script);
    visuals_free(&visuals);
    SDL_DestroyRenderer(r);
    SDL_DestroyWindow(w);
    IMG_Quit();
    SDL_Quit();
    return 0;
}
