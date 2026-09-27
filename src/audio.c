#include "audio.h"

#include <SDL_mixer.h>
#include <stdio.h>
#include <string.h>

#include "wiiu.h"

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

#ifdef __WIIU__
/* Output-zone routing (Wii U only): the patched SDL port maps SDL-LEFT to
 * the GamePad (both speakers, mono) and SDL-RIGHT to the TV (both
 * speakers, mono), so panning steers content per output. Cam UI goes
 * LEFT (pads); everything else goes RIGHT (TV). Volumes untouched.
 * Non-WiiU builds keep the original stereo image (untouched). */
static void wiiu_zone_pans(FnaeGame *g) {
    /* Cam UI counts as open through the flip transitions too: the flip-up
     * blip fires while view is still 0 (and flip-down while it drops), so
     * gate on cam_anim as well or the transitions leak to the TV. */
    int cams = (g->frame == FRAME_NIGHT &&
                (g->view > 0 || g->cam_anim != CAM_DOWN));
    /* CH_CAMAU reuses: title blip (TV) vs night cam loop (pads). CH_FLIP
     * reuses: mask on/off (office/TV) vs cam flip blips (pads). CH_CHANGE
     * reuses: title/menu blips (TV) vs cam-switch + connection-lost blips
     * (pads). */
    Mix_SetPanning(CH_CAMAU, cams ? 255 : 0, cams ? 0 : 255);
    Mix_SetPanning(CH_FLIP, cams ? 255 : 0, cams ? 0 : 255);
    Mix_SetPanning(CH_CHANGE, cams ? 255 : 0, cams ? 0 : 255);
    Mix_SetPanning(CH_MELODY, 255, 0);
    Mix_SetPanning(CH_WINDUP, 255, 0);
    Mix_SetPanning(CH_LURE, 255, 0);
    Mix_SetPanning(CH_STARE, 255, 0);
    Mix_SetPanning(CH_FAN, 0, 255);
    Mix_SetPanning(CH_DEPTHS, 0, 255);
    Mix_SetPanning(CH_CALL, 0, 255);
    Mix_SetPanning(CH_WALK, 0, 255);
    Mix_SetPanning(CH_FOOT, 0, 255);
    Mix_SetPanning(CH_DOOR, 0, 255);
    Mix_SetPanning(CH_CLOSE, 0, 255);
    Mix_SetPanning(CH_BREATH, 0, 255);
    Mix_SetPanning(CH_BUZZ, 0, 255);
    Mix_SetPanning(CH_MANGLE, 0, 255);
    Mix_SetPanning(CH_PHBB, 0, 255);
    Mix_SetPanning(CH_POWER, 0, 255);
    Mix_SetPanning(CH_JACK, 0, 255);
    Mix_SetPanning(CH_GOBLIN, 0, 255);
}
#endif

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

 /* Staged loading-screen progress (see fnae_core.h); aborting drops into
 * the silent path (ok/init stay 0) while the caller checks its own flag. */
#define FNAE_AUDIO_PCT(p) do { \
  if (progress && progress((p), pctx)) return 0; \
} while (0)

int fnae_audio_init(FnaeAudio *a, const char *dir, FnaeLoadProgress progress, void *pctx) {
  memset(a, 0, sizeof *a);
  a->prev_frame = -1;
#ifndef __WIIU__
  /* No MP3s ship anymore (all calls are WAV); desktop keeps the init for
   * SDL_mixer's decoder setup. Skipped on Wii U, where the static portlib
   * has no MP3 decoder and this only prints a red-herring error. */
  if (Mix_Init(MIX_INIT_MP3) == 0)
   fprintf(stderr, "AUDIO: mp3 decoder unavailable: %s\n", Mix_GetError());
#endif
#ifdef __WIIU__
  /* Native rate first: Wii U AX runs at 48000 Hz, and a 44100 open is
   * suspected of stalling the SDL driver on hardware -- stuck before the
   * first frame, i.e. frozen on the OS loading screen. */
  static const int freq_try[] = { 48000, 44100, 22050 };
#else
  static const int freq_try[] = { 44100, 48000, 22050 };
#endif
 int freq_got = 0;
 for (size_t i = 0; i < sizeof freq_try / sizeof freq_try[0]; ++i) {
  if (Mix_OpenAudio(freq_try[i], MIX_DEFAULT_FORMAT, 2, 2048) == 0) {
   freq_got = freq_try[i];
   break;
  }
  fprintf(stderr, "AUDIO: Mix_OpenAudio(%d) failed: %s\n",
   freq_try[i], Mix_GetError());
 }
 if (!freq_got) {
  fprintf(stderr, "AUDIO: no rate opened (silent)\n");
  return 0;
 }
 {
  int qf = 0; Uint16 qfmt = 0; int qch = 0;
  Mix_QuerySpec(&qf, &qfmt, &qch);
  printf("AUDIO opened=%dHz ch=%d\n", qf, qch);
  fflush(stdout);
 }
  Mix_AllocateChannels(40);
  FNAE_AUDIO_PCT(10);

  /* Wii U content probe (code/content/meta, .wuhb, or flat folder): every
   * sample dir goes through the pinned asset root ("assets/" on desktop). */
  char fulldir[576];
  snprintf(fulldir, sizeof fulldir, "%s%s", fnae_asset_root(), dir);
  dir = fulldir;

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
  FNAE_AUDIO_PCT(30);
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
  FNAE_AUDIO_PCT(50);
 a->garble = load_one(dir, "garble1.wav");
 a->manglebreath = load_one(dir, "breathing.wav");
 a->phbb = load_one(dir, "scream3.wav");
 a->powerdown = load_one(dir, "powerdown.wav");
 a->jack = load_one(dir, "jackinthebox.wav");
 a->puppet = load_one(dir, "money-counter-95830.wav");
 a->freddy = load_one(dir, "XSCREAM.wav");
 a->foxy = load_one(dir, "dinosaur-roar-390283.wav");
 a->spring = load_one(dir, "scream3.wav");
  FNAE_AUDIO_PCT(70);
 a->gf = load_one(dir, "XScream2.wav");
 a->title_static = load_one(dir, "static.wav");
 a->darkness = load_one(dir, "darkness music.wav");
 a->finalbox = load_one(dir, "music box.wav");
 a->chimes = load_one(dir, "Clock Chimes.wav");
 a->goblin = load_one(dir, "Crying Goblin (Clash Royale) Sound Effect - YTSFX (youtube).wav");
  a->call1 = load_one(dir, "call-1b.wav");
  a->call2 = load_one(dir, "call-2b.wav");
  a->call3 = load_one(dir, "call-3b.wav");
 a->call4 = load_one(dir, "call-4b.wav");
 a->call5 = load_one(dir, "call-5b.wav");
 a->call6 = load_one(dir, "call-6b.wav");
  FNAE_AUDIO_PCT(90);

  FNAE_AUDIO_PCT(100);

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
    a->call3, a->call4, a->call5, a->call6, a->manglebreath };
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
   a->call3, a->call4, a->call5, a->call6, a->manglebreath };
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
 /* deepbreaths/stare/buzzlight start silent: the Fusion edge events set
  * them to 50/50/70 only once the mask is down / signal is lost /
  * flashlight is on. Starting them loud blasts for a tick. */
 play(a, CH_BREATH, a->breath, -1, 0);
 play(a, CH_STARE, a->stare, -1, 0);
 play(a, CH_BUZZ, a->buzz, -1, 0);
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
  /* One-shot channels keep the Fusion Start-of-Frame levels: ch #4/#5
   * (Change, flips, mask) play at 50, not full volume. */
  Mix_Volume(CH_CHANGE, V(50));
  Mix_Volume(CH_FLIP, V(50));
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
   case FNAE_SND_CAM_UP: play(a, CH_FLIP, a->flip_up, 0, V(50)); break;
   case FNAE_SND_CAM_DOWN: play(a, CH_FLIP, a->flip_down, 0, V(50)); break;
   case FNAE_SND_MASK_ON: play(a, CH_FLIP, a->mask_on, 0, V(50)); break;
   case FNAE_SND_MASK_OFF: play(a, CH_FLIP, a->mask_off, 0, V(50)); break;
   case FNAE_SND_WINDUP: play(a, CH_WINDUP, a->windup, 0, V(75)); break;
   case FNAE_SND_CALL_STOP: Mix_HaltChannel(CH_CALL); break;
   /* Title/customize menu blips live on ch #3, night camera Change blips
    * on ch #4 (Frame 8 Events.txt plays Change on ch #3). */
   case FNAE_SND_TITLE_CHANGE:
    play(a, (g->frame == FRAME_TITLE || g->frame == FRAME_CUSTOMIZE)
        ? CH_CAMAU : CH_CHANGE, a->change, 0, V(50));
    break;
   case FNAE_SND_PHBB: play(a, CH_PHBB, a->phbb, 0, V(50)); break;
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
#ifdef __WIIU__
 /* Zone routing first: one-shots played below inherit this frame's pan. */
 wiiu_zone_pans(g);
#endif
 drain_queue(a, g);

 int frame = (int)g->frame;
 if (frame != a->prev_frame || (frame == FRAME_NIGHT && g->night != a->prev_night)) {
  /* Every frame entry stops everything first (Fusion "Stop any sample"). */
  Mix_HaltChannel(-1);
  switch (frame) {
   case FRAME_TITLE:
    play(a, CH_FAN, a->title_static, 0, V(50));
    play(a, CH_DEPTHS, a->darkness, -1, V(50));
    /* Title Start-of-Frame also fires a Change blip on ch #3. */
    play(a, CH_CAMAU, a->change, 0, V(50));
    break;
   case FRAME_WHICH_NIGHT:
    play(a, CH_FAN, a->change, 0, V(50));
    break;
  case FRAME_NIGHT:
   start_night(a);
   break;
  case FRAME_DEATH:
   play(a, CH_GOBLIN, a->goblin, -1, MIX_MAX_VOLUME);
   break;
   case FRAME_FINAL:
    play(a, CH_FAN, a->finalbox, 0, V(50));
    break;
   case FRAME_6AM:
    play(a, CH_FAN, a->chimes, 0, V(50));
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
  a->prev_mangle_act = 0;
  a->prev_annoy_end = 0;
  a->prev_move = g->movement_out;
  a->prev_foxy_pos = g->foxy.pos;
  a->prev_freddy_pos = g->freddy.pos;
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
  /* Doorway arrivals knock on ch #12. Fusion fires on the overlap
   * (Foxy pos 5 = Right Door, Freddy pos 6 = Left Door) with no view
   * gate, so track the AI positions -- not the view-gated stand
   * flags -- or knocks go missing while the cameras are up. */
  if (g->freddy.pos == 6 && a->prev_freddy_pos != 6)
   play(a, CH_FOOT, a->steps, 0, V(40));
  if (g->foxy.pos == 5 && a->prev_foxy_pos != 5)
   play(a, CH_FOOT, a->thud, 0, V(40));
   /* Camera Change blip (Change on ch #4, vol 50). Fusion sets Change=1
    * on the View>0 edge (flip-up), on CAM 01 clicks (camera switch),
    * and on the Connection Lost edge, each playing Change on ch #4. */
   if ((g->view > 0 && g->camera != a->prev_camera) ||
       (g->view > 0 && a->prev_view == 0) ||
       (g->movement_out > 0 && a->prev_move == 0))
    play(a, CH_CHANGE, a->change, 0, V(50));
   /* Phantom Mangle (ch #17): garble1 loops while the office annoy
    * descends (C==1, Annoy A>0, B<7); breathing plays once B hits 7.
    * Both Stop the channel first, like the Fusion events. The
    * camera-haunt phase (A==1/B counting, C==0) stays silent here, so
    * cam open/close keeps its stereo-cassette flip on ch #5. */
   {
    int garble = g->ph_mangle_c && g->ph_annoy_a > 0 && g->ph_annoy_b < 7;
    int ended = g->ph_mangle_c && g->ph_annoy_b >= 7;
    if (ended && !a->prev_annoy_end) {
     Mix_HaltChannel(CH_MANGLE);
     play(a, CH_MANGLE, a->manglebreath, 0, V(50));
    } else if (garble && !a->prev_mangle_act) {
     Mix_HaltChannel(CH_MANGLE);
     play(a, CH_MANGLE, a->garble, -1, V(50));
    } else if (!garble && a->prev_mangle_act) {
     Mix_HaltChannel(CH_MANGLE);
    }
    a->prev_mangle_act = garble;
    a->prev_annoy_end = ended;
   }

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
   /* stare loops under the Connection Lost overlay (ch #9): silent
    * while the feed is live, 50 on signal loss. */
   Mix_Volume(CH_STARE, g->movement_out > 0 ? V(50) : 0);
   /* buzzlight (ch #10): silent until the flashlight is on (70). */
   Mix_Volume(CH_BUZZ, g->flashlight ? V(70) : 0);
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
  a->prev_move = g->movement_out;
  a->prev_foxy_pos = g->foxy.pos;
  a->prev_freddy_pos = g->freddy.pos;
}
