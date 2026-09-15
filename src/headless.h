#pragma once

#include <SDL.h>

#include "fnae_core.h"
#include "visuals.h"
#include "audio.h"

typedef struct {
    int frames;              /* fixed-step frame count (default 600) */
    const char *screenshot;  /* output path, .png or .bmp (default "screenshots/headless.png") */
} HeadlessOptions;

/* Script-file input event. Script format (one per line):
 *   <frame> key <KEY>     press a key (KEY: return/enter, esc/escape, space,
 *                             up/down/left/right, or a single character)
 *   <frame> keyup <KEY>   release a key
 *   <frame> click <X> <Y> left-click at window coordinates
 *   <frame> mouse <X> <Y> move the pointer (no click; drives office panning)
 *   <frame> shot <PATH>   save the current frame (extra screenshots mid-run)
  *   <frame> ai <WHO> <POS> debug pose: place an animatronic (WHO: freddy,
  *                             foxy) at route position POS (freddy 6 = left
  *                             door, foxy 5 = right door)
  *   <frame> clock <HOUR> debug warp: jump the night clock to HOUR with 5 s
  *                             left in it (captures 6 AM without playing the
  *                             full shift)
 *   <frame> pad <BTN> <down|up> GamePad button event (BTN: 0-15 in vpad
 *                             order A B X Y STICK_L STICK_R L R ZL ZR PLUS
 *                             MINUS LEFT UP RIGHT DOWN, see src/wiiu.h)
 * Blank lines and '#' comments are ignored. <frame> is the 60fps frame
 * index at which the event fires. */

typedef enum {
    HEV_KEY,
    HEV_KEYUP,
    HEV_CLICK,
    HEV_MOUSE,
    HEV_SHOT,
    HEV_AI,
    HEV_CLOCK,
    HEV_PAD
} HeadlessEvType;

typedef struct {
    int frame;
    HeadlessEvType type;
    int key;           /* HEV_KEY/HEV_KEYUP: SDL_Keycode */
    int x, y;          /* HEV_CLICK/HEV_MOUSE: window coordinates;
                         * HEV_AI: x = 0 freddy / 1 foxy, y = route pos;
                         * HEV_CLOCK: x = hour to jump to;
                         * HEV_PAD: x = button index, y = down (1) / up (0) */
    char shot[256];    /* HEV_SHOT: output path */
} HeadlessEvent;

typedef struct {
    HeadlessEvent *events; /* malloc'd array, free with headless_free_script */
    int count;
} HeadlessScript;

/* Loads a script file. Returns 0 on success; prints the error (with line
 * number) and returns nonzero on failure. */
int headless_load_script(const char *path, HeadlessScript *out);
void headless_free_script(HeadlessScript *s);

/* Runs the scripted headless session (fixed 1/60 dt) and saves the final
 * frame. If script is NULL, a built-in demo sequence is used.
 * audio may be NULL (silent); otherwise it is pumped every tick so the
 * sound queue is exercised under the dummy audio driver.
 * Returns 0 on success, nonzero on failure. */
int headless_run(SDL_Renderer *r, FnaeVisuals *v, FnaeGame *game,
                 const HeadlessOptions *opt, const HeadlessScript *script,
                 FnaeAudio *audio);

/* Saves the renderer's current output. Creates missing parent directories.
 * ".png" suffix uses IMG_SavePNG, anything else uses SDL_SaveBMP. */
int headless_save_screenshot(SDL_Renderer *r, const char *path);
