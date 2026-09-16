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

/* Connection Lost banner (Frame 3 "[ Camera Out ]" group): 579x53, drawn
 * centered on black while Movement Out > 0 (character moved while viewing).
 * Fusion parks it at [640,60] over the feed; native centers it on the
 * blacked-out feed per owner request. */
#define IMG_CONNECTION_LOST 333

#define IMG_DEATH         1   /* GAME OVER screen */
/* RIP Text (Frame 4) frame 0: 126x54 red box shown while RIP B==0 (fading
 * out); B>1 switches to the GAME OVER frame (1.png, fading back in).
 * Owner-confirmed bank ID. */
#define IMG_RIP_TEXT      404 /* RIP Text first frame, 126x54 */
#define IMG_GOODJOB       2   /* GOOD JOB CAPTAIN final screen (Frame 5) */
/* Frame 9 (6 AM) "which AM" odometer (53x72 digit at [533,324]): Stopped =
 * 389 ("5"); Timer > 03'' starts the roll up to "6" (bank order below,
 * owner-verified frame by frame; ends on 492). Time Text ("AM" at
 * [624,324]) is an invisible Alterable-Value carrier in Fusion (no visual
 * events ever reference it), so the digit draws alone on black. */
#define IMG_WHICH_AM_COUNT 27
/* Frame 5 Final win screens (Frame 5 Events.txt creates one of Night 5/6/7
 * at (0,0) from the "6th or 7th night" counter): night 5 -> 2.png
 * (weekly paycheck), night 6 -> 4.png (overtime paycheck), night 7 ->
 * 7.png (termination notice). */
#define IMG_FINAL_N5      2   /* = IMG_GOODJOB (kept alias) */
#define IMG_FINAL_N6      4   /* night-6 win (overtime paycheck) */
#define IMG_FINAL_N7      7   /* night-7 win (termination notice) */
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

#define IMG_NIGHT_FIRST   246 /* Which Night cards (Frame 6), 7 (246-252):
 * "12:00 AM / Nth Night" full cards, centered on black. NOT the title's
 * The Night, which is a Counter showing just the digit (bitmap font). */
#define IMG_NIGHT_COUNT   7

/* Frame 3 doors: 16-frame open->closed shutter runs. Stopped (open) is
 * frame 0, Close plays 0->15, Closed holds 15, Open plays 15->0.
 * Left Door sits at [119,0], Right Door at [1263,0] (see Objects.txt).
 * The 144-159 run is the Left Door art (223x720), 160-175 the Right Door
 * art (248x720) — verified against the original; do not re-swap by width. */
#define IMG_DOOR_LEFT_FIRST   144 /* Left Door frames, 16 (144-159), 223x720 */
#define IMG_DOOR_RIGHT_FIRST  160 /* Right Door frames, 16 (160-175), 248x720 */
#define IMG_DOOR_FRAMES       16

/* Door buttons (Frame 3 "[ Doors ]" group, Layer #2 world objects that pan
 * with the office scroll). Owner-confirmed pair, both 51x56,
 * center-anchored: Stopped (dark red) while the door Alterable A is 0
 * (open) or 3 (opening), Animation 12 (olive) while A is 1 (closing) or
 * 2 (closed). Button Left sits at [105,500], Button Right at [1489,500]. */
#define IMG_DOORBTN_OFF       176 /* button released (Stopped) */
#define IMG_DOORBTN_ON        177 /* button pressed (Animation 12) */

/* Doorway figures (Layer #2 office overlay, world objects at 1.1 scale,
 * center-anchored). Owner-confirmed, both 182x358: Freddy (red Edward
 * figure, matches the 344 portrait / 312 cam feed) at [260,788] while
 * Freddy Collision overlaps Left Door Collision (office view only);
 * Foxy (blue dino, matches the 340 portrait / 348 cam feed) at
 * [1287,331] while Foxy Collision overlaps Right Door Collision. */
#define IMG_FREDDY_DOOR       213 /* Freddy at your door */
#define IMG_FOXY_STAND        228 /* Foxy Stand */

#define IMG_DESK_SCENE    238 /* 1066x511 office desk scene, at [266,177] */

/* Frame 3 camera minimap (Layer #5 UI, visible only while a camera is up).
 * Positions are Frame 3 Objects.txt hotspots: the map draws top-left
 * (see docs/COORDINATES.md) while the 60x40 button boxes are
 * center-anchored, which is what seats each 31x25 label inside its box.
 * Exception: CAM 01 draws 32px above its [1016,339] hotspot (owner
 * request); see draw_minimap in visuals.c and the click zones in
 * fnae_core.c, which stay in sync. */
#define IMG_MINIMAP       28  /* 372x322 white line-art map (YOU baked in), at [882,265] */
#define IMG_CAMBTN_OFF    29  /* 60x40 gray cam button box (CAM 01 Stopped) */
#define IMG_CAMBTN_ON     30  /* 60x40 green cam button box (CAM 01 Animation 12, selected) */
#define IMG_CAMTXT_FIRST  31  /* "CAM 01".."CAM 04" labels, 4 (31-34), 31x25 each */
/* Button boxes ("CAM 01" x4) and their text labels ("Cam 0X Text" x4). */
#define IMG_CAMBTN_COUNT  4

/* Audio-lure + Springtrap (Frame 3 "[ Springtrap (Audio Lure) ]" group).
 * The Lure Button sits at [744,296] in the camera UI (Layer #5): visible
 * while a camera is up except on Cam 04 (music box). Its Stopped frame is
 * the "Lure" card; Animation 12 is the 4-frame cooldown run of white
 * square dots (1 dot -> 2 -> 3 -> 4) that plays over the ~2 s cooldown
 * window, independent of the Lure Area marker: destroying the marker
 * never shortens the cooldown, and a new lure requires the cooldown to
 * have fully elapsed. The Lure Area spawns at (0,0) from the viewed
 * CAM 01 button, so the gray circle draws mostly transparent
 * (alpha ~70) centered over the lured camera's minimap button until it
 * resolves. Springtrap Stand is the full-body blue
 * Edward with stars (299x715) at [416,-24] in the office overlay
 * (Layer #2), drawn over the feed while viewing Springtrap's camera. */
#define IMG_LURE_BUTTON     381 /* "Lure" button, 128x64, Stopped */
#define IMG_LURE_CD_1       313 /* cooldown frame 1: 1 white square dot, 128x64 */
#define IMG_LURE_CD_2       338 /* cooldown frame 2: 2 white square dots, 128x64 */
#define IMG_LURE_CD_3       334 /* cooldown frame 3: 3 white square dots, 128x64 */
#define IMG_LURE_CD_4       341 /* cooldown frame 4: 4 white square dots, 128x64 */
#define IMG_LURE_AREA       390 /* lure marker: gray circle, 256x256, centered over the lured cam button */
#define IMG_SPRINGTRAP_STAND 236 /* blue Edward full body with stars, 299x715 */

/* Frame 2 Background: 515 is Stopped; Random(50)=1 plays one of the
 * RRandom(12,14) flash sequences (516/517/518), cut back to Stopped
 * after 0.2 s. Each flash sequence is a single held frame. */
#define IMG_TITLE_BG_ANIM_FIRST 516 /* 3 flash frames (516-518), 1280x720 */
#define IMG_TITLE_BG_ANIM_COUNT 3

/* Mute Call button ("MUTE CALL", 121x31 at [100,55]): reappears while a
 * night's phone call plays (ch #16) and stops it when clicked. */
#define IMG_MUTECALL      415 /* Mute Call button art */
/* Mask (Frame 3 "[ Mask ]" group, office view only, Layer #5 UI).
 * Owner-confirmed frame order: Flip Mask Down 134-140 (1280x720,
 * drawn at [0,0]) plays while putting the mask on, worn mask 129
 * (1480x870, drawn at [-100,-66] per the Start-of-Frame event) shows
 * while it is down, Flip Mask Up 141-143 (1280x720, at [0,0]) plays
 * while taking it off. */
#define IMG_MASK_FLIPDN_FIRST 134 /* put-on run, 7 frames (134-140) */
#define IMG_MASK_FLIPDN_COUNT 7
#define IMG_MASK_WORN         129 /* worn mask still */
#define IMG_MASK_FLIPUP_FIRST 141 /* take-off run, 3 frames (141-143) */
#define IMG_MASK_FLIPUP_COUNT 3
#define IMG_MASK_FRAMES       11  /* put-on 0-6, worn 7, take-off 8-10 */
/* Phantom Mangle + Phantom BB (Frame 3 "[ Phantom Mangle ]" / "[ Phantom BB ]"
 * groups, Layer #6 top overlays at [0,0] with scale 2.7, plus the office
 * Annoy riser on Layer #3). Owner-confirmed IDs: 405 = Ph Mangle Camera
 * overlay (480x270, x2.7 ~= fullscreen), 380 = Ph Mangle Annoy office
 * figure (400x225, starts at [508,720], climbs 224px). 352 = Ph BB
 * Camera overlay, 349 = Ph BB Scare fade (480x270 gray grinning top-hat
 * face, per owner).
 * 480x270 x 2.7 = 1296x729, i.e. the overlay slightly overflows the
 * 1280x720 view to the right/bottom like the Fusion scale. */
#define IMG_PHMANGLE_CAM   405 /* gray cracked-rock face, 480x270 */
#define IMG_PHMANGLE_ANNOY 380 /* gray cracked-rock figure, 400x225 */
#define IMG_PHBB_CAM       352 /* gray grin + top hat, 480x270 */
#define IMG_PHBB_SCARE     349 /* gray grin + top hat, 480x270, alpha-faded */
/* Jumpscares (Frame 3 "[ Jumpscares ]" group): 480x270 art created at
 * (0,0) with the Fusion scale 2.8 (1344x756, overflowing the 1280x720
 * view right/bottom) and shown fullscreen over the shaking office during
 * the 60-tick death wait. Owner-confirmed runs: Springtrap 314-331 (18),
 * Freddy 353-363 + 365 (12; 364 is a 35x75 UI dot), Puppet 493-510 (18),
 * Foxy 536-539 + 562-572 (15; 4 + 11), Golden Freddy still 458. */
#define IMG_SCARE_SPRING_FIRST 314
#define IMG_SCARE_SPRING_COUNT 18
#define IMG_SCARE_FREDDY_FIRST 353 /* +365 (see IMG_SCARE_FREDDY_LAST) */
#define IMG_SCARE_FREDDY_LAST  365 /* 364 missing: 12 loaded frames */
#define IMG_SCARE_PUPPET_FIRST 493
#define IMG_SCARE_PUPPET_COUNT 18
#define IMG_SCARE_FOXY_A_FIRST 536
#define IMG_SCARE_FOXY_A_COUNT 4
#define IMG_SCARE_FOXY_B_FIRST 562
#define IMG_SCARE_FOXY_B_COUNT 11
#define IMG_SCARE_GF           458 /* still */
/* Golden Freddy's office figure (Frame 3 "[ Golden Freddy ]" group):
 * GF Sit sits on Layer #2 (overlay office) at [440,240] and reappears
 * while GF Random == 1 (hidden otherwise). 150x200 close-up of the
 * red bottle with yellow highlights (same character as the 458 still). */
#define IMG_GF_SIT             310 /* GF Sit office figure, 150x200 */
/* Death frame (Frame 4): devil-card Death Anim backdrop cycling at
 * [630,390] (600x507, owner-confirmed 233/460). RIP Text is IMG_DEATH
 * (1.png GAME OVER, 479x54) at [640,650]; Red Fade In is a plain red
 * fullscreen rect (no bank art). */
#define IMG_DEATH_DEVIL_A      233 /* devil card, sad eyes */
#define IMG_DEATH_DEVIL_B      460 /* devil card, wide eyes */
/* Frame 8 Customize screen (Frame 8 Objects.txt). Portrait mapping is a
 * best-effort visual match (all 150x200 except BB 146x196): columns run
 * Freddy / Mangle / Foxy / Golden / Springtrap / BB / Puppet left to
 * right like the Layer #2 globals. Owner to confirm each character. */
#define IMG_CUST_FREDDY     344 /* red round head, 150x200 */
#define IMG_CUST_MANGLE     408 /* gray cracked-rock head, 150x200 */
#define IMG_CUST_FOXY       340 /* blue dino head, 150x200 */
#define IMG_CUST_GOLDEN     310 /* red bottle close-up, 150x200 (= IMG_GF_SIT) */
#define IMG_CUST_SPRING     256 /* blue head w/ green antennae, 150x200 */
#define IMG_CUST_BB         237 /* gray grin + top hat, 146x196 */
#define IMG_CUST_PUPPET     285 /* red head + top hat, 150x200 */
#define IMG_CUST_SELECT     422 /* Select Box frame, 150x200 black w/ white border */
#define IMG_CUST_ARROW      336 /* up triangle, 50x25, drawn @1.3 scale */
#define IMG_CUST_GO         459 /* GO! (Start Night), 250x55 */
#define IMG_CUST_SET20      511 /* Set 20, 235x55 */
#define IMG_CUST_ADD1       513 /* Add 1, 235x55 */
#define IMG_CUST_CHECK      253 /* checkbox outline, 38x39 */
#define IMG_CUST_BG_FIRST   409 /* Cool Background tiles, 40x40. 409/410/414
                                 * are the 3 Random(3) frames (411-413 are
                                 * other sizes, not used). */
#define IMG_CUST_BG_2       410
#define IMG_CUST_BG_3       414
/* Unidentified (owner to confirm object/sequence): */
#define IMG_UNKNOWN_179   179 /* 1280x720 gray-face frame; NOT the title Background */
#define IMG_DEVIL_SAD     233 /* = IMG_DEATH_DEVIL_A (kept alias) */
#define IMG_DEVIL_SHOCKED 460 /* = IMG_DEATH_DEVIL_B (kept alias) */

/* Music-box crank (Frame 3 "[ Music Box ]" group, Cam 04 view only).
 * Layer #5 UI positions from Objects.txt: button box center-anchored at
 * [569,497], Wind Text top-left at [497,475] (sits inside the box),
 * Click & Hold top-left at [491,534] (just under the box), wind gauge
 * top-left at [418,474] (the Music Left counter spot, left of the box).
 * Alterable A (held crank) swaps the box Stopped <-> Animation 12; the
 * drain (+100/0.35 s held, -Night*2/0.07 s released) and the warning
 * badges run in core. The box pair is corroborated by the reference
 * shot (dark-slate box + "Give $ To Business Edward" + "click & hold"
 * + pie with a wedge missing): 133 dark-slate Stopped = released,
 * 178 olive Animation 12 = held. */
#define IMG_MUSICBTN_OFF  133 /* crank released: dark-slate box, 156x65 */
#define IMG_MUSICBTN_ON   178 /* crank held: olive box, 156x65 */
#define IMG_MUSIC_WIND_TEXT 210 /* "Give $ To Business Edward", 142x37 */
#define IMG_MUSIC_CLICKHOLD 180 /* "click & hold" hint, 154x14 */
/* Wind gauge: 22 pie frames (181-202), 54x54, empty -> full disc.
 * Frame index follows Music Left (0-2000): holding the crank winds it
 * up (pie fills), releasing drains it (pie loses wedges). Replaces the
 * "MUSIC: n" bitmap readout (the value stays in the window title). */
#define IMG_MUSIC_PIE_FIRST 181
#define IMG_MUSIC_PIE_COUNT 22
/* Low-music badges ("[ Warning Messages ]"): <600 steady Stopped,
 * <200 flashing Animation 12, <=0 hidden. Two badge objects with their
 * own frame banks (bank IDs 35-42): "Warning out of cam" 35-38 at
 * [1228,672] on the office screen, "warning in cam" 39-42 at [1215,506]
 * on any camera view. Per badge the bank holds two triangle sizes, each
 * followed by its transparent blink frame (36 pairs 35, 38 pairs 37,
 * 40 pairs 39, 42 pairs 41): Stopped holds the first triangle steady,
 * Animation 12 blinks the second triangle against its transparent frame
 * on the shared static tick. */
#define IMG_WARN_OUT_STEADY 35 /* out-of-cam Stopped: triangle 57x49 */
#define IMG_WARN_OUT_FLASH  37 /* out-of-cam Animation 12: triangle 63x55 */
#define IMG_WARN_OUT_BLANK  38 /* out-of-cam Animation 12: transparent 63x55 */
#define IMG_WARN_IN_STEADY  39 /* in-cam Stopped: triangle 33x29 */
#define IMG_WARN_IN_FLASH   41 /* in-cam Animation 12: triangle 37x32 */
#define IMG_WARN_IN_BLANK   42 /* in-cam Animation 12: transparent 37x32 */
