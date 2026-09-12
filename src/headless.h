#pragma once

#include <SDL.h>

#include "fnae_core.h"
#include "visuals.h"

typedef struct {
    int frames;              /* fixed-step frame count (default 600) */
    const char *screenshot;  /* output path, .png or .bmp (default "screenshots/headless.png") */
} HeadlessOptions;

/* Runs the scripted headless session (fixed 1/60 dt) and saves the final
 * frame. Returns 0 on success, nonzero on failure. */
int headless_run(SDL_Renderer *r, FnaeVisuals *v, FnaeGame *game,
                 const HeadlessOptions *opt);

/* Saves the renderer's current output. Creates missing parent directories.
 * ".png" suffix uses IMG_SavePNG, anything else uses SDL_SaveBMP. */
int headless_save_screenshot(SDL_Renderer *r, const char *path);
