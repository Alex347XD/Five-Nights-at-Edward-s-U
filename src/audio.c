#include "audio.h"

#include <SDL_mixer.h>
#include <stdio.h>
#include <string.h>

/* Fusion Sound channels (Frame 3 Events.txt [ Audio ] group). */
#define CH_FAN 1
#define CH_DEPTHS 2
#define CH_CAMAU 3
#define CH_CHANGE 4
#define CH_FLIP 5
#define CH_BREATH 6
#define CH_DOOR 7
#define CH_MELODY 8
#define CH_STARE 9
#define CH_BUZZ 10
#define CH_WINDUP 11
#define CH_FOOT 12
#define CH_CLOSE 13
#define CH_LURE 14
#define CH_WALK 15
#define CH_CALL 16
#define CH_MANGLE 17
#define CH_PHBB 18
#define CH_POWER 19
#define CH_JACK 20
#define CH_GOBLIN 32

/* Fusion 0-100 volumes mapped to mixer 0-128. */
#define V(x) ((x)*128/100)

static Mix_Chunk *load_one(const char *dir, const char *name) {
 char path[512];
 snprintf(path, sizeof path, "%s/%s", dir, name);
 Mix_Chunk *c = Mix_LoadWAV(path);
 if (!c)
  fprintf(stderr, "AUDIO: cannot load %s: %s\n", path, Mix_GetError());
 return c;
}

int fnae_audio_init(FnaeAudio *a, const char *dir) {
 memset(a, 0, sizeof *a);
 a->prev_frame = -1;
 if (Mix_Init(MIX_INIT_MP3) == 0)
  fprintf(stderr, "AUDIO: mp3 decoder unavailable: %s\n", Mix_GetError());
 if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) != 0) {
  fprintf(stderr, "AUDIO: Mix_OpenAudio failed (silent): %s\n", Mix_GetError());
  return 0;
 }
 Mix_AllocateChannels(40);

 a->fan = load_one(dir, "fansound.wav");
 a->depths = load_one(dir, "In The Depths C.wav");
 a->camau = load_one(dir, "Camera Audio.wav");
 a->change = load_one(dir, "Change.wav");
 a->flip_up = load_one(dir, "STEREO_CASSETTE__90097704.wav");
 a->flip_down = load_one(dir, "STEREO_CASSETTE__90097701.wav");
 a->mask_on = load_one(dir, "FENCING_43_GEN-HDF10954.wav");
 a->mask_off = load_one(dir, "FENCING_42_GEN-HDF10953.wav");
 a->breath = load_one(dir, "deepbreaths.wav");
 a->door = load_one(dir, "SFXBible_12478.wav");
 a->melody = load_one(dir, "Music_Box_Melody_Playful.wav");
 a->stare = load_one(dir, "stare.wav");
 a->buzz = load_one(dir, "buzzlight.wav");
 a->windup = load_one(dir, "windup2.wav");
 a->thud = load_one(dir, "videogame_or_not-metallic-thud-449652 (1).wav");
 a->steps = load_one(dir, "deep steps.wav");
 a->closeamb = load_one(dir, "With_S2.wav");
 a->echo1 = load_one(dir, "echo1.wav");
 a->echo3b = load_one(dir, "echo3b.wav");
 a->echo4b = load_one(dir, "echo4b.wav");
 a->stop = load_one(dir, "stop.wav");
 a->walk = load_one(dir, "walk1.wav");
 a->garble = load_one(dir, "garble1.wav");
 a->phbb = load_one(dir, "scream3.wav");
 a->powerdown = load_one(dir, "powerdown.wav");
 a->jack = load_one(dir, "jackinthebox.wav");
 a->puppet = load_one(dir, "money-counter-95830.wav");
 a->freddy = load_one(dir, "XSCREAM.wav");
 a->foxy = load_one(dir, "dinosaur-roar-390283.wav");
 a->spring = load_one(dir, "scream3.wav");
 a->gf = load_one(dir, "XScream2.wav");
 a->title_static = load_one(dir, "static.wav");
 a->darkness = load_one(dir, "darkness music.wav");
 a->finalbox = load_one(dir, "music box.wav");
 a->chimes = load_one(dir, "Clock Chimes.wav");
 a->goblin = load_one(dir, "Crying Goblin (Clash Royale) Sound Effect - YTSFX (youtube).wav");
 a->call1 = load_one(dir, "call1b (1).mp3");
 a->call2 = load_one(dir, "call2b (3).mp3");
 a->call3 = load_one(dir, "call3b (3).mp3");
 a->call4 = load_one(dir, "call-4b-44100hz.wav");
 a->call5 = load_one(dir, "call-5b-44100hz.wav");
 a->call6 = load_one(dir, "call-6b-44100hz.wav");

 /* Footstep knocks come from the left (Fusion pan -100 on ch #12). */
 Mix_SetPanning(CH_FOOT, 255, 0);
 a->ok = 1;
 a->init = 1;
 {
  Mix_Chunk *cs[] = { a->fan, a->depths, a->camau, a->change, a->flip_up,
   a->flip_down, a->mask_on, a->mask_off, a->breath, a->door, a->melody,
   a->stare, a->buzz, a->windup, a->thud, a->steps, a->closeamb, a->echo1,
   a->echo3b, a->echo4b, a->stop, a->walk, a->garble, a->phbb, a->powerdown,
   a->jack, a->puppet, a->freddy, a->foxy, a->spring, a->gf, a->title_static,
   a->darkness, a->finalbox, a->chimes, a->goblin, a->call1, a->call2,
   a->call3, a->call4, a->call5, a->call6 };
  int n = 0;
  for (size_t i = 0; i < sizeof cs / sizeof cs[0]; ++i)
   if (cs[i])
    ++n;
  printf("AUDIO loaded=%d/%d dir=%s\n", n, (int)(sizeof cs / sizeof cs[0]), dir);
  fflush(stdout);
 }
 return 0;
}

static void free_all(FnaeAudio *a) {
 Mix_Chunk *cs[] = { a->fan, a->depths, a->camau, a->change, a->flip_up,
  a->flip_down, a->mask_on, a->mask_off, a->breath, a->door, a->melody,
  a->stare, a->buzz, a->windup, a->thud, a->steps, a->closeamb, a->echo1,
  a->echo3b, a->echo4b, a->stop, a->walk, a->garble, a->phbb, a->powerdown,
  a->jack, a->puppet, a->freddy, a->foxy, a->spring, a->gf, a->title_static,
  a->darkness, a->finalbox, a->chimes, a->goblin, a->call1, a->call2,
  a->call3, a->call4, a->call5, a->call6 };
 for (size_t i = 0; i < sizeof cs / sizeof cs[0]; ++i)
  if (cs[i])
   Mix_FreeChunk(cs[i]);
}

void fnae_audio_free(FnaeAudio *a) {
 if (!a->init)
  return;
 Mix_HaltChannel(-1);
 free_all(a);
 Mix_CloseAudio();
 Mix_Quit();
 memset(a, 0, sizeof *a);
 a->prev_frame = -1;
}

static void play(FnaeAudio *a, int ch, Mix_Chunk *c, int loops, int vol) {
 if (!c)
  return;
 Mix_HaltChannel(ch);
 Mix_Volume(ch, vol);
 Mix_PlayChannel(ch, c, loops);
}

/* Night ambience loops, straight from the [ Audio ] Start-of-Frame event. */
static void start_night(FnaeAudio *a) {
 Mix_HaltChannel(-1);
 play(a, CH_FAN, a->fan, -1, V(30));
 play(a, CH_DEPTHS, a->depths, -1, V(50));
 play(a, CH_CAMAU, a->camau, -1, 0);
 play(a, CH_BREATH, a->breath, -1, V(50));
 play(a, CH_STARE, a->stare, -1, MIX_MAX_VOLUME);
 play(a, CH_BUZZ, a->buzz, -1, MIX_MAX_VOLUME);
 play(a, CH_CLOSE, a->closeamb, -1, 0);
 play(a, CH_MELODY, a->melody, -1, 0);
 Mix_Volume(CH_WINDUP, V(75));
 Mix_Volume(CH_FOOT, V(40));
 Mix_Volume(CH_LURE, V(50));
 Mix_Volume(CH_WALK, V(50));
 Mix_Volume(CH_CALL, V(50));
 Mix_Volume(CH_MANGLE, V(50));
 Mix_Volume(CH_PHBB, V(50));
 Mix_Volume(CH_POWER, V(50));
 Mix_Volume(CH_JACK, V(30));
}

/* Nonzero while a phone call is playing (drives the Mute Call button). */
int fnae_audio_call_playing(FnaeAudio *a) {
 if (!a || !a->ok)
  return 0;
 return Mix_Playing(CH_CALL);
}

static void drain_queue(FnaeAudio *a, FnaeGame *g) {
 int snd;
 while ((snd = fnae_pop_sound(g)) >= 0) {
  switch (snd) {
  case FNAE_SND_LURE1: play(a, CH_LURE, a->echo1, 0, V(50)); break;
  case FNAE_SND_LURE2: play(a, CH_LURE, a->echo3b, 0, V(50)); break;
  case FNAE_SND_LURE3: play(a, CH_LURE, a->echo4b, 0, V(50)); break;
  case FNAE_SND_LURE_STOP: play(a, CH_LURE, a->stop, 0, V(50)); break;
  case FNAE_SND_DOOR: play(a, CH_DOOR, a->door, 0, V(50)); break;
  case FNAE_SND_CAM_UP: play(a, CH_FLIP, a->flip_up, 0, MIX_MAX_VOLUME); break;
  case FNAE_SND_CAM_DOWN: play(a, CH_FLIP, a->flip_down, 0, MIX_MAX_VOLUME); break;
  case FNAE_SND_MASK_ON: play(a, CH_FLIP, a->mask_on, 0, MIX_MAX_VOLUME); break;
  case FNAE_SND_MASK_OFF: play(a, CH_FLIP, a->mask_off, 0, MIX_MAX_VOLUME); break;
  case FNAE_SND_WINDUP: play(a, CH_WINDUP, a->windup, 0, V(75)); break;
  case FNAE_SND_CALL_STOP: Mix_HaltChannel(CH_CALL); break;
  case FNAE_SND_TITLE_CHANGE: play(a, CH_CHANGE, a->change, 0, MIX_MAX_VOLUME); break;
  default: break;
  }
 }
}

static void jumpscare(FnaeAudio *a, int death) {
 Mix_HaltChannel(-1);
 Mix_Chunk *c = NULL;
 switch (death) {
 case 1: c = a->puppet; break;
 case 2: c = a->freddy; break;
 case 3: c = a->foxy; break;
 case 4: c = a->spring; break;
 case 5: c = a->gf; break;
 default: break;
 }
 play(a, CH_DEPTHS, c, 0, MIX_MAX_VOLUME);
}

void fnae_audio_frame(FnaeAudio *a, FnaeGame *g) {
 if (!a->ok)
  return;
 drain_queue(a, g);

 int frame = (int)g->frame;
 if (frame != a->prev_frame || (frame == FRAME_NIGHT && g->night != a->prev_night)) {
  /* Every frame entry stops everything first (Fusion "Stop any sample"). */
  Mix_HaltChannel(-1);
  switch (frame) {
  case FRAME_TITLE:
   play(a, CH_FAN, a->title_static, 0, MIX_MAX_VOLUME);
   play(a, CH_DEPTHS, a->darkness, -1, MIX_MAX_VOLUME);
   break;
  case FRAME_WHICH_NIGHT:
   play(a, CH_FAN, a->change, 0, MIX_MAX_VOLUME);
   break;
  case FRAME_NIGHT:
   start_night(a);
   break;
  case FRAME_DEATH:
   play(a, CH_GOBLIN, a->goblin, -1, MIX_MAX_VOLUME);
   break;
  case FRAME_FINAL:
   play(a, CH_FAN, a->finalbox, -1, MIX_MAX_VOLUME);
   break;
  case FRAME_6AM:
   play(a, CH_FAN, a->chimes, 0, MIX_MAX_VOLUME);
   break;
  default:
   break;
  }
  a->prev_frame = frame;
  a->prev_night = g->night;
  a->prev_death = g->death;
  a->prev_view = g->view;
  a->prev_camera = g->camera;
  a->prev_call = g->current_call;
  a->prev_power = g->hidden_power;
  a->prev_empty = g->music_left <= 0;
  a->prev_spring = g->springtrap_pos;
  a->prev_fdoor = g->freddy_door;
  a->prev_fstand = g->foxy_stand;
  a->prev_mangle = g->ph_mangle_c;
  return;
 }

 if (frame != FRAME_NIGHT)
  return;

 /* Jumpscare: Fusion stops everything, then the death sample on ch #2. */
 if (g->death != 0 && a->prev_death == 0)
  jumpscare(a, g->death);
 /* Power loss: stop everything, then powerdown on ch #19 (once). */
 if (a->prev_power > 0 && g->hidden_power <= 0) {
  Mix_HaltChannel(-1);
  play(a, CH_POWER, a->powerdown, 0, V(50));
 }
 if (g->death == 0 && g->hidden_power > 0) {
  /* Phone call for this night (MP3 on ch #16 for 1-3, WAV for 4-6). */
  if (g->current_call != 0 && g->current_call != a->prev_call) {
   Mix_Chunk *c = NULL;
   switch (g->night) {
   case 1: c = a->call1; break;
   case 2: c = a->call2; break;
   case 3: c = a->call3; break;
   case 4: c = a->call4; break;
   case 5: c = a->call5; break;
   case 6: c = a->call6; break;
   default: break;
   }
   play(a, CH_CALL, c, 0, V(50));
  }
  /* Empty music box loops the jack-in-the-box on ch #20 (run once). */
  if (g->music_left <= 0 && !a->prev_empty)
   play(a, CH_JACK, a->jack, -1, V(30));
  /* Springtrap stepping onto the Forest cam knocks (walk1, ch #15). */
  if (g->springtrap_pos == 3 && a->prev_spring != 3)
   play(a, CH_WALK, a->walk, 0, V(50));
  /* Doorway arrivals knock on ch #12. */
  if (g->freddy_door && !a->prev_fdoor)
   play(a, CH_FOOT, a->steps, 0, V(40));
  if (g->foxy_stand && !a->prev_fstand)
   play(a, CH_FOOT, a->thud, 0, V(40));
  /* Camera switch blip (Change on ch #4). */
  if (g->view > 0 && g->camera != a->prev_camera)
   play(a, CH_CHANGE, a->change, 0, MIX_MAX_VOLUME);
  /* Phantom Mangle static burst (garble1 on ch #17). */
  if (g->ph_mangle_c && !a->prev_mangle)
   play(a, CH_MANGLE, a->garble, 0, V(50));

  /* Continuous volumes. */
  Mix_Volume(CH_FAN, g->view > 0 ? V(10) : V(30));
  Mix_Volume(CH_CAMAU, g->view > 0 ? V(50) : 0);
  {
   int mv = 0;
   if (g->view == 2)
    mv = V(20);
   else if (g->view == 3)
    mv = V(10);
   else if (g->view == 4)
    mv = V(50);
   Mix_Volume(CH_MELODY, mv);
  }
  Mix_Volume(CH_CLOSE, (g->foxy_stand || g->freddy_door) ? V(50) : 0);
  Mix_Volume(CH_BREATH, g->mask_anim == MASK_DOWN ? V(50) : 0);
 }

 a->prev_death = g->death;
 a->prev_view = g->view;
 a->prev_camera = g->camera;
 a->prev_call = g->current_call;
 a->prev_power = g->hidden_power;
 a->prev_empty = g->music_left <= 0;
 a->prev_spring = g->springtrap_pos;
 a->prev_fdoor = g->freddy_door;
 a->prev_fstand = g->foxy_stand;
 a->prev_mangle = g->ph_mangle_c;
}
