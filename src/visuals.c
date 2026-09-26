#include "visuals.h"
#include "fnae_assets.h"
#include "fnae_core.h"
#include "wiiu.h"
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
    /* fnae_asset_root() already ends at the assets dir (or "assets/" on
     * desktop), so no "assets/" infix here. */
    snprintf(dst, n, "%simages/%d.png", fnae_asset_root(), id);
}

static SDL_Texture *load_id(SDL_Renderer *r, int id) {
    char p[256];
    path_for(p, sizeof p, id);
    return load_png(r, p);
}

/* Report staged-init progress (see visuals.h); abort on hook request. */
#define FNAE_LOAD_PCT(p) do { \
    if (progress && progress((p), pctx)) return -1; \
} while (0)

int visuals_init(FnaeVisuals *v, SDL_Renderer *r, FnaeLoadProgress progress, void *pctx, int subset) {
    memset(v, 0, sizeof *v);

    /* Real extracted gameplay assets (see src/fnae_assets.h). The office
     * view never shows on the GamePad (cams closed = black there). */
    if (!subset)
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
    FNAE_LOAD_PCT(16);
    v->connection_lost = load_id(r, IMG_CONNECTION_LOST);
    for (int i = 0; i < IMG_CAMFLIP_COUNT; ++i)
        v->cam_flip[i] = load_id(r, IMG_CAMFLIP_FIRST + i);
    FNAE_LOAD_PCT(20);
    /* Frame 9 "which AM" odometer (see fnae_assets.h): Stopped "5"
     * first, then the roll up to "6" in bank order, plus the "AM" card.
     * TV-only (Frame 9 never shows on the GamePad). */
    if (!subset) {
        static const int ids[IMG_WHICH_AM_COUNT] = {389, 403, 406, 423,
            424, 425, 426, 428, 429, 430, 431, 432, 433, 434, 435, 436,
            437, 438, 439, 440, 441, 442, 462, 487, 489, 491, 492};
        for (int i = 0; i < IMG_WHICH_AM_COUNT; ++i)
            v->which_am[i] = load_id(r, ids[i]);
        v->am_text = load_id(r, IMG_AM_TEXT);
    }
    FNAE_LOAD_PCT(28);
    /* TV-only results screens (6AM odometer, night-end cards). The death
     * card stays: a death with cameras up still overlays it on the DRC. */
    if (!subset) {
        v->final_n6 = load_id(r, IMG_FINAL_N6);
        v->final_n7 = load_id(r, IMG_FINAL_N7);
    }
    v->death = load_id(r, IMG_DEATH);
    if (!subset) {
        v->newspaper = load_id(r, IMG_NEWSPAPER);
        v->final_screen = load_id(r, IMG_GOODJOB);
    }
    v->warning = load_id(r, IMG_WARNING);
    FNAE_LOAD_PCT(32);

    /*
     * Frame 2 (Title) assets mapped from the exported object layout
     * (see src/fnae_assets.h and docs/TITLE_ASSET_MAP.md).
     *
     * The three Star objects all use the same source image in Fusion.
     */
    /* Title scene, desk, doors and doorway figures: the office view
     * never shows on the GamePad. */
    if (!subset) {
        v->title_bg = load_id(r, IMG_TITLE_BG);
        for (int i = 0; i < IMG_TITLE_BG_ANIM_COUNT; ++i)
            v->title_bg_anim[i] = load_id(r, IMG_TITLE_BG_ANIM_FIRST + i);
        v->desk = load_id(r, IMG_DESK_SCENE);
        for (int i = 0; i < IMG_DOOR_FRAMES; ++i) {
            v->door_left[i] = load_id(r, IMG_DOOR_LEFT_FIRST + i);
            v->door_right[i] = load_id(r, IMG_DOOR_RIGHT_FIRST + i);
        }
    }
    FNAE_LOAD_PCT(44);
    /* Door buttons (off = Stopped, on = Animation 12) + doorway figures. */
    if (!subset) {
        v->door_btn[0] = load_id(r, IMG_DOORBTN_OFF);
        v->door_btn[1] = load_id(r, IMG_DOORBTN_ON);
        v->freddy_door = load_id(r, IMG_FREDDY_DOOR);
        v->foxy_stand = load_id(r, IMG_FOXY_STAND);
    }
    /* Camera minimap + buttons (see src/fnae_assets.h for layout). */
    v->minimap = load_id(r, IMG_MINIMAP);
    v->cam_btn_off = load_id(r, IMG_CAMBTN_OFF);
    v->cam_btn_on = load_id(r, IMG_CAMBTN_ON);
    for (int i = 0; i < IMG_CAMBTN_COUNT; ++i)
        v->cam_txt[i] = load_id(r, IMG_CAMTXT_FIRST + i);
    FNAE_LOAD_PCT(50);
    v->lure_button = load_id(r, IMG_LURE_BUTTON);
    v->lure_cd[0] = load_id(r, IMG_LURE_CD_1);
    v->lure_cd[1] = load_id(r, IMG_LURE_CD_2);
    v->lure_cd[2] = load_id(r, IMG_LURE_CD_3);
    v->lure_cd[3] = load_id(r, IMG_LURE_CD_4);
    v->lure_area = load_id(r, IMG_LURE_AREA);
    v->springtrap_stand = load_id(r, IMG_SPRINGTRAP_STAND);
    v->musicbtn_off = load_id(r, IMG_MUSICBTN_OFF);
    v->musicbtn_on = load_id(r, IMG_MUSICBTN_ON);
    v->music_wind = load_id(r, IMG_MUSIC_WIND_TEXT);
    v->music_hold = load_id(r, IMG_MUSIC_CLICKHOLD);
    for (int i = 0; i < IMG_MUSIC_PIE_COUNT; ++i)
        v->music_pie[i] = load_id(r, IMG_MUSIC_PIE_FIRST + i);
    FNAE_LOAD_PCT(56);
    v->warn_out_steady = load_id(r, IMG_WARN_OUT_STEADY);
    v->warn_out_flash = load_id(r, IMG_WARN_OUT_FLASH);
    v->warn_out_blank = load_id(r, IMG_WARN_OUT_BLANK);
    v->warn_in_steady = load_id(r, IMG_WARN_IN_STEADY);
    v->warn_in_flash = load_id(r, IMG_WARN_IN_FLASH);
    v->warn_in_blank = load_id(r, IMG_WARN_IN_BLANK);
    v->mutecall = load_id(r, IMG_MUTECALL);
    FNAE_LOAD_PCT(60);
    /* Mask overlay (see fnae_assets.h): put-on 134-140, worn 129,
     * take-off 141-143. All frames use per-pixel alpha (transparent
     * eye holes / fade edges), so force BLEND like the lure-area
     * marker: without it the holes render as stored black on
     * renderers that honor BLENDMODE_NONE strictly. Office-only. */
    if (!subset) {
        for (int i = 0; i < IMG_MASK_FLIPDN_COUNT; ++i)
            v->mask_anim[i] = load_id(r, IMG_MASK_FLIPDN_FIRST + i);
        v->mask_anim[IMG_MASK_FLIPDN_COUNT] = load_id(r, IMG_MASK_WORN);
        for (int i = 0; i < IMG_MASK_FLIPUP_COUNT; ++i)
            v->mask_anim[IMG_MASK_FLIPDN_COUNT + 1 + i] = load_id(r, IMG_MASK_FLIPUP_FIRST + i);
        for (int i = 0; i < IMG_MASK_FRAMES; ++i)
            if (v->mask_anim[i]) SDL_SetTextureBlendMode(v->mask_anim[i], SDL_BLENDMODE_BLEND);
    }
    FNAE_LOAD_PCT(66);
    v->phmangle_cam = load_id(r, IMG_PHMANGLE_CAM);
    v->phmangle_annoy = load_id(r, IMG_PHMANGLE_ANNOY);
    v->phbb_cam = load_id(r, IMG_PHBB_CAM);
    v->phbb_scare = load_id(r, IMG_PHBB_SCARE);
    FNAE_LOAD_PCT(68);
    /* Jumpscare runs (see fnae_assets.h for the verified bank ranges).
     * Freddy skips the 35x75 UI dot at 364: 353-363 + 365. Foxy is two
     * runs back to back: 536-539 then 562-572. Full-screen scares below
     * are TV-only (a death with cameras up overlays the death card, kept
     * above, not these runs). */
    if (!subset) {
        for (int i = 0; i < IMG_SCARE_SPRING_COUNT; ++i)
            v->scare_spring[i] = load_id(r, IMG_SCARE_SPRING_FIRST + i);
        for (int i = 0; i < 11; ++i)
            v->scare_freddy[i] = load_id(r, IMG_SCARE_FREDDY_FIRST + i);
        v->scare_freddy[11] = load_id(r, IMG_SCARE_FREDDY_LAST);
        for (int i = 0; i < IMG_SCARE_PUPPET_COUNT; ++i)
            v->scare_puppet[i] = load_id(r, IMG_SCARE_PUPPET_FIRST + i);
        for (int i = 0; i < IMG_SCARE_FOXY_A_COUNT; ++i)
            v->scare_foxy[i] = load_id(r, IMG_SCARE_FOXY_A_FIRST + i);
        for (int i = 0; i < IMG_SCARE_FOXY_B_COUNT; ++i)
            v->scare_foxy[IMG_SCARE_FOXY_A_COUNT + i] = load_id(r, IMG_SCARE_FOXY_B_FIRST + i);
    }
    FNAE_LOAD_PCT(84);
    /* Title menu, night cards and customize UI: TV-only. */
    if (!subset) {
        v->scare_gf = load_id(r, IMG_SCARE_GF);
        v->gf_sit = load_id(r, IMG_GF_SIT);
        v->death_devil[0] = load_id(r, IMG_DEATH_DEVIL_A);
        v->death_devil[1] = load_id(r, IMG_DEATH_DEVIL_B);
        v->death_rip = load_id(r, IMG_RIP_TEXT);
        v->title_new = load_id(r, IMG_TITLE_NEW);
        v->title_continue = load_id(r, IMG_TITLE_CONTINUE);
        v->title_6night = load_id(r, IMG_TITLE_6NIGHT);
        v->title_custom = load_id(r, IMG_TITLE_CUSTOM);
        v->title_arrow = load_id(r, IMG_TITLE_ARROW);
        v->title_star = load_id(r, IMG_TITLE_STAR);
        /* Template Title is the "Five Nights at Edward's" text card (464,
         * 266x271) at the Template Title position (64,96). The 600x507 devil
         *  cards (233/460) are the Frame 4 Death Anim backdrop cycle. */
        v->title_template = load_id(r, IMG_TITLE_TEXT);
        for (int i = 0; i < IMG_NIGHT_COUNT; ++i)
            v->title_nights[i] = load_id(r, IMG_NIGHT_FIRST + i);
    }
    FNAE_LOAD_PCT(93);
    /* Frame 8 Customize screen (see src/fnae_assets.h for the mapping). */
    if (!subset) {
        v->cust_bg[0] = load_id(r, IMG_CUST_BG_FIRST);
        v->cust_bg[1] = load_id(r, IMG_CUST_BG_2);
        v->cust_bg[2] = load_id(r, IMG_CUST_BG_3);
        {
            static const int ids[7] = {IMG_CUST_FREDDY, IMG_CUST_MANGLE,
                IMG_CUST_FOXY, IMG_CUST_GOLDEN, IMG_CUST_SPRING,
                IMG_CUST_BB, IMG_CUST_PUPPET};
            for (int i = 0; i < 7; ++i)
                v->cust_portrait[i] = load_id(r, ids[i]);
        }
        v->cust_select = load_id(r, IMG_CUST_SELECT);
        v->cust_arrow = load_id(r, IMG_CUST_ARROW);
        v->cust_go = load_id(r, IMG_CUST_GO);
        v->cust_set20 = load_id(r, IMG_CUST_SET20);
        v->cust_add1 = load_id(r, IMG_CUST_ADD1);
        v->cust_check = load_id(r, IMG_CUST_CHECK);
    }

    FNAE_LOAD_PCT(100);
    /* Full set validates on the title card; the subset (no title_bg by
     * design) validates on the first camera feed instead. */
    if (subset)
        return v->cams[0][0] ? 0 : -1;
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
    destroy_texture(&v->connection_lost);
    for (int i = 0; i < IMG_CAMFLIP_COUNT; ++i)
        destroy_texture(&v->cam_flip[i]);
    for (int i = 0; i < IMG_WHICH_AM_COUNT; ++i)
        destroy_texture(&v->which_am[i]);
    destroy_texture(&v->am_text);
    destroy_texture(&v->final_n6);
    destroy_texture(&v->final_n7);
    destroy_texture(&v->death);
    destroy_texture(&v->death_rip);
    destroy_texture(&v->title_bg);
    for (int i = 0; i < IMG_TITLE_BG_ANIM_COUNT; ++i)
        destroy_texture(&v->title_bg_anim[i]);
    for (int i = 0; i < IMG_DOOR_FRAMES; ++i) {
        destroy_texture(&v->door_left[i]);
        destroy_texture(&v->door_right[i]);
    }
    destroy_texture(&v->door_btn[0]);
    destroy_texture(&v->door_btn[1]);
    destroy_texture(&v->freddy_door);
    destroy_texture(&v->foxy_stand);
    destroy_texture(&v->desk);
    destroy_texture(&v->minimap);
    destroy_texture(&v->cam_btn_off);
    destroy_texture(&v->cam_btn_on);
    for (int i = 0; i < IMG_CAMBTN_COUNT; ++i)
        destroy_texture(&v->cam_txt[i]);
    destroy_texture(&v->lure_button);
    for (int i = 0; i < 4; ++i)
        destroy_texture(&v->lure_cd[i]);
    destroy_texture(&v->lure_area);
    destroy_texture(&v->springtrap_stand);
    destroy_texture(&v->musicbtn_off);
    destroy_texture(&v->musicbtn_on);
    destroy_texture(&v->music_wind);
    destroy_texture(&v->music_hold);
    for (int i = 0; i < IMG_MUSIC_PIE_COUNT; ++i)
        destroy_texture(&v->music_pie[i]);
    destroy_texture(&v->warn_out_steady);
    destroy_texture(&v->warn_out_flash);
    destroy_texture(&v->warn_out_blank);
    destroy_texture(&v->warn_in_steady);
    destroy_texture(&v->warn_in_flash);
    destroy_texture(&v->warn_in_blank);
    destroy_texture(&v->mutecall);
    for (int i = 0; i < IMG_MASK_FRAMES; ++i)
        destroy_texture(&v->mask_anim[i]);
    destroy_texture(&v->phmangle_cam);
    destroy_texture(&v->phmangle_annoy);
    destroy_texture(&v->phbb_cam);
    destroy_texture(&v->phbb_scare);
    for (int i = 0; i < IMG_SCARE_SPRING_COUNT; ++i)
        destroy_texture(&v->scare_spring[i]);
    for (int i = 0; i < 12; ++i)
        destroy_texture(&v->scare_freddy[i]);
    for (int i = 0; i < IMG_SCARE_PUPPET_COUNT; ++i)
        destroy_texture(&v->scare_puppet[i]);
    for (int i = 0; i < IMG_SCARE_FOXY_A_COUNT + IMG_SCARE_FOXY_B_COUNT; ++i)
        destroy_texture(&v->scare_foxy[i]);
    destroy_texture(&v->scare_gf);
    destroy_texture(&v->gf_sit);
    destroy_texture(&v->death_devil[0]);
    destroy_texture(&v->death_devil[1]);
    destroy_texture(&v->title_new);
    destroy_texture(&v->title_continue);
    destroy_texture(&v->title_6night);
    destroy_texture(&v->title_custom);
    destroy_texture(&v->title_arrow);
    destroy_texture(&v->title_star);
    destroy_texture(&v->title_template);
    for (int i = 0; i < 7; ++i)
        destroy_texture(&v->title_nights[i]);
    for (int i = 0; i < 3; ++i)
        destroy_texture(&v->cust_bg[i]);
    for (int i = 0; i < 7; ++i)
        destroy_texture(&v->cust_portrait[i]);
    destroy_texture(&v->cust_select);
    destroy_texture(&v->cust_arrow);
    destroy_texture(&v->cust_go);
    destroy_texture(&v->cust_set20);
    destroy_texture(&v->cust_add1);
    destroy_texture(&v->cust_check);
    destroy_texture(&v->newspaper);
    destroy_texture(&v->final_screen);
    destroy_texture(&v->warning);
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

void visuals_draw_cam_flip(FnaeVisuals *v, SDL_Renderer *r, int frame) {
    if (frame < 0 || frame >= IMG_CAMFLIP_COUNT) return;
    if (v->cam_flip[frame]) fit_center(r, v->cam_flip[frame]);
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

/* Camera-feed pan: the 1600px-wide feed is drawn cover-scaled and
 * cropped to the 1280px-wide view, offset by scroll source px
 * (the display left edge). The scroll is clamped to the image ends
 * so no black bars show past the feed edges. */
static void draw_cam_pan(SDL_Renderer *r, SDL_Texture *t, int scroll, int ox, int oy) {
    if (!t) return;
    int rw, rh, tw, th;
    SDL_GetRendererOutputSize(r, &rw, &rh);
    SDL_QueryTexture(t, NULL, NULL, &tw, &th);

    float sx = (float)rw / (float)tw;
    float sy = (float)rh / (float)th;
    float s = sx > sy ? sx : sy;

    if (scroll < FNAE_CAM_SCROLL_MIN) scroll = FNAE_CAM_SCROLL_MIN;
    if (scroll > FNAE_CAM_SCROLL_MAX) scroll = FNAE_CAM_SCROLL_MAX;

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

/* Scaled world-layer object with an explicit Fusion-hotspot anchor
 * (doorway figures render at 1.1 scale, center-anchored). */
static void draw_world_scaled(SDL_Renderer *r, SDL_Texture *t, int fx, int fy,
                              int scroll, int ox, int oy, float scale, FnaeAnchor anchor) {
    if (!t) return;
    int w, h;
    SDL_QueryTexture(t, NULL, NULL, &w, &h);
    int sw = (int)(w * scale);
    int sh = (int)(h * scale);
    int x = fx - scroll + ox, y = fy + oy;
    switch (anchor) {
    case FNAE_ANCHOR_CENTER: x -= sw / 2; y -= sh / 2; break;
    case FNAE_ANCHOR_RIGHT_CENTER: x -= sw; y -= sh / 2; break;
    case FNAE_ANCHOR_TOP_LEFT: default: break;
    }
    SDL_Rect d = {x, y, sw, sh};
    SDL_RenderCopy(r, t, NULL, &d);
}

/* Bitmap-font text (defined below; counters/strings are rendered text
 * in Fusion, not PNG frames). */
static void draw_text(SDL_Renderer *r, const char *s, int x, int y, int scale);

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
    /* 6 Night reappears iff Star is visible (progress > 0), Custom iff
     * Star 2 is visible (progress > 1) — Frame 2 Events.txt. Matches the
     * input gating in fnae_key/fnae_click, so a locked item is neither
     * shown nor reachable. */
    if (progress > 0) draw_texture(r, v->title_6night,   96, 576);
    if (progress > 1) draw_texture(r, v->title_custom,   96, 640);

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

    /* The Night next to Continue is a Counter, not a night card: just
     * the saved night number (the 246-252 "12:00 AM / Nth Night" cards
     * belong to the Which Night screen — drawing one here sprawls a
     * "12:00 AM" header over the menu). Bitmap font like every other
     * Fusion counter/string. X keeps the verbatim [326] origin; Y centers
     * the digit on the Continue item (512 + (34-28)/2 = 515) instead of
     * the verbatim 545, which sat a row low — the counter's hotspot/font
     * metrics are unrecoverable from the dump, so this follows the same
     * owner-request pattern as the CAM 01 button offset. */
    if (a == 1) {
        int n = night < 1 ? 1 : night > 7 ? 7 : night;
        char num[4];
        snprintf(num, sizeof num, "%d", n);
        draw_text(r, num, 326, 515, 4);
    }
}

/* Tiny 5x7 bitmap font for the Fusion counter/string HUD (time, night,
 * power %, usage, cam room names). Counters in Fusion are rendered text,
 * not PNG frames, and this port links only SDL2 + SDL2_image (no TTF),
 * so glyphs are drawn as filled rects. Only the chars the HUD needs. */
static const unsigned char font5x7_digits[10][7] = {
    {0x0E,0x11,0x13,0x15,0x19,0x11,0x0E}, /* 0 */
    {0x04,0x0C,0x04,0x04,0x04,0x04,0x0E}, /* 1 */
    {0x0E,0x11,0x01,0x06,0x08,0x10,0x1F}, /* 2 */
    {0x1F,0x02,0x04,0x02,0x01,0x11,0x0E}, /* 3 */
    {0x02,0x06,0x0A,0x12,0x1F,0x02,0x02}, /* 4 */
    {0x1F,0x10,0x1E,0x01,0x01,0x11,0x0E}, /* 5 */
    {0x06,0x08,0x10,0x1E,0x11,0x11,0x0E}, /* 6 */
    {0x1F,0x01,0x02,0x04,0x08,0x08,0x08}, /* 7 */
    {0x0E,0x11,0x11,0x0E,0x11,0x11,0x0E}, /* 8 */
    {0x0E,0x11,0x11,0x0F,0x01,0x02,0x0C}, /* 9 */
};
static void font_rows(char c, unsigned char out[7]) {
    if (c >= '0' && c <= '9') {
        for (int i = 0; i < 7; ++i) out[i] = font5x7_digits[c - '0'][i];
        return;
    }
    switch (c) {
    case 'A': { static const unsigned char g[7]={0x0E,0x11,0x11,0x1F,0x11,0x11,0x11}; for(int i=0;i<7;++i)out[i]=g[i]; break; }
    case 'B': { static const unsigned char g[7]={0x1E,0x11,0x11,0x1E,0x11,0x11,0x1E}; for(int i=0;i<7;++i)out[i]=g[i]; break; }
    case 'C': { static const unsigned char g[7]={0x0E,0x11,0x10,0x10,0x10,0x11,0x0E}; for(int i=0;i<7;++i)out[i]=g[i]; break; }
    case 'D': { static const unsigned char g[7]={0x1E,0x11,0x11,0x11,0x11,0x11,0x1E}; for(int i=0;i<7;++i)out[i]=g[i]; break; }
    case 'E': { static const unsigned char g[7]={0x1F,0x10,0x10,0x1E,0x10,0x10,0x1F}; for(int i=0;i<7;++i)out[i]=g[i]; break; }
    case 'F': { static const unsigned char g[7]={0x1F,0x10,0x10,0x1E,0x10,0x10,0x10}; for(int i=0;i<7;++i)out[i]=g[i]; break; }
    case 'G': { static const unsigned char g[7]={0x0E,0x11,0x10,0x1B,0x11,0x11,0x0F}; for(int i=0;i<7;++i)out[i]=g[i]; break; }
    case 'H': { static const unsigned char g[7]={0x11,0x11,0x11,0x1F,0x11,0x11,0x11}; for(int i=0;i<7;++i)out[i]=g[i]; break; }
    case 'I': { static const unsigned char g[7]={0x0E,0x04,0x04,0x04,0x04,0x04,0x0E}; for(int i=0;i<7;++i)out[i]=g[i]; break; }
    case 'J': { static const unsigned char g[7]={0x07,0x02,0x02,0x02,0x02,0x12,0x0C}; for(int i=0;i<7;++i)out[i]=g[i]; break; }
    case 'K': { static const unsigned char g[7]={0x11,0x12,0x14,0x18,0x14,0x12,0x11}; for(int i=0;i<7;++i)out[i]=g[i]; break; }
    case 'L': { static const unsigned char g[7]={0x10,0x10,0x10,0x10,0x10,0x10,0x1F}; for(int i=0;i<7;++i)out[i]=g[i]; break; }
    case 'M': { static const unsigned char g[7]={0x11,0x1B,0x15,0x11,0x11,0x11,0x11}; for(int i=0;i<7;++i)out[i]=g[i]; break; }
    case 'N': { static const unsigned char g[7]={0x11,0x19,0x19,0x15,0x13,0x13,0x11}; for(int i=0;i<7;++i)out[i]=g[i]; break; }
    case 'O': { static const unsigned char g[7]={0x0E,0x11,0x11,0x11,0x11,0x11,0x0E}; for(int i=0;i<7;++i)out[i]=g[i]; break; }
    case 'P': { static const unsigned char g[7]={0x1E,0x11,0x11,0x1E,0x10,0x10,0x10}; for(int i=0;i<7;++i)out[i]=g[i]; break; }
    case 'Q': { static const unsigned char g[7]={0x0E,0x11,0x11,0x11,0x15,0x12,0x0D}; for(int i=0;i<7;++i)out[i]=g[i]; break; }
    case 'R': { static const unsigned char g[7]={0x1E,0x11,0x11,0x1E,0x14,0x12,0x11}; for(int i=0;i<7;++i)out[i]=g[i]; break; }
    case 'S': { static const unsigned char g[7]={0x0F,0x10,0x10,0x0E,0x01,0x01,0x1E}; for(int i=0;i<7;++i)out[i]=g[i]; break; }
    case 'T': { static const unsigned char g[7]={0x1F,0x04,0x04,0x04,0x04,0x04,0x04}; for(int i=0;i<7;++i)out[i]=g[i]; break; }
    case 'U': { static const unsigned char g[7]={0x11,0x11,0x11,0x11,0x11,0x11,0x0E}; for(int i=0;i<7;++i)out[i]=g[i]; break; }
    case 'V': { static const unsigned char g[7]={0x11,0x11,0x11,0x11,0x11,0x0A,0x04}; for(int i=0;i<7;++i)out[i]=g[i]; break; }
    case 'W': { static const unsigned char g[7]={0x11,0x11,0x11,0x15,0x15,0x1B,0x11}; for(int i=0;i<7;++i)out[i]=g[i]; break; }
    case 'X': { static const unsigned char g[7]={0x11,0x11,0x0A,0x04,0x0A,0x11,0x11}; for(int i=0;i<7;++i)out[i]=g[i]; break; }
    case 'Y': { static const unsigned char g[7]={0x11,0x11,0x0A,0x04,0x04,0x04,0x04}; for(int i=0;i<7;++i)out[i]=g[i]; break; }
    case 'Z': { static const unsigned char g[7]={0x1F,0x01,0x02,0x04,0x08,0x10,0x1F}; for(int i=0;i<7;++i)out[i]=g[i]; break; }
    case ':': { static const unsigned char g[7]={0x00,0x04,0x00,0x00,0x00,0x04,0x00}; for(int i=0;i<7;++i)out[i]=g[i]; break; }
    case '%': { static const unsigned char g[7]={0x19,0x1A,0x02,0x04,0x08,0x14,0x13}; for(int i=0;i<7;++i)out[i]=g[i]; break; }
    case ' ': default: { for (int i = 0; i < 7; ++i) out[i] = 0x00; break; }
    }
}

/* Draws uppercase text at 1280x720 coordinates, scale 2 or 3. */
static void draw_text(SDL_Renderer *r, const char *s, int x, int y, int scale) {
    if (!s || scale < 1) return;
    SDL_SetRenderDrawColor(r, 255, 255, 255, 255);
    int cx = x;
    for (const char *p = s; *p; ++p) {
        char c = *p;
        if (c >= 'a' && c <= 'z') c -= 'a' - 'A';
        if (c == ' ') { cx += 5 * scale + 1 * scale; continue; }
        unsigned char rows[7];
        font_rows(c, rows);
        for (int row = 0; row < 7; ++row) {
            for (int col = 0; col < 5; ++col) {
                if (rows[row] & (0x10 >> col)) {
                    SDL_Rect d = {cx + col * scale, y + row * scale, scale, scale};
                    SDL_RenderFillRect(r, &d);
                }
            }
        }
        cx += 5 * scale + 1 * scale;
    }
}

static int text_width(const char *s, int scale) {
    int n = 0;
    for (const char *p = s; *p; ++p) ++n;
    if (n == 0) return 0;
    return n * 5 * scale + (n - 1) * 1 * scale;
}

/* Thin hollow white frame around the camera feed (Fusion "White Frame
 * Camera" at [-1,0], Layer #5, visible only while a camera is up). */
static void draw_white_frame(SDL_Renderer *r) {
    SDL_SetRenderDrawColor(r, 255, 255, 255, 255);
    const int t = 2, m = 8;
    int rw, rh;
    SDL_GetRendererOutputSize(r, &rw, &rh);
    SDL_Rect top = {m, m, rw - 2 * m, t};
    SDL_Rect bottom = {m, rh - m - t, rw - 2 * m, t};
    SDL_Rect left = {m, m, t, rh - 2 * m};
    SDL_Rect right = {rw - m - t, m, t, rh - 2 * m};
    SDL_RenderFillRect(r, &top);
    SDL_RenderFillRect(r, &bottom);
    SDL_RenderFillRect(r, &left);
    SDL_RenderFillRect(r, &right);
}

/* Night HUD, drawn on every Frame 3 screen (office and camera views).
 * Positions follow Frame 3 Objects.txt: time of day [1186,65] + am
 * [1200,37] top-right, Which Night? [759,85] + The Night [1245,101],
 * Power [129,627] + Power Left [24,616] + Usage Text [24,632]
 * bottom-left, Cam Labels [888,272] room name while a camera is up. */
static void draw_night_hud(SDL_Renderer *r, int camera, int camera_up,
                            int night, int hour, int power, int usage) {
    char time_s[16], night_s[16], power_s[24];
    int h12 = hour;
    if (h12 < 1) h12 = 12;
    if (h12 > 12) h12 = ((h12 - 1) % 12) + 1;
    snprintf(time_s, sizeof time_s, "%d AM", h12);
    int n = night < 1 ? 1 : night > 7 ? 7 : night;
    snprintf(night_s, sizeof night_s, "NIGHT %d", n);
    int pct = power / 100;
    if (pct < 0) pct = 0;
    if (pct > 100) pct = 100;
    snprintf(power_s, sizeof power_s, "POWER: %d%%", pct);

    /* Top-right: time above night, right edges aligned near x=1256. */
    int tx = 1256 - text_width(time_s, 3);
    draw_text(r, time_s, tx, 37, 3);
    int nx = 1256 - text_width(night_s, 2);
    draw_text(r, night_s, nx, 72, 2);

    /* Bottom-left power + usage (Fusion Power/Power Left/Usage Text). */
    draw_text(r, power_s, 24, 600, 2);
    draw_text(r, "USAGE:", 24, 632, 2);
    int bx = 24 + text_width("USAGE: ", 2);
    int u = usage < 1 ? 1 : usage > 5 ? 5 : usage;
    SDL_SetRenderDrawColor(r, 255, 255, 255, 255);
    for (int i = 0; i < 5; ++i) {
        SDL_Rect bar = {bx + i * 20, 632, 14, 14};
        if (i < u)
            SDL_RenderFillRect(r, &bar);
        else
            SDL_RenderDrawRect(r, &bar);
    }

    /* Cam room name while a camera is up (Fusion Cam Labels string). */
    if (camera_up) {
        static const char *cam_names[4] = {"HELL", "MOUNTAIN", "FOREST", "DINOSAUR EXHIBIT"};
        int cam = camera - 1;
        if (cam < 0 || cam > 3) cam = 0;
        draw_text(r, cam_names[cam], 888, 272, 2);
    }
}

/* Frame 3 camera minimap (Layer #5 UI). Button hotspots come from
 * Objects.txt (see src/fnae_assets.h); the map itself draws top-left
 * like the other scenery, but the button boxes use FNAE_ANCHOR_CENTER:
 * with a top-left box the 31x25 label at (-21,-12) would hang off the
 * box corner, while centered the label sits inside the 60x40 box like
 * the Fusion layout. The button under You highlights green (CAM 01
 * Animation 12), the rest stay gray (Stopped); the native selected
 * index is the viewed camera.
 * CAM 01 sits 32px above its Objects.txt hotspot (339 -> 307): at the
 * verbatim spot the box sat fully inside its room outline instead of
 * straddling the top edge like the other buttons, verified with
 * headless screenshots (owner request). */
static void draw_minimap(SDL_Renderer *r, FnaeVisuals *v, int camera) {
    static const int btn_x[IMG_CAMBTN_COUNT] = {1016, 1179, 953, 1161};
    static const int btn_y[IMG_CAMBTN_COUNT] = {307, 371, 469, 505};
    static const int txt_x[IMG_CAMBTN_COUNT] = {995, 1158, 932, 1141};
    static const int txt_y[IMG_CAMBTN_COUNT] = {295, 359, 457, 493};

    draw_texture(r, v->minimap, 882, 265);
    for (int i = 0; i < IMG_CAMBTN_COUNT; ++i) {
        visuals_draw_anchored(r,
            (i == camera - 1) ? v->cam_btn_on : v->cam_btn_off,
            btn_x[i], btn_y[i], FNAE_ANCHOR_CENTER);
        draw_texture(r, v->cam_txt[i], txt_x[i], txt_y[i]);
    }
}

/* Low-music badges ("[ Warning Messages ]"): level 1 (<600) holds the
 * badge's Stopped triangle steady, level 2 (<200) blinks its Animation 12
 * triangle against the transparent frame on the shared static tick;
 * level 0/3 hides the badge. Each badge draws from its own bank range:
 * "Warning out of cam" (35-38) at [1228,672] on the office screen,
 * "warning in cam" (39-42) at [1215,506] on any camera view. */
static void draw_warning(SDL_Renderer *r, int warning, int static_frame,
                         SDL_Texture *steady, SDL_Texture *flash,
                         SDL_Texture *blank, int x, int y) {
    if (warning < 1 || warning > 2) return;
    SDL_Texture *t = steady;
    if (warning == 2)
        t = ((static_frame & 1) && blank) ? blank : (flash ? flash : steady);
    visuals_draw_anchored(r, t, x, y, FNAE_ANCHOR_CENTER);
}

/* Fullscreen overlay from 480x270 art at [0,0] with a Fusion scale
 * (phantoms 2.7 = 1296x729, jumpscares 2.8 = 1344x756, both slightly
 * overflowing the 1280x720 view right and bottom, like the original).
 * Baked per-pixel alpha blends the face over the scene; fading overlays
 * additionally fade via the global alpha 0->255. */
static void draw_phantom_cam(SDL_Renderer *r, SDL_Texture *t, int alpha,
                             int ox, int oy, float scale) {
    if (!t || alpha <= 0) return;
    int w, h;
    SDL_QueryTexture(t, NULL, NULL, &w, &h);
    SDL_Rect d = {ox, oy, (int)(w * scale), (int)(h * scale)};
    if (alpha < 255) {
        SDL_SetTextureBlendMode(t, SDL_BLENDMODE_BLEND);
        SDL_SetTextureAlphaMod(t, (Uint8)alpha);
    }
    SDL_RenderCopy(r, t, NULL, &d);
    if (alpha < 255)
        SDL_SetTextureAlphaMod(t, 255);
}

/* Frame 3 "[ Music Box ]" UI (Cam 04 view only), Layer #5 order: crank
 * box at [569,497] (Stopped released, Animation 12 held), Wind Text on
 * top of it ([497,475] top-left, inside the 156x65 box), Click & Hold
 * under it ([491,534] top-left), and the wind-gauge pie at the Music
 * Left counter spot ([418,474] top-left, left of the box). The pie
 * frame follows Music Left (0-2000 -> empty 181 -> full 202), so it
 * fills while the crank is held and loses wedges when released. The
 * lure button stays hidden here (Fusion hides it over Cam 04). */
static void draw_music_box(SDL_Renderer *r, FnaeVisuals *v,
                           int winding, int music) {
    visuals_draw_anchored(r, winding ? v->musicbtn_on : v->musicbtn_off,
                          569, 497, FNAE_ANCHOR_CENTER);
    int m = music < 0 ? 0 : music > 2000 ? 2000 : music;
    int idx = (m * (IMG_MUSIC_PIE_COUNT - 1) + 1000) / 2000;
    if (idx < 0) idx = 0;
    if (idx >= IMG_MUSIC_PIE_COUNT) idx = IMG_MUSIC_PIE_COUNT - 1;
    draw_texture(r, v->music_pie[idx], 418, 474);
    draw_texture(r, v->music_wind, 497, 475);
    draw_texture(r, v->music_hold, 491, 534);
}

/* Frame 8 Customize screen (Frame 8 Objects.txt). Box/column numbers
 * mirror the core hit rects in fnae_core.c (cust_box_x/y); counters sit
 * at the verbatim AI Level hotspots (+~140 x from each column's global).
 * Portraits/boxes draw top-left (the 150px art tiles the 160px grid);
 * the 50x25 arrows draw center-anchored @1.3 scale like the title arrow
 * pattern (see docs/COORDINATES.md). AI order here matches fnae_set_custom
 * (freddy, foxy, spring, golden, mangle, bb, puppet), NOT column order. */
static void draw_customize(SDL_Renderer *r, FnaeVisuals *v,
                           const int *ai, int sel, int ch, int b,
                           int check, int cool) {
    static const int box_x[7] = {85, 246, 406, 566, 726, 886, 1046};
    static const int box_y[7] = {52, 53, 53, 53, 53, 53, 53};
    /* AI Level counters, verbatim Objects.txt hotspots, paired to columns
     * left to right (each sits ~+140 x from its column's global). */
    static const int cnt_x[7] = {225, 389, 544, 705, 864, 1019, 1185};
    static const int cnt_y[7] = {248, 244, 247, 246, 246, 246, 247};
    /* Column order: Freddy, Mangle, Foxy, Golden, Springtrap, BB, Puppet;
     * ai[] order: freddy, foxy, spring, golden, mangle, bb, puppet. */
    static const int col_ai[7] = {0, 4, 1, 3, 2, 5, 6};

    /* Cool Background tiles 40x40 across the 1280x720 view. */
    SDL_Texture *bg = NULL;
    if (cool >= 0 && cool < 3) bg = v->cust_bg[cool];
    if (bg) {
        int w = 0, h = 0;
        SDL_QueryTexture(bg, NULL, NULL, &w, &h);
        if (w < 1) w = 40;
        if (h < 1) h = 40;
        for (int y = 0; y < 720; y += h)
            for (int x = 0; x < 1280; x += w) {
                SDL_Rect d = {x, y, w, h};
                SDL_RenderCopy(r, bg, NULL, &d);
            }
    }
    /* Select frames + portraits, top row. */
    for (int i = 0; i < 7; ++i) {
        draw_texture(r, v->cust_select, box_x[i], box_y[i]);
        draw_texture(r, v->cust_portrait[i], box_x[i], box_y[i]);
    }
    /* Empty Select Box rows, verbatim Frame 8 Objects.txt Layer #2:
     * middle row of 7 at y=261, bottom row of 6 at y=469 (the 7th
     * bottom slot holds the Set 20 button instead of a box). These
     * have no portraits/counters — they fill the grid like the
     * reference shot. */
    static const int row2_x[7] = {85, 246, 406, 566, 726, 886, 1046};
    for (int i = 0; i < 7; ++i)
        draw_texture(r, v->cust_select, row2_x[i], 261);
    static const int row3_x[6] = {85, 246, 406, 566, 726, 886};
    for (int i = 0; i < 6; ++i)
        draw_texture(r, v->cust_select, row3_x[i], 469);
    /* AI Level counters (Fusion counters -> bitmap text). */
    if (ai) {
        for (int i = 0; i < 7; ++i) {
            char num[8];
            int val = ai[col_ai[i]];
            if (val < 0) val = 0;
            snprintf(num, sizeof num, "%d", val);
            draw_text(r, num, cnt_x[i], cnt_y[i], 2);
        }
    }
    /* Hover arrows over the selected column: up at select+(36,88),
     * down (arrow 2) at select+(36,152), @1.3 scale. */
    if (sel >= 0 && sel < 7) {
        int upx = box_x[sel] + 36, upy = box_y[sel] + 88;
        int dnx = box_x[sel] + 36, dny = box_y[sel] + 152;
        if (v->cust_arrow) {
            int w = 0, h = 0;
            SDL_QueryTexture(v->cust_arrow, NULL, NULL, &w, &h);
            float s = 1.3f;
            SDL_Rect u = {(int)(upx - w * s / 2), (int)(upy - h * s / 2),
                          (int)(w * s), (int)(h * s)};
            SDL_Rect d = {(int)(dnx - w * s / 2), (int)(dny - h * s / 2),
                          (int)(w * s), (int)(h * s)};
            SDL_RenderCopy(r, v->cust_arrow, NULL, &u);
            /* Down arrow (arrow 2): same art flipped vertically. */
            SDL_RenderCopyEx(r, v->cust_arrow, NULL, &d, 0.0, NULL,
                             SDL_FLIP_VERTICAL);
        }
    }
    /* Set 20 / Add 1 / GO! buttons, verbatim top-left positions. */
    draw_texture(r, v->cust_set20, 1039, 469);
    draw_texture(r, v->cust_add1, 1039, 541);
    draw_texture(r, v->cust_go, 1032, 616);
    /* Challenge arrows: same triangle rotated to point left/right,
     * center-anchored on [88,16] / [456,16]. */
    if (v->cust_arrow) {
        int w = 0, h = 0;
        SDL_QueryTexture(v->cust_arrow, NULL, NULL, &w, &h);
        float s = 1.3f;
        SDL_Rect l = {(int)(88 - h * s / 2), (int)(16 - w * s / 2),
                      (int)(h * s), (int)(w * s)};
        SDL_Rect rr = {(int)(456 - h * s / 2), (int)(16 - w * s / 2),
                       (int)(h * s), (int)(w * s)};
        SDL_RenderCopyEx(r, v->cust_arrow, NULL, &l, -90.0, NULL,
                         SDL_FLIP_NONE);
        SDL_RenderCopyEx(r, v->cust_arrow, NULL, &rr, 90.0, NULL,
                         SDL_FLIP_NONE);
    }
    /* Challenge Label ([128,16] string, alpha 200 while B==0, hidden
     * while the preset holds). Bitmap text has no alpha fade here. */
    if (b == 0) {
        static const char *names[4] = {"NO CHALLENGE", "THE CLASSICS",
            "BROKEN DOWN", "SOY SAUCE EDWARD"};
        int c = ch < 0 ? 0 : ch > 3 ? 3 : ch;
        draw_text(r, names[c], 128, 16, 2);
    }
    /* Check marks flank the top bar while the selected challenge is
     * beaten (Fusion: Check visible iff its A = Ini Challenge<N> > 0). */
    if (check) {
        draw_texture(r, v->cust_check, 8, 16);
        draw_texture(r, v->cust_check, 1208, 16);
    }
}

void visuals_render(FnaeVisuals *v, SDL_Renderer *r, int frame, int camera,
                     int camera_up, int night, int hour, int power,
                     int left_door, int right_door, int mask, int arrow, int progress,
                     int static_frame, int static_alpha, int office_scroll,
                      int left_door_frame, int right_door_frame, int mask_frame, int title_bg_frame,
                      float six_timer,
                      int foxy_pos, int freddy_pos, int cam_static_alpha,
                     int death, int music, int cam_scroll, int usage,
                      int stand, int lure_area, int lure_cam,
                      int lure_cd, float lure_cd_timer,
                      int winding, int warning, int mute_visible,
                      int ph_mangle_cam, int ph_bb_cam,
                      int ph_bb_scare, int ph_bb_scare_on, int ph_annoy_a,
                       int death_addup, int death_red, int death_red_peaked,
                       int death_rip_a, int death_rip_b, int death_ticks, int gf_sit,
                       int freddy_door, int foxy_stand,
                      const int *cust_ai, int cust_sel, int cust_ch,
                      int cust_b, int cust_check, int cust_cool,
                      int movement_out, int cam_flip_frame) {
    SDL_SetRenderDrawColor(r, 0, 0, 0, 255);
    SDL_RenderClear(r);

    /* Jumpscare shake: the night scene jumps X/Y +/-Random(5) on any death
     * (GF shakes +/-Random(8)). Applied to the office/cam draws below.
     * Derived from the per-tick death_addup counter (not rand() per render),
     * so the offset holds steady across renders of the same tick and steps
     * at the Fusion 60 Hz cadence on any refresh rate: re-rolling every
     * present strobed/vibrated on uncapped or high-Hz loops. */
    int ox = 0, oy = 0;
    if (frame == 3 && death > 0) {
        int j = (death == 5) ? 8 : 5;
        unsigned s1 = (unsigned)(death_addup < 0 ? 0 : death_addup) * 1103515245u + 12345u;
        unsigned s2 = (unsigned)(death_addup < 0 ? 0 : death_addup) * 22695477u + 1u;
        ox = (int)(s1 % (unsigned)(2 * j + 1)) - j;
        oy = (int)((s2 >> 8) % (unsigned)(2 * j + 1)) - j;
    }

    if (frame == 1) {
        /* Frame 1 Warning: fullscreen 1280x720 card at [0,0],
         * like the title background. */
        if (v->warning) {
            SDL_Rect d = {0, 0, 1280, 720};
            SDL_RenderCopy(r, v->warning, NULL, &d);
        }
    } else if (frame == 2) {
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
            /* Movement Out ("Connection Lost"): the feed cuts to black with
             * the 333 banner centered (owner request; Fusion parks it at
             * [640,60] over the live feed). The screen is already cleared
             * black above, so just skip the feed/static/stand here. */
            if (movement_out) {
                int rw, rh;
                SDL_GetRendererOutputSize(r, &rw, &rh);
                visuals_draw_anchored(r, v->connection_lost,
                                      rw / 2, rh / 2, FNAE_ANCHOR_CENTER);
            } else {
                /* The feed auto-pans left <-> right on the Camera Center
                 * Object (see update_cam_scroll); drawn cover-cropped so the
                 * full 1600px width scrolls through the 1280px view. */
                draw_cam_pan(r, v->cams[idx][occupied], cam_scroll, ox, oy);
                draw_static(r, v->static_frames[static_frame & 7], cam_static_alpha);
                /* Layer order mirrors Fusion: feed (#1), Springtrap Stand (#2),
                 * then the camera UI (#5: minimap, lure button, frame, HUD).
                 * The Stand is a world object at its Objects.txt position,
                 * shifted by the feed scroll like doors shift with the office.
                 * It reappears only while viewing Springtrap's camera
                 * ("View > 0 + You overlapping Springtrap"). */
                if (stand)
                    draw_world(r, v->springtrap_stand, 416, -24, cam_scroll, ox, oy);
            }
            /* UI sits above the feed static (Static precedes minimap in Layer #5). */
            int sel = camera < 1 ? 1 : camera > IMG_CAMBTN_COUNT ? IMG_CAMBTN_COUNT : camera;
            draw_minimap(r, v, sel);
            /* Lure Area marker (gray circle): spawns at (0,0) from the
             * viewed CAM 01 button, so it draws center-anchored over the
             * lured camera's minimap button until the lure resolves
             * (~2 s). Hidden with the rest of the camera UI when the
             * cameras are down. Drawn mostly transparent so the cam
             * button stays readable underneath. */
            if (lure_area && lure_cam >= 1 && lure_cam <= IMG_CAMBTN_COUNT) {
                static const int btn_x[IMG_CAMBTN_COUNT] = {1016, 1179, 953, 1161};
                static const int btn_y[IMG_CAMBTN_COUNT] = {307, 371, 469, 505};
                SDL_SetTextureBlendMode(v->lure_area, SDL_BLENDMODE_BLEND);
                SDL_SetTextureAlphaMod(v->lure_area, 70);
                visuals_draw_anchored(r, v->lure_area,
                                      btn_x[lure_cam - 1], btn_y[lure_cam - 1],
                                      FNAE_ANCHOR_CENTER);
                SDL_SetTextureAlphaMod(v->lure_area, 255);
            }
            /* Lure Button reappears with the camera UI except on Cam 04
             * (the music-box camera). Center-anchored like the cam
             * buttons. While the button cooldown is active the button
             * plays its Animation 12 cooldown (1 -> 2 -> 3 -> 4 square
             * dots over the ~2 s window, transparent frames so only the
             * dots show); otherwise the Stopped "Lure" frame shows.
             * The cooldown is independent of the marker: destroying the
             * Lure Area never shortens it. */
            if (sel != 4) {
                SDL_Texture *lure = v->lure_button;
                if (lure_cd) {
                    int f = (int)(lure_cd_timer / 2.0f * 4.0f);
                    if (f < 0) f = 0;
                    if (f > 3) f = 3;
                    if (v->lure_cd[f]) lure = v->lure_cd[f];
                }
                visuals_draw_anchored(r, lure, 744, 296,
                                      FNAE_ANCHOR_CENTER);
            }
            /* White Frame Camera reappears with the rest of the camera UI. */
            draw_white_frame(r);
            if (sel == 4)
                draw_music_box(r, v, winding, music);
            /* Low-music badge for the camera screens ("warning in cam",
             * bank 39-42 at [1215,506], center-anchored): reappears on ANY
             * camera view while 0 < Music Left < 600 (Fusion
             * "[ Warning Messages ]" gates on View > 0, not on Cam 04),
             * steady Stopped (39) below 600, blinking Animation 12
             * (41/42) below 200, hidden when empty. */
            draw_warning(r, warning, static_frame,
                         v->warn_in_steady, v->warn_in_flash, v->warn_in_blank,
                         1215, 506);
            /* Mute Call button shows on every Frame 3 screen while the
             * night's call plays (Fusion reappears it every 3 s). */
            if (mute_visible)
                visuals_draw_anchored(r, v->mutecall, 100, 55,
                                      FNAE_ANCHOR_CENTER);
            draw_night_hud(r, sel, 1, night, hour, power, usage);
        } else {
            draw_office_pan(r, v->office, office_scroll, ox, oy);
            /* Layer order mirrors Fusion Layer #2 (overlay office): each
             * doorway figure sits BEHIND its door shutter, so a closed
             * door covers the character. Per-side order is Freddy, left
             * door, left button, then Foxy, right door, right button.
             * Doors/desk are world objects at verbatim Objects.txt
             * positions, shifted by the pan scroll. */
            if (left_door_frame < 0) left_door_frame = 0;
            if (left_door_frame >= IMG_DOOR_FRAMES) left_door_frame = IMG_DOOR_FRAMES - 1;
            if (right_door_frame < 0) right_door_frame = 0;
            if (right_door_frame >= IMG_DOOR_FRAMES) right_door_frame = IMG_DOOR_FRAMES - 1;
            /* Freddy at the left door (213 @1.1, center-anchored) while
             * his collision overlaps it (office view only). Drawn at
             * [230,360]: his verbatim Objects.txt spot [260,788] sits
             * below the 720 screen and showed antennae only, so he is
             * centered in the left doorway (door [119,0] is 223 wide,
             * center x~230) at Foxy's height (owner request). */
            if (freddy_door)
                draw_world_scaled(r, v->freddy_door, 230, 360, office_scroll, ox, oy,
                                  1.1f, FNAE_ANCHOR_CENTER);
            draw_world(r, v->door_left[left_door_frame], 119, 0, office_scroll, ox, oy);
            /* Left door button (center-anchored at its Objects.txt spot):
             * Stopped while the door is open/opening (A 0/3), Animation
             * 12 while closing/closed (A 1/2). left_door carries A. */
            visuals_draw_anchored(r, v->door_btn[(left_door == 1 || left_door == 2) ? 1 : 0],
                                  105 - office_scroll + ox, 500 + oy,
                                  FNAE_ANCHOR_CENTER);
            /* Foxy at the right door (228 @1.1, center-anchored) while her
             * collision overlaps it. Drawn at [1387,331]: her verbatim
             * Objects.txt spot [1287,331] sat 100px left of the right
             * doorway (door [1263,0] is 248 wide, center x~1387), so she
             * is centered in it like Freddy (owner request). */
            if (foxy_stand)
                draw_world_scaled(r, v->foxy_stand, 1387, 331, office_scroll, ox, oy,
                                  1.1f, FNAE_ANCHOR_CENTER);
            draw_world(r, v->door_right[right_door_frame], 1263, 0, office_scroll, ox, oy);
            visuals_draw_anchored(r, v->door_btn[(right_door == 1 || right_door == 2) ? 1 : 0],
                                  1489 - office_scroll + ox, 500 + oy,
                                  FNAE_ANCHOR_CENTER);
            /* GF Sit (Layer #2 office overlay, above the doors): reappears
             * while GF Random == 1, invisible otherwise. A world object
             * like the doors, so it pans with the office. Drawn before
             * the desk, so the desk front/papers overlap its base: the
             * bottle stands behind the desk, like the reference shot.
             * Position is owner-matched to the reference (base planted on
             * the desk surface among the paper balls), not the verbatim
             * Objects.txt [440,240], which left it floating mid-air. */
            if (gf_sit)
                draw_world(r, v->gf_sit, 528, 305, office_scroll, ox, oy);
            draw_world(r, v->desk, 266, 177, office_scroll, ox, oy);
            /* Ph Mangle Annoy (Layer #3, above the desk): rises from
             * [508,720] by Annoy A px while C==1, then sinks back once
             * B>=7. A world object, so it pans with the office scroll. */
            if (ph_annoy_a > 0)
                draw_world(r, v->phmangle_annoy, 508, 720 - ph_annoy_a,
                           office_scroll, ox, oy);
            /* Low-music badge for the office screen ("Warning out of cam",
             * bank 35-38 at [1228,672], center-anchored). */
            draw_warning(r, warning, static_frame,
                         v->warn_out_steady, v->warn_out_flash, v->warn_out_blank,
                         1228, 672);
            if (mute_visible)
                visuals_draw_anchored(r, v->mutecall, 100, 55,
                                      FNAE_ANCHOR_CENTER);
            /* Power/time/night HUD stays up on the office screen too. */
            int sel = camera < 1 ? 1 : camera > IMG_CAMBTN_COUNT ? IMG_CAMBTN_COUNT : camera;
            draw_night_hud(r, sel, 0, night, hour, power, usage);
            /* Mask overlay (Layer #5 UI, above the HUD like the Fusion
             * order): put-on flip 0-6 at [0,0], worn mask 7
             * (1480x870 at [-100,-66]), take-off flip 8-10 at [0,0].
             * Unshaken UI layer, like the HUD. */
            if (mask_frame >= 0 && mask_frame < IMG_MASK_FRAMES) {
                SDL_Texture *mt = v->mask_anim[mask_frame];
                if (mt) {
                    if (mask_frame == IMG_MASK_FLIPDN_COUNT)
                        draw_texture(r, mt, -100, -66);
                    else
                        draw_texture(r, mt, 0, 0);
                }
            }
        }
        /* Cam flip flash (Frame 3 "[ Camera ]" flip events, Layer #5 UI):
         * 573-581 fit to screen on flip-up, reversed on flip-down, over
         * the office/cam UI below and under the Layer #6 overlays. The
         * frames are black-background, so the contain fit blends in. */
        if (cam_flip_frame >= 0 && cam_flip_frame < IMG_CAMFLIP_COUNT)
            fit_center(r, v->cam_flip[cam_flip_frame]);
        /* Layer #6 top overlays (Phantom Mangle / Phantom BB camera haunts
         * + the BB scare fade): above feed, office, and camera UI alike,
         * in Objects.txt layer order. The camera haunts show while A==1
         * AND the cameras are up: A only clears once View hits 0, so
         * without the cameras-up gate the face lingered over the office
         * through the 0.55 s camera-down transition. The scare fades in
         * over the office after the B>80 force-down (gated on the first
         * trigger in core), so it stays ungated. */
        if (camera_up && ph_mangle_cam)
            draw_phantom_cam(r, v->phmangle_cam, 255, ox, oy, 2.7f);
        if (camera_up && ph_bb_cam)
            draw_phantom_cam(r, v->phbb_cam, 255, ox, oy, 2.7f);
        if (ph_bb_scare_on && ph_bb_scare > 0)
            draw_phantom_cam(r, v->phbb_scare, ph_bb_scare, ox, oy, 2.7f);
        /* Jumpscare (Frame 3 "[ Jumpscares ]"): created at (0,0) x2.8 on
         * death, fullscreen over the shaking office for the 60-tick wait.
         * Frames loop at 20fps (every 3rd death_addup tick); GF is a still.
         * Sounds (stop-all + ch #2 sample) already fire from audio.c. */
        if (death > 0) {
            SDL_Texture **tab = NULL;
            int n = 0;
            switch (death) {
            case 1: tab = v->scare_puppet; n = IMG_SCARE_PUPPET_COUNT; break;
            case 2: tab = v->scare_freddy; n = 12; break;
            case 3: tab = v->scare_foxy; n = IMG_SCARE_FOXY_A_COUNT + IMG_SCARE_FOXY_B_COUNT; break;
            case 4: tab = v->scare_spring; n = IMG_SCARE_SPRING_COUNT; break;
            case 5: tab = &v->scare_gf; n = 1; break;
            default: break;
            }
            if (tab && n > 0) {
                int tick = death_addup < 0 ? 0 : death_addup;
                SDL_Texture *t = tab[(tick / 3) % n];
                if (t) draw_phantom_cam(r, t, 255, ox, oy, 2.8f);
            }
        }
    } else if (frame == 6) {
        draw_which_night(r, v, night);
    } else if (frame == 9) {
        /* Frame 9 (6 AM): black screen with the "which AM" odometer at its
         * verbatim [533,324] (top-left): Stopped "5" for the first 3 s,
         * then the roll up to "6" at 20fps, holding the last frame (Frame
         * 9 Events.txt: Timer > 03'' + once -> Start animation). The win
         * screens (2/4/7.png) only show on Frame 5 Final after nights
         * 5/6/7, never here. */
        int widx = 0;
        if (six_timer >= 3.0f) {
            widx = 1 + (int)((six_timer - 3.0f) * 20.0f);
            if (widx >= IMG_WHICH_AM_COUNT) widx = IMG_WHICH_AM_COUNT - 1;
        }
        draw_texture(r, v->which_am[widx], 533, 324);
        draw_texture(r, v->am_text, 624, 324);
    } else if (frame == 4) {
        /* Frame 4 Death: fullscreen red flash first (drains back to 0 so
         * the animation owns the rest of the screen). The devil-card Death
         * Anim backdrop (cycling at 20fps, center-anchored on its [630,390]
         * hotspot) and the RIP Text at [640,650] (the 404 frame while B==0
         * fading out, GAME OVER 1.png once B>1 fading back in) stay hidden
         * until the flash first peaks, so the red shows alone. Red is
         * Layer #2 above Layer #1. */
        if (death_red_peaked) {
        SDL_Texture *devil = v->death_devil[(death_ticks / 3) & 1];
        if (devil) visuals_draw_anchored(r, devil, 630, 390, FNAE_ANCHOR_CENTER);
        SDL_Texture *rip = (death_rip_b == 0) ? v->death_rip : v->death;
        if (rip && death_rip_a > 0) {
            int ra = death_rip_a > 255 ? 255 : death_rip_a;
            SDL_SetTextureBlendMode(rip, SDL_BLENDMODE_BLEND);
            SDL_SetTextureAlphaMod(rip, (Uint8)ra);
            visuals_draw_anchored(r, rip, 640, 650, FNAE_ANCHOR_CENTER);
            SDL_SetTextureAlphaMod(rip, 255);
        }
        }
        if (death_red > 0) {
            int ra = death_red > 255 ? 255 : death_red;
            int rw, rh;
            SDL_GetRendererOutputSize(r, &rw, &rh);
            SDL_Rect d = {0, 0, rw, rh};
            SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(r, 255, 0, 0, (Uint8)ra);
            SDL_RenderFillRect(r, &d);
            SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_NONE);
        }
    } else if (frame == 5) {
        /* Frame 5 Final win screens (Frame 5 Events.txt creates one of
         * Night 5/6/7 from the "6th or 7th night" counter): night 5 ->
         * weekly paycheck (2.png), night 6 -> overtime paycheck (4.png),
         * night 7/custom -> termination notice (7.png). Final is only
         * reachable from nights 5-7, so night selects directly. */
        SDL_Texture *win = v->final_screen;
        if (night == 6) win = v->final_n6;
        else if (night == 7) win = v->final_n7;
        fit_center(r, win);
    } else if (frame == 7) {
        fit_center(r, v->newspaper);
    } else if (frame == 8) {
        draw_customize(r, v, cust_ai, cust_sel, cust_ch, cust_b,
                       cust_check, cust_cool);
    }

    static const char *cam_names[4] = {"Hell", "Mountain", "Forest", "Dino"};
    int cam = camera - 1;
    if (cam < 0 || cam > 3) cam = 0;
    char title[256];
    snprintf(title, sizeof title,
             "Five Nights at Edward's | Native | Night %d | %d AM | Power %d%% | Cam %s%s | doors L%d R%d%s | music %d",
             night, hour, power / 100, cam_names[cam], camera_up ? " (up)" : "",
             left_door > 0, right_door > 0, mask ? " | MASK" : "",
             music);
    /* Live state in the window title, but only pushed on change: calling
     * SDL_SetWindowTitle 60-144x/sec makes the title bar visibly flicker
     * on Windows and wastes a non-client repaint every present. */
    SDL_Window *win = SDL_RenderGetWindow(r);
    if (win) {
        static SDL_Window *last_win = NULL;
        static char last_title[256] = {0};
        if (win != last_win || strcmp(title, last_title) != 0) {
            last_win = win;
            snprintf(last_title, sizeof last_title, "%s", title);
            SDL_SetWindowTitle(win, title);
        }
    }
}
