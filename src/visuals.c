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

    /* Real extracted gameplay assets. */
    v->office = load_id(r, 227);
    v->cams[0] = load_id(r, 211);
    /* Cam 01 = Hell, Cam 02 = Mountain/forest feed, Cam 03 = Forest, Cam 04 = Dinosaur Exhibit. */
    v->cams[1] = load_id(r, 379);
    v->cams[2] = load_id(r, 350);
    v->cams[3] = load_id(r, 312);
    v->static_tex = load_id(r, 46);
    v->six_am = load_id(r, 4);
    v->death = load_id(r, 1);
    v->newspaper = load_id(r, 7);
    v->final_screen = load_id(r, 7);

    /*
     * Frame 2 (Title) assets mapped from the exported object layout:
     *
     * Background  -> 179 (1280x720 title background)
     * New         -> 239
     * Continue    -> 240
     * 6 Night     -> 241
     * Custom      -> 242
     * Arrow       -> 245
     * Star        -> 232
     *
     * The three Star objects all use the same source image in Fusion.
     */
    v->title_bg = load_id(r, 179);
    v->title_new = load_id(r, 239);
    v->title_continue = load_id(r, 240);
    v->title_6night = load_id(r, 241);
    v->title_custom = load_id(r, 242);
    v->title_arrow = load_id(r, 245);
    v->title_star = load_id(r, 232);
    /* Template Title is 233 (600x507). 238 is a death-screen animation frame and must NOT be used here. */
    v->title_template = load_id(r, 233);
    for (int i = 0; i < 7; ++i)
        v->title_nights[i] = load_id(r, 246 + i);

    return v->title_bg ? 0 : -1;
}

static void destroy_texture(SDL_Texture **t) {
    if (*t) {
        SDL_DestroyTexture(*t);
        *t = NULL;
    }
}

void visuals_free(FnaeVisuals *v) {
    destroy_texture(&v->office);
    for (int i = 0; i < 4; ++i)
        destroy_texture(&v->cams[i]);
    destroy_texture(&v->static_tex);
    destroy_texture(&v->six_am);
    destroy_texture(&v->death);
    destroy_texture(&v->title_bg);
    destroy_texture(&v->title_new);
    destroy_texture(&v->title_continue);
    destroy_texture(&v->title_6night);
    destroy_texture(&v->title_custom);
    destroy_texture(&v->title_arrow);
    destroy_texture(&v->title_star);
    destroy_texture(&v->title_template);
    for (int i = 0; i < 7; ++i)
        destroy_texture(&v->title_nights[i]);
    destroy_texture(&v->newspaper);
    destroy_texture(&v->final_screen);
    memset(v, 0, sizeof *v);
}

static void draw_texture(SDL_Renderer *r, SDL_Texture *t, int x, int y) {
    if (!t) return;
    int w, h;
    SDL_QueryTexture(t, NULL, NULL, &w, &h);
    SDL_Rect d = {x, y, w, h};
    SDL_RenderCopy(r, t, NULL, &d);
}

static void fit_center(SDL_Renderer *r, SDL_Texture *t) {
    if (!t) return;
    int rw, rh, tw, th;
    SDL_GetRendererOutputSize(r, &rw, &rh);
    SDL_QueryTexture(t, NULL, NULL, &tw, &th);

    float sx = (float)rw / (float)tw;
    float sy = (float)rh / (float)th;
    float s = sx < sy ? sx : sy;

    int w = (int)(tw * s);
    int h = (int)(th * s);
    SDL_Rect d = {(rw - w) / 2, (rh - h) / 2, w, h};
    SDL_RenderCopy(r, t, NULL, &d);
}

static void draw_static(SDL_Renderer *r, SDL_Texture *t, int alpha) {
    if (!t) return;
    int rw, rh;
    SDL_GetRendererOutputSize(r, &rw, &rh);
    SDL_Rect d = {0, 0, rw, rh};
    SDL_SetTextureBlendMode(t, SDL_BLENDMODE_BLEND);
    SDL_SetTextureAlphaMod(t, (Uint8)alpha);
    SDL_RenderCopy(r, t, NULL, &d);
    SDL_SetTextureAlphaMod(t, 255);
}

static void draw_power(SDL_Renderer *r, int power) {
    int w = 250, h = 18;
    SDL_Rect border = {30, 30, w, h};
    SDL_RenderDrawRect(r, &border);

    int fill = (w - 4) * power / 10000;
    if (fill < 0) fill = 0;
    SDL_Rect bar = {32, 32, fill, h - 4};
    SDL_RenderFillRect(r, &bar);
}

static void draw_door_indicators(SDL_Renderer *r, int left, int right) {
    int rw, rh;
    SDL_GetRendererOutputSize(r, &rw, &rh);
    SDL_Rect l = {35, rh - 70, 100, 32};
    SDL_Rect rr = {rw - 135, rh - 70, 100, 32};
    SDL_RenderDrawRect(r, &l);
    SDL_RenderDrawRect(r, &rr);
    if (left) SDL_RenderFillRect(r, &l);
    if (right) SDL_RenderFillRect(r, &rr);
}

static void draw_camera_labels(SDL_Renderer *r, int camera) {
    int rw, rh;
    SDL_GetRendererOutputSize(r, &rw, &rh);
    for (int i = 0; i < 4; ++i) {
        SDL_Rect b = {rw - 250, 90 + i * 52, 215, 40};
        if (i == camera) SDL_RenderFillRect(r, &b);
        else SDL_RenderDrawRect(r, &b);
    }
    (void)rh;
}

static void draw_title(SDL_Renderer *r, FnaeVisuals *v, int night, int arrow, int progress) {
    /* Frame 2 uses a fixed 1280x720 playfield. */
    if (v->title_bg) {
        SDL_Rect d = {0, 0, 1280, 720};
        SDL_RenderCopy(r, v->title_bg, NULL, &d);
    }

    /* These are the actual Frame 2 object positions from Objects.txt. */
    draw_texture(r, v->title_template, 64, 96);
    draw_texture(r, v->title_new,      96, 448);
    draw_texture(r, v->title_continue, 96, 512);
    draw_texture(r, v->title_6night,   96, 576);
    draw_texture(r, v->title_custom,   96, 640);

    /* Arrow is positioned relative to the selected menu object.
     * Fusion (Frame 2 Events.txt) places it at (-10,+16/17/19/19) from
     * each item's top-left, but Fusion coordinates refer to the arrow's
     * hotspot -- the pointing tip at its right-center -- while SDL draws
     * from the top-left. Shifting by the arrow size keeps the Fusion
     * rhythm and lands the tip beside the text instead of on top of it:
     * x = 96-10-43 = 43, y = item_y + yoff - 13. */
    static const int item_y[4] = {448, 512, 576, 640};
    static const int yoff[4] = {16, 17, 19, 19};
    int a = arrow < 0 ? 0 : arrow > 3 ? 3 : arrow;
    int aw = 43, ah = 26;
    if (v->title_arrow)
        SDL_QueryTexture(v->title_arrow, NULL, NULL, &aw, &ah);
    draw_texture(r, v->title_arrow, 96 - 10 - aw, item_y[a] + yoff[a] - ah / 2);

    /* The three stars unlock with progress, exactly like the Fusion events. */
    if (progress > 0) draw_texture(r, v->title_star, 348, 75);
    if (progress > 1) draw_texture(r, v->title_star, 428, 75);
    if (progress > 2) draw_texture(r, v->title_star, 508, 75);

    /* The Night counter is only shown when Continue is selected. */
    if (a == 1) {
        int n = night < 1 ? 1 : night > 7 ? 7 : night;
        draw_texture(r, v->title_nights[n - 1], 326, 545);
    }
}

void visuals_render(FnaeVisuals *v, SDL_Renderer *r, int frame, int camera,
                    int camera_up, int night, int hour, int power,
                    int left_door, int right_door, int mask, int arrow, int progress) {
    SDL_SetRenderDrawColor(r, 0, 0, 0, 255);
    SDL_RenderClear(r);

    if (frame == 2) {
        draw_title(r, v, night, arrow, progress);
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
                int rw, rh;
                SDL_GetRendererOutputSize(r, &rw, &rh);
                SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
                SDL_SetRenderDrawColor(r, 0, 0, 0, 180);
                SDL_Rect m = {0, 0, rw, rh};
                SDL_RenderFillRect(r, &m);
            }
        }
    } else if (frame == 9) {
        fit_center(r, v->six_am);
    } else if (frame == 4) {
        fit_center(r, v->death);
    } else if (frame == 5) {
        fit_center(r, v->final_screen);
    } else if (frame == 7) {
        fit_center(r, v->newspaper);
    } else {
        fit_center(r, v->title_bg);
    }

    char title[256];
    snprintf(title, sizeof title,
             "Five Nights at Edward's | Native | Night %d | %d AM | Power %d%%",
             night, hour, power / 100);
    SDL_SetWindowTitle(SDL_GetWindowFromID(1), title);
}
