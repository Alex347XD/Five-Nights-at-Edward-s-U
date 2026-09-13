#include "visuals.h"
#include "fnae_assets.h"
#include <stdio.h>
#include <stdlib.h>
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
    /* Cam 01 = Hell, Cam 02 = Mountain, Cam 03 = Forest,
     * Cam 04 = Dinosaur Exhibit. Each camera has an empty base frame
     * plus an occupied frame for its haunting animatronic. */
    v->cams[0][0] = load_id(r, IMG_CAM_HELL);
    v->cams[0][1] = load_id(r, IMG_CAM_HELL_FRED);
    v->cams[1][0] = load_id(r, IMG_CAM_MOUNTAIN);
    v->cams[1][1] = load_id(r, IMG_CAM_MOUNTAIN_FOXY);
    v->cams[2][0] = load_id(r, IMG_CAM_FOREST);
    v->cams[2][1] = load_id(r, IMG_CAM_FOREST_FRED);
    v->cams[3][0] = load_id(r, IMG_CAM_DINO);
    v->cams[3][1] = load_id(r, IMG_CAM_DINO_FOXY);
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
        for (int j = 0; j < 2; ++j)
            destroy_texture(&v->cams[i][j]);
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

static void fit_center_off(SDL_Renderer *r, SDL_Texture *t, int ox, int oy) {
    if (!t) return;
    int rw, rh, tw, th;
    SDL_GetRendererOutputSize(r, &rw, &rh);
    SDL_QueryTexture(t, NULL, NULL, &tw, &th);

    float sx = (float)rw / (float)tw;
    float sy = (float)rh / (float)th;
    float s = sx < sy ? sx : sy;

    int w = (int)(tw * s);
    int h = (int)(th * s);
    SDL_Rect d = {(rw - w) / 2 + ox, (rh - h) / 2 + oy, w, h};
    SDL_RenderCopy(r, t, NULL, &d);
}

static void fit_center(SDL_Renderer *r, SDL_Texture *t) {
    fit_center_off(r, t, 0, 0);
}

static void draw_static(SDL_Renderer *r, SDL_Texture *t, int alpha) {
    if (!t || alpha <= 0) return;
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
static void draw_office_pan(SDL_Renderer *r, SDL_Texture *t, int scroll, int ox, int oy) {
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
    SDL_Rect d = {-(int)(scroll * s) + ox, (rh - h) / 2 + oy, w, h};
    SDL_RenderCopy(r, t, NULL, &d);
}

/* World-layer object: Fusion frame position minus the pan scroll
 * (the display is centered on the Office Center Object). */
static void draw_world(SDL_Renderer *r, SDL_Texture *t, int fx, int fy, int scroll, int ox, int oy) {
    if (!t) return;
    int w, h;
    SDL_QueryTexture(t, NULL, NULL, &w, &h);
    SDL_Rect d = {fx - scroll + ox, fy + oy, w, h};
    SDL_RenderCopy(r, t, NULL, &d);
}

/* Frame 6 interstitial: black screen with the night card centered
 * (Fusion parks Which Night at (640,360)). Reuses the 246-252 night
 * cards, which already read "12:00 AM / Nth Night". */
static void draw_which_night(SDL_Renderer *r, FnaeVisuals *v, int night) {
    int n = night < 1 ? 1 : night > 7 ? 7 : night;
    int rw, rh;
    SDL_GetRendererOutputSize(r, &rw, &rh);
    visuals_draw_anchored(r, v->title_nights[n - 1],
                          rw / 2, rh / 2, FNAE_ANCHOR_CENTER);
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
                    int left_door_frame, int right_door_frame, int title_bg_frame,
                    int foxy_pos, int freddy_pos, int cam_static_alpha,
                    int death, int music) {
    SDL_SetRenderDrawColor(r, 0, 0, 0, 255);
    SDL_RenderClear(r);

    /* Jumpscare shake: the night scene jumps X/Y +/-Random(5) on any death
     * (GF shakes +/-Random(8)). Applied to the office/cam draws below. */
    int ox = 0, oy = 0;
    if (frame == 3 && death > 0) {
        int j = (death == 5) ? 8 : 5;
        ox = rand() % (2 * j + 1) - j;
        oy = rand() % (2 * j + 1) - j;
    }

    if (frame == 2) {
        draw_title(r, v, night, arrow, progress, static_frame, static_alpha,
                   title_bg_frame);
    } else if (frame == 3) {
        if (camera_up) {
            /* Core camera numbers are 1-based (1..4); the image bank is 0-based. */
            int idx = camera - 1;
            if (idx < 0 || idx > 3) idx = 0;
            /* Occupied frame follows the haunting animatronic's route:
             * Freddy Cam 01 (pos 1) -> Cam 03 (pos 3);
             * Foxy Cam 02 (pos 2) -> Cam 04 (pos 4). */
            int occupied = 0;
            if (idx == 0 && freddy_pos == 1) occupied = 1;
            else if (idx == 1 && foxy_pos == 2) occupied = 1;
            else if (idx == 2 && freddy_pos == 3) occupied = 1;
            else if (idx == 3 && foxy_pos == 4) occupied = 1;
            fit_center_off(r, v->cams[idx][occupied], ox, oy);
            draw_static(r, v->static_frames[static_frame & 7], cam_static_alpha);
        } else {
            draw_office_pan(r, v->office, office_scroll, ox, oy);
            /* Layer order mirrors Fusion: office (#1), doors (#2), desk (#3).
             * Doors/desk are world objects at verbatim Objects.txt positions,
             * shifted by the pan scroll. */
            if (left_door_frame < 0) left_door_frame = 0;
            if (left_door_frame >= IMG_DOOR_FRAMES) left_door_frame = IMG_DOOR_FRAMES - 1;
            if (right_door_frame < 0) right_door_frame = 0;
            if (right_door_frame >= IMG_DOOR_FRAMES) right_door_frame = IMG_DOOR_FRAMES - 1;
            draw_world(r, v->door_left[left_door_frame], 119, 0, office_scroll, ox, oy);
            draw_world(r, v->door_right[right_door_frame], 1263, 0, office_scroll, ox, oy);
            draw_world(r, v->desk, 266, 177, office_scroll, ox, oy);
        }
    } else if (frame == 6) {
        draw_which_night(r, v, night);
    } else if (frame == 9) {
        fit_center(r, v->six_am);
    } else if (frame == 4) {
        fit_center(r, v->death);
    } else if (frame == 5) {
        fit_center(r, v->final_screen);
    } else if (frame == 7) {
        fit_center(r, v->newspaper);
    }
    /* Frames 1 (Warning) and 8 (Customize) stay black: their UI art is
     * still unmapped, and title_bg was the wrong image there. */

    static const char *cam_names[4] = {"Hell", "Mountain", "Forest", "Dino"};
    int cam = camera - 1;
    if (cam < 0 || cam > 3) cam = 0;
    char title[256];
    snprintf(title, sizeof title,
             "Five Nights at Edward's | Native | Night %d | %d AM | Power %d%% | Cam %s%s | doors L%d R%d%s | music %d",
             night, hour, power / 100, cam_names[cam], camera_up ? " (up)" : "",
             left_door > 0, right_door > 0, mask ? " | MASK" : "",
             music);
    SDL_SetWindowTitle(SDL_GetWindowFromID(1), title);
}
