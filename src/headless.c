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

/* Minimal built-in input so headless exercises title -> night -> camera
 * when no --script file is given. */
static void headless_default_script(FnaeGame *game, int frame) {
    switch (frame) {
    case 5: fnae_key(game, SDLK_RETURN); break;   /* Warning -> Title */
    case 10: fnae_key(game, SDLK_RETURN); break;  /* Title: New -> Newspaper */
    case 40: fnae_key(game, SDLK_RETURN); break;  /* Newspaper -> Which Night */
    case 70: fnae_key(game, SDLK_RETURN); break;  /* Which Night -> start night (skip 2s timer) */
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

/* Resolves a KEY token to an SDL_Keycode. Named keys plus single
 * characters (letters lowercased to match fnae_key expectations). */
static int headless_keycode(const char *tok) {
    if (strcmp(tok, "return") == 0 || strcmp(tok, "enter") == 0)
        return SDLK_RETURN;
    if (strcmp(tok, "esc") == 0 || strcmp(tok, "escape") == 0)
        return SDLK_ESCAPE;
    if (strcmp(tok, "space") == 0)
        return SDLK_SPACE;
    if (strcmp(tok, "up") == 0)
        return SDLK_UP;
    if (strcmp(tok, "down") == 0)
        return SDLK_DOWN;
    if (strcmp(tok, "left") == 0)
        return SDLK_LEFT;
    if (strcmp(tok, "right") == 0)
        return SDLK_RIGHT;
    if (strlen(tok) == 1) {
        int c = (unsigned char)tok[0];
        if (c >= 'A' && c <= 'Z')
            c += 'a' - 'A';
        return c;
    }
    return SDLK_UNKNOWN;
}

int headless_load_script(const char *path, HeadlessScript *out) {
    out->events = NULL;
    out->count = 0;

    FILE *f = fopen(path, "r");
    if (!f) {
        fprintf(stderr, "HEADLESS: cannot open script %s\n", path);
        return 1;
    }

    int cap = 0;
    char line[512];
    int lineno = 0;
    int rc = 0;
    while (fgets(line, sizeof line, f)) {
        ++lineno;
        /* Strip leading whitespace; skip blanks and '#' comments. */
        char *p = line;
        while (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n')
            ++p;
        if (*p == '\0' || *p == '#')
            continue;

        int frame = -1;
        char action[16] = {0};
        char arg1[256] = {0};
        char arg2[64] = {0};
        int nf = sscanf(p, "%d %15s %255s %63s", &frame, action, arg1, arg2);
        if (nf < 3 || frame < 0) {
            fprintf(stderr, "HEADLESS: %s:%d: bad line (want '<frame> <key|keyup|click|shot> <args>')\n",
                path, lineno);
            rc = 1;
            break;
        }

        if (out->count == cap) {
            int ncap = cap ? cap * 2 : 64;
            HeadlessEvent *nev = (HeadlessEvent *)realloc(out->events,
                (size_t)ncap * sizeof *nev);
            if (!nev) {
                fprintf(stderr, "HEADLESS: out of memory\n");
                rc = 1;
                break;
            }
            out->events = nev;
            cap = ncap;
        }
        HeadlessEvent *ev = &out->events[out->count];
        memset(ev, 0, sizeof *ev);
        ev->frame = frame;

        if (strcmp(action, "key") == 0 || strcmp(action, "keyup") == 0) {
            ev->key = headless_keycode(arg1);
            if (ev->key == SDLK_UNKNOWN) {
                fprintf(stderr, "HEADLESS: %s:%d: unknown key '%s'\n", path, lineno, arg1);
                rc = 1;
                break;
            }
            ev->type = strcmp(action, "key") == 0 ? HEV_KEY : HEV_KEYUP;
        } else if (strcmp(action, "click") == 0 || strcmp(action, "mouse") == 0) {
            if (nf < 4) {
                fprintf(stderr, "HEADLESS: %s:%d: %s needs X Y\n", path, lineno, action);
                rc = 1;
                break;
            }
            ev->type = strcmp(action, "click") == 0 ? HEV_CLICK : HEV_MOUSE;
            ev->x = atoi(arg1);
            ev->y = atoi(arg2);
        } else if (strcmp(action, "shot") == 0) {
            ev->type = HEV_SHOT;
            strncpy(ev->shot, arg1, sizeof ev->shot - 1);
        } else {
            fprintf(stderr, "HEADLESS: %s:%d: unknown action '%s'\n", path, lineno, action);
            rc = 1;
            break;
        }
        ++out->count;
    }
    fclose(f);

    if (rc != 0) {
        headless_free_script(out);
        return rc;
    }
    printf("HEADLESS script=%s events=%d\n", path, out->count);
    fflush(stdout);
    return 0;
}

void headless_free_script(HeadlessScript *s) {
    free(s->events);
    s->events = NULL;
    s->count = 0;
}

int headless_run(SDL_Renderer *r, FnaeVisuals *v, FnaeGame *game,
                 const HeadlessOptions *opt, const HeadlessScript *script) {
    int frames = opt->frames < 1 ? 1 : opt->frames;
    const float dt = 1.0f / 60.0f;

    for (int f = 0; f < frames && game->running; ++f) {
        if (script) {
            for (int i = 0; i < script->count; ++i) {
                const HeadlessEvent *ev = &script->events[i];
                if (ev->frame != f)
                    continue;
                switch (ev->type) {
                case HEV_KEY: fnae_key(game, ev->key); break;
                case HEV_KEYUP: fnae_key_up(game, ev->key); break;
                case HEV_CLICK: fnae_click(game, ev->x, ev->y); break;
                case HEV_MOUSE: fnae_mouse_move(game, ev->x, ev->y); break;
                case HEV_SHOT:
                    printf("HEADLESS shot frame=%d path=%s game_frame=%s\n",
                        f, ev->shot, fnae_frame_name(game->frame));
                    fflush(stdout);
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
                        game->progress,
                        game->static_frame,
                        game->static_alpha,
                        (int)game->office_scroll,
                        game->left_door_frame,
                        game->right_door_frame,
                        game->title_bg_frame,
                        game->foxy.pos,
                        game->freddy.pos,
                        game->cam_static_alpha,
                        game->death,
                        game->music_left,
                        (int)game->cam_scroll,
                        game->power_left
                    );
                    SDL_RenderPresent(r);
                    if (headless_save_screenshot(r, ev->shot) != 0)
                        return 1;
                    break;
                }
            }
        } else {
            headless_default_script(game, f);
        }
        /* Drain any pending SDL events (e.g. quit) without blocking. */
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT)
                game->running = 0;
        }
        fnae_update(game, dt);
        fnae_static_tick(game);
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
            game->progress,
            game->static_frame,
            game->static_alpha,
            (int)game->office_scroll,
            game->left_door_frame,
            game->right_door_frame,
            game->title_bg_frame,
            game->foxy.pos,
            game->freddy.pos,
            game->cam_static_alpha,
            game->death,
            game->music_left,
            (int)game->cam_scroll,
            game->power_left
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
