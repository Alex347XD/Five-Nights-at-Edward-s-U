#include "fnae_core.h"
#include "fnae_assets.h"
#include <SDL_keycode.h>
#include <stdlib.h>
#include <string.h>

static int rnd(int n){ return n<=0?0:rand()%n; }
static int rr(int a,int b){ return a + rand()%(b-a+1); }
static void ai_reset(FnaeAI* a,int level,int start){ a->ai=level; a->pos=start; a->move=0; a->at_door=0; }
static void mark(FnaeGame* g,int h,int id){ if(h>=0&&h<13&&id>=0&&id<13) g->hour_events[h][id]=1; }
static int did(FnaeGame* g,int h,int id){ return h>=0&&h<13&&id>=0&&id<13&&g->hour_events[h][id]; }

const char* fnae_frame_name(FnaeFrame f){
 switch(f){case FRAME_WARNING:return "Warning";case FRAME_TITLE:return "Title";case FRAME_NIGHT:return "Night";case FRAME_DEATH:return "Death";case FRAME_FINAL:return "Final";case FRAME_WHICH_NIGHT:return "Which Night";case FRAME_NEWSPAPER:return "Newspaper";case FRAME_CUSTOMIZE:return "Customize";case FRAME_6AM:return "6 AM";} return "Unknown";
}

static void difficulty(FnaeGame* g,int h){
 int n=g->night;
 if(n==1 && h==12 && !did(g,h,0)){g->foxy_ai=0;g->freddy_ai=3;g->springtrap_ai=3;g->ph_mangle_ai=3;g->ph_bb_ai=3;mark(g,h,0);}
 if(n==1 && h==3 && !did(g,h,1)){g->foxy_ai=rnd(2)+1;g->freddy_ai+=rnd(1);mark(g,h,1);}
 if(n==2 && h==12 && !did(g,h,2)){g->freddy_ai=3+(rnd(2)+1);g->foxy_ai=rnd(4)+2;g->springtrap_ai=4;g->ph_mangle_ai=3+rnd(2);g->ph_bb_ai=3+rnd(2);g->golden_ai=1;mark(g,h,2);}
 if(n==2 && h==2 && !did(g,h,3)){g->freddy_ai=1+rnd(2);g->foxy_ai+=1+rnd(2);g->springtrap_ai=5;g->golden_ai+=rnd(2);mark(g,h,3);}
 if(n==3 && h==12 && !did(g,h,4)){g->freddy_ai=4+(rnd(4)+1);g->foxy_ai=6;g->springtrap_ai=4;g->ph_mangle_ai=4+rnd(2);g->ph_bb_ai=4+rnd(2);g->golden_ai=3;mark(g,h,4);}
 if(n==3 && h==2 && !did(g,h,5)){g->freddy_ai+=rnd(1)+1;g->foxy_ai+=rnd(1)+1;g->springtrap_ai=7;g->ph_mangle_ai+=1;g->ph_bb_ai+=1;g->golden_ai+=rnd(2);mark(g,h,5);}
 if(n==4 && h==12 && !did(g,h,6)){g->freddy_ai=5+(rnd(4)+1);g->foxy_ai=6+rnd(2);g->springtrap_ai=6;g->ph_mangle_ai=5+rnd(2);g->ph_bb_ai=5+rnd(2);g->golden_ai=5;mark(g,h,6);}
 if(n==4 && h==1 && !did(g,h,7)){g->freddy_ai+=rnd(1)+2;g->foxy_ai+=rnd(2)+1;g->springtrap_ai+=2;g->ph_mangle_ai+=1+rnd(2);g->ph_bb_ai+=1+rnd(2);g->golden_ai+=rnd(2);mark(g,h,7);}
 if(n==5 && h==12 && !did(g,h,8)){g->freddy_ai=6+(rnd(4)+1);g->foxy_ai=7+rnd(2);g->springtrap_ai=5;g->ph_mangle_ai=6+rnd(2);g->ph_bb_ai=6+rnd(2);g->golden_ai=6;mark(g,h,8);}
 if(n==5 && h==1 && !did(g,h,9)){g->freddy_ai+=rnd(1)+2;g->foxy_ai+=rnd(2)+1;g->springtrap_ai+=3+rnd(2);g->ph_mangle_ai+=1+rnd(3);g->ph_bb_ai+=1+rnd(3);g->golden_ai+=rnd(2);mark(g,h,9);}
 if(n==6 && h==12 && !did(g,h,10)){g->freddy_ai=9+(rnd(4)+1);g->foxy_ai=10+rnd(2);g->springtrap_ai=8;g->ph_mangle_ai=9+rnd(2);g->ph_bb_ai=9+rnd(2);g->golden_ai=7;mark(g,h,10);}
 if(n==6 && h==1 && !did(g,h,11)){g->freddy_ai+=rnd(1)+2;g->foxy_ai+=rnd(2)+1;g->springtrap_ai+=1+rnd(5);g->ph_mangle_ai+=1+rnd(3);g->ph_bb_ai+=1+rnd(3);g->golden_ai+=rnd(2);mark(g,h,11);}
 if(n==7 && h==12 && !did(g,h,12)){g->freddy_ai=g->custom_freddy;g->foxy_ai=g->custom_foxy;g->springtrap_ai=g->custom_springtrap;g->ph_mangle_ai=g->custom_mangle;g->ph_bb_ai=g->custom_bb;g->golden_ai=g->custom_golden;mark(g,h,12);}
 g->freddy.ai=g->freddy_ai; g->foxy.ai=g->foxy_ai; g->springtrap_alive=g->springtrap_ai>0;
}

static void enter_death(FnaeGame* g,int who){
 if(g->death==0) g->death=who;
 if(g->death==0) return;
 /* Fusion forces the cameras down and the mask off on any death. */
 if(g->cam_anim==CAM_UP){g->cam_anim=CAM_DOWN_ANIM;g->cam_anim_timer=0;}
 g->view=0; g->camera_up_check=0;
 if(g->mask_anim==MASK_DOWN){g->mask_anim=MASK_DOWN_ANIM;g->mask_anim_timer=0;}
 g->flashlight=0;
}

static void ai_move(FnaeGame* g){
 if(g->death) return;
 if(rnd(30)<g->foxy_ai && g->foxy.move==0) g->foxy.move=1;
 if(g->foxy.move){
  if(g->foxy.pos==2)g->foxy.pos=4;
  else if(g->foxy.pos==4)g->foxy.pos=5;
  else if(g->foxy.pos==5){ if(g->right_door==0)g->foxy.pos=6; else g->foxy.pos=2; }
  g->foxy.move=0;
 }
 if(g->foxy.pos==6 && g->view>0 && g->hidden_power>0) enter_death(g,3);
 if(rnd(30)<g->freddy_ai && g->freddy.move==0) g->freddy.move=1;
 if(g->freddy.move){
  if(g->freddy.pos==1)g->freddy.pos=3;
  else if(g->freddy.pos==3)g->freddy.pos=6;
  else if(g->freddy.pos==6){ if(g->left_door==0)g->freddy.pos=7; else g->freddy.pos=1; }
  g->freddy.move=0;
 }
 if(g->freddy.pos==7 && g->view>0 && g->hidden_power>0) enter_death(g,2);
 if(g->springtrap_ai>0 && rnd(30)+1<g->springtrap_ai && g->springtrap_a==0)g->springtrap_a=1;
}

static void update_phantoms(FnaeGame* g){
 if(g->view>0 && g->ph_bb_ai>0 && g->ph_bb_a==0) g->ph_bb_a=rnd(23-g->ph_bb_ai);
 if(g->view==0) { g->ph_bb_a=0; g->ph_bb_b=0; }
 if(g->ph_bb_a==1){g->ph_bb_b++; if(g->ph_bb_b>80){g->ph_bb_a=0;g->ph_bb_b=0;g->force_down=5;}}
 if(g->view>0 && g->ph_mangle_ai>0 && g->ph_mangle_c==0) g->ph_mangle_a=rnd(23-g->ph_mangle_ai);
 if(g->view==0) g->ph_mangle_a=0;
 if(g->ph_mangle_a==1){g->ph_mangle_b++; if(g->ph_mangle_b>60){g->ph_mangle_c=1;g->ph_mangle_a=0;g->ph_mangle_b=0;g->force_down=5;}}
}

static void update_gf(FnaeGame* g){
 if(g->cam_anim==CAM_DOWN_ANIM && g->golden_ai>0 && g->gf_random!=1) g->gf_random=rnd(22-g->golden_ai);
 if(g->cam_anim==CAM_UP_ANIM && g->gf_random==1)g->gf_random=0;
 if(g->mask_anim==MASK_DOWN && g->gf_random==1)g->gf_random=0;
 if(g->gf_random==1) g->gf_death_addup++; else g->gf_death_addup=0;
 if(g->gf_death_addup>90) enter_death(g,5);
}

static void update_music(FnaeGame* g,float dt){
 if(g->view!=4){g->music_winding=0;return;}
 g->music_tick+=dt;
 if(!g->music_winding && g->music_left>0 && g->music_tick>=0.07f){g->music_tick=0;g->music_left-=g->night==7?g->golden_ai*2:g->night*2;if(g->music_left<0)g->music_left=0;}
 if(g->music_winding && g->music_tick>=0.35f){g->music_tick=0;if(g->music_left>0)g->music_left+=100;if(g->music_left>2000)g->music_left=2000;}
 if(g->music_left<=0 && g->hidden_power>0 && g->death==0 && ((g->cam_anim==CAM_UP&&rnd(5)==1)||(g->mask_anim==MASK_DOWN&&rnd(5)==1))) enter_death(g,1);
}

void fnae_init(FnaeGame* g){ memset(g,0,sizeof(*g)); g->running=1; g->frame=FRAME_TITLE; g->night=1; g->progress=0; g->arrow=0; g->pc_mobile=0; g->static_frame=0; g->static_alpha=200; g->mouse_x=640; g->mouse_y=360; g->office_scroll=FNAE_OFFICE_SCROLL_MAX/2; g->cam_static_alpha=185; g->power_out_alpha=255;
 /* Customize-screen defaults straight from Frame 8: everything 0 except Puppet (7). */
 g->custom_freddy=0; g->custom_foxy=0; g->custom_springtrap=0; g->custom_golden=0;
 g->custom_mangle=0; g->custom_bb=0; g->custom_puppet=7; }

void fnae_set_custom(FnaeGame* g,int freddy,int foxy,int springtrap,int golden,int mangle,int bb,int puppet){
 if(freddy<0)freddy=0; if(freddy>20)freddy=20;
 if(foxy<0)foxy=0; if(foxy>20)foxy=20;
 if(springtrap<0)springtrap=0; if(springtrap>20)springtrap=20;
 if(golden<0)golden=0; if(golden>20)golden=20;
 if(mangle<0)mangle=0; if(mangle>20)mangle=20;
 if(bb<0)bb=0; if(bb>20)bb=20;
 if(puppet<1)puppet=1; if(puppet>7)puppet=7;
 g->custom_freddy=freddy; g->custom_foxy=foxy; g->custom_springtrap=springtrap;
 g->custom_golden=golden; g->custom_mangle=mangle; g->custom_bb=bb; g->custom_puppet=puppet;
}

/* TV-static animation state, shared by the title overlay and the cameras.
 * Mirrors Frame 2 Events.txt: on Random(10)=1 the flicker alpha becomes
 * 100+Random(100). The static frames advance every 3rd tick (~20 fps);
 * advancing every tick strobed far faster than the original.
 * Also drives the title background flash: on Random(50)=1 the Background
 * plays one RRandom(12,14) sequence — a single frame (516/517/518) held
 * for the 0.2 s cut window (12 ticks) — then back to Stopped (515). */
void fnae_static_tick(FnaeGame* g){
 if(++g->static_div>=3){g->static_div=0;g->static_frame=(g->static_frame+1)&7;}
 if(rnd(10)==1)g->static_alpha=100+rnd(100);
 if(g->frame!=FRAME_TITLE){g->title_bg_frame=0;g->title_bg_timer=0;return;}
 if(g->title_bg_frame>0){
  if(++g->title_bg_timer>=12){g->title_bg_frame=0;g->title_bg_timer=0;}
 } else if(rnd(50)==1){
  g->title_bg_frame=1+rnd(3);g->title_bg_timer=0;
 }
}

/* Frame 6 routing: 0 = normal night (g->night), 1 = 6th, 2 = 7th/custom.
 * Matches Frame 6 Start-of-Frame events; the frame then auto-advances
 * after 2 seconds (see fnae_update). */
static void enter_which_night(FnaeGame* g){
 if(g->six_or_seven==1) g->night=6;
 else if(g->six_or_seven==2) g->night=7;
 if(g->night<1)g->night=1; if(g->night>7)g->night=7;
 g->frame=FRAME_WHICH_NIGHT; g->which_timer=0;
}

void fnae_start_night(FnaeGame* g,int night){
 memset(g->hour_events,0,sizeof(g->hour_events));
 g->night=night<1?1:(night>7?7:night); g->frame=FRAME_NIGHT; g->time_of_day=12; g->time_to_hour=0;
 g->death=0; g->death_addup=0; g->gf_random=0; g->gf_death_addup=0;
 g->cam_anim=CAM_DOWN; g->mask_anim=MASK_UP; g->prevent_flip=0; g->force_down=0; g->view=0; g->camera=1;
  g->left_door=0; g->right_door=0; g->flashlight=0; g->hidden_power=10000; g->power_left=1; g->power_tick=0;
  g->mouse_x=640; g->mouse_y=360; g->office_scroll=FNAE_OFFICE_SCROLL_MAX/2;
   g->left_door_frame=0; g->right_door_frame=0; g->title_bg_frame=0; g->title_bg_timer=0;
  g->movement_out=0; g->movement_timer=0; g->movement_half_tick=0; g->movement_force_tick=0;
  g->camera_up_check=0; g->cam_static_alpha=185; g->cam_static_tick=0;
  g->warning=0; g->power_out_alpha=255;
  g->springtrap_pos=1; g->springtrap_stand=0; g->lure_area=0; g->lure_cam=0; g->lure_timer=0;
  g->foxy_stand=0; g->freddy_door=0;
 g->music_left=2000; g->music_winding=0; g->music_tick=0; g->current_call=0; g->call_muted=0;
 g->cam_anim_timer=g->mask_anim_timer=g->left_door_timer=g->right_door_timer=0;
 g->ai_timer=g->power_out_timer=g->springtrap_timer=g->phantom_timer=0;
 ai_reset(&g->foxy,0,2); ai_reset(&g->freddy,0,1); g->springtrap_a=0;g->springtrap_b=0;
 g->ph_mangle_a=g->ph_mangle_b=g->ph_mangle_c=0;g->ph_bb_a=g->ph_bb_b=0;
 g->golden_ai=0;g->foxy_ai=g->freddy_ai=g->springtrap_ai=g->ph_mangle_ai=g->ph_bb_ai=0;
 difficulty(g,12);
 /* All-20 star reads the Customize-screen globals, not the nightly rolls. */
 g->all20=(g->custom_freddy==20&&g->custom_foxy==20&&g->custom_springtrap==20&&
  g->custom_golden==20&&g->custom_mangle==20&&g->custom_bb==20&&g->custom_puppet==7)?1:0;
}

static void hour(FnaeGame* g){
 g->time_of_day++; if(g->time_of_day>12)g->time_of_day=1;
 difficulty(g,g->time_of_day);
 if(g->time_of_day>5 && g->time_of_day!=12)g->frame=FRAME_6AM;
}

void fnae_mouse_move(FnaeGame* g, int x, int y){
 g->mouse_x=x; g->mouse_y=y;
}

/* Office panning ("[ Office Panning ]" in Frame 3 Events.txt).
 * Fusion scrolls the display by moving the Office Center Object while the
 * pointer hovers the Left 1/2/3 / Right 1/2/3 edge zones (at X 225/168/119
 * and 1025/1088/1143) at 2/4/6 px per tick, clamped so the 1280-wide view
 * stays inside the frame. Desktop only (PC/Mobile = 0), office view only.
 * Speeds are per 1/60 tick, hence the dt*60 scaling. */
static void update_office_pan(FnaeGame* g, float dt){
 if(g->death || g->view!=0 || g->cam_anim!=CAM_DOWN) return;
 if(g->hidden_power<=0 || g->pc_mobile!=0) return;
 float speed=0;
 if(g->mouse_x<120) speed=-6; else if(g->mouse_x<170) speed=-4; else if(g->mouse_x<230) speed=-2;
 else if(g->mouse_x>1140) speed=6; else if(g->mouse_x>1085) speed=4; else if(g->mouse_x>1020) speed=2;
 else return;
 g->office_scroll+=speed*dt*60.0f;
 if(g->office_scroll<0) g->office_scroll=0;
 if(g->office_scroll>FNAE_OFFICE_SCROLL_MAX) g->office_scroll=(float)FNAE_OFFICE_SCROLL_MAX;
}

void fnae_update(FnaeGame* g,float dt){
 /* Frame 6 interstitial: Every 02'' -> Night, no input required. */
 if(g->frame==FRAME_WHICH_NIGHT){
  g->which_timer+=dt;
  if(g->which_timer>=2.0f) fnae_start_night(g,g->night);
  return;
 }
 if(g->frame!=FRAME_NIGHT)return;
 if(g->death){g->death_addup++;if(g->death_addup>=60)g->frame=FRAME_DEATH;return;}
 update_office_pan(g,dt);

 /* Fusion's transition objects have visible animation phases. */
 if(g->cam_anim==CAM_UP_ANIM){
  g->cam_anim_timer+=dt;
  if(g->cam_anim_timer>=0.55f){g->cam_anim_timer=0;g->cam_anim=CAM_UP;g->view=g->camera;g->camera_up_check=1;}
 } else if(g->cam_anim==CAM_DOWN_ANIM){
  g->cam_anim_timer+=dt;
  if(g->cam_anim_timer>=0.55f){g->cam_anim_timer=0;g->cam_anim=CAM_DOWN;g->view=0;g->camera_up_check=0;}
 }
 if(g->mask_anim==MASK_UP_ANIM){g->mask_anim_timer+=dt;if(g->mask_anim_timer>=0.45f){g->mask_anim_timer=0;g->mask_anim=MASK_DOWN;}}
 else if(g->mask_anim==MASK_DOWN_ANIM){g->mask_anim_timer+=dt;if(g->mask_anim_timer>=0.45f){g->mask_anim_timer=0;g->mask_anim=MASK_UP;}}

 if(g->left_door==1){g->left_door_timer+=dt;if(g->left_door_timer>=0.25f){g->left_door=2;g->left_door_timer=0;}}
 else if(g->left_door==3){g->left_door_timer+=dt;if(g->left_door_timer>=0.25f){g->left_door=0;g->left_door_timer=0;}}
 if(g->right_door==1){g->right_door_timer+=dt;if(g->right_door_timer>=0.25f){g->right_door=2;g->right_door_timer=0;}}
 else if(g->right_door==3){g->right_door_timer+=dt;if(g->right_door_timer>=0.25f){g->right_door=0;g->right_door_timer=0;}}

 /* Door shutter frames (Left/Right Door objects): Fusion Alterable Value A
  * 0=open(Stopped frame 0), 1=closing(Close 0->15), 2=closed(hold 15),
  * 3=opening(Open 15->0). Timers above already model the A transitions. */
 {
  int f=(int)(15.0f*g->left_door_timer/0.25f);
  if(f<0)f=0; if(f>15)f=15;
  if(g->left_door==1)g->left_door_frame=f;
  else if(g->left_door==2)g->left_door_frame=15;
  else if(g->left_door==3)g->left_door_frame=15-f;
  else g->left_door_frame=0;
 }
 {
  int f=(int)(15.0f*g->right_door_timer/0.25f);
  if(f<0)f=0; if(f>15)f=15;
  if(g->right_door==1)g->right_door_frame=f;
  else if(g->right_door==2)g->right_door_frame=15;
  else if(g->right_door==3)g->right_door_frame=15-f;
  else g->right_door_frame=0;
 }

 g->time_to_hour+=dt;
 if(g->time_to_hour>=50){g->time_to_hour-=50;hour(g);if(g->frame!=FRAME_NIGHT)return;}

 g->power_left=1+g->camera_up_check+(g->left_door>0)+(g->right_door>0)+(g->flashlight?1:0)+(g->ph_mangle_c*2);
 if(g->power_left<1)g->power_left=1;if(g->power_left>5)g->power_left=5;
 static const float intervals[6]={0,2,.5,.25,.15,.10};
 g->power_tick+=dt;
 if(g->power_tick>=intervals[g->power_left]){g->power_tick=0;int sub=10*((g->night/5)+1);g->hidden_power-=sub;if(g->hidden_power<0)g->hidden_power=0;}

  if(g->hidden_power<=0){
   g->force_down=5;g->cam_anim=CAM_DOWN;g->view=0;g->camera_up_check=0;g->mask_anim=MASK_UP;g->flashlight=0;
   /* Closed doors swing open on power loss (door A 2 -> 3). */
   if(g->left_door==2){g->left_door=3;g->left_door_timer=0;}
   if(g->right_door==2){g->right_door=3;g->right_door_timer=0;}
   /* Power Out overlay fades 255 -> 0; death rolls start once it is gone. */
   if(g->power_out_alpha>0){g->power_out_alpha-=3;if(g->power_out_alpha<0)g->power_out_alpha=0;}
   g->power_out_timer+=dt;
   if(g->power_out_alpha<=0 && g->death==0 && g->power_out_timer>=5){g->power_out_timer=0;enter_death(g,rnd(4));}
  }
  if(g->death) g->flashlight=0;
  if(g->force_down>0)g->force_down--;

  g->ai_timer+=dt;
 if(g->ai_timer>=5){g->ai_timer-=5;ai_move(g);
  /* Springtrap steps between cameras on its move flag, then clears it. */
  if(g->springtrap_a && g->death==0){
   g->springtrap_a=0;
   int p=g->springtrap_pos;
   if(p==1) g->springtrap_pos=3;
   else if(p==2) g->springtrap_pos=(g->springtrap_b==0)?4:1;
   else if(p==4) g->springtrap_pos=(g->springtrap_b==0)?2:1;
   /* pos 3 is the kill room: Springtrap waits there for the death rolls. */
   if(g->springtrap_pos==1||g->springtrap_pos==2) g->springtrap_b=rr(0,1);
  }
 }
 /* Audio lure: 2s after placing it, 50% to pull Springtrap to the lured cam. */
 if(g->lure_area){
  g->lure_timer+=dt;
  if(g->lure_timer>=2.0f){g->lure_timer-=2.0f;
   if(rnd(2)==1){g->springtrap_pos=g->lure_cam;g->movement_out=1;g->movement_half_tick=0;g->movement_force_tick=0;}
   g->lure_area=0;}
 }
 /* Camera Out ("Connection Lost"): 50% re-tune every 0.5s, forced after 2s. */
 if(g->movement_out>0){
  g->movement_half_tick+=dt; g->movement_force_tick+=dt;
  if(g->movement_half_tick>=0.5f){g->movement_half_tick-=0.5f;if(rnd(2)==1)g->movement_out=0;}
  if(g->movement_force_tick>=2.0f){g->movement_force_tick=0;g->movement_out=0;}
 } else { g->movement_half_tick=0; g->movement_force_tick=0; }
 /* Camera static: 150+Random(50) every 0.08s while the feed is live, 0 on signal loss. */
 g->cam_static_tick+=dt;
 if(g->cam_static_tick>=0.08f){g->cam_static_tick=0;
  if(g->view>0 && g->movement_out==0) g->cam_static_alpha=150+rnd(50);
  else g->cam_static_alpha=0;}
 /* Music-box warnings: <600 low, <200 critical, <=0 empty. */
 if(g->music_left<=0) g->warning=3;
 else if(g->music_left<200) g->warning=2;
 else if(g->music_left<600) g->warning=1;
 else g->warning=0;
 /* Doorway figures: Freddy at the left door, Foxy at the right, Springtrap on its viewed cam. */
 g->foxy_stand=(g->foxy.pos==5 && g->view==0 && g->hidden_power>0)?1:0;
 g->freddy_door=(g->freddy.pos==6 && g->view==0 && g->hidden_power>0)?1:0;
 g->springtrap_stand=(g->view>0 && g->view==g->springtrap_pos && g->hidden_power>0)?1:0;
 update_phantoms(g); update_gf(g); update_music(g,dt);
 if(g->springtrap_pos==3 && g->view==3 && g->hidden_power>0 && g->death==0){g->springtrap_timer+=dt;if(g->springtrap_timer>=4){g->springtrap_timer=0;if(rnd(2)==1)enter_death(g,4);}}
 if(g->current_call==0 && g->time_to_hour>=3)g->current_call=g->night;
 if(g->cam_anim==CAM_UP)g->view=g->camera; else if(g->cam_anim==CAM_DOWN)g->view=0;
 g->camera_up_check=(g->view>0);
}

void fnae_key(FnaeGame* g,int key){
 if(key==SDLK_ESCAPE){g->running=0;return;}
 if(g->frame==FRAME_TITLE){
  if(key==SDLK_RETURN){
   if(g->arrow==0){g->six_or_seven=0;g->frame=FRAME_NEWSPAPER;}
   else if(g->arrow==1){g->six_or_seven=0;enter_which_night(g);}
   else if(g->arrow==2){g->six_or_seven=1;enter_which_night(g);}
   else if(g->arrow==3){g->six_or_seven=2;g->frame=FRAME_CUSTOMIZE;}
  } else if(key==SDLK_UP || key=='w')g->arrow--;
  else if(key==SDLK_DOWN || key=='s')g->arrow++;
  if(g->arrow<0)g->arrow=0;int max=g->progress+1;if(max>3)max=3;if(g->arrow>max)g->arrow=max;return;
 }
 if(g->frame==FRAME_NEWSPAPER){if(key==SDLK_RETURN)enter_which_night(g);return;}
 if(g->frame==FRAME_WHICH_NIGHT){if(key==SDLK_RETURN)fnae_start_night(g,g->night);return;}
 if(g->frame==FRAME_CUSTOMIZE){if(key==SDLK_RETURN){g->six_or_seven=2;enter_which_night(g);}return;}
 if(g->frame==FRAME_6AM){if(key==SDLK_RETURN){
  /* Fusion: nights 6/7 or Night Story >= 5 -> Final, else next night -> Which Night. */
  if(g->six_or_seven>0||g->night>=5)g->frame=FRAME_FINAL;
  else {g->night++;g->six_or_seven=0;enter_which_night(g);}
 }return;}
 if(g->frame==FRAME_DEATH){if(key==SDLK_RETURN)g->frame=FRAME_TITLE;return;}
 if(g->frame==FRAME_FINAL){if(key==SDLK_RETURN)g->frame=FRAME_TITLE;return;}
 if(g->frame!=FRAME_NIGHT)return;

 if(key=='a'&&g->view==0&&g->hidden_power>0){if(g->left_door==0)g->left_door=1;else if(g->left_door==2)g->left_door=3;}
 if(key=='d'&&g->view==0&&g->hidden_power>0){if(g->right_door==0)g->right_door=1;else if(g->right_door==2)g->right_door=3;}
 if(key=='s' && g->hidden_power>0){
  if(g->cam_anim==CAM_DOWN && g->mask_anim==MASK_UP){g->cam_anim=CAM_UP_ANIM;g->cam_anim_timer=0;}
  else if(g->cam_anim==CAM_UP && g->mask_anim==MASK_UP){g->cam_anim=CAM_DOWN_ANIM;g->cam_anim_timer=0;}
 }
 if(key=='m'&&g->hidden_power>0&&g->cam_anim==CAM_DOWN){if(g->mask_anim==MASK_UP){g->mask_anim=MASK_UP_ANIM;g->mask_anim_timer=0;}else if(g->mask_anim==MASK_DOWN){g->mask_anim=MASK_DOWN_ANIM;g->mask_anim_timer=0;}}
 if(key=='z'||key==SDLK_LALT)g->flashlight=1;
 if(g->cam_anim==CAM_UP){if(key>='1'&&key<='4')g->camera=key-'0';}
 /* Audio lure: E while watching a camera feed (never from the music-box cam)
  * puts a Lure Area on the viewed camera; Springtrap may follow (see update). */
 if(key=='e'&&g->view>0&&g->view!=4&&g->hidden_power>0&&g->death==0&&g->lure_area==0){
  g->lure_area=1; g->lure_cam=g->view; g->lure_timer=0;}
 /* R is a keyboard test/control for winding the music box; mouse uses fnae_click. */
 if(key=='r'&&g->view==4)g->music_winding=1;
}

void fnae_key_up(FnaeGame* g,int key){
 if((key=='z'||key==SDLK_LALT) && g->frame==FRAME_NIGHT)g->flashlight=0;
 if(key=='r' && g->frame==FRAME_NIGHT)g->music_winding=0;
}

void fnae_click(FnaeGame* g,int x,int y){
 g->mouse_x=x; g->mouse_y=y;
 if(g->frame==FRAME_TITLE){
  if(x>=70&&x<=430&&y>=430&&y<495){g->arrow=0;g->frame=FRAME_NEWSPAPER;return;}
  if(x>=70&&x<=430&&y>=495&&y<560){g->arrow=1;g->six_or_seven=0;enter_which_night(g);return;}
  if(x>=70&&x<=430&&y>=560&&y<625 && g->progress>0){g->arrow=2;g->six_or_seven=1;enter_which_night(g);return;}
  if(x>=70&&x<=430&&y>=625&&y<700 && g->progress>1){g->arrow=3;g->six_or_seven=2;g->frame=FRAME_CUSTOMIZE;return;}
  return;
 }
 /* Newspaper advances on any click, like the Fusion event. */
 if(g->frame==FRAME_NEWSPAPER){enter_which_night(g);return;}
 if(g->frame!=FRAME_NIGHT)return;
 if(g->view==0 && g->hidden_power>0){
  /* Door click zones follow the panning doors: compare in frame space
   * (screen x + scroll) so the zones stay glued to the door art. */
  int fx=x+(int)g->office_scroll;
  if(fx<180 && y>500){if(g->left_door==0)g->left_door=1;else if(g->left_door==2)g->left_door=3;return;}
  if(fx>1100 && y>500){if(g->right_door==0)g->right_door=1;else if(g->right_door==2)g->right_door=3;return;}
  if(x>500 && x<780 && y>560){g->cam_anim=CAM_UP_ANIM;g->cam_anim_timer=0;return;}
 }
 if(g->cam_anim==CAM_UP && y>80){
  if(x>980){ if(y<150)g->camera=1; else if(y<220)g->camera=2; else if(y<290)g->camera=3; else if(y<360)g->camera=4; }
 }
 if(g->view==4 && x>500 && x<780 && y>500){g->music_winding=1;}
}
