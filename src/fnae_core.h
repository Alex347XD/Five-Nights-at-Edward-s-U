#pragma once
#include <stdint.h>

typedef enum { FRAME_WARNING=1, FRAME_TITLE=2, FRAME_NIGHT=3, FRAME_DEATH=4, FRAME_FINAL=5, FRAME_WHICH_NIGHT=6, FRAME_NEWSPAPER=7, FRAME_CUSTOMIZE=8, FRAME_6AM=9 } FnaeFrame;
typedef enum { CAM_DOWN=0, CAM_UP_ANIM=1, CAM_UP=2, CAM_DOWN_ANIM=3 } CamAnim;
typedef enum { MASK_UP=0, MASK_UP_ANIM=1, MASK_DOWN=2, MASK_DOWN_ANIM=3 } MaskAnim;

typedef struct { int ai, pos, move, at_door; } FnaeAI;

/* One-shot sound requests: core pushes, audio drains (see src/audio.c).
 * Channel numbers mirror the Fusion Sound object channels. */
typedef enum {
 FNAE_SND_LURE1 = 1, /* echo1 on ch #14 */
 FNAE_SND_LURE2,     /* echo3b on ch #14 */
 FNAE_SND_LURE3,     /* echo4b on ch #14 */
 FNAE_SND_LURE_STOP, /* stop sample on ch #14 (Animation 12 over) */
 FNAE_SND_DOOR,      /* SFXBible_12478 on ch #7 */
 FNAE_SND_CAM_UP,    /* STEREO_CASSETTE__90097704 on ch #5 */
 FNAE_SND_CAM_DOWN,  /* STEREO_CASSETTE__90097701 on ch #5 */
 FNAE_SND_MASK_ON,   /* FENCING_43 on ch #5 */
 FNAE_SND_MASK_OFF,  /* FENCING_42 on ch #5 */
 FNAE_SND_WINDUP,    /* windup2 on ch #11 (every 0.50 s while winding) */
 FNAE_SND_TITLE_CHANGE, /* Change blip (title moves = ch #3, cam switch = ch #4) */
 FNAE_SND_CALL_STOP, /* halt the phone call on ch #16 (Mute Call button) */
 FNAE_SND_PHBB /* scream3 on ch #18 (Phantom BB 80-tick scare) */
} FnaeSound;

#define FNAE_SND_QUEUE 32
typedef struct {
 FnaeFrame frame; int running; int night; int six_or_seven; int arrow; int progress; int challenge;
 int time_of_day; float time_to_hour;
 float which_timer; /* Frame 6 auto-advance: Every 02'' -> Night */
 float warn_timer; /* Frame 1 auto-advance: Timer equals 05'' -> Title */
 int death; int death_addup; int gf_random; int gf_death_addup;
 int camera; CamAnim cam_anim; MaskAnim mask_anim; int prevent_flip; int force_down; int view;
 int left_door, right_door; int flashlight; int pc_mobile;
 int hidden_power; int power_left; float power_tick;
 int movement_out; float movement_timer; int camera_up_check;
 int foxy_stand; int freddy_door;
 FnaeAI foxy, freddy; int springtrap_a, springtrap_b; int springtrap_alive;
    int ph_mangle_a, ph_mangle_b, ph_mangle_c; int ph_bb_a, ph_bb_b;
    int ph_prev_view, ph_prev_cam; /* edge detect: phantoms roll once on cam open/switch, stable while viewing */
   int ph_bb_scare, ph_bb_scare_on; /* Scare overlay alpha 0-255 (+7/tick while <255, no gate) + first-trigger gate (see update_phantoms) */
   float ph_bb_scare_timer; /* hold at full opacity before the jumpscare clears */
  int ph_annoy_a, ph_annoy_b; /* office-annoy descent (A 0-224) / linger (B 0-7) once C==1 */
 int golden_ai, foxy_ai, freddy_ai, springtrap_ai, ph_mangle_ai, ph_bb_ai;
   int music_left; int music_winding; float music_tick;
   int current_call; int call_muted;
   int mouse_down; int key_wind; /* held inputs for the music-box crank */
   float windup_snd_tick; /* windup2 repeats every 0.50 s while winding */
   int snd_queue[FNAE_SND_QUEUE]; int snd_head, snd_tail; /* one-shot requests */
   int static_frame; int static_alpha; /* TV-static anim state (shared title/cameras) */
   int static_div; /* ticks since last static frame advance (slows 60 Hz ticks to ~20 fps) */
   int cam_static_alpha; float cam_static_tick; /* camera static: 150+Random(50) every 0.08s, 0 while signal lost */
   int warning; /* music-box warning level: 0 ok, 1 low (<600), 2 critical (<200), 3 empty */
   int power_out_alpha; /* Power Out overlay fade 255->0 once power is gone */
   int springtrap_pos; /* Springtrap camera index 1-4 */
   int springtrap_stand; /* Springtrap visible in office (viewing its cam) */
   int lure_area; int lure_cam; float lure_timer; /* audio-lure marker + pull delay */
   int lure_cd; float lure_cd_timer; /* lure-button cooldown (Animation 12): gates placement, independent of marker destroy */
   float movement_half_tick, movement_force_tick; /* Camera Out re-tune timers */
   int custom_freddy, custom_foxy, custom_springtrap, custom_golden;
   int custom_mangle, custom_bb, custom_puppet; /* Customize-frame AI levels for night 7 */
  int mouse_x, mouse_y;   /* last known pointer position (1280x720 space) */
   float office_scroll;    /* office pan in source px, 0 = leftmost.
                            * Mirrors the Fusion Office Center Object X minus
                            * Game Width / 2; clamped to [0, FNAE_OFFICE_SCROLL_MAX]. */
   float cam_scroll;       /* camera-feed pan: display left edge in source px.
                            * Drifts in [FNAE_CAM_SCROLL_MIN, FNAE_CAM_SCROLL_MAX]
                            * (clamped to the feed image ends, no overscan). */
   int cam_scroll_dir;     /* 0 = panning right (+1 px/tick), 1 = panning left */
  int left_door_frame, right_door_frame; /* shutter frame 0..15 (open->closed) */
   int title_bg_frame;     /* 0 = Stopped (515.png), 1..3 = flash 516.png+frame-1 */
   int title_bg_timer;     /* ticks the current flash frame has been held (0.2 s = 12 ticks) */
 int all20; int left_challenge, left_challenge_active;
 float cam_anim_timer, mask_anim_timer, left_door_timer, right_door_timer;
 float ai_timer, power_out_timer, springtrap_timer, phantom_timer;
 int hour_events[13][13];
} FnaeGame;

/* Office pan range: 1600px-wide office scene over a 1280px-wide view. */
#define FNAE_OFFICE_SCROLL_MAX 320

/* Camera-feed pan range: the 1600px-wide feed over the 1280px-wide view.
 * Unlike the raw Fusion Center-Object bounds (which overshoot 120 px each
 * end), the pan is clamped to the image ends so no black bars show. */
#define FNAE_CAM_SCROLL_MIN 0
#define FNAE_CAM_SCROLL_MAX 320

void fnae_init(FnaeGame* g);
void fnae_static_tick(FnaeGame* g);
void fnae_set_custom(FnaeGame* g,int freddy,int foxy,int springtrap,int golden,int mangle,int bb,int puppet);
void fnae_start_night(FnaeGame* g, int night);
void fnae_update(FnaeGame* g, float dt);
void fnae_key(FnaeGame* g, int key);
void fnae_key_up(FnaeGame* g, int key);
void fnae_click(FnaeGame* g, int x, int y);
void fnae_press(FnaeGame* g, int x, int y);
void fnae_release(FnaeGame* g);
void fnae_mouse_move(FnaeGame* g, int x, int y);
const char* fnae_frame_name(FnaeFrame f);
/* Pushes a one-shot sound request (drops it when the queue is full). */
void fnae_push_sound(FnaeGame* g, int snd);
/* Pops the oldest request, or -1 when the queue is empty. */
int fnae_pop_sound(FnaeGame* g);
