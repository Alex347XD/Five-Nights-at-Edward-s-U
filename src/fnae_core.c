#include "fnae_core.h"
#include "fnae_assets.h"
#include "save.h"
#include <SDL_keycode.h>
#include <stdio.h> /* TMP-DEBUG */
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

void fnae_push_sound(FnaeGame* g,int snd){
 int next=(g->snd_tail+1)%FNAE_SND_QUEUE;
 if(next==g->snd_head) return; /* full: drop, never block gameplay */
 g->snd_queue[g->snd_tail]=snd;
 g->snd_tail=next;
}

int fnae_pop_sound(FnaeGame* g){
 if(g->snd_head==g->snd_tail) return -1;
 int snd=g->snd_queue[g->snd_head];
 g->snd_head=(g->snd_head+1)%FNAE_SND_QUEUE;
 return snd;
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
  if(n==7 && h==12 && !did(g,h,12)){g->freddy_ai=g->custom_freddy;g->foxy_ai=g->custom_foxy;g->springtrap_ai=g->custom_springtrap;g->ph_mangle_ai=g->custom_mangle;g->ph_bb_ai=g->custom_bb;g->golden_ai=g->custom_golden;g->puppet_ai=g->custom_puppet;mark(g,h,12);}
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

static void update_phantoms(FnaeGame* g,float dt){
  /* Phantom rolls are edge-triggered on cam open (view 0 -> >0) or cam
   * switch (camera index changed while viewing). The overlay then stays
   * stable while viewing instead of flickering: re-rolling every tick
   * re-randomized A 60x/sec. */
  int cam_opened=(g->view>0 && g->ph_prev_view==0);
  int cam_switched=(g->view>0 && g->ph_prev_view>0 && g->camera!=g->ph_prev_cam);
  int cam_edge=(cam_opened||cam_switched);
  if(cam_edge && g->ph_bb_ai>0) g->ph_bb_a=rnd(23-g->ph_bb_ai);
 if(g->view==0) { g->ph_bb_a=0; g->ph_bb_b=0; }
  /* B>80 completion (Fusion plays scream3 on ch #18 here): leaving the
   * cameras clears A/B silently above, so only this path screams.
   * The B>80 event also resets the Scare overlay alpha to 0 (it then
   * fades back in via +7/tick below). */
   if(g->ph_bb_a==1){g->ph_bb_b++; if(g->ph_bb_b>80){g->ph_bb_a=0;g->ph_bb_b=0;g->ph_bb_scare=0;g->ph_bb_scare_on=1;g->ph_bb_scare_timer=0;g->force_down=5;fnae_push_sound(g,FNAE_SND_PHBB);}}
   /* Ph BB Scare alpha: +7 per tick while <255, with no view gate (mirrors
    * the Fusion event). Night start parks it at 255 (the Fusion initial
    * Alterable A) so the fade only ever runs after a B>80 trigger; the
    * renderer additionally gates on ph_bb_scare_on so nothing draws before
    * the first trigger (Fusion visibility at frame start is unrecoverable
    * from the text dump, and an always-on overlay is clearly wrong). */
   if(g->ph_bb_scare<255){g->ph_bb_scare+=7;if(g->ph_bb_scare>255)g->ph_bb_scare=255;}
   /* Jumpscare clears: the Fusion dump never resets the Scare, but the
    * original jumpscare flashes then goes away, so hold ~1.5 s at full
    * opacity then clear the gate (alpha stays parked at 255 like night
    * start, drawing nothing until the next trigger). */
   if(g->ph_bb_scare_on && g->ph_bb_scare>=255){
    g->ph_bb_scare_timer+=dt;
    if(g->ph_bb_scare_timer>=1.5f){g->ph_bb_scare_on=0;g->ph_bb_scare_timer=0;}
   }
  /* Phantom Mangle roll shares the same cam-open/switch edge, gated on
   * C==0 (no haunt while the office annoy runs). Stays stable (A==1)
   * while viewing; B>60 below then forces the cameras down. */
  if(cam_edge && g->ph_mangle_ai>0 && g->ph_mangle_c==0)
   g->ph_mangle_a=rnd(23-g->ph_mangle_ai);
 /* Fusion resets Camera B alongside A while the cameras are down
  * (A==0 + B<>0 -> B=0), so a stale count never shortens the next haunt. */
  if(g->view==0) { g->ph_mangle_a=0; g->ph_mangle_b=0; }
  g->ph_prev_view=g->view; g->ph_prev_cam=g->camera;
 if(g->ph_mangle_a==1){g->ph_mangle_b++; if(g->ph_mangle_b>60){g->ph_mangle_c=1;g->ph_mangle_a=0;g->ph_mangle_b=0;g->force_down=5;}}
 /* Office annoy ([ Phantom Mangle ] Annoy events): once C==1 the Annoy
  * descends (A 0->224 while B==0), lingers (B+1 every 1s at A>=224),
  * then ascends (A->0 once B>=7) and C clears. ch17 audio keys off
  * Annoy A/B (see audio.c); the camera-haunt phase above stays silent,
  * so cam open/close keeps its stereo-cassette flip. */
 if(g->ph_mangle_c==1){
  if(g->ph_annoy_b>=7){
   if(g->ph_annoy_a>0) g->ph_annoy_a--;
   else { g->ph_annoy_a=0; g->ph_annoy_b=0; g->ph_mangle_c=0; }
   } else if(g->ph_annoy_a>=224){
    g->phantom_timer+=dt;
    if(g->phantom_timer>=1.0f){g->phantom_timer=0;g->ph_annoy_b++;}
   } else g->ph_annoy_a++;
  }
 { static long tdbg=0; if(++tdbg%120==0) printf("TMP-PH t=%ld view=%d bbai=%d a=%d b=%d sc=%d on=%d mai=%d ma=%d mb=%d mc=%d anA=%d anB=%d\n",tdbg,g->view,g->ph_bb_ai,g->ph_bb_a,g->ph_bb_b,g->ph_bb_scare,g->ph_bb_scare_on,g->ph_mangle_ai,g->ph_mangle_a,g->ph_mangle_b,g->ph_mangle_c,g->ph_annoy_a,g->ph_annoy_b); }
}

static void update_gf(FnaeGame* g,float dt){
 /* The whole group runs per game tick in Fusion (GF Random roll on the
  * camera-down anim, GF Sit show/hide, +1 Death Addup while shown), so
  * consume whole 1/60 ticks from the accumulator: at fixed 1/60 dt this
  * is exactly one pass per call, like before. */
 g->gf_tick_acc+=dt*60.0f;
 int steps=(int)g->gf_tick_acc; g->gf_tick_acc-=steps;
 if(steps>4)steps=4;
 for(int i=0;i<steps;i++){
  if(g->cam_anim==CAM_DOWN_ANIM && g->golden_ai>0 && g->gf_random!=1) g->gf_random=rnd(22-g->golden_ai);
  if(g->cam_anim==CAM_UP_ANIM && g->gf_random==1)g->gf_random=0;
  if(g->mask_anim==MASK_DOWN && g->gf_random==1)g->gf_random=0;
  if(g->gf_random==1) g->gf_death_addup++; else g->gf_death_addup=0;
  if(g->gf_death_addup>90) enter_death(g,5);
 }
}

/* Music-box crank hover: the button is center-anchored (156x65) at
 * its [569,497] hotspot, so the pointer is over it in
 * x 491..647, y 465..529 (matches the renderer math). */
static int over_music_button(FnaeGame* g){
 return g->mouse_x>=491&&g->mouse_x<647&&g->mouse_y>=465&&g->mouse_y<529;
}

static void update_music(FnaeGame* g,float dt){
 /* Fusion holds Alterable A while the crank is held: pointer over the
  * button with the mouse down, or the R test key — evaluated every tick,
  * so releasing (or leaving the button or the cam) stops the wind. */
 if(g->view==4&&g->death==0&&(g->key_wind||(g->mouse_down&&over_music_button(g))))
  g->music_winding=1;
 else
  g->music_winding=0;
 /* The Fusion drain/wind timers have no view gate (only the crank press
  * needs View 4): A==0 drains every 0.07 s on every screen, A>0 winds
  * every 0.35 s while held. */
 g->music_tick+=dt;
  if(!g->music_winding && g->music_left>0 && g->music_tick>=0.07f){g->music_tick=0;g->music_left-=g->night==7?g->puppet_ai*2:g->night*2;if(g->music_left<0)g->music_left=0;}
 if(g->music_winding && g->music_tick>=0.35f){g->music_tick=0;if(g->music_left>0)g->music_left+=100;if(g->music_left>2000)g->music_left=2000;}
 /* Fusion replays windup2 every 00''-50 while the button is held. */
 if(g->music_winding && g->music_left>0){
  g->windup_snd_tick+=dt;
  if(g->windup_snd_tick>=0.5f){g->windup_snd_tick=0;fnae_push_sound(g,FNAE_SND_WINDUP);}
 } else g->windup_snd_tick=0;
 if(g->music_left<=0 && g->hidden_power>0 && g->death==0 && ((g->cam_anim==CAM_UP&&rnd(5)==1)||(g->mask_anim==MASK_DOWN&&rnd(5)==1))) enter_death(g,1);
}

void fnae_init(FnaeGame* g){ memset(g,0,sizeof(*g)); g->running=1; g->frame=FRAME_WARNING; { FnaeSave s; fnae_save_load(&s); g->night=s.night; g->progress=s.progress; } g->arrow=0; g->pc_mobile=0; g->static_frame=0; g->static_alpha=200; g->mouse_x=640; g->mouse_y=360; g->office_scroll=FNAE_OFFICE_SCROLL_MAX/2; g->cam_scroll=FNAE_CAM_SCROLL_MIN; g->cam_scroll_dir=0; g->cam_static_alpha=185; g->power_out_alpha=255;
 /* Customize-screen defaults: everything 0, including Puppet (owner
  * request — Fusion ships Puppet Global at 7, but starting at 0 means
  * the Add 1 / Set 20 buttons visibly work on the Puppet column too
  * instead of clamping 7 -> 7). */
 g->custom_freddy=0; g->custom_foxy=0; g->custom_springtrap=0; g->custom_golden=0;
 g->custom_mangle=0; g->custom_bb=0; g->custom_puppet=0; g->mask_frame=-1; }

void fnae_set_custom(FnaeGame* g,int freddy,int foxy,int springtrap,int golden,int mangle,int bb,int puppet){
 if(freddy<0)freddy=0; if(freddy>20)freddy=20;
 if(foxy<0)foxy=0; if(foxy>20)foxy=20;
 if(springtrap<0)springtrap=0; if(springtrap>20)springtrap=20;
 if(golden<0)golden=0; if(golden>20)golden=20;
 if(mangle<0)mangle=0; if(mangle>20)mangle=20;
 if(bb<0)bb=0; if(bb>20)bb=20;
  if(puppet<0)puppet=0; if(puppet>7)puppet=7;
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

/* Save helpers (Fusion INI group "Base"): load-modify-store so the
 * Challenge flags round-trip alongside Night/Progress.
 * Store failures are silent by design (see save.h). */
static void save_night(FnaeGame* g){
 FnaeSave s; fnae_save_load(&s);
 s.night=g->night;
 fnae_save_store(&s);
}
static void save_progress(FnaeGame* g,int earned){
 if(earned<=g->progress) return;
 FnaeSave s; fnae_save_load(&s);
 if(earned>s.progress) s.progress=earned;
 g->progress=s.progress;
 fnae_save_store(&s);
}
/* Challenge completion (Frame 5 Final: Night 7 + challenge A/B>0 writes
 * Challenge<A>=1). Persists the Left Challenge selector across the night
 * like the original (its A/B still read nonzero on the Final frame). */
static void save_challenge(FnaeGame* g,int ch){
 if(ch<1||ch>3) return;
 FnaeSave s; fnae_save_load(&s);
 s.challenge[ch]=1;
 fnae_save_store(&s);
 g->custom_check[ch]=1;
}

/* Frame 8 Customize (Frame 8 Events.txt): columns x-ordered like the
 * Layer #2 globals; boxes are the Select Box hotspots (top-left,
 * 150x200 like the portraits). Keep in sync with draw_customize in
 * visuals.c, which duplicates these rects for rendering. */
static const int cust_box_x[7]={85,246,406,566,726,886,1046};
static const int cust_box_y[7]={52,53,53,53,53,53,53};
#define CUST_BOX_W 150
#define CUST_BOX_H 200
/* up arrow at select+(36,88), down arrow 2 at select+(36,152); the
 * 50x25 triangle is drawn @1.3 scale (~65x33), center-anchored. */
#define CUST_ARROW_W 65
#define CUST_ARROW_H 33
/* Buttons (top-left art): GO! 250x55, Set 20 / Add 1 235x55. */
#define CUST_GO_X 1032
#define CUST_GO_Y 616
#define CUST_GO_W 250
#define CUST_GO_H 55
#define CUST_SET20_X 1039
#define CUST_SET20_Y 469
#define CUST_ADD1_X 1039
#define CUST_ADD1_Y 541
#define CUST_BTNW 235
#define CUST_BTNH 55
/* Challenge arrows flank the top bar (50x25 art, center-anchored). */
#define CUST_LCH_X 88
#define CUST_LCH_Y 16
#define CUST_RCH_X 456
#define CUST_RCH_Y 16
#define CUST_CH_W 50
#define CUST_CH_H 25

static int* custom_ai_ptr(FnaeGame* g,int idx){
 switch(idx){
  case 0: return &g->custom_freddy;
  case 1: return &g->custom_mangle;
  case 2: return &g->custom_foxy;
  case 3: return &g->custom_golden;
  case 4: return &g->custom_springtrap;
  case 5: return &g->custom_bb;
  case 6: return &g->custom_puppet;
  default: return NULL;
 }
}
static void custom_clamp(FnaeGame* g,int idx){
 int *p=custom_ai_ptr(g,idx); if(!p) return;
 int max=(idx==6)?7:20;
 if(*p<0)*p=0; if(*p>max)*p=max;
}
/* Challenge presets ([ Challenges ] group): switching challenge resets
 * the board to 0, then the listed levels apply while B>0. */
static void custom_preset(FnaeGame* g){
 int *f=&g->custom_freddy,*x=&g->custom_foxy,*s=&g->custom_springtrap,
     *o=&g->custom_golden,*m=&g->custom_mangle,*b=&g->custom_bb,*p=&g->custom_puppet;
 *f=*x=*s=*o=*m=*b=*p=0;
 if(g->custom_ch==1){*f=20;*x=5;*o=20;} /* The Classics */
 else if(g->custom_ch==2){*o=10;*b=20;*s=20;*m=20;} /* Broken Down */
 else if(g->custom_ch==3){*m=15;*o=20;*x=10;*f=15;*b=20;*s=8;*p=7;} /* Soy Sauce Edward */
}
static void custom_step(FnaeGame* g,int idx,int d){
 int *p=custom_ai_ptr(g,idx); if(!p) return;
 *p+=d; custom_clamp(g,idx);
 g->custom_b=0; /* any manual edit clears the preset hold */
 fnae_push_sound(g,FNAE_SND_TITLE_CHANGE); /* Change blip on ch #3 */
}
/* Hovered column from the pointer (-1 when over no Select Box). */
static int custom_hover(FnaeGame* g){
 for(int i=0;i<7;i++){
  if(g->mouse_x>=cust_box_x[i]&&g->mouse_x<cust_box_x[i]+CUST_BOX_W&&
     g->mouse_y>=cust_box_y[i]&&g->mouse_y<cust_box_y[i]+CUST_BOX_H) return i;
 }
 return -1;
}
static int custom_over_up(FnaeGame* g,int idx){
 int cx=cust_box_x[idx]+36, cy=cust_box_y[idx]+88;
 return g->mouse_x>=cx-CUST_ARROW_W/2&&g->mouse_x<cx+CUST_ARROW_W/2&&
        g->mouse_y>=cy-CUST_ARROW_H/2&&g->mouse_y<cy+CUST_ARROW_H/2;
}
static int custom_over_down(FnaeGame* g,int idx){
 int cx=cust_box_x[idx]+36, cy=cust_box_y[idx]+152;
 return g->mouse_x>=cx-CUST_ARROW_W/2&&g->mouse_x<cx+CUST_ARROW_W/2&&
        g->mouse_y>=cy-CUST_ARROW_H/2&&g->mouse_y<cy+CUST_ARROW_H/2;
}
static int custom_over_lch(FnaeGame* g){
 return g->mouse_x>=CUST_LCH_X-CUST_CH_W/2&&g->mouse_x<CUST_LCH_X+CUST_CH_W/2&&
        g->mouse_y>=CUST_LCH_Y-CUST_CH_H/2&&g->mouse_y<CUST_LCH_Y+CUST_CH_H/2;
}
static int custom_over_rch(FnaeGame* g){
 return g->mouse_x>=CUST_RCH_X-CUST_CH_W/2&&g->mouse_x<CUST_RCH_X+CUST_CH_W/2&&
        g->mouse_y>=CUST_RCH_Y-CUST_CH_H/2&&g->mouse_y<CUST_RCH_Y+CUST_CH_H/2;
}
/* Per-tick Customize update: hover selects, held arrows repeat every
 * 0.10 s (Every 00''-10), presets hold while B>0. */
static void update_customize(FnaeGame* g,float dt){
 int hov=custom_hover(g);
 if(hov>=0) g->custom_sel=hov;
 if(g->custom_b>0) custom_preset(g);
 if(g->mouse_down&&g->custom_arrow_dir!=0){
  int s=g->custom_sel;
  int over=(g->custom_arrow_dir>0)?custom_over_up(g,s):custom_over_down(g,s);
  if(!over){g->custom_arrow_dir=0;g->custom_arrow_tick=0;return;}
  g->custom_arrow_tick+=dt;
  while(g->custom_arrow_tick>=0.10f){
   g->custom_arrow_tick-=0.10f;
   custom_step(g,s,g->custom_arrow_dir);
  }
 } else g->custom_arrow_tick=0;
}
/* Frame 8 entry (Start of Frame): AI globals persist across visits (only
 * the challenge selector, hold state, background frame and the cached
 * Check flags reset). */
static void enter_customize(FnaeGame* g){
 g->frame=FRAME_CUSTOMIZE;
 g->custom_sel=0; g->custom_ch=0; g->custom_b=0;
 g->custom_arrow_dir=0; g->custom_arrow_tick=0;
 g->custom_cool=rnd(3);
 g->custom_check[1]=g->custom_check[2]=g->custom_check[3]=0;
 { FnaeSave s; if(fnae_save_load(&s)==0)
   for(int i=1;i<=3;i++) g->custom_check[i]=s.challenge[i]?1:0; }
}

/* Frame 2 Title entry: re-read Night/Progress from the save like the
 * Fusion Start-of-Frame events (The Night counter = INI Night, stars =
 * INI Progress). This keeps the Continue counter on the saved story
 * night: 6th/custom runs set g->night=6/7 without saving, so returning
 * to Title must not keep showing 6/7. Also clears the Frame 3 death
 * state, which is frame-local in Fusion but global here. */
static void enter_title(FnaeGame* g){
 FnaeSave s; if(fnae_save_load(&s)==0){ g->night=s.night; g->progress=s.progress; }
 if(g->night<1)g->night=1; if(g->night>7)g->night=7;
 if(g->progress<0)g->progress=0; if(g->progress>3)g->progress=3;
 g->death=0; g->death_addup=0; g->death_tick_acc=0;
 g->death_red=0; g->death_red_peaked=0; g->death_rip_a=255; g->death_rip_b=0; g->death_timer=0; g->death_ticks=0;
 g->time_to_hour=0; g->power_out_timer=0;
 g->frame=FRAME_TITLE;
}
/* New Game: Newspaper start resets story Night to 1 in the save
 * (Fusion), so Continue restarts the story but keeps star Progress. */
static void enter_newspaper(FnaeGame* g){
 g->six_or_seven=0; g->night=1; save_night(g); g->frame=FRAME_NEWSPAPER;
}
/* Frame 6 routing: 0 = normal night (re-read from the save, like the
 * Fusion Which-Night/Night Start-of-Frame events), 1 = 6th, 2 = 7th/custom.
 * Matches Frame 6 Start-of-Frame events; the frame then auto-advances
 * after 2 seconds (see fnae_update). */
static void enter_which_night(FnaeGame* g){
 if(g->six_or_seven==1) g->night=6;
 else if(g->six_or_seven==2) g->night=7;
 else { FnaeSave s; if(fnae_save_load(&s)==0) g->night=s.night; }
 if(g->night<1)g->night=1; if(g->night>7)g->night=7;
 g->frame=FRAME_WHICH_NIGHT; g->which_timer=0;
}

void fnae_start_night(FnaeGame* g,int night){
 memset(g->hour_events,0,sizeof(g->hour_events));
 g->night=night<1?1:(night>7?7:night); g->frame=FRAME_NIGHT; g->time_of_day=12; g->time_to_hour=0;
  g->death=0; g->death_addup=0; g->gf_random=0; g->gf_death_addup=0;
  g->death_tick_acc=0; g->gf_tick_acc=0;
 g->death_red=0; g->death_red_peaked=0; g->death_rip_a=255; g->death_rip_b=0; g->death_timer=0; g->death_ticks=0;
 g->cam_anim=CAM_DOWN; g->mask_anim=MASK_UP; g->prevent_flip=0; g->force_down=0; g->view=0; g->camera=1;
  g->left_door=0; g->right_door=0; g->flashlight=0; g->hidden_power=10000; g->power_left=1; g->power_tick=0;
  g->mouse_x=640; g->mouse_y=360; g->office_scroll=FNAE_OFFICE_SCROLL_MAX/2;
  g->cam_scroll=FNAE_CAM_SCROLL_MIN; g->cam_scroll_dir=0;
   g->left_door_frame=0; g->right_door_frame=0; g->title_bg_frame=0; g->title_bg_timer=0;
  g->movement_out=0; g->movement_timer=0; g->movement_half_tick=0; g->movement_force_tick=0;
  g->camera_up_check=0; g->cam_static_alpha=185; g->cam_static_tick=0;
  g->warning=0; g->power_out_alpha=255;
  /* Springtrap starts on Cam 02's box ("Start of Frame + Cam 02 Text
   * overlapping CAM 01 -> Springtrap at (0,0) from CAM 01"). */
  g->springtrap_pos=2; g->springtrap_stand=0; g->lure_area=0; g->lure_cam=0; g->lure_timer=0;
  g->lure_cd=0; g->lure_cd_timer=0;
  g->foxy_stand=0; g->freddy_door=0;
 g->music_left=2000; g->music_winding=0; g->music_tick=0; g->current_call=0; g->call_muted=0;
 g->mouse_down=0; g->key_wind=0;
 g->windup_snd_tick=0; g->snd_head=g->snd_tail=0;
 g->cam_anim_timer=g->mask_anim_timer=g->left_door_timer=g->right_door_timer=0; g->mask_frame=-1;
 g->ai_timer=g->power_out_timer=g->springtrap_timer=g->phantom_timer=0;
 ai_reset(&g->foxy,0,2); ai_reset(&g->freddy,0,1); g->springtrap_a=0;g->springtrap_b=0;
    g->ph_mangle_a=g->ph_mangle_b=g->ph_mangle_c=0;g->ph_annoy_a=g->ph_annoy_b=0;g->ph_bb_a=g->ph_bb_b=0;
    g->ph_prev_view=0;g->ph_prev_cam=0;
    g->ph_bb_scare=255;g->ph_bb_scare_on=0;g->ph_bb_scare_timer=0;
 g->golden_ai=0;g->foxy_ai=g->freddy_ai=g->springtrap_ai=g->ph_mangle_ai=g->ph_bb_ai=0;g->puppet_ai=0;
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

/* Camera-feed auto-pan ("[ Camera Scrolling ]" in Frame 3 Events.txt).
 * The Camera Center Object drifts +/-1 px per tick and bounces direction
 * at each end; while a camera feed is up the display follows it, so the
 * feed slowly pans left <-> right. The Fusion bounds overshoot 120 px
 * past each feed edge, but the native pan is clamped to the image ends
 * so no black bars show. Alterable Value B is set to 1 at Start of Frame
 * and never changes, so the drift runs unconditionally (even with the
 * cameras down).
 * Speeds are per 1/60 tick, hence the dt*60 scaling. */
static void update_cam_scroll(FnaeGame* g, float dt){
 if(g->frame!=FRAME_NIGHT) return;
 float step=dt*60.0f;
 if(g->cam_scroll_dir==0){
  g->cam_scroll+=step;
  if(g->cam_scroll>=FNAE_CAM_SCROLL_MAX){g->cam_scroll=FNAE_CAM_SCROLL_MAX;g->cam_scroll_dir=1;}
 } else {
  g->cam_scroll-=step;
  if(g->cam_scroll<=FNAE_CAM_SCROLL_MIN){g->cam_scroll=FNAE_CAM_SCROLL_MIN;g->cam_scroll_dir=0;}
 }
}

void fnae_update(FnaeGame* g,float dt){
  /* Frame 1 interstitial: Timer equals 05'' -> Title, no input required. */
 if(g->frame==FRAME_WARNING){
  g->warn_timer+=dt;
  if(g->warn_timer>=5.0f) enter_title(g);
  return;
 }
 /* Frame 6 interstitial: Every 02'' -> Night, no input required. */
 if(g->frame==FRAME_WHICH_NIGHT){
  g->which_timer+=dt;
  if(g->which_timer>=2.0f) fnae_start_night(g,g->night);
  return;
 }
 /* Frame 8 Customize runs its own tick (hover/arrows/presets). */
 if(g->frame==FRAME_CUSTOMIZE){ update_customize(g,dt); return; }
  if(g->frame!=FRAME_NIGHT){
   /* Frame 4 Death animation (Frame 4 Events.txt): fullscreen red flashes
    * +7/tick to 255, then drains back to 0 (owner: a brief flash, not a
    * permanent overlay) while the RIP Text fades -7/tick out (B==0),
    * waits the 0.5 s gates (B 0->1->2, switching RIP frame 404 to the
    * GAME OVER frame), fades +7/tick back in (B>1), then jumps to Title
    * (which stops the ch #32 goblin loop via frame entry). The Fusion
    * gates are 1 s; the native halves them per owner (snappier pause).
    * The Fusion RIP fade keys off red>=255; the native latches that moment
    * (death_red_peaked) so the text keeps running as the flash clears.
    * The devil-card Death Anim cycles underneath the whole time, but the
    * renderer holds it (and the RIP text) until the flash first peaks, so
    * the red shows alone first. */
    if(g->frame==FRAME_DEATH){
     /* Red/RIP fades are per-tick (+7) in Fusion: run them through the
      * tick accumulator so the Death screen lasts the same real time
      * on any refresh rate (at 144 Hz+ it used to fly by). The B-gates
      * below are already dt-based and run once per call; they are 0.5 s
      * each (owner-shortened from the Fusion 1 s) so the RIP to GAME
      * OVER pause is snappier. */
     g->death_tick_acc+=dt*60.0f;
     int dsteps=(int)g->death_tick_acc; g->death_tick_acc-=dsteps;
     if(dsteps>4)dsteps=4;
     for(int i=0;i<dsteps;i++){
      if(!g->death_red_peaked){
       g->death_red+=7;
       if(g->death_red>=255){g->death_red=255;g->death_red_peaked=1;}
      }
      else {
       if(g->death_red>0){g->death_red-=7;if(g->death_red<0)g->death_red=0;}
       if(g->death_rip_a>0 && g->death_rip_b==0){g->death_rip_a-=7;if(g->death_rip_a<0)g->death_rip_a=0;}
       else if(g->death_rip_a<255 && g->death_rip_b>1){g->death_rip_a+=7;if(g->death_rip_a>255)g->death_rip_a=255;}
       if(g->death_rip_b>0 && g->death_rip_a>=255){enter_title(g);return;}
      }
      g->death_ticks++;
     }
     if(g->death_red_peaked){
      if(g->death_rip_a<=0){
       g->death_timer+=dt;
       if(g->death_timer>=0.5f){g->death_timer=0;if(g->death_rip_b==0)g->death_rip_b=1;else if(g->death_rip_b==1)g->death_rip_b=2;}
      } else g->death_timer=0;
     } else g->death_timer=0;
     return;
    }
   /* Night simulation (power, AI, phantoms, music box, death rolls) is
    * Frame 3-only in Fusion. Without this gate the title/newspaper/
    * customize/6AM/final screens kept draining power and rolling deaths
    * (power_out fades 255->0 then enter_death fires ~6 s after boot),
    * so idling on the title ended on the death screen. */
   return;
  }
  update_cam_scroll(g,dt);
  /* Death wait ([ Jumpscares ]: fullscreen scare over the shaking office
   * for 60 ticks ~= 1.0 s, then the Death frame): tick-accumulated like
   * the Death-frame fades so the hold lasts a real second at any Hz. */
  if(g->death){
   g->death_tick_acc+=dt*60.0f;
   int wsteps=(int)g->death_tick_acc; g->death_tick_acc-=wsteps;
   if(wsteps>4)wsteps=4;
   for(int i=0;i<wsteps;i++){
    g->death_addup++;
    if(g->death_addup>=60){g->frame=FRAME_DEATH;g->death_red=0;g->death_red_peaked=0;g->death_rip_a=255;g->death_rip_b=0;g->death_timer=0;g->death_ticks=0;g->death_tick_acc=0;break;}
   }
   return;
  }
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

 /* Mask overlay frames (Frame 3 "[ Mask ]", owner-confirmed order:
  * Flip Mask Down 134-140 at [0,0] while putting on, worn mask 129
  * (1480x870 at [-100,-66]) while down, Flip Mask Up 141-143 at [0,0]
  * while taking off. Played proportionally over the 0.45 s transitions
  * above, like the door shutters. */
 {
  int f=-1;
  if(g->mask_anim==MASK_UP_ANIM){f=(int)(7.0f*g->mask_anim_timer/0.45f);if(f<0)f=0;if(f>6)f=6;}
  else if(g->mask_anim==MASK_DOWN)f=7;
  else if(g->mask_anim==MASK_DOWN_ANIM){f=8+(int)(3.0f*g->mask_anim_timer/0.45f);if(f>10)f=10;}
  g->mask_frame=f;
 }

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
   /* [ Force Down ] (Fusion: Sub 1, then while >0 force Anim 2 -> 3).
    * This is what actually drops the cameras on the phantom scares
    * (BB B>80, Mangle B>60). View clears immediately like the Fusion
    * Anim==3 event, and the down cassette plays. */
   if(g->force_down>0){
    g->force_down--;
    if(g->force_down>0 && g->cam_anim==CAM_UP){
     g->cam_anim=CAM_DOWN_ANIM;g->cam_anim_timer=0;
     g->view=0;g->camera_up_check=0;
     fnae_push_sound(g,FNAE_SND_CAM_DOWN);
    }
   }

  g->ai_timer+=dt;
 if(g->ai_timer>=5){g->ai_timer-=5;ai_move(g);
   /* Springtrap steps between cameras on its move flag, then clears it.
    * Routes mirror the "[ Springtrap (Audio Lure) ]" overlap events:
    * Cam 01 -> Cam 03 (B=0) / Cam 02 (B=1), Cam 02 -> Cam 04 (B=0) /
    * Cam 01 (B=1), Cam 03 -> Cam 01, Cam 04 -> Cam 02. B re-rolls on
    * Cam 01/02 like the Fusion RRandom(0,1) sets. */
   if(g->springtrap_a && g->death==0){
    g->springtrap_a=0;
    int p=g->springtrap_pos;
    if(p==1) g->springtrap_pos=(g->springtrap_b==0)?3:2;
    else if(p==2) g->springtrap_pos=(g->springtrap_b==0)?4:1;
    else if(p==3) g->springtrap_pos=1;
    else if(p==4) g->springtrap_pos=2;
    /* pos 3 is the kill room: Springtrap waits there for the death rolls. */
    if(g->springtrap_pos==1||g->springtrap_pos==2) g->springtrap_b=rr(0,1);
   }
 }
  /* Audio lure marker: 2s after placing it, 50% to pull Springtrap to
   * the lured cam. Destroying the marker does NOT end the button
   * cooldown (see below); placement is gated on the cooldown only. */
  if(g->lure_area){
   g->lure_timer+=dt;
   if(g->lure_timer>=2.0f){g->lure_timer-=2.0f;
    if(rnd(2)==1){g->springtrap_pos=g->lure_cam;g->movement_out=1;g->movement_half_tick=0;g->movement_force_tick=0;}
    g->lure_area=0;}
  }
  /* Lure-button cooldown (Animation 12, ~2 s window): runs independently
   * of the marker so an early destroy never shortens it. A new lure
   * requires the cooldown to have fully elapsed. */
  if(g->lure_cd){
   g->lure_cd_timer+=dt;
   /* Fusion plays stop on ch #14 when Animation 12 is over. */
   if(g->lure_cd_timer>=2.0f){g->lure_cd=0;g->lure_cd_timer=0;fnae_push_sound(g,FNAE_SND_LURE_STOP);}
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
   update_phantoms(g,dt); update_gf(g,dt); update_music(g,dt);
 if(g->springtrap_pos==3 && g->view==3 && g->hidden_power>0 && g->death==0){g->springtrap_timer+=dt;if(g->springtrap_timer>=4){g->springtrap_timer=0;if(rnd(2)==1)enter_death(g,4);}}
 if(g->current_call==0 && g->time_to_hour>=3)g->current_call=g->night;
 if(g->cam_anim==CAM_UP)g->view=g->camera; else if(g->cam_anim==CAM_DOWN)g->view=0;
 g->camera_up_check=(g->view>0);
}

void fnae_key(FnaeGame* g,int key){
 /* Fusion jumps Customize -> Title on Escape; everywhere else Escape quits. */
 if(key==SDLK_ESCAPE){
  if(g->frame==FRAME_CUSTOMIZE){enter_title(g);return;}
  g->running=0;return;
 }
 /* Frame 1: Upon pressing any key -> Title (Fusion has no click event here). */
 if(g->frame==FRAME_WARNING){enter_title(g);return;}
 if(g->frame==FRAME_TITLE){
   if(key==SDLK_RETURN){
    if(g->arrow==0)enter_newspaper(g);
    else if(g->arrow==1){g->six_or_seven=0;enter_which_night(g);}
   else if(g->arrow==2){g->six_or_seven=1;enter_which_night(g);}
   else if(g->arrow==3){g->six_or_seven=2;enter_customize(g);}
  } else if(key==SDLK_UP || key=='w'){if(g->arrow>0){g->arrow--;fnae_push_sound(g,FNAE_SND_TITLE_CHANGE);} }
  else if(key==SDLK_DOWN || key=='s'){int max=g->progress+1;if(max>3)max=3;if(g->arrow<max){g->arrow++;fnae_push_sound(g,FNAE_SND_TITLE_CHANGE);} }
  if(g->arrow<0)g->arrow=0;{int max=g->progress+1;if(max>3)max=3;if(g->arrow>max)g->arrow=max;}return;
 }
 if(g->frame==FRAME_NEWSPAPER){if(key==SDLK_RETURN)enter_which_night(g);return;}
 if(g->frame==FRAME_WHICH_NIGHT){if(key==SDLK_RETURN)fnae_start_night(g,g->night);return;}
 if(g->frame==FRAME_CUSTOMIZE){
  if(key==SDLK_RETURN){g->custom_arrow_dir=0;g->six_or_seven=2;enter_which_night(g);}
  return;
 }
  if(g->frame==FRAME_6AM){if(key==SDLK_RETURN){
   /* Fusion: nights 6/7 or Night Story >= 5 -> Final, else next night -> Which Night. */
    if(g->six_or_seven>0||g->night>=5){
     /* Final-frame Progress writes: story night 5 -> 1, 6th -> 2,
      * 7th/custom all-20 -> 3 (a plain night-7 clear writes nothing).
      * A custom clear with an unmodified challenge preset (A/B>0)
      * additionally writes Challenge<A>=1 (Frame 5 Final). */
     if(g->six_or_seven==2){
      if(g->all20) save_progress(g,3);
      if(g->custom_ch>0&&g->custom_b>0) save_challenge(g,g->custom_ch);
     }
     else if(g->six_or_seven==1) save_progress(g,2);
     else save_progress(g,1);
     g->frame=FRAME_FINAL;
    }
   else {g->night++;g->six_or_seven=0;save_night(g);enter_which_night(g);}
  }return;}
 if(g->frame==FRAME_DEATH){if(key==SDLK_RETURN)enter_title(g);return;}
 if(g->frame==FRAME_FINAL){if(key==SDLK_RETURN)enter_title(g);return;}
 if(g->frame!=FRAME_NIGHT)return;

 if(key=='a'&&g->view==0&&g->hidden_power>0){if(g->left_door==0){g->left_door=1;fnae_push_sound(g,FNAE_SND_DOOR);}else if(g->left_door==2){g->left_door=3;fnae_push_sound(g,FNAE_SND_DOOR);} }
 if(key=='d'&&g->view==0&&g->hidden_power>0){if(g->right_door==0){g->right_door=1;fnae_push_sound(g,FNAE_SND_DOOR);}else if(g->right_door==2){g->right_door=3;fnae_push_sound(g,FNAE_SND_DOOR);} }
 if(key=='s' && g->hidden_power>0){
  if(g->cam_anim==CAM_DOWN && g->mask_anim==MASK_UP){g->cam_anim=CAM_UP_ANIM;g->cam_anim_timer=0;fnae_push_sound(g,FNAE_SND_CAM_UP);}
  else if(g->cam_anim==CAM_UP && g->mask_anim==MASK_UP){g->cam_anim=CAM_DOWN_ANIM;g->cam_anim_timer=0;fnae_push_sound(g,FNAE_SND_CAM_DOWN);}
 }
 if(key=='m'&&g->hidden_power>0&&g->cam_anim==CAM_DOWN){if(g->mask_anim==MASK_UP){g->mask_anim=MASK_UP_ANIM;g->mask_anim_timer=0;fnae_push_sound(g,FNAE_SND_MASK_ON);}else if(g->mask_anim==MASK_DOWN){g->mask_anim=MASK_DOWN_ANIM;g->mask_anim_timer=0;fnae_push_sound(g,FNAE_SND_MASK_OFF);} }
 if(key=='z'||key==SDLK_LALT)g->flashlight=1;
 if(g->cam_anim==CAM_UP){if(key>='1'&&key<='4')g->camera=key-'0';}
  /* Audio lure: E while watching a camera feed (never from the music-box cam)
   * places a Lure Area on the viewed camera; Springtrap may follow (see update).
   * Gated on the button cooldown only — marker state is unrelated. */
  if(key=='e'&&g->view>0&&g->view!=4&&g->hidden_power>0&&g->death==0&&g->lure_cd==0){
   g->lure_area=1; g->lure_cam=g->view; g->lure_timer=0; g->lure_cd=1; g->lure_cd_timer=0;
   /* Fusion sets Lure which to Random(3)+1 for the echo1/3b/4b sample. */
   fnae_push_sound(g,FNAE_SND_LURE1+rnd(3));}
 /* R is a keyboard test/control for winding the music box (held state;
  * the per-tick evaluation in update_music applies the view/death gates). */
 if(key=='r'&&g->view==4)g->key_wind=1;
}

void fnae_key_up(FnaeGame* g,int key){
 if((key=='z'||key==SDLK_LALT) && g->frame==FRAME_NIGHT)g->flashlight=0;
 if(key=='r' && g->frame==FRAME_NIGHT)g->key_wind=0;
}

void fnae_press(FnaeGame* g,int x,int y){
 g->mouse_x=x; g->mouse_y=y;
 if(g->frame==FRAME_NIGHT||g->frame==FRAME_CUSTOMIZE)g->mouse_down=1;
}

void fnae_release(FnaeGame* g){
 g->mouse_down=0;
 g->custom_arrow_dir=0; g->custom_arrow_tick=0;
}

void fnae_click(FnaeGame* g,int x,int y){
 g->mouse_x=x; g->mouse_y=y;
  if(g->frame==FRAME_TITLE){
   if(x>=70&&x<=430&&y>=430&&y<495){g->arrow=0;enter_newspaper(g);fnae_push_sound(g,FNAE_SND_TITLE_CHANGE);return;}
  if(x>=70&&x<=430&&y>=495&&y<560){g->arrow=1;g->six_or_seven=0;enter_which_night(g);fnae_push_sound(g,FNAE_SND_TITLE_CHANGE);return;}
  if(x>=70&&x<=430&&y>=560&&y<625 && g->progress>0){g->arrow=2;g->six_or_seven=1;enter_which_night(g);fnae_push_sound(g,FNAE_SND_TITLE_CHANGE);return;}
  if(x>=70&&x<=430&&y>=625&&y<700 && g->progress>1){g->arrow=3;g->six_or_seven=2;enter_customize(g);fnae_push_sound(g,FNAE_SND_TITLE_CHANGE);return;}
   return;
  }
 /* Newspaper advances on any click, like the Fusion event. */
 if(g->frame==FRAME_NEWSPAPER){enter_which_night(g);return;}
 /* Frame 8 Customize clicks (Frame 8 Events.txt): arrows hold-repeat
  * (first step fires here, repeats in update_customize), Set 20 / Add 1
  * bump the hovered column, challenge arrows swap presets, GO! starts. */
 if(g->frame==FRAME_CUSTOMIZE){
  int hov=custom_hover(g);
  if(hov>=0) g->custom_sel=hov;
  int s=g->custom_sel;
  if(custom_over_up(g,s)){custom_step(g,s,+1);g->custom_arrow_dir=+1;g->custom_arrow_tick=0;return;}
  if(custom_over_down(g,s)){custom_step(g,s,-1);g->custom_arrow_dir=-1;g->custom_arrow_tick=0;return;}
  if(x>=CUST_SET20_X&&x<CUST_SET20_X+CUST_BTNW&&y>=CUST_SET20_Y&&y<CUST_SET20_Y+CUST_BTNH){
   int *p=custom_ai_ptr(g,s); if(p){*p=20;custom_clamp(g,s);g->custom_b=0;fnae_push_sound(g,FNAE_SND_TITLE_CHANGE);}return;}
  if(x>=CUST_ADD1_X&&x<CUST_ADD1_X+CUST_BTNW&&y>=CUST_ADD1_Y&&y<CUST_ADD1_Y+CUST_BTNH){
   custom_step(g,s,+1);return;}
  if(custom_over_lch(g)){g->custom_ch--;if(g->custom_ch<0)g->custom_ch=3;g->custom_b=1;custom_preset(g);g->custom_arrow_dir=0;fnae_push_sound(g,FNAE_SND_TITLE_CHANGE);return;}
  if(custom_over_rch(g)){g->custom_ch++;if(g->custom_ch>3)g->custom_ch=0;g->custom_b=1;custom_preset(g);g->custom_arrow_dir=0;fnae_push_sound(g,FNAE_SND_TITLE_CHANGE);return;}
  if(x>=CUST_GO_X&&x<CUST_GO_X+CUST_GO_W&&y>=CUST_GO_Y&&y<CUST_GO_Y+CUST_GO_H){
   g->custom_arrow_dir=0;g->six_or_seven=2;enter_which_night(g);return;}
  g->custom_arrow_dir=0;return;
 }
 if(g->frame!=FRAME_NIGHT)return;
 /* Mute Call button (121x31 center-anchored at [100,55]): stops the
  * night's phone call while it plays. The button only shows then, so a
  * click elsewhere here is a no-op for it. */
 if(g->current_call!=0&&x>=40&&x<160&&y>=40&&y<70){g->call_muted=1;fnae_push_sound(g,FNAE_SND_CALL_STOP);return;}
 if(g->view==0 && g->hidden_power>0){
  /* Door click zones follow the panning doors: compare in frame space
   * (screen x + scroll) so the zones stay glued to the door art. */
  int fx=x+(int)g->office_scroll;
  if(fx<180 && y>500){if(g->left_door==0){g->left_door=1;fnae_push_sound(g,FNAE_SND_DOOR);}else if(g->left_door==2){g->left_door=3;fnae_push_sound(g,FNAE_SND_DOOR);}return;}
  if(fx>1100 && y>500){if(g->right_door==0){g->right_door=1;fnae_push_sound(g,FNAE_SND_DOOR);}else if(g->right_door==2){g->right_door=3;fnae_push_sound(g,FNAE_SND_DOOR);}return;}
  if(x>500 && x<780 && y>560){g->cam_anim=CAM_UP_ANIM;g->cam_anim_timer=0;fnae_push_sound(g,FNAE_SND_CAM_UP);return;}
 }
  /* Camera buttons: clicks on any "CAM 01" box move You onto it and the
   * view follows ("[ Is Up ]" + "[ Cam 01 ]" groups). Rects are the verbatim
   * Frame 3 Objects.txt button hotspots (60x40 boxes, center-anchored like
   * the minimap renderer in visuals.c); the viewed feed follows g->camera in
   * fnae_update, like the Fusion You-overlap events. */
    if(g->cam_anim==CAM_UP && g->hidden_power>0){
     /* CAM 01 rides 32px above its Objects.txt hotspot (see visuals.c). */
     static const int btn_x[4]={1016,1179,953,1161};
     static const int btn_y[4]={307,371,469,505};
    for(int i=0;i<4;i++){
     if(x>=btn_x[i]-30&&x<btn_x[i]+30&&y>=btn_y[i]-20&&y<btn_y[i]+20){
      g->camera=i+1;
      /* Clicking a cam button dismisses Phantom BB (Fusion "User clicks
       * with left button on CAM 01 + View > 0 -> A/B = 0"). */
      if(g->view>0){g->ph_bb_a=0;g->ph_bb_b=0;}
      break;}
    }
     /* Audio-lure button ("Lure" 128x64 at [744,296], center-anchored
      * like the cam buttons): clicking it lures like the E key, except
      * on the music-box camera (Fusion hides it over Cam 04 Text).
      * Gated on the button cooldown only — marker state is unrelated. */
     if(g->view>0&&g->view!=4&&g->death==0&&g->lure_cd==0){
      if(x>=744-64&&x<744+64&&y>=296-32&&y<296+32){g->lure_area=1;g->lure_cam=g->view;g->lure_timer=0;g->lure_cd=1;g->lure_cd_timer=0;fnae_push_sound(g,FNAE_SND_LURE1+rnd(3));}
    }
   }
  /* Winding is hold-driven (see fnae_press/fnae_release + update_music);
  * a click alone never latches it. */
 }
