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
  float six_timer; /* Frame 9: which-AM roll starts 3 s in (Timer > 03'' -> Start animation) */
  int death; int death_addup; int gf_random; int gf_death_addup;
  /* Fractional-tick accumulators: Fusion event logic runs per game tick
   * (1/60 s), but fnae_update receives real-time dt, so at >60 Hz displays
   * per-call counters (+7 fades, Death Addup, GF Addup) would run fast.
   * Accumulating dt*60 and applying whole ticks keeps the pacing at
   * real-time speed on any refresh rate (identical to before at 1/60). */
  float death_tick_acc; float gf_tick_acc;
  /* Frame 4 Death animation (Frame 4 Events.txt): Red Fade In alpha 0-255
   * (+7/tick), RIP Text alpha (starts 255, -7/tick out then +7/tick back
   * once B>1) + B state with 1 s gates, tick counter for the Death Anim
   * devil-card cycle. Initialized on the Night -> Death transition. */
   int death_red, death_rip_a, death_rip_b, death_ticks; float death_timer;
   /* Latched once Red Fade In first hits full opacity: the RIP fade/gates
    * below key off this rather than red>=255, because the fullscreen red
    * is a brief flash that drains back to 0 (owner) while the text runs. */
   int death_red_peaked;
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
 int puppet_ai; /* Night-7 music-box drain rate (= custom_puppet); story nights use Night*2 */
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
   /* Frame 8 Customize screen state (Frame 8 Events.txt). Columns are
    * x-ordered like the Layer #2 globals: 0 Freddy, 1 Mangle, 2 Foxy,
    * 3 Golden, 4 Springtrap, 5 BB, 6 Puppet. */
   int custom_sel; /* last-hovered column (arrows/Set20/Add1 target) */
   int custom_ch; /* Left Challenge A: 0 none, 1 Classics, 2 Broken, 3 Soy */
   int custom_b; /* Left Challenge B: 1 = preset holds, 0 = manually edited */
   int custom_arrow_dir; /* held-arrow repeat: +1 up, -1 down, 0 none */
   float custom_arrow_tick; /* hold-repeat accumulator (0.10 s steps) */
   int custom_cool; /* Cool Background frame 0-2 (Random(3) on entry) */
   int custom_check[4]; /* challenge completion flags [1..3] cached from save */
  int mouse_x, mouse_y;   /* last known pointer position (1280x720 space) */
   float office_scroll;    /* office pan in source px, 0 = leftmost.
                            * Mirrors the Fusion Office Center Object X minus
                            * Game Width / 2; clamped to [0, FNAE_OFFICE_SCROLL_MAX]. */
   float cam_scroll;       /* camera-feed pan: display left edge in source px.
                            * Drifts in [FNAE_CAM_SCROLL_MIN, FNAE_CAM_SCROLL_MAX]
                            * (clamped to the feed image ends, no overscan). */
   int cam_scroll_dir;     /* 0 = panning right (+1 px/tick), 1 = panning left */
  int left_door_frame, right_door_frame; /* shutter frame 0..15 (open->closed) */
 int mask_frame; /* mask overlay frame 0..10 (put-on 0-6, worn 7, take-off 8-10), -1 = no mask */
 int cam_flip_frame; /* cam flip flash 0..8 (573-581 open, reversed on close), -1 = no flip */
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

/* Staged-init progress hook (visuals + audio): percent runs 0..100 within
 * one init call. It fires on the calling thread between load groups so the
 * caller can repaint a loading screen and pump the OS; return nonzero to
 * abort (treated like a load failure -- callers free the partial set).
 * NULL = load everything silently, as before. */
typedef int (*FnaeLoadProgress)(int percent, void *ctx);
