#include "visuals.h"
#include "fnae_assets.h"
#include "fnae_core.h"
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
    v->warning = load_id(r, IMG_WARNING);

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
    /* Camera minimap + buttons (see src/fnae_assets.h for layout). */
    v->minimap = load_id(r, IMG_MINIMAP);
    v->cam_btn_off = load_id(r, IMG_CAMBTN_OFF);
    v->cam_btn_on = load_id(r, IMG_CAMBTN_ON);
    for (int i = 0; i < IMG_CAMBTN_COUNT; ++i)
        v->cam_txt[i] = load_id(r, IMG_CAMTXT_FIRST + i);
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
    v->warn_off = load_id(r, IMG_WARNBADGE_OFF);
    v->warn_on = load_id(r, IMG_WARNBADGE_ON);
    v->mutecall = load_id(r, IMG_MUTECALL);
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
    destroy_texture(&v->warn_off);
    destroy_texture(&v->warn_on);
    destroy_texture(&v->mutecall);
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

/* Low-music badge shared by both warning objects: level 1 (<600) shows
 * the Stopped frame steady, level 2 (<200) flashes the Animation 12
 * frame on the shared static tick; level 0/3 hides the badge. */
static void draw_warning(SDL_Renderer *r, FnaeVisuals *v,
                         int warning, int static_frame, int x, int y) {
    if (warning < 1 || warning > 2) return;
    SDL_Texture *t = v->warn_off;
    if (warning == 2 && (static_frame & 1) && v->warn_on) t = v->warn_on;
    visuals_draw_anchored(r, t, x, y, FNAE_ANCHOR_CENTER);
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
                           int winding, int music, int warning,
                           int static_frame) {
    visuals_draw_anchored(r, winding ? v->musicbtn_on : v->musicbtn_off,
                          569, 497, FNAE_ANCHOR_CENTER);
    int m = music < 0 ? 0 : music > 2000 ? 2000 : music;
    int idx = (m * (IMG_MUSIC_PIE_COUNT - 1) + 1000) / 2000;
    if (idx < 0) idx = 0;
    if (idx >= IMG_MUSIC_PIE_COUNT) idx = IMG_MUSIC_PIE_COUNT - 1;
    draw_texture(r, v->music_pie[idx], 418, 474);
    draw_texture(r, v->music_wind, 497, 475);
    draw_texture(r, v->music_hold, 491, 534);
    draw_warning(r, v, warning, static_frame, 1215, 506);
}

void visuals_render(FnaeVisuals *v, SDL_Renderer *r, int frame, int camera,
                     int camera_up, int night, int hour, int power,
                     int left_door, int right_door, int mask, int arrow, int progress,
                     int static_frame, int static_alpha, int office_scroll,
                     int left_door_frame, int right_door_frame, int title_bg_frame,
                     int foxy_pos, int freddy_pos, int cam_static_alpha,
                     int death, int music, int cam_scroll, int usage,
                     int stand, int lure_area, int lure_cam,
                     int lure_cd, float lure_cd_timer,
                     int winding, int warning, int mute_visible) {
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
                draw_music_box(r, v, winding, music, warning, static_frame);
            /* Mute Call button shows on every Frame 3 screen while the
             * night's call plays (Fusion reappears it every 3 s). */
            if (mute_visible)
                visuals_draw_anchored(r, v->mutecall, 100, 55,
                                      FNAE_ANCHOR_CENTER);
            draw_night_hud(r, sel, 1, night, hour, power, usage);
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
            /* Low-music badge for the office screen (Warning out of cam). */
            draw_warning(r, v, warning, static_frame, 1228, 672);
            if (mute_visible)
                visuals_draw_anchored(r, v->mutecall, 100, 55,
                                      FNAE_ANCHOR_CENTER);
            /* Power/time/night HUD stays up on the office screen too. */
            int sel = camera < 1 ? 1 : camera > IMG_CAMBTN_COUNT ? IMG_CAMBTN_COUNT : camera;
            draw_night_hud(r, sel, 0, night, hour, power, usage);
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
    /* Frame 8 (Customize) stays black: its UI art is still unmapped. */

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
