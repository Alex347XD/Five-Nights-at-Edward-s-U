#pragma once

/* Wii U GamePad controls (+ dual-screen window flags).
 *
 * Portable header (C11 + SDL keycodes only, no WUT includes): the mapping
 * is expressed through the existing fnae_key / fnae_key_up / fnae_click /
 * fnae_mouse_move API, so it behaves identically to the desktop controls
 * and stays testable via headless `pad` script events. On Wii U hardware
 * (guarded by __WIIU__ in main.c) the events come from the SDL WiiU port:
 * the GamePad is joystick 0 with the button order below (see
 * sdl-wiiu SDL_wiiujoystick.h vpad_button_map), both sticks are axes 0-3,
 * and the GamePad touchscreen arrives as SDL finger events.
 *
 * Layout: cameras live on the GamePad screen (ZL opens them), everything
 * else stays on the TV. While the cameras are closed the GamePad shows
 * just a black screen.
 */

#include <SDL_keycode.h>

#include <stdio.h>
#include <string.h>

#include "fnae_core.h"

/* WiiU-only SDL window flags (from the sdl-wiiu port's SDL_video.h).
 * Desktop SDL2 headers lack them, so fall back to the port's values. */
#ifndef SDL_WINDOW_WIIU_GAMEPAD_ONLY
#define SDL_WINDOW_WIIU_GAMEPAD_ONLY 0x01000000u
#endif
#ifndef SDL_WINDOW_WIIU_TV_ONLY
#define SDL_WINDOW_WIIU_TV_ONLY 0x02000000u
#endif
/* Single-swap discipline (current wiiu-sdl2 portlibs, 2.32.x): every
 * SDL_RenderPresent ends in GX2SwapScanBuffers(), which flips BOTH the TV
 * and the DRC scan buffers. Presenting the TV and GamePad renderers back
 * to back therefore swaps twice per game frame, so each screen alternates
 * between the fresh frame and a stale back-buffer frame -- full-screen
 * flicker on both Cemu windows (plus two vsync waits per frame). A window
 * created with PREVENT_SWAP copies its colour buffer to its scan buffer
 * without swapping, so the dual-screen loop presents the GamePad renderer
 * first (no swap) and the TV renderer last (the frame's single swap).
 * Older portlibs ignore the bit (flicker remains there); rebuild against
 * current wiiu-sdl2 to pick the fix up. */
#ifndef SDL_WINDOW_WIIU_PREVENT_SWAP
#define SDL_WINDOW_WIIU_PREVENT_SWAP 0x04000000u
#endif

/* GamePad joystick button indices (vpad_button_map order). */
typedef enum {
    FNAE_PAD_A = 0,    /* confirm / advance */
    FNAE_PAD_B = 1,    /* flashlight (hold) */
    FNAE_PAD_X = 2,    /* audio lure (camera up) */
    FNAE_PAD_Y = 3,    /* mask */
    FNAE_PAD_STICK_L = 4,
    FNAE_PAD_STICK_R = 5,
    FNAE_PAD_L = 6,    /* left door */
    FNAE_PAD_R = 7,    /* right door */
    FNAE_PAD_ZL = 8,   /* cameras open/close */
    FNAE_PAD_ZR = 9,   /* music-box wind (hold, Cam 04) */
    FNAE_PAD_PLUS = 10,  /* confirm / advance */
    FNAE_PAD_MINUS = 11, /* mute phone call */
    FNAE_PAD_LEFT = 12,
    FNAE_PAD_UP = 13,
    FNAE_PAD_RIGHT = 14,
    FNAE_PAD_DOWN = 15
} FnaePadButton;

/* Asset root probe (Wii U only): content can live at fs:/vol/content
 * (.wuhb bundle, code/content/meta title folder) or next to the binary
 * (dev working dir, flat HBL folder). Probe once for the title card and
 * pin the prefix every image/audio path goes through, so the game boots
 * no matter which layout launched it. Everywhere else this is "" — the
 * desktop layout never changes. */
static inline const char *fnae_asset_root(void) {
#ifdef __WIIU__
    static char root[64] = {0};
    static int probed = 0;
    if (!probed) {
        probed = 1;
        FILE *f = fopen("fs:/vol/content/assets/images/515.png", "rb");
        if (f) {
            fclose(f);
            strcpy(root, "fs:/vol/content/");
        } else {
            root[0] = '\0';
        }
    }
    return root;
#else
    return "";
#endif
}

/* Pads that act as held states (flashlight / wind) report down=1 on press
 * and down=0 on release; every other pad only fires on press (down=1). */
static inline void fnae_pad_button(FnaeGame *g, int btn, int down) {
    switch (btn) {
    case FNAE_PAD_ZL: /* cameras (S key equivalent) */
        if (down) fnae_key(g, 's');
        break;
    case FNAE_PAD_L: /* left door (A key equivalent) */
        if (down) fnae_key(g, 'a');
        break;
    case FNAE_PAD_R: /* right door (D key equivalent) */
        if (down) fnae_key(g, 'd');
        break;
    case FNAE_PAD_Y: /* mask (M key equivalent) */
        if (down) fnae_key(g, 'm');
        break;
    case FNAE_PAD_B: /* flashlight (Z key equivalent, hold) */
        if (down) fnae_key(g, 'z');
        else fnae_key_up(g, 'z');
        break;
    case FNAE_PAD_X: /* audio lure (E key equivalent) */
        if (down) fnae_key(g, 'e');
        break;
    case FNAE_PAD_ZR: /* music-box wind (R key equivalent, hold) */
        if (down) fnae_key(g, 'r');
        else fnae_key_up(g, 'r');
        break;
    case FNAE_PAD_A: /* confirm / advance */
    case FNAE_PAD_PLUS:
        if (down) fnae_key(g, SDLK_RETURN);
        break;
    case FNAE_PAD_MINUS: /* mute call: same zone as the MUTE CALL button */
        if (down) fnae_click(g, 100, 55);
        break;
    case FNAE_PAD_UP:
        if (down) fnae_key(g, SDLK_UP);
        break;
    case FNAE_PAD_DOWN:
        if (down) fnae_key(g, SDLK_DOWN);
        break;
    case FNAE_PAD_LEFT:
        if (down) {
            if (g->frame == FRAME_NIGHT && g->view > 0 && g->hidden_power > 0 && g->death == 0) {
                if (g->camera > 1) {
                    g->camera--;
                    fnae_push_sound(g, FNAE_SND_TITLE_CHANGE);
                }
            } else {
                fnae_key(g, SDLK_LEFT);
            }
        }
        break;
    case FNAE_PAD_RIGHT:
        if (down) {
            if (g->frame == FRAME_NIGHT && g->view > 0 && g->hidden_power > 0 && g->death == 0) {
                if (g->camera < 4) {
                    g->camera++;
                    fnae_push_sound(g, FNAE_SND_TITLE_CHANGE);
                }
            } else {
                fnae_key(g, SDLK_RIGHT);
            }
        }
        break;
    default:
        break;
    }
}

/* Left-stick X (SDL joystick units, -32768..32767): drives office panning
 * through the same pointer zones as the mouse, so no core changes are
 * needed. Centered stick parks the pointer mid-screen (no pan). */
static inline void fnae_pad_stick(FnaeGame *g, int x) {
    if (x < -8000) fnae_mouse_move(g, 100, 360);
    else if (x > 8000) fnae_mouse_move(g, 1180, 360);
    else fnae_mouse_move(g, 640, 360);
}

/* GamePad touchscreen taps (1280x720 space): press+click on touch down,
 * pointer tracking while held (music-box crank), release on lift. */
static inline void fnae_pad_touch(FnaeGame *g, int phase, int x, int y) {
    if (phase == 0) { /* down */
        fnae_press(g, x, y);
        fnae_click(g, x, y);
    } else if (phase == 1) { /* motion */
        fnae_mouse_move(g, x, y);
    } else { /* up */
        fnae_release(g);
    }
}
