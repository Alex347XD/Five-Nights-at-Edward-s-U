#pragma once

#include "fnae_core.h"

/* SDL_mixer sound bank. Channel numbers mirror the Fusion Sound object
 * channels from Frame 3 Events.txt ([ Audio ], [ Audio Lure ], ...).
 * Core pushes one-shots into FnaeGame.snd_queue; this module drains them
 * and derives loops/volumes by polling game state (core-owns-state). */
typedef struct FnaeAudio FnaeAudio;

struct FnaeAudio {
 int ok; /* 0 = silent stub (init failed or headless without audio) */
 /* Wavetable: NULL entries are skipped, never crash. */
 struct Mix_Chunk *fan, *depths, *camau, *change, *flip_up, *flip_down;
 struct Mix_Chunk *mask_on, *mask_off, *breath, *door, *melody, *stare;
 struct Mix_Chunk *buzz, *windup, *thud, *steps, *closeamb, *echo1, *echo3b;
 struct Mix_Chunk *echo4b, *stop, *walk, *garble, *phbb, *powerdown, *jack;
 struct Mix_Chunk *manglebreath; /* breathing.wav: Phantom Mangle annoy end (ch #17) */
 struct Mix_Chunk *puppet, *freddy, *foxy, *spring, *gf;
 struct Mix_Chunk *title_static, *darkness, *finalbox, *chimes, *goblin;
 struct Mix_Chunk *call1, *call2, *call3, *call4, *call5, *call6;
 /* Previous-frame state for edge detection. */
 int init, prev_frame, prev_night, prev_death, prev_view, prev_camera;
 int prev_call, prev_power, prev_empty, prev_spring, prev_fdoor, prev_fstand;
 int prev_mangle, prev_mangle_act; /* garble-loop activity + annoy-end edge */
 int prev_annoy_end; /* set once Annoy B hits 7 (breathing played) */
 int prev_move; /* movement_out edge (Connection Lost sets Change=1) */
 int prev_foxy_pos, prev_freddy_pos; /* doorway-knock edges (view-independent) */
};

/* Loads every sample under dir, resolved against fnae_asset_root()
 * (e.g. "audio" -> "assets/audio" on desktop). Reports staged progress
 * like visuals_init (NULL = silent). Never fatal:
 * on failure ok=0 and fnae_audio_frame is a silent no-op. */
int fnae_audio_init(FnaeAudio *a, const char *dir, FnaeLoadProgress progress, void *ctx);
void fnae_audio_free(FnaeAudio *a);
/* Drain the core queue, restart loops on frame edges, refresh volumes. */
void fnae_audio_frame(FnaeAudio *a, FnaeGame *g);
/* Nonzero while a phone call is playing (drives the Mute Call button). */
int fnae_audio_call_playing(FnaeAudio *a);
