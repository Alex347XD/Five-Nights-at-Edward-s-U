#pragma once
#include <SDL.h>
#include <SDL_image.h>

typedef struct {
    SDL_Texture *office;
    SDL_Texture *cams[4];
    SDL_Texture *static_tex;
    SDL_Texture *six_am;
    SDL_Texture *death;

    /* Title-screen components from the exported Fusion asset bank. */
    SDL_Texture *title_bg;
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
                    int left_door, int right_door, int mask, int arrow, int progress);
