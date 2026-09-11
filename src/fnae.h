#pragma once
#include <SDL.h>
#include <SDL_image.h>

#define FNAE_CAM_COUNT 4

typedef enum { MENU, OFFICE, CAMERAS, NIGHT_STORY, SIX_AM, GAME_OVER } State;
typedef struct { int ai, pos; float timer; int active; } Enemy;
typedef struct {
    State state; int running, night, camera; float night_time, power;
    int camera_up, left_door, right_door, mask, flashlight, audio_lure;
    float lure_timer; int power_out;
    Enemy edward, nomnom, shiny, boulder, smiley, soy, business;
} Game;

typedef struct { SDL_Texture *tex; int w, h; } Texture;

int assets_init(SDL_Renderer *r);
void assets_shutdown(void);
SDL_Texture *asset_image(int id);
void draw_asset(SDL_Renderer *r, int id, int fit);

void game_init(Game *g);
void game_start_night(Game *g, int n);
void game_event(Game *g, const SDL_Event *e);
void game_update(Game *g, float dt);
void game_render(SDL_Renderer *r, Game *g);
