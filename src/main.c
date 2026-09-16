#define SDL_MAIN_HANDLED

#include <stdio.h>
#include <stdlib.h>
#include <SDL.h>
#include <SDL_image.h>

#include "fnae_core.h"
#include "headless.h"
#include "visuals.h"
#include "audio.h"
#include "wiiu.h"

#include <string.h>

static void print_usage(const char *prog) {    printf("Usage: %s [--headless] [--frames N] [--screenshot PATH] [--script PATH]\n", prog);
    printf("  --headless          run without a visible window (hidden window,\n");
    printf("                      software renderer fallback, fixed 1/60 dt)\n");
    printf("  --frames N          headless frame count (default 600)\n");
    printf("  --screenshot PATH   save final frame (.png or .bmp,\n");
    printf("                      default screenshots/headless.png)\n");
    printf("  --script PATH       headless input script (key/keyup/click/mouse/shot\n");
    printf("                      events by frame; see scripts/headless/example.txt)\n");
    printf("Screenshots land under screenshots/; clean them with:\n");
    printf("  cmake --build build --target clean-screenshots\n");
}

#ifdef __WIIU__
/* Wii U dual-screen loop: office/everything-else on the TV window, camera
 * feeds on the GamePad (DRC) window. The sdl-wiiu port routes each window
 * to its screen via SDL_WINDOW_WIIU_TV_ONLY / GAMEPAD_ONLY (see src/wiiu.h
 * for the values); each window needs its own renderer + texture set.
 * Presents are ordered DRC-then-TV so the frame performs exactly one
 * GX2SwapScanBuffers (see below); two swaps per frame flickered on Cemu. */
static int run_wiiu_dualscreen(FnaeGame *game, FnaeAudio *audio) {
    SDL_Window *wtv = SDL_CreateWindow(
        "Five Nights at Edward's",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        1280, 720, SDL_WINDOW_SHOWN | SDL_WINDOW_WIIU_TV_ONLY);
    /* The GamePad window carries PREVENT_SWAP (see src/wiiu.h): its
     * Present copies the camera image to the DRC scan buffer without
     * swapping, and the TV Present below performs the frame's single
     * GX2SwapScanBuffers. Swapping in both Presents flickered TV and DRC
     * on Cemu (each screen alternated fresh/stale every swap). */
    SDL_Window *wdrc = SDL_CreateWindow(
        "Five Nights at Edward's - Cameras",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        1280, 720, SDL_WINDOW_SHOWN | SDL_WINDOW_WIIU_GAMEPAD_ONLY |
                   SDL_WINDOW_WIIU_PREVENT_SWAP);
    if (!wtv || !wdrc) {
        fprintf(stderr, "WiiU CreateWindow failed: %s\n", SDL_GetError());
        if (wtv) SDL_DestroyWindow(wtv);
        if (wdrc) SDL_DestroyWindow(wdrc);
        return 1;
    }
    SDL_Renderer *rtv = SDL_CreateRenderer(
        wtv, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    SDL_Renderer *rdrc = SDL_CreateRenderer(
        wdrc, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!rtv || !rdrc) {
        if (!rtv) rtv = SDL_CreateRenderer(wtv, -1, SDL_RENDERER_SOFTWARE);
        if (!rdrc) rdrc = SDL_CreateRenderer(wdrc, -1, SDL_RENDERER_SOFTWARE);
    }
    if (!rtv || !rdrc) {
        fprintf(stderr, "WiiU CreateRenderer failed: %s\n", SDL_GetError());
        if (rtv) SDL_DestroyRenderer(rtv);
        if (rdrc) SDL_DestroyRenderer(rdrc);
        SDL_DestroyWindow(wtv);
        SDL_DestroyWindow(wdrc);
        return 1;
    }

    FnaeVisuals vtv, vdrc;
    if (visuals_init(&vtv, rtv) != 0 || visuals_init(&vdrc, rdrc) != 0) {
        visuals_free(&vtv);
        visuals_free(&vdrc);
        SDL_DestroyRenderer(rtv);
        SDL_DestroyRenderer(rdrc);
        SDL_DestroyWindow(wtv);
        SDL_DestroyWindow(wdrc);
        return 1;
    }

    /* GamePad is joystick 0 in the sdl-wiiu port (VPAD). Touchscreen taps
     * arrive as finger events; the left stick pans the office. */
    SDL_InitSubSystem(SDL_INIT_JOYSTICK);
    SDL_Joystick *pad = NULL;
    if (SDL_NumJoysticks() > 0)
        pad = SDL_JoystickOpen(0);

    Uint64 last = SDL_GetPerformanceCounter();
    float static_acc = 0.0f;
    while (game->running) {
        Uint64 now = SDL_GetPerformanceCounter();
        float dt = (float)((double)(now - last) /
                           (double)SDL_GetPerformanceFrequency());
        last = now;
        if (dt > 0.1f) dt = 0.1f;

        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                game->running = 0;
            } else if (e.type == SDL_JOYBUTTONDOWN) {
                fnae_pad_button(game, (int)e.jbutton.button, 1);
            } else if (e.type == SDL_JOYBUTTONUP) {
                fnae_pad_button(game, (int)e.jbutton.button, 0);
            } else if (e.type == SDL_JOYAXISMOTION && e.jaxis.axis == 0) {
                fnae_pad_stick(game, (int)e.jaxis.value);
            } else if (e.type == SDL_FINGERDOWN) {
                fnae_pad_touch(game, 0,
                    (int)(e.tfinger.x * 1280.0f), (int)(e.tfinger.y * 720.0f));
            } else if (e.type == SDL_FINGERMOTION) {
                fnae_pad_touch(game, 1,
                    (int)(e.tfinger.x * 1280.0f), (int)(e.tfinger.y * 720.0f));
            } else if (e.type == SDL_FINGERUP) {
                fnae_pad_touch(game, 2, 0, 0);
            }
        }

        fnae_update(game, dt);
        /* Fixed 1/60 static steps (see the desktop loop): keeps the static
         * cadence at real-time speed on any display refresh. */
        static_acc += dt;
        while (static_acc >= 1.0f / 60.0f) {
            static_acc -= 1.0f / 60.0f;
            fnae_static_tick(game);
        }
        fnae_audio_frame(audio, game);

        int cam_up = game->frame == FRAME_NIGHT && game->cam_anim == CAM_UP;
        /* TV shows everything with the cameras forced down (office view). */
        visuals_render(
            &vtv, rtv,
            (int)game->frame,
            game->camera,
            0,
            game->night,
            game->time_of_day,
            game->hidden_power,
            game->left_door,
            game->right_door,
            game->mask_anim == MASK_DOWN,
            game->arrow,
            game->progress,
            game->static_frame,
            game->static_alpha,
            (int)game->office_scroll,
            game->left_door_frame,
            game->right_door_frame,
            game->mask_frame,
            game->title_bg_frame,
            game->six_timer,
            game->foxy.pos,
            game->freddy.pos,
            game->cam_static_alpha,
            game->death,
            game->music_left,
            (int)game->cam_scroll,
            game->power_left,
            game->springtrap_stand,
            game->lure_area,
            game->lure_cam,
            game->lure_cd,
            game->lure_cd_timer,
            game->music_winding,
            game->warning,
            fnae_audio_call_playing(audio),
            game->ph_mangle_a == 1,
            game->ph_bb_a == 1,
            game->ph_bb_scare,
            game->ph_bb_scare_on,
            game->ph_annoy_a,
            game->death_addup,
            game->death_red,
            game->death_red_peaked,
            game->death_rip_a,
            game->death_rip_b,
            game->death_ticks,
            game->gf_random == 1,
            game->freddy_door,
            game->foxy_stand,
            (const int[]){game->custom_freddy, game->custom_foxy,
                game->custom_springtrap, game->custom_golden,
                game->custom_mangle, game->custom_bb, game->custom_puppet},
            game->custom_sel, game->custom_ch, game->custom_b,
            (game->custom_ch > 0 && game->custom_ch <= 3
                && game->custom_check[game->custom_ch]) ? 1 : 0,
            game->custom_cool,
            game->movement_out
        );

        if (cam_up) {
            /* GamePad shows the live camera UI (feed + minimap + lure +
             * music box), exactly like the single-screen camera view. */
            visuals_render(
                &vdrc, rdrc,
                (int)game->frame,
                game->camera,
                1,
                game->night,
                game->time_of_day,
                game->hidden_power,
                game->left_door,
                game->right_door,
                0,
                game->arrow,
                game->progress,
                game->static_frame,
                game->static_alpha,
                (int)game->office_scroll,
                game->left_door_frame,
                game->right_door_frame,
                game->mask_frame,
                game->title_bg_frame,
                game->six_timer,
                game->foxy.pos,
                game->freddy.pos,
                game->cam_static_alpha,
                game->death,
                game->music_left,
                (int)game->cam_scroll,
                game->power_left,
                game->springtrap_stand,
                game->lure_area,
                game->lure_cam,
                game->lure_cd,
                game->lure_cd_timer,
                game->music_winding,
                game->warning,
                fnae_audio_call_playing(audio),
                game->ph_mangle_a == 1,
                game->ph_bb_a == 1,
                game->ph_bb_scare,
                game->ph_bb_scare_on,
                game->ph_annoy_a,
                game->death_addup,
                game->death_red,
                game->death_red_peaked,
                game->death_rip_a,
                game->death_rip_b,
                game->death_ticks,
                game->gf_random == 1,
                game->freddy_door,
                game->foxy_stand,
                NULL,
                0, 0, 0, 0, 0,
                game->movement_out
            );
        } else {
            /* Cameras closed: just a black screen on the GamePad. */
            SDL_SetRenderDrawColor(rdrc, 0, 0, 0, 255);
            SDL_RenderClear(rdrc);
        }
        /* One GX2SwapScanBuffers per game frame: the GamePad window was
         * created with PREVENT_SWAP, so this copies the DRC image without
         * swapping, and the TV Present below flips both scan buffers with
         * fresh images on each side. Presenting the TV first (or swapping
         * in both) alternated each screen between fresh and stale buffers
         * -- the Cemu flicker on both windows. */
        SDL_RenderPresent(rdrc);
        SDL_RenderPresent(rtv);
        /* Hold ~60 presents/s when vsync is unavailable (see desktop loop). */
        {
            Uint64 end = SDL_GetPerformanceCounter();
            double elapsed = (double)(end - now) /
                             (double)SDL_GetPerformanceFrequency();
            double target = 1.0 / 60.0;
            if (elapsed < target)
                SDL_Delay((Uint32)((target - elapsed) * 1000.0));
        }
    }

    if (pad) SDL_JoystickClose(pad);
    visuals_free(&vtv);
    visuals_free(&vdrc);
    SDL_DestroyRenderer(rtv);
    SDL_DestroyRenderer(rdrc);
    SDL_DestroyWindow(wtv);
    SDL_DestroyWindow(wdrc);
    return 0;
}
#endif /* __WIIU__ */

int main(int argc, char *argv[]) {
    int headless = 0;
    HeadlessOptions hopt;
    hopt.frames = 600;
    hopt.screenshot = "screenshots/headless.png";
    const char *script_path = NULL;

    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--headless") == 0) {
            headless = 1;
        } else if (strcmp(argv[i], "--frames") == 0 && i + 1 < argc) {
            hopt.frames = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--screenshot") == 0 && i + 1 < argc) {
            hopt.screenshot = argv[++i];
        } else if (strcmp(argv[i], "--script") == 0 && i + 1 < argc) {
            script_path = argv[++i];
        } else if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
            print_usage(argv[0]);
            return 0;
        } else {
            fprintf(stderr, "Unknown argument: %s\n", argv[i]);
            print_usage(argv[0]);
            return 2;
        }
    }
    if (hopt.frames < 1) hopt.frames = 1;

    printf("FNaE STARTED\n");
    if (headless)
        printf("HEADLESS frames=%d screenshot=%s script=%s\n",
            hopt.frames, hopt.screenshot, script_path ? script_path : "(built-in)");
    fflush(stdout);

    HeadlessScript script;
    memset(&script, 0, sizeof script);
    if (script_path && !headless) {
        fprintf(stderr, "Warning: --script ignored without --headless\n");
        script_path = NULL;
    }
    if (script_path && headless_load_script(script_path, &script) != 0)
        return 1;

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_EVENTS) != 0) {
        headless_free_script(&script);
        return 1;
    }
    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        headless_free_script(&script);
        SDL_Quit();
        return 1;
    }

    SDL_Window *w = SDL_CreateWindow(
        "Five Nights at Edward's",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        1280, 720, headless ? SDL_WINDOW_HIDDEN : SDL_WINDOW_SHOWN);

    if (!w) {
        fprintf(stderr, "SDL_CreateWindow failed: %s\n", SDL_GetError());
        headless_free_script(&script);
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    SDL_Renderer *r = SDL_CreateRenderer(
        w, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!r) {
        /* Accelerated without vsync (driver override) still beats software;
         * the main loop caps presents to ~60/s itself in that case. */
        r = SDL_CreateRenderer(w, -1, SDL_RENDERER_ACCELERATED);
    }
    if (!r) {
        /* Dummy video driver (headless/CI) has no accelerated renderer. */
        r = SDL_CreateRenderer(w, -1, SDL_RENDERER_SOFTWARE);
    }

    if (!r) {
        fprintf(stderr, "SDL_CreateRenderer failed: %s\n", SDL_GetError());
        headless_free_script(&script);
        SDL_DestroyWindow(w);
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    FnaeVisuals visuals;
    if (visuals_init(&visuals, r) != 0) {
        headless_free_script(&script);
        SDL_DestroyRenderer(r);
        SDL_DestroyWindow(w);
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    FnaeGame game;
    fnae_init(&game);

    /* Audio is best-effort: init failure means silent play, never a crash.
     * Works headless too (SDL_AUDIODRIVER=dummy), draining the queue. */
    FnaeAudio audio;
    fnae_audio_init(&audio, "assets/audio");

#ifdef __WIIU__
    /* On hardware the game runs dual-screen (TV + GamePad) through its own
     * windows/renderers instead of the single desktop window below.
     * Headless keeps the single-window path. */
    if (!headless) {
        int rc = run_wiiu_dualscreen(&game, &audio);
        headless_free_script(&script);
        fnae_audio_free(&audio);
        IMG_Quit();
        SDL_Quit();
        return rc;
    }
#endif

    if (headless) {
        int rc = headless_run(r, &visuals, &game, &hopt,
                              script_path ? &script : NULL, &audio);
        headless_free_script(&script);
        fnae_audio_free(&audio);
        visuals_free(&visuals);
        SDL_DestroyRenderer(r);
        SDL_DestroyWindow(w);
        IMG_Quit();
        SDL_Quit();
        return rc;
    }

    Uint64 last = SDL_GetPerformanceCounter();
    float static_acc = 0.0f;

    while (game.running) {
        Uint64 now = SDL_GetPerformanceCounter();
        float dt = (float)((double)(now-last) /
                           (double)SDL_GetPerformanceFrequency());
        last = now;
        if (dt > 0.1f) dt = 0.1f;

        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                game.running = 0;
            } else if (e.type == SDL_KEYDOWN && !e.key.repeat) {
                fnae_key(&game, e.key.keysym.sym);
            } else if (e.type == SDL_KEYUP && !e.key.repeat) {
                fnae_key_up(&game, e.key.keysym.sym);
            } else if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT) {
                fnae_press(&game, e.button.x, e.button.y);
                fnae_click(&game, e.button.x, e.button.y);
            } else if (e.type == SDL_MOUSEBUTTONUP && e.button.button == SDL_BUTTON_LEFT) {
                fnae_release(&game);
            } else if (e.type == SDL_MOUSEMOTION) {
                fnae_mouse_move(&game, e.motion.x, e.motion.y);
            }
        }

        fnae_update(&game, dt);
        /* TV-static + title-flash timing is defined in Fusion ticks (1/60):
         * stepping them here keeps the ~20 fps static and the 0.2 s flash
         * at real-time speed on any refresh rate instead of strobing. */
        static_acc += dt;
        while (static_acc >= 1.0f / 60.0f) {
            static_acc -= 1.0f / 60.0f;
            fnae_static_tick(&game);
        }
        fnae_audio_frame(&audio, &game);

        visuals_render(
            &visuals, r,
            (int)game.frame,
            game.camera,
            game.cam_anim == CAM_UP,
            game.night,
            game.time_of_day,
            game.hidden_power,
            game.left_door,
            game.right_door,
            game.mask_anim == MASK_DOWN,
            game.arrow,
            game.progress,
            game.static_frame,
            game.static_alpha,
            (int)game.office_scroll,
            game.left_door_frame,
            game.right_door_frame,
            game.mask_frame,
            game.title_bg_frame,
            game.six_timer,
            game.foxy.pos,
            game.freddy.pos,
            game.cam_static_alpha,
            game.death,
            game.music_left,
            (int)game.cam_scroll,
            game.power_left,
            game.springtrap_stand,
            game.lure_area,
            game.lure_cam,
            game.lure_cd,
            game.lure_cd_timer,
            game.music_winding,
            game.warning,
            fnae_audio_call_playing(&audio),
            game.ph_mangle_a == 1,
            game.ph_bb_a == 1,
            game.ph_bb_scare,
            game.ph_bb_scare_on,
            game.ph_annoy_a,
            game.death_addup,
            game.death_red,
            game.death_red_peaked,
            game.death_rip_a,
            game.death_rip_b,
            game.death_ticks,
            game.gf_random == 1,
            game.freddy_door,
            game.foxy_stand,
            (const int[]){game.custom_freddy, game.custom_foxy,
                game.custom_springtrap, game.custom_golden,
                game.custom_mangle, game.custom_bb, game.custom_puppet},
            game.custom_sel, game.custom_ch, game.custom_b,
            (game.custom_ch > 0 && game.custom_ch <= 3
                && game.custom_check[game.custom_ch]) ? 1 : 0,
            game.custom_cool,
            game.movement_out
        );

        SDL_RenderPresent(r);
        /* When vsync is unavailable (software fallback, driver override)
         * Present returns immediately and the loop would spin at 1000+ fps,
         * tearing and strobing every per-frame effect: hold ~60 presents/s.
         * With working vsync this never fires (a present already took the
         * whole budget). */
        {
            Uint64 end = SDL_GetPerformanceCounter();
            double elapsed = (double)(end - now) /
                             (double)SDL_GetPerformanceFrequency();
            double target = 1.0 / 60.0;
            if (elapsed < target)
                SDL_Delay((Uint32)((target - elapsed) * 1000.0));
        }
    }

    headless_free_script(&script);
    fnae_audio_free(&audio);
    visuals_free(&visuals);
    SDL_DestroyRenderer(r);
    SDL_DestroyWindow(w);
    IMG_Quit();
    SDL_Quit();
    return 0;
}
