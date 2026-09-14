#pragma once
#include <SDL.h>
#include <SDL_image.h>

#include "fnae_assets.h"

typedef struct {
    SDL_Texture *office;
    SDL_Texture *cams[4][2]; /* per camera: [0] empty base, [1] animatronic present */
    SDL_Texture *static_frames[IMG_STATIC_COUNT]; /* TV-static animation, shared title/cameras */
    SDL_Texture *six_am;
    SDL_Texture *death;

    /* Title-screen components from the exported Fusion asset bank. */
    SDL_Texture *title_bg;
    SDL_Texture *title_bg_anim[IMG_TITLE_BG_ANIM_COUNT]; /* 3-frame bg flash (516-518) */
    SDL_Texture *title_new;
    SDL_Texture *title_continue;
    SDL_Texture *title_6night;
    SDL_Texture *title_custom;
    SDL_Texture *title_arrow;
    SDL_Texture *title_star;
    SDL_Texture *title_template;
    SDL_Texture *title_nights[7];

    SDL_Texture *newspaper;
    SDL_Texture *final_screen;
    SDL_Texture *warning; /* Frame 1 warning screen */

    /* Frame 3 night-shift art: door shutters (16 frames each) + desk. */
    SDL_Texture *door_left[IMG_DOOR_FRAMES];
    SDL_Texture *door_right[IMG_DOOR_FRAMES];
    SDL_Texture *desk;

    /* Frame 3 camera minimap (Layer #5 UI): line-art map plus the four
     * clickable cam buttons (gray/green box + "CAM 0X" label each). */
    SDL_Texture *minimap;
    SDL_Texture *cam_btn_off;
    SDL_Texture *cam_btn_on;
    SDL_Texture *cam_txt[IMG_CAMBTN_COUNT];
    /* Audio-lure button (Layer #5 UI) + cooldown dots (Animation 12) +
     * Lure Area marker (Layer #5, on the lured cam button) + Springtrap
     * stand figure (Layer #2 office overlay, over the viewed feed). */
    SDL_Texture *lure_button;
    SDL_Texture *lure_cd[4];
    SDL_Texture *lure_area;
    SDL_Texture *springtrap_stand;
    /* Music-box crank (Cam 04 view only) + low-music warning badges. */
    SDL_Texture *musicbtn_off;
    SDL_Texture *musicbtn_on;
    SDL_Texture *music_wind; /* "Give $ To Business Edward" overlay text */
    SDL_Texture *music_hold; /* "click & hold" hint under the box */
    SDL_Texture *music_pie[IMG_MUSIC_PIE_COUNT]; /* wind gauge, empty->full */
    SDL_Texture *warn_off;
    SDL_Texture *warn_on;
    /* Mute Call button (night calls only, while one plays). */
    SDL_Texture *mutecall;
    /* Phantom Mangle + Phantom BB (Layer #6 top overlays + office Annoy). */
    SDL_Texture *phmangle_cam;   /* camera overlay (A==1 while haunting) */
    SDL_Texture *phmangle_annoy; /* office riser (Layer #3, above the desk) */
    SDL_Texture *phbb_cam;       /* camera overlay (A==1 while haunting) */
    SDL_Texture *phbb_scare;     /* post-scare fade (alpha 0->255) */
    /* Jumpscares (Frame 3 "[ Jumpscares ]"): 480x270 art at (0,0) x2.8,
     * fullscreen over the shaking office during the death wait. Counts
     * mirror the verified bank runs (see fnae_assets.h). */
    SDL_Texture *scare_spring[IMG_SCARE_SPRING_COUNT];
    SDL_Texture *scare_freddy[12]; /* 353-363 + 365 (364 is a UI dot) */
    SDL_Texture *scare_puppet[IMG_SCARE_PUPPET_COUNT];
    SDL_Texture *scare_foxy[IMG_SCARE_FOXY_A_COUNT + IMG_SCARE_FOXY_B_COUNT]; /* 4 + 11 */
    SDL_Texture *scare_gf;       /* still (458) */
    SDL_Texture *gf_sit;         /* office figure (310) while GF Random == 1 */
    /* Death frame (Frame 4): devil-card Death Anim backdrop cycle. */
    SDL_Texture *death_devil[2];
} FnaeVisuals;

int visuals_init(FnaeVisuals *v, SDL_Renderer *r);
void visuals_free(FnaeVisuals *v);

/* Fusion object coordinates address the object's hotspot, while SDL draws
 * textures from the top-left (see docs/COORDINATES.md). The anchor selects
 * which point of the texture lands on the given (x, y). */
typedef enum {
    FNAE_ANCHOR_TOP_LEFT,    /* hotspot at the object's top-left (Fusion default look) */
    FNAE_ANCHOR_CENTER,      /* hotspot at the object center */
    FNAE_ANCHOR_RIGHT_CENTER /* hotspot at the middle of the right edge (pointer tips) */
} FnaeAnchor;

void visuals_draw_anchored(SDL_Renderer *r, SDL_Texture *t,
                           int x, int y, FnaeAnchor anchor);
void visuals_render(FnaeVisuals *v, SDL_Renderer *r, int frame, int camera,
                     int camera_up, int night, int hour, int power,
                     int left_door, int right_door, int mask, int arrow, int progress,
                     int static_frame, int static_alpha, int office_scroll,
                     int left_door_frame, int right_door_frame, int title_bg_frame,
                      int foxy_pos, int freddy_pos, int cam_static_alpha,
                      int death, int music, int cam_scroll, int usage,
                      int springtrap_stand, int lure_area, int lure_cam,
                      int lure_cd, float lure_cd_timer,
                      int winding, int warning, int mute_visible,
                      int ph_mangle_cam, int ph_bb_cam,
                      int ph_bb_scare, int ph_bb_scare_on, int ph_annoy_a,
                      int death_addup, int death_red, int death_rip_a,
                      int death_ticks, int gf_sit);
