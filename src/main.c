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
/* Loading-screen state for the staged dualscreen texture init (two full
 * texture sets, one per renderer -- minutes on hardware SD). */
struct FnaeLoadCtx {
    SDL_Renderer *rtv;
    SDL_Renderer *rdrc;
    int base;  /* percent offset for this init phase */
    int span;  /* percent range for this init phase */
    int abort; /* set on quit request */
    /* Night-transition loads hide the bar: the hook repaints the static
     * Which Night card instead (needs the visuals + game for it). */
    int card;
    FnaeVisuals *vtv;
    FnaeGame *game;
};

/* Progress hook (see visuals.h): paints a loading bar on both screens
 * between texture groups. Needs no textures -- nothing is loaded yet.
 * Returns nonzero to abort the init. */
static int dualscreen_load_progress(int percent, void *ctx) {
    struct FnaeLoadCtx *c = (struct FnaeLoadCtx *)ctx;
    int p = c->base + percent * c->span / 100;
    if (p < 0) p = 0;
    if (p > 100) p = 100;

    if (c->card) {
        /* Night-transition load: static Which Night card on the TV (its
         * texture came with the title phase), black GamePad. No bar. */
        draw_which_night(c->rtv, c->vtv, c->game->night);
        SDL_SetRenderDrawColor(c->rdrc, 0, 0, 0, 255);
        SDL_RenderClear(c->rdrc);
        SDL_RenderPresent(c->rdrc);
        SDL_RenderPresent(c->rtv);
        return 0;
    }

    /* DRC first, then TV: same single-swap discipline as the game loop
     * (DRC copies without swapping, TV flips both scan buffers). No OS
     * pump here (or anywhere: ProcUIProcessMessages hangs on hardware,
     * so the title runs legacy mode with no ProcUI registration at all);
     * quit during load is not honored (abort stays 0). */
    SDL_Renderer *rs[2] = { c->rdrc, c->rtv };
    for (int i = 0; i < 2; ++i) {
        SDL_Renderer *r = rs[i];
        SDL_SetRenderDrawColor(r, 0, 0, 0, 255);
        SDL_RenderClear(r);
        SDL_SetRenderDrawColor(r, 255, 255, 255, 255);
        SDL_Rect frame = { 340, 340, 600, 40 };
        SDL_RenderDrawRect(r, &frame);
        SDL_Rect fill = { 344, 344, (600 - 8) * p / 100, 32 };
        if (fill.w > 0)
            SDL_RenderFillRect(r, &fill);
        SDL_RenderPresent(r);
    }
    return 0;
}

/* Wii U dual-screen loop: office/everything-else on the TV window, camera
 * feeds on the GamePad (DRC) window. The sdl-wiiu port routes each window
 * to its screen via SDL_WINDOW_WIIU_TV_ONLY / GAMEPAD_ONLY (see src/wiiu.h
 * for the values); each window needs its own renderer + texture set.
 * Presents are ordered DRC-then-TV so the frame performs exactly one
 * GX2SwapScanBuffers (see below); two swaps per frame flickered on Cemu. */
static int run_wiiu_dualscreen(FnaeGame *game) {
    /* TV + DRC windows only (a third window's scanbuffers would eat the
     * same GPU pool textures allocate from). */
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

    /* Boot loads the title phase only (bar 0-100%: TV title set, then
     * GamePad custom art): title + menus + customize + odometer + static.
     * The night sets stream in later at the Which Night transition,
     * behind the static card with no bar. Audio all loads here too (WAV
     * reads are fast, no inflate). Abort funnels into the existing
     * failure path -- free tolerates partial sets. */
    FnaeVisuals vtv, vdrc;
    FnaeAudio audio;
    memset(&audio, 0, sizeof audio);
    struct FnaeLoadCtx lctx;
    lctx.rtv = rtv;
    lctx.rdrc = rdrc;
    lctx.base = 0;
    lctx.span = 90;
    lctx.abort = 0;
    lctx.card = 0;
    lctx.vtv = &vtv;
    lctx.game = game;
    /* Paint 0% before texture #1: from here on a silent screen always
     * means pre-video, never asset loading. */
    int rct = dualscreen_load_progress(0, &lctx) != 0;
    int rctd = rct;
    if (rct == 0 && !lctx.abort) {
        rct = visuals_init_title(&vtv, rtv, dualscreen_load_progress, &lctx, 0);
        rctd = rct;
    }
    if (rct == 0 && !lctx.abort) {
        lctx.base = 90;
        lctx.span = 10;
        rctd = visuals_init_title(&vdrc, rdrc, dualscreen_load_progress, &lctx, 1);
    }
    if (rct == 0 && !lctx.abort) {
        fnae_audio_init(&audio, "audio", NULL, NULL);
    }
    if (rct != 0 || rctd != 0 || lctx.abort) {
        fnae_audio_free(&audio);
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
    int night_loaded = 0; /* night sets stream in once, at the transition */
    /* No ProcUI pump/gate anywhere: ProcUIProcessMessages never returns
     * on hardware, and InForeground / IsRunning only refresh via the
     * pump, so the title runs legacy mode (no ProcUI registration) and
     * the OS owns HOME transitions outright. The game runs flat-out. */
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
            } else if (e.type == SDL_JOYDEVICEADDED) {
                /* GamePad (re)connect or late attach: (re)open it. Without
                 * this a reconnect leaves a dead handle and all inputs
                 * stop, and a pad attached after boot never works. */
                if (!pad) pad = SDL_JoystickOpen(e.jdevice.which);
            } else if (e.type == SDL_JOYDEVICEREMOVED) {
                /* Drop the stale handle now; ADDED reopens on reconnect. */
                if (pad) {
                    SDL_JoystickClose(pad);
                    pad = NULL;
                }
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

        /* Night-transition load (once): runs only after the 2 s card
         * read, extending the card as long as loading takes -- 2 seconds
         * + load time if needed, never load-then-2. The card texture came
         * with the title phase, so it draws while the rest loads; input
         * during the load applies after. Later nights skip (resident).
         * One shot even on failure (a retry loop would hang the card
         * forever on persistent errors; render tolerates missing art). */
        if (!night_loaded && game->frame == FRAME_WHICH_NIGHT && game->which_timer >= 2.0f) {
            struct FnaeLoadCtx nctx;
            nctx.rtv = rtv;
            nctx.rdrc = rdrc;
            nctx.base = 0;
            nctx.span = 100;
            nctx.abort = 0;
            nctx.card = 1;
            nctx.vtv = &vtv;
            nctx.game = game;
            visuals_init_night(&vtv, rtv, dualscreen_load_progress, &nctx, 0);
            visuals_init_night(&vdrc, rdrc, dualscreen_load_progress, &nctx, 1);
            night_loaded = 1;
        }

        fnae_update(game, dt);
        /* Fixed 1/60 static steps (see the desktop loop): keeps the static
         * cadence at real-time speed on any display refresh. */
        static_acc += dt;
        while (static_acc >= 1.0f / 60.0f) {
            static_acc -= 1.0f / 60.0f;
            fnae_static_tick(game);
        }
        fnae_audio_frame(&audio, game);

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
            fnae_audio_call_playing(&audio),
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
            game->movement_out,
            -1 /* cam flip flash lives on the GamePad (DRC), never the TV */
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
                fnae_audio_call_playing(&audio),
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
                game->movement_out,
                game->cam_flip_frame
            );
        } else if (game->frame == FRAME_CUSTOMIZE) {
            /* Custom night shows on the GamePad too: same UI as the TV,
             * rendered from the DRC texture subset (customize art loads
             * in both sets). */
            visuals_render(
                &vdrc, rdrc,
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
                fnae_audio_call_playing(&audio),
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
                game->movement_out,
                -1
            );
        } else if (game->frame == FRAME_NIGHT && game->cam_flip_frame >= 0) {
            /* The flip-flash blip opens the cams: while it plays (the
             * 0.55 s CAM_UP/DOWN_ANIM transitions) the GamePad shows just
             * the blip over black -- no feed until the flip lands -- while
             * the TV stays on the office. */
            SDL_SetRenderDrawColor(rdrc, 0, 0, 0, 255);
            SDL_RenderClear(rdrc);
            visuals_draw_cam_flip(&vdrc, rdrc, game->cam_flip_frame);
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
    fnae_audio_free(&audio);
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

#ifdef __WIIU__
    /* Hardware fast-path: straight into dual-screen. The desktop window /
     * renderer / texture set below would triple texture memory and then
     * be thrown away. Headless keeps the single-window path. */
    if (!headless) {
        FnaeGame wgame;
        fnae_init(&wgame);
        /* Night sets load at the transition (see the loop gate): block
         * the Which Night auto-start until they land. */
        wgame.night_ready = 0;
        int wrc = run_wiiu_dualscreen(&wgame);
        headless_free_script(&script);
        IMG_Quit();
        SDL_Quit();
        return wrc;
    }
#endif

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
    /* Desktop/headless loads from SSD in seconds: no progress hook, full set. */
    if (visuals_init(&visuals, r, NULL, NULL, 0) != 0) {
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
    /* Bare "audio": fnae_asset_root() already ends at the assets dir. */
    fnae_audio_init(&audio, "audio", NULL, NULL);

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
            game.movement_out,
            game.cam_flip_frame
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
