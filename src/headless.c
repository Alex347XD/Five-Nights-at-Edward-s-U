#include "headless.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <direct.h>
#else
#include <sys/stat.h>
#include <sys/types.h>
#endif

/* Minimal scripted input so headless exercises title -> night -> camera. */
static void headless_script(FnaeGame *game, int frame) {
    switch (frame) {
    case 10: fnae_key(game, 13); break;          /* Title: New -> Newspaper */
    case 40: fnae_key(game, 13); break;          /* Newspaper -> Which Night */
    case 70: fnae_key(game, 13); break;          /* Which Night -> start night */
    case 120: fnae_key(game, 's'); break;        /* camera up */
    case 180: fnae_key(game, 's'); break;        /* camera down */
    case 210: fnae_click(game, 90, 100); break;  /* harmless office click */
    default: break;
    }
}

static int make_dir(const char *path) {
#ifdef _WIN32
    return _mkdir(path);
#else
    return mkdir(path, 0755);
#endif
}

/* Creates missing parent directories for path (handles '/' and '\\'). */
static void ensure_parent_dir(const char *path) {
    char tmp[512];
    size_t n = strlen(path);
    if (n == 0 || n >= sizeof tmp)
        return;
    strcpy(tmp, path);
    /* Drop the filename, keep the directory part. */
    char *sep = strrchr(tmp, '/');
    char *bsep = strrchr(tmp, '\\');
    if (bsep && (!sep || bsep > sep))
        sep = bsep;
    if (!sep)
        return; /* bare filename, CWD exists */
    *sep = '\0';

    /* Walk and create each level so nested paths work too. */
    for (char *p = tmp + 1; *p; ++p) {
        if (*p == '/' || *p == '\\') {
            char c = *p;
            *p = '\0';
            make_dir(tmp);
            *p = c;
        }
    }
    make_dir(tmp);
}

int headless_save_screenshot(SDL_Renderer *r, const char *path) {
    int w, h;
    if (SDL_GetRendererOutputSize(r, &w, &h) != 0) {
        fprintf(stderr, "HEADLESS: GetRendererOutputSize failed: %s\n", SDL_GetError());
        return -1;
    }
    SDL_Surface *s = SDL_CreateRGBSurfaceWithFormat(0, w, h, 24, SDL_PIXELFORMAT_RGB24);
    if (!s) {
        fprintf(stderr, "HEADLESS: CreateRGBSurface failed: %s\n", SDL_GetError());
        return -1;
    }
    if (SDL_RenderReadPixels(r, NULL, SDL_PIXELFORMAT_RGB24, s->pixels, s->pitch) != 0) {
        fprintf(stderr, "HEADLESS: RenderReadPixels failed: %s\n", SDL_GetError());
        SDL_FreeSurface(s);
        return -1;
    }
    ensure_parent_dir(path);
    int rc;
    size_t n = strlen(path);
    if (n > 4 && strcmp(path + n - 4, ".png") == 0)
        rc = IMG_SavePNG(s, path);
    else
        rc = SDL_SaveBMP(s, path);
    if (rc != 0) {
        fprintf(stderr, "HEADLESS: failed to save %s: %s\n", path, SDL_GetError());
        SDL_FreeSurface(s);
        return -1;
    }
    SDL_FreeSurface(s);
    return 0;
}

int headless_run(SDL_Renderer *r, FnaeVisuals *v, FnaeGame *game,
                 const HeadlessOptions *opt) {
    int frames = opt->frames < 1 ? 1 : opt->frames;
    const float dt = 1.0f / 60.0f;

    for (int f = 0; f < frames && game->running; ++f) {
        headless_script(game, f);
        /* Drain any pending SDL events (e.g. quit) without blocking. */
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT)
                game->running = 0;
        }
        fnae_update(game, dt);
        visuals_render(
            v, r,
            (int)game->frame,
            game->camera,
            game->cam_anim == CAM_UP,
            game->night,
            game->time_of_day,
            game->hidden_power,
            game->left_door,
            game->right_door,
            game->mask_anim == MASK_DOWN,
            game->arrow,
            game->progress
        );
        SDL_RenderPresent(r);
        if ((f + 1) % 60 == 0) {
            printf("HEADLESS frame=%d game_frame=%s night=%d tod=%d power=%d\n",
                f + 1, fnae_frame_name(game->frame),
                game->night, game->time_of_day, game->hidden_power);
            fflush(stdout);
        }
    }
    printf("HEADLESS DONE frames=%d game_frame=%s\n",
        frames, fnae_frame_name(game->frame));
    fflush(stdout);

    if (headless_save_screenshot(r, opt->screenshot) != 0)
        return 1;
    printf("HEADLESS screenshot=%s\n", opt->screenshot);
    fflush(stdout);
    return 0;
}
