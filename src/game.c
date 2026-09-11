#include "fnae.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static void enemy_reset(Enemy *e,int ai){ memset(e,0,sizeof(*e)); e->ai=ai; e->active=ai>0; }

void game_start_night(Game *g,int n){
    if(n<1)n=1; if(n>7)n=7;
    g->night=n; g->state=OFFICE; g->night_time=0; g->power=100;
    g->camera_up=g->left_door=g->right_door=g->mask=g->flashlight=g->audio_lure=0;
    g->lure_timer=0; g->power_out=0; g->camera=0;
    enemy_reset(&g->edward,n); enemy_reset(&g->nomnom,n);
    enemy_reset(&g->shiny,n>=2?n:0); enemy_reset(&g->boulder,n>=2?n:0);
    enemy_reset(&g->smiley,n>=3?n:0); enemy_reset(&g->soy,n>=4?n:0);
    enemy_reset(&g->business,n>=2?n:0);
}

void game_init(Game *g){ memset(g,0,sizeof(*g)); g->running=1; g->night=1; g->state=MENU; }

static void move_ai(Enemy *e){
    if(!e->active) return;
    if((rand()%20)<e->ai){ e->pos++; if(e->pos>3)e->pos=3; }
}

void game_update(Game *g,float dt){
    if(g->state!=OFFICE && g->state!=CAMERAS) return;
    g->night_time+=dt;
    g->power -= dt * .055f * (1.0f + g->camera_up*.2f + g->left_door*.35f + g->right_door*.35f + g->mask*.1f + g->boulder.active*.05f);
    if(g->power<=0){g->power=0;g->power_out=1;g->camera_up=0;}
    if(g->lure_timer>0){g->lure_timer-=dt;if(g->lure_timer<=0)g->audio_lure=0;}
    static float ai_tick=0; ai_tick+=dt;
    if(ai_tick>=5){ai_tick-=5; move_ai(&g->edward);move_ai(&g->nomnom);move_ai(&g->shiny);move_ai(&g->boulder);move_ai(&g->smiley);move_ai(&g->soy);move_ai(&g->business);}
    if(g->soy.pos>=3 && !g->mask) g->state=GAME_OVER;
    if(g->edward.pos>=3 && !g->left_door && !g->right_door) g->state=GAME_OVER;
    if(g->night_time>=360){g->state=SIX_AM;}
}

void game_event(Game *g,const SDL_Event *e){
    if(e->type==SDL_QUIT){g->running=0;return;}
    if(e->type!=SDL_KEYDOWN||e->key.repeat)return;
    SDL_Keycode k=e->key.keysym.sym;
    if(g->state==MENU){if(k==SDLK_RETURN||k==SDLK_SPACE)game_start_night(g,g->night);return;}
    if(g->state==SIX_AM){if(k==SDLK_RETURN||k==SDLK_SPACE)game_start_night(g,g->night<7?g->night+1:7);return;}
    if(g->state==GAME_OVER){if(k==SDLK_RETURN||k==SDLK_SPACE)game_start_night(g,g->night);return;}
    if(g->state==OFFICE){
        if(k==SDLK_c){g->state=CAMERAS;g->camera_up=1;}
        else if(k==SDLK_l)g->left_door=!g->left_door;
        else if(k==SDLK_r)g->right_door=!g->right_door;
        else if(k==SDLK_m)g->mask=!g->mask;
        else if(k==SDLK_f)g->flashlight=!g->flashlight;
    } else if(g->state==CAMERAS){
        if(k==SDLK_c||k==SDLK_ESCAPE){g->state=OFFICE;g->camera_up=0;}
        else if(k>=SDLK_1&&k<=SDLK_4)g->camera=k-SDLK_1;
        else if(k==SDLK_l){g->audio_lure=1;g->lure_timer=3;}
    }
}

void game_render(SDL_Renderer *r,Game *g){
    SDL_SetRenderDrawColor(r,0,0,0,255);SDL_RenderClear(r);
    if(g->state==MENU){ draw_asset(r,2,1); }
    else if(g->state==OFFICE){
        /* 350/379 are the forest-sized scene assets; 227 is a major red scene.
           The actual frame mapping is isolated here so it can be corrected
           without changing game logic. */
        draw_asset(r,227,1);
    } else if(g->state==CAMERAS){
        static const int cams[FNAE_CAM_COUNT]={211,212,350,379};
        draw_asset(r,cams[g->camera],1);
    } else if(g->state==SIX_AM){ draw_asset(r,4,1); }
    else { draw_asset(r,7,1); }
    SDL_RenderPresent(r);
}
