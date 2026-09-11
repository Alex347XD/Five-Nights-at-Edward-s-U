#include "visuals.h"
#include <stdio.h>
#include <string.h>

static SDL_Texture *load_png(SDL_Renderer *r, const char *path) {
    SDL_Surface *s = IMG_Load(path);

    if (!s) {
        fprintf(stderr, "FAILED TO LOAD IMAGE: %s\n", path);
        fprintf(stderr, "SDL_image error: %s\n", IMG_GetError());
        return NULL;
    }

    SDL_Texture *t = SDL_CreateTextureFromSurface(r, s);

    if (!t) {
        fprintf(stderr, "FAILED TO CREATE TEXTURE: %s\n", path);
        fprintf(stderr, "SDL error: %s\n", SDL_GetError());
    }

    SDL_FreeSurface(s);
    return t;
}

static void path_for(char *dst, size_t n, int id) {
    snprintf(dst, n, "assets/images/%d.png", id);
}

static SDL_Texture *load_id(SDL_Renderer *r, int id) {
    char p[256];
    path_for(p, sizeof p, id);
    return load_png(r, p);
}

int visuals_init(FnaeVisuals *v, SDL_Renderer *r) {
    memset(v, 0, sizeof *v);

    /*
     * These are real extracted images from the supplied FNaE asset bank:
     * 227 = office scene
     * 211/212 = volcano/Hell camera scene variants
     * 350/379 = forest camera scene variants
     * 312 = dinosaur exhibit scene
     * 2/4/7 = title/final/newspaper-sized screens
     */
    v->office = load_id(r, 227);
    v->cams[0] = load_id(r, 211);
    v->cams[1] = load_id(r, 350);
    v->cams[2] = load_id(r, 227);
    v->cams[3] = load_id(r, 312);

    v->static_tex = load_id(r, 46);
    v->title = load_id(r, 2);
    v->six_am = load_id(r, 4);
    v->final_screen = load_id(r, 7);

    /* 227 is also a useful fallback if an individual frame asset fails. */
    return v->office ? 0 : -1;
}

void visuals_free(FnaeVisuals *v) {
    if (v->office) SDL_DestroyTexture(v->office);
    if (v->static_tex) SDL_DestroyTexture(v->static_tex);
    if (v->title) SDL_DestroyTexture(v->title);
    if (v->six_am) SDL_DestroyTexture(v->six_am);
    if (v->final_screen) SDL_DestroyTexture(v->final_screen);

    for (int i = 0; i < 4; ++i) {
        if (v->cams[i]) {
            SDL_DestroyTexture(v->cams[i]);
        }
    }

    memset(v, 0, sizeof *v);
}

static void fit_center(SDL_Renderer *r, SDL_Texture *t) {
    if (!t) return;
    int rw, rh, tw, th;
    SDL_GetRendererOutputSize(r, &rw, &rh);
    SDL_QueryTexture(t, NULL, NULL, &tw, &th);

    float sx = (float)rw / (float)tw;
    float sy = (float)rh / (float)th;
    float s = sx < sy ? sx : sy;

    int w = (int)(tw*s), h = (int)(th*s);
    SDL_Rect d = {(rw-w)/2, (rh-h)/2, w, h};
    SDL_RenderCopy(r, t, NULL, &d);
}

static void draw_static(SDL_Renderer *r, SDL_Texture *t, int alpha) {
    if (!t) return;
    int rw, rh;
    SDL_GetRendererOutputSize(r, &rw, &rh);
    SDL_Rect d = {0,0,rw,rh};
    SDL_SetTextureBlendMode(t, SDL_BLENDMODE_BLEND);
    SDL_SetTextureAlphaMod(t, (Uint8)alpha);
    SDL_RenderCopy(r, t, NULL, &d);
    SDL_SetTextureAlphaMod(t, 255);
}

static void draw_power(SDL_Renderer *r, int power) {
    int rw, rh;
    SDL_GetRendererOutputSize(r, &rw, &rh);
    int w = 250, h = 18;
    SDL_Rect border = {30, 30, w, h};
    SDL_RenderDrawRect(r, &border);

    int fill = (w-4) * power / 10000;
    if (fill < 0) fill = 0;
    SDL_Rect bar = {32, 32, fill, h-4};
    SDL_RenderFillRect(r, &bar);

    (void)rh;
}

static void draw_door_indicators(SDL_Renderer *r, int left, int right) {
    int rw, rh;
    SDL_GetRendererOutputSize(r, &rw, &rh);
    SDL_Rect l = {35, rh-70, 100, 32};
    SDL_Rect rr = {rw-135, rh-70, 100, 32};
    SDL_RenderDrawRect(r, &l);
    SDL_RenderDrawRect(r, &rr);
    if (left) SDL_RenderFillRect(r, &l);
    if (right) SDL_RenderFillRect(r, &rr);
}

static void draw_camera_labels(SDL_Renderer *r, int camera) {
    int rw, rh;
    SDL_GetRendererOutputSize(r, &rw, &rh);
    const char *names[4] = {"CAM 01 - HELL", "CAM 02 - FOREST",
                           "CAM 03 - OFFICE", "CAM 04 - DINOSAUR EXHIBIT"};
    for (int i=0;i<4;i++) {
        SDL_Rect b = {rw-250, 90+i*52, 215, 40};
        if (i == camera) SDL_RenderFillRect(r, &b);
        else SDL_RenderDrawRect(r, &b);
        (void)names; /* text is supplied by the title/debug window for now */
    }
    (void)rh;
}

void visuals_render(FnaeVisuals *v, SDL_Renderer *r, int frame, int camera,
                    int camera_up, int night, int hour, int power,
                    int left_door, int right_door, int mask) {
    SDL_SetRenderDrawColor(r, 0, 0, 0, 255);
    SDL_RenderClear(r);

    if (frame == 2) {
        fit_center(r, v->title);
    } else if (frame == 3) {
        if (camera_up) {
            if (camera < 0 || camera > 3) camera = 0;
            fit_center(r, v->cams[camera]);
            draw_static(r, v->static_tex, 35);
            draw_camera_labels(r, camera);
        } else {
            fit_center(r, v->office);
            draw_power(r, power);
            draw_door_indicators(r, left_door, right_door);
            if (mask) {
                /* A dark overlay gives the correct mask-down visual behavior
                   while the extracted mask animation is wired in later. */
                int rw, rh;
                SDL_GetRendererOutputSize(r, &rw, &rh);
                SDL_SetRenderDrawColor(r, 0, 0, 0, 180);
                SDL_Rect m = {0,0,rw,rh};
                SDL_RenderFillRect(r, &m);
            }
        }
    } else if (frame == 9) {
        fit_center(r, v->six_am);
    } else if (frame == 4) {
        fit_center(r, v->final_screen);
    } else if (frame == 5) {
        fit_center(r, v->final_screen);
    } else if (frame == 7) {
        fit_center(r, v->final_screen);
    } else {
        fit_center(r, v->title);
    }

    char title[256];
    snprintf(title, sizeof title,
             "Five Nights at Edward's | Native | Night %d | %d AM | Power %d%%",
             night, hour, power/100);
    SDL_SetWindowTitle(SDL_GetWindowFromID(1), title);
}
