#pragma once

/* Human-readable names for CTFAK image-bank IDs (assets/images/<id>.png).
 *
 * IDs come straight from the dump; names are assigned as each image's
 * Fusion object is identified via Objects.txt / Events.txt / visual ID.
 * Unassigned IDs keep their numbers at the call site until identified.
 * A "?" suffix means visually identified but awaiting owner confirmation.
 */

#define IMG_OFFICE        227 /* Frame 3 office scene */
/* Camera feeds, 1600x720. Each camera is an empty base scene plus an
 * occupied frame showing the animatronic that haunts it (Freddy: Cam 01
 * -> Cam 03; Foxy: Cam 02 -> Cam 04; see Frame 3 Events.txt). */
#define IMG_CAM_HELL        343 /* Cam 01 base (Hell ride, "Welcome To HELL") */
#define IMG_CAM_HELL_FRED   312 /* Cam 01 with Freddy (red Edward) */
#define IMG_CAM_MOUNTAIN    347 /* Cam 02 base (mountain waterfall/cave) */
#define IMG_CAM_MOUNTAIN_FOXY 348 /* Cam 02 with Foxy (blue creature) */
#define IMG_CAM_FOREST      350 /* Cam 03 base (empty forest) */
#define IMG_CAM_FOREST_FRED 379 /* Cam 03 with Freddy (red Edward) */
#define IMG_CAM_DINO        212 /* Cam 04 base (volcano/dinosaurs) */
#define IMG_CAM_DINO_FOXY   211 /* Cam 04 with Foxy (blue creature) */

#define IMG_STATIC_FIRST  46  /* TV-static animation, 8 frames (46-53), */
#define IMG_STATIC_COUNT  8   /* shared by title Static + camera static */

#define IMG_DEATH         1   /* GAME OVER screen */
#define IMG_GOODJOB       2   /* GOOD JOB CAPTAIN final screen (Frame 5) */
#define IMG_SIX_AM        4   /* overtime paycheck (Frame 9) */
#define IMG_NEWSPAPER     520 /* HELP WANTED newspaper (Frame 7) */
#define IMG_WARNING       351 /* Frame 1 warning screen (1280x720) at [0,0] */

#define IMG_TITLE_BG      515 /* Frame 2 Background (Stopped sequence) */
#define IMG_TITLE_NEW     239
#define IMG_TITLE_CONTINUE 240
#define IMG_TITLE_6NIGHT  241
#define IMG_TITLE_CUSTOM  242
#define IMG_TITLE_ARROW   245
#define IMG_TITLE_STAR    232
#define IMG_TITLE_TEXT    464 /* "Five Nights at Edward's" text card */

#define IMG_NIGHT_FIRST   246 /* The Night counter frames, 7 (246-252) */
#define IMG_NIGHT_COUNT   7

/* Frame 3 doors: 16-frame open->closed shutter runs. Stopped (open) is
 * frame 0, Close plays 0->15, Closed holds 15, Open plays 15->0.
 * Left Door sits at [119,0], Right Door at [1263,0] (see Objects.txt).
 * The 144-159 run is the Left Door art (223x720), 160-175 the Right Door
 * art (248x720) — verified against the original; do not re-swap by width. */
#define IMG_DOOR_LEFT_FIRST   144 /* Left Door frames, 16 (144-159), 223x720 */
#define IMG_DOOR_RIGHT_FIRST  160 /* Right Door frames, 16 (160-175), 248x720 */
#define IMG_DOOR_FRAMES       16

#define IMG_DESK_SCENE    238 /* 1066x511 office desk scene, at [266,177] */

/* Frame 2 Background: 515 is Stopped; Random(50)=1 plays one of the
 * RRandom(12,14) flash sequences (516/517/518), cut back to Stopped
 * after 0.2 s. Each flash sequence is a single held frame. */
#define IMG_TITLE_BG_ANIM_FIRST 516 /* 3 flash frames (516-518), 1280x720 */
#define IMG_TITLE_BG_ANIM_COUNT 3

/* Unidentified (owner to confirm object/sequence): */
#define IMG_UNKNOWN_179   179 /* 1280x720 gray-face frame; NOT the title Background */
#define IMG_DEVIL_SAD     233 /* 600x507 devil card, sad eyes */
#define IMG_DEVIL_SHOCKED 460 /* 600x507 devil card, wide eyes */
