# Native FNaE port status

## Current state

The native SDL2 build contains the recovered gameplay state machine and extracted visual assets.

### Frame 1 Warning

The game boots on the warning screen (`351.png`, 1280x720 at [0,0]:
"WARNING! This game contains flashing lights, loud noises, and lots of
jumpscares!"). It auto-advances to Title after 5 s (Fusion `Timer equals
05''`) and any key skips it, matching Frame 1 Events.txt (no click
event there, so clicks don't advance it).

### Frame 2 title screen
The title screen now uses the correct Frame 2 asset mapping:

- `515.png` — Background (Stopped sequence)
- `516.png`–`518.png` — Background flash frames (Random(50)=1 plays one
  RRandom(12,14) sequence, held for the 0.2 s cut window, then back to Stopped)
- `46.png`–`53.png` — animated Static overlay (alpha flickers 100+Random(100),
  frames advance every 3rd tick, ~20 fps)
- `464.png` — Template Title ("Five Nights at Edward's" text card at (64,96))
- `239.png` — New
- `240.png` — Continue
- `241.png` — 6 Night
- `242.png` — Custom
- `245.png` — Arrow
- `232.png` — Star / Star 2 / Star 3
- The Night — a Counter (no PNG): just the saved night number at
  (326,545), shown while Continue is selected, drawn with the bitmap
  font. (`246.png`–`252.png` are the "12:00 AM / Nth Night" cards for
  the Which Night screen, not the title — drawing one here sprawled a
  "12:00 AM" header over the menu.)

The previous incorrect mapping of `233.png` has been removed. `233.png` / `460.png` are the devil-card Death Anim backdrop cycle (Frame 4, [630,390]).

The menu arrow (`245.png`) is placed at the Fusion offsets from Frame 2
Events.txt ((-10,+16/17/19/19) from each item's top-left), adjusted for the
hotspot: Fusion positions the arrow by its pointing tip (right-center) while
SDL draws from the top-left, so the native renderer draws it with
`FNAE_ANCHOR_RIGHT_CENTER` (see `COORDINATES.md`).

Title entry re-reads Night/Progress from the save (Fusion Start-of-Frame),
so the Continue counter always shows the saved story night — 6th/custom
runs no longer leave a stale 6/7 on the menu — and clears the Frame 3
death state. Night simulation (power, AI, phantoms, music box, death
rolls) runs on Frame 3 only; idling on the title no longer drains the
power-out fade into a death screen.

## Functional controls

- Title: Up/Down or W/S, Enter, mouse menu selection
- Night: A/D doors, S camera, M mask, Z/Ctrl flashlight, 1–4 camera selection, E or Lure-button click for audio lure, R or press-and-hold on the crank for music-box winding
- Night: mouse position pans the office view — pointer in the left/right edge
  zones scrolls toward that side at the Fusion 2/4/6 px-per-tick speeds,
  clamped to the 1600px-wide office scene (see `CONVERTED_LOGIC.md`)
- Camera and door transition timers are implemented
- Night: doors render their 16-frame shutter animation (`144.png`–`159.png`
  left at [119,0], `160.png`–`175.png` right at [1263,0]), and the desk
  (`238.png`) sits at [266,177] — all panning with the office view
- Flashlight and music-box controls release correctly on key-up
- Title menu responds to physical Up/Down keys (SDL1-era keycodes `273`/`274`/`308` replaced with `SDLK_` constants; `308` was Left Alt for the flashlight)

### Frame 6 Which Night

Fixed: the frame no longer falls through to the title background. It shows
the night card (`246.png`–`252.png`) centered on black, routes 0 = normal /
1 = 6th / 2 = 7th-custom (Custom no longer lands on night 6), and
auto-advances to Night after 2 s like the Fusion `Every 02''` event. 6 AM
now progresses to the next night instead of replaying the same one.

### Frame 7 Newspaper

New Game shows the HELP WANTED newspaper (`520.png`) before Which Night;
Continue / 6 Night skip it, matching the Fusion jumps. (`7.png` is the
termination notice, not the newspaper.)

### Night scene

- Placeholder HUD rectangles (power bar, door boxes, camera label boxes,
  mask overlay) removed; live state (camera name, doors, mask, music) stays
  visible in the window title
- Camera static follows the Fusion cadence (150+Random(50) every 0.08 s,
  0 on signal loss); the scene shakes during the death wait, with the
  killer's jumpscare fullscreen over it (see `CONVERTED_LOGIC.md`), then
   the animated Death frame (red flash alone first, then devil cards
   cycling throughout, RIP (404) fading out then GAME OVER fading in
   with a shortened 0.5 s-gate pause, auto-Title).
   Death pacing is real-time now: the 60-tick jumpscare hold plus the
   ~171-tick Death animation (~3.9 s total) run through a tick accumulator,
  so on 144 Hz+ displays the Death screen no longer flies by in ~2 s.
- Golden Freddy haunts the office: while GF Random == 1 (rolled on each
   camera close, nights 2+) the GF Sit figure (`310.png`, 150x200 bottle
   close-up at [528,305], panning with the office) sits in the office view;
  staring for ~1.5 s kills (death 5, `458.png` still jumpscare + XScream2).
- Camera feeds auto-pan left <-> right on the Camera Center Object drift
  (+/-1 px per tick, clamped to the feed image ends so no black bars show),
  drawn cover-cropped so the full 1600px feed scrolls through the view
  instead of letterboxing
- Camera minimap (`28.png` line art at [882,265], YOU baked in) draws while
  a camera is up, with the four clickable cam buttons on top: gray box
  (`29.png`) normally, green box (`30.png`) under the viewed camera, each
  with its `31.png`–`34.png` "CAM 0X" label. Buttons sit at the Frame 3
  Objects.txt hotspots except CAM 01, which rides 32px above its hotspot
  ([1016,339] → [1016,307], label with it) so the box straddles its
  room's top edge like the other buttons (owner request, verified with
  headless screenshots); boxes are center-anchored (labels top-left).
  Clicking a button switches the feed like keys 1–4. The thin hollow
  white feed border (Fusion White Frame Camera at [-1,0], camera-up
  only) is drawn as a 2px outline inset 8px so a slight gap shows to
  the screen edge.
  The "Cam Labels" room-name string ("Hell", ...) has no PNG (Fusion
  String object) so it renders as bitmap text at [888,272].
- Audio lure (`381.png` "Lure" button at [744,296], camera-up only,
  hidden on Cam 04): E or click places a lure on the viewed camera,
  resolved 2 s later with a 50% pull + static burst. While the lure is
  active the button plays its Animation 12 cooldown (1 → 2 → 3 → 4 white
  square dots: `313.png`/`338.png`/`334.png`/`341.png`, 128x64) and a
  gray-circle Lure Area (`390.png`, 256x256) sits over the lured
  camera's minimap button. Springtrap (full-body blue Edward with stars,
  `236.png`, 299x715 stand at [416,-24]) starts on Cam 02 and steps the
  exported routes (Cam 03 kill room); its stand overlays the feed while
  viewed.
 - Music box (Cam 04 view): crank box (`133.png` dark-slate released /
  `178.png` olive held, 156x65 center-anchored at [569,497]) with
  Stopped↔Animation-12 swap on the held crank, Wind Text (`210.png`
  "Give $ To Business Edward", 142x37 top-left at [497,475], inside the
  box), Click & Hold hint (`180.png`, 154x14 top-left at [491,534],
  under the box), and the wind-gauge pie (22 frames `181.png`–`202.png`,
  54x54 empty→full, top-left at the Music Left counter spot [418,474],
  left of the box; frame follows Music Left 0–2000, so it fills while
  winding and loses wedges when released — verified against the
   reference shot with headless screenshots), plus low-music badges
   ("Warning out of cam" `35.png`–`38.png` steady `35` below 600 /
   blinking `37`/`38` below 200 at [1228,672] on the office screen;
   "warning in cam" `39.png`–`42.png` steady `39` / blinking `41`/`42`
   at [1215,506] on any camera view, not just Cam 04; both hidden when
   empty — verified with headless screenshots at each music level).
  press-and-hold on the button or held R — evaluated every tick, so
  release/leave/stop ends the wind (the old click latched it on).
- Phone calls play per night with a clickable MUTE CALL button (`415.png`
  at [100,55], visible on office and camera screens while the call plays,
  stops it on click).
- Phantoms render: Phantom Mangle's camera face (`405.png` x2.7, Layer #6
  top) and office Annoy (`380.png` rising from [508,720], Layer #3 above
  the desk, panning with the office), Phantom BB's camera face (`352.png`
  x2.7, Layer #6 top, dismissed by clicking a cam button) and post-scare
  fade (`349.png` x2.7, alpha 0→255 after the 80-tick force-down).
- Night HUD renders on all Frame 3 screens (office and camera views):
  "12 AM"-style clock top-right (time of day [1186,65] + am [1200,37]),
  "NIGHT n" under it (Which Night? [759,85] + The Night [1245,101]),
  "POWER: n%" + "USAGE:" bars bottom-left (Power [129,627] + Power Left
  [24,616] + Usage Text [24,632]). Fusion counters/strings are text, not
  PNG frames, and this port has no font library, so glyphs are a minimal
  built-in 5x7 bitmap (full A-Z, so "DINOSAUR EXHIBIT" renders intact). Live state also stays in the window title.

### Saving

Progress persists across restarts in `%APPDATA%\MMFApplications\Edward`
(working-directory `Edward` is imported once as a legacy fallback),
matching the Fusion INI group/items (`[Base]`
Night/Progress/Challenge1..3, savestring "Edward"). New Game resets to
night 1, 6 AM advances the saved night, beating nights 5/6/7 unlocks the
stars (6th night, custom night, all-20 third star) which gate the title
menu exactly like the Fusion `Progress` reads. Custom-night AI levels
still reset each launch (Fusion never wrote those to the INI either),
and the `ChallengeN` flags are now read (Check marks) and written
(unmodified-preset night-7 clear) by the Customize screen.

### Frame 8 Customize

The custom-night screen is fully wired (logic + rendering + input),
verified with headless screenshots (`scripts/headless/local.txt`
scratch script): Cool Background tiles (`409.png`/`410.png`/`414.png`,
Random(3) on entry), seven 150x200 portraits
(`344/408/340/310/256/237/285.png` left to right = Freddy / Mangle /
 Foxy / Golden / Springtrap / BB / Puppet — best-effort visual match,
 owner to confirm each character), Select Box frames (`422.png`) drawn
 as the full Objects.txt grid (top portrait row + middle row of 7 +
 bottom row of 6, the 7th bottom slot holding Set 20), hover
 arrows (`336.png` @1.3, down arrow flipped), AI counters as bitmap text
 at the verbatim hotspots, GO! / Set 20 / Add 1 buttons
 (`459/511/513.png`) targeting the selected column, rotated-triangle
 challenge arrows, bitmap
 Challenge Label (`NO CHALLENGE` / `THE CLASSICS` / `BROKEN DOWN` /
 `SOY SAUCE EDWARD`, visible while B==0), and checkbox outlines
 (`253.png`) while the selected challenge is beaten. Change blips play
 on ch #3 like the original. All levels start at 0, Puppet included
 (disables the music-box drain) per owner request — Fusion ships
 Puppet at 7 and clamps it 1-7.

### Audio

Sound is wired via SDL_mixer (`src/audio.c/h`, 43 samples from
`assets/audio/`): night ambience loops (fan/depths/camera-audio/deepbreaths/
stare/buzzlight/close-ambience/music-box melody) with the Fusion per-view,
mask/door/signal-loss/flashlight volume ducking (deepbreaths/stare/buzzlight
start silent; stare 0 live / 50 on Connection Lost, buzzlight 0 / 70 with
flashlight; Change/flip/mask one-shots at 50, title menu blips on ch #3), one-shots for lure echoes + stop, doors,
camera/mask flips (stereo-cassette; the camera-haunt phase stays silent on
ch #17 so cam open/close never plays garble1), cam-change blips, footsteps,
phantom scares (Mangle garble loop + breathing, Ph BB scream3), windup,
jack-in-the-box, power-down, jumpscares, phone calls (MP3 1-3, WAV 4-6),
and frame jingles (title static + darkness music, Which-Night Change,
Final music box, 6 AM chimes, Death goblin). Core pushes one-shots into
a queue (`fnae_push_sound`); loops/volumes are derived by polling game
state, so core still owns state. Headless-safe (dummy driver = silent).

## Known limitations

The project is still a native reconstruction rather than a byte-for-byte Clickteam runtime replacement. Exact character animation still needs further conversion work. Mask/flashlight/button/counter art is identified only where named in `src/fnae_assets.h`; unmapped objects render as nothing (no invented stand-ins).
