#include "visuals.h"
#include "fnae_assets.h"
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

    /* Real extracted gameplay assets (see src/fnae_assets.h). */
    v->office = load_id(r, IMG_OFFICE);
    v->cams[0] = load_id(r, IMG_CAM_HELL);
    /* Cam 01 = Hell, Cam 02 = Mountain/forest feed, Cam 03 = Forest, Cam 04 = Dinosaur Exhibit. */
    v->cams[1] = load_id(r, IMG_CAM_MOUNTAIN);
    v->cams[2] = load_id(r, IMG_CAM_FOREST);
    v->cams[3] = load_id(r, IMG_CAM_DINO);
    for (int i = 0; i < IMG_STATIC_COUNT; ++i)
        v->static_frames[i] = load_id(r, IMG_STATIC_FIRST + i);
    v->six_am = load_id(r, IMG_SIX_AM);
    v->death = load_id(r, IMG_DEATH);
    v->newspaper = load_id(r, IMG_NEWSPAPER);
    v->final_screen = load_id(r, IMG_GOODJOB);

    /*
     * Frame 2 (Title) assets mapped from the exported object layout
     * (see src/fnae_assets.h and docs/TITLE_ASSET_MAP.md).
     *
     * The three Star objects all use the same source image in Fusion.
     */
    v->title_bg = load_id(r, IMG_TITLE_BG);
    for (int i = 0; i < IMG_TITLE_BG_ANIM_COUNT; ++i)
        v->title_bg_anim[i] = load_id(r, IMG_TITLE_BG_ANIM_FIRST + i);
    v->desk = load_id(r, IMG_DESK_SCENE);
    for (int i = 0; i < IMG_DOOR_FRAMES; ++i) {
        v->door_left[i] = load_id(r, IMG_DOOR_LEFT_FIRST + i);
        v->door_right[i] = load_id(r, IMG_DOOR_RIGHT_FIRST + i);
    }
    v->title_new = load_id(r, IMG_TITLE_NEW);
    v->title_continue = load_id(r, IMG_TITLE_CONTINUE);
    v->title_6night = load_id(r, IMG_TITLE_6NIGHT);
    v->title_custom = load_id(r, IMG_TITLE_CUSTOM);
    v->title_arrow = load_id(r, IMG_TITLE_ARROW);
    v->title_star = load_id(r, IMG_TITLE_STAR);
    /* Template Title is the "Five Nights at Edward's" text card (464,
     * 266x271) at the Template Title position (64,96). The 600x507 devil
     *  cards (233/460, sad/wide eyes) are unassigned animation frames —
     *  owner to confirm which object/sequence they belong to. */
    v->title_template = load_id(r, IMG_TITLE_TEXT);
    for (int i = 0; i < IMG_NIGHT_COUNT; ++i)
        v->title_nights[i] = load_id(r, IMG_NIGHT_FIRST + i);

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
    for (int i = 0; i < 8; ++i)
        destroy_texture(&v->static_frames[i]);
    destroy_texture(&v->six_am);
    destroy_texture(&v->death);
    destroy_texture(&v->title_bg);
    for (int i = 0; i < IMG_TITLE_BG_ANIM_COUNT; ++i)
        destroy_texture(&v->title_bg_anim[i]);
    for (int i = 0; i < IMG_DOOR_FRAMES; ++i) {
        destroy_texture(&v->door_left[i]);
        destroy_texture(&v->door_right[i]);
    }
    destroy_texture(&v->desk);
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

void visuals_draw_anchored(SDL_Renderer *r, SDL_Texture *t,
                           int x, int y, FnaeAnchor anchor) {
    if (!t) return;
    int w = 0, h = 0;
    SDL_QueryTexture(t, NULL, NULL, &w, &h);
    switch (anchor) {
    case FNAE_ANCHOR_CENTER: x -= w / 2; y -= h / 2; break;
    case FNAE_ANCHOR_RIGHT_CENTER: x -= w; y -= h / 2; break;
    case FNAE_ANCHOR_TOP_LEFT: default: break;
    }
    draw_texture(r, t, x, y);
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

/* Office pan: the 1600px-wide office scene is drawn cover-scaled and
 * cropped to the 1280px-wide view, offset by scroll source px
 * (0 = leftmost). Fusion centers the display on the Office Center Object;
 * scroll is that X minus half the view width. */
static void draw_office_pan(SDL_Renderer *r, SDL_Texture *t, int scroll) {
    if (!t) return;
    int rw, rh, tw, th;
    SDL_GetRendererOutputSize(r, &rw, &rh);
    SDL_QueryTexture(t, NULL, NULL, &tw, &th);

    float sx = (float)rw / (float)tw;
    float sy = (float)rh / (float)th;
    float s = sx > sy ? sx : sy;

    int max_scroll = tw - (int)((float)rw / s);
    if (max_scroll < 0) max_scroll = 0;
    if (scroll < 0) scroll = 0;
    if (scroll > max_scroll) scroll = max_scroll;

    int w = (int)(tw * s);
    int h = (int)(th * s);
    SDL_Rect d = {-(int)(scroll * s), (rh - h) / 2, w, h};
    SDL_RenderCopy(r, t, NULL, &d);
}

/* World-layer object: Fusion frame position minus the pan scroll
 * (the display is centered on the Office Center Object). */
static void draw_world(SDL_Renderer *r, SDL_Texture *t, int fx, int fy, int scroll) {
    if (!t) return;
    int w, h;
    SDL_QueryTexture(t, NULL, NULL, &w, &h);
    SDL_Rect d = {fx - scroll, fy, w, h};
    SDL_RenderCopy(r, t, NULL, &d);
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

static void draw_title(SDL_Renderer *r, FnaeVisuals *v, int night, int arrow, int progress,
                       int static_frame, int static_alpha, int title_bg_frame) {
    /* Frame 2 uses a fixed 1280x720 playfield. The background is usually
     * the Stopped frame (515); Random(50)=1 briefly flashes one
     * RRandom(12,14) sequence (516/517/518) for the 0.2 s cut window. */
    SDL_Texture *bg = v->title_bg;
    if (title_bg_frame >= 1 && title_bg_frame <= IMG_TITLE_BG_ANIM_COUNT)
        bg = v->title_bg_anim[title_bg_frame - 1];
    if (bg) {
        SDL_Rect d = {0, 0, 1280, 720};
        SDL_RenderCopy(r, bg, NULL, &d);
    }

    /* These are the actual Frame 2 object positions from Objects.txt.
     * Layer order matters: Static lives in the UI layer beneath the
     * template and menu, so it is drawn before them. */
    draw_static(r, v->static_frames[static_frame & 7], static_alpha);
    draw_texture(r, v->title_template, 64, 96);
    draw_texture(r, v->title_new,      96, 448);
    draw_texture(r, v->title_continue, 96, 512);
    draw_texture(r, v->title_6night,   96, 576);
    draw_texture(r, v->title_custom,   96, 640);

    /* Arrow is positioned relative to the selected menu object.
     * Coordinates are verbatim Fusion hotspot positions from Frame 2
     * Events.txt: each menu item's top-left plus (-10,+16/17/19/19).
     * The arrow's hotspot is its pointing tip (right-center), so draw it
     * with FNAE_ANCHOR_RIGHT_CENTER (see docs/COORDINATES.md) instead of
     * baking the size offset into the numbers. */
    static const int arrow_x[4] = {86, 86, 86, 86};
    static const int arrow_y[4] = {464, 529, 595, 659};
    int a = arrow < 0 ? 0 : arrow > 3 ? 3 : arrow;
    visuals_draw_anchored(r, v->title_arrow, arrow_x[a], arrow_y[a],
                          FNAE_ANCHOR_RIGHT_CENTER);

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
                    int left_door, int right_door, int mask, int arrow, int progress,
                    int static_frame, int static_alpha, int office_scroll,
                    int left_door_frame, int right_door_frame, int title_bg_frame) {
    SDL_SetRenderDrawColor(r, 0, 0, 0, 255);
    SDL_RenderClear(r);

    if (frame == 2) {
        draw_title(r, v, night, arrow, progress, static_frame, static_alpha,
                   title_bg_frame);
    } else if (frame == 3) {
        if (camera_up) {
            if (camera < 0 || camera > 3) camera = 0;
            fit_center(r, v->cams[camera]);
            draw_static(r, v->static_frames[static_frame & 7], 35);
            draw_camera_labels(r, camera);
        } else {
            draw_office_pan(r, v->office, office_scroll);
            /* Layer order mirrors Fusion: office (#1), doors (#2), desk (#3).
             * Doors/desk are world objects at verbatim Objects.txt positions,
             * shifted by the pan scroll. */
            if (left_door_frame < 0) left_door_frame = 0;
            if (left_door_frame >= IMG_DOOR_FRAMES) left_door_frame = IMG_DOOR_FRAMES - 1;
            if (right_door_frame < 0) right_door_frame = 0;
            if (right_door_frame >= IMG_DOOR_FRAMES) right_door_frame = IMG_DOOR_FRAMES - 1;
            draw_world(r, v->door_left[left_door_frame], 119, 0, office_scroll);
            draw_world(r, v->door_right[right_door_frame], 1263, 0, office_scroll);
            draw_world(r, v->desk, 266, 177, office_scroll);
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
