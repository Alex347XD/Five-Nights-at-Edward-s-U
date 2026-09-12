#pragma once

/* Human-readable names for CTFAK image-bank IDs (assets/images/<id>.png).
 *
 * IDs come straight from the dump; names are assigned as each image's
 * Fusion object is identified via Objects.txt / Events.txt / visual ID.
 * Unassigned IDs keep their numbers at the call site until identified.
 * A "?" suffix means visually identified but awaiting owner confirmation.
 */

#define IMG_OFFICE        227 /* Frame 3 office scene */
#define IMG_CAM_HELL      211 /* Cam 01 */
#define IMG_CAM_MOUNTAIN  379 /* Cam 02 */
#define IMG_CAM_FOREST    350 /* Cam 03 */
#define IMG_CAM_DINO      312 /* Cam 04 (Dinosaur Exhibit) */

#define IMG_STATIC_FIRST  46  /* TV-static animation, 8 frames (46-53), */
#define IMG_STATIC_COUNT  8   /* shared by title Static + camera static */

#define IMG_DEATH         1   /* GAME OVER screen */
#define IMG_GOODJOB       2   /* GOOD JOB CAPTAIN final screen (Frame 5) */
#define IMG_SIX_AM        4   /* overtime paycheck (Frame 9) */
#define IMG_NEWSPAPER     7   /* termination notice (Frame 7) */

#define IMG_TITLE_BG      179 /* Frame 2 background (Stopped sequence) */
#define IMG_TITLE_NEW     239
#define IMG_TITLE_CONTINUE 240
#define IMG_TITLE_6NIGHT  241
#define IMG_TITLE_CUSTOM  242
#define IMG_TITLE_ARROW   245
#define IMG_TITLE_STAR    232
#define IMG_TITLE_TEXT    464 /* "Five Nights at Edward's" text card */

#define IMG_NIGHT_FIRST   246 /* The Night counter frames, 7 (246-252) */
#define IMG_NIGHT_COUNT   7

/* Identified but unassigned (owner to confirm object/sequence): */
#define IMG_DEVIL_SAD     233 /* 600x507 devil card, sad eyes */
#define IMG_DEVIL_SHOCKED 460 /* 600x507 devil card, wide eyes */
#define IMG_DESK_SCENE    238 /* 1066x511 office desk scene */
