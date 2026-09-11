#pragma once
#include <stdint.h>

typedef enum { FRAME_WARNING=1, FRAME_TITLE=2, FRAME_NIGHT=3, FRAME_DEATH=4, FRAME_FINAL=5, FRAME_WHICH_NIGHT=6, FRAME_NEWSPAPER=7, FRAME_CUSTOMIZE=8, FRAME_6AM=9 } FnaeFrame;
typedef enum { CAM_DOWN=0, CAM_UP_ANIM=1, CAM_UP=2, CAM_DOWN_ANIM=3 } CamAnim;
typedef enum { MASK_UP=0, MASK_UP_ANIM=1, MASK_DOWN=2, MASK_DOWN_ANIM=3 } MaskAnim;

typedef struct { int ai, pos, move, at_door; } FnaeAI;
typedef struct {
 FnaeFrame frame; int running; int night; int six_or_seven; int arrow; int progress; int challenge;
 int time_of_day; float time_to_hour;
 int death; int death_addup; int gf_random; int gf_death_addup;
 int camera; CamAnim cam_anim; MaskAnim mask_anim; int prevent_flip; int force_down; int view;
 int left_door, right_door; int flashlight; int pc_mobile;
 int hidden_power; int power_left; float power_tick;
 int movement_out; float movement_timer; int camera_up_check;
 int foxy_stand; int freddy_door;
 FnaeAI foxy, freddy; int springtrap_a, springtrap_b; int springtrap_alive;
 int ph_mangle_a, ph_mangle_b, ph_mangle_c; int ph_bb_a, ph_bb_b;
 int golden_ai, foxy_ai, freddy_ai, springtrap_ai, ph_mangle_ai, ph_bb_ai;
 int music_left; int music_winding; float music_tick;
 int current_call; int call_muted;
 int all20; int left_challenge, left_challenge_active;
 float cam_anim_timer, mask_anim_timer, left_door_timer, right_door_timer;
 float ai_timer, power_out_timer, springtrap_timer, phantom_timer;
 int hour_events[8][13];
} FnaeGame;

void fnae_init(FnaeGame* g);
void fnae_start_night(FnaeGame* g, int night);
void fnae_update(FnaeGame* g, float dt);
void fnae_key(FnaeGame* g, int key);
void fnae_key_up(FnaeGame* g, int key);
void fnae_click(FnaeGame* g, int x, int y);
const char* fnae_frame_name(FnaeFrame f);
