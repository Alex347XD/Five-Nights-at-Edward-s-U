# Native FNaE port status

## Current state

The native SDL2 build contains the recovered gameplay state machine and extracted visual assets.

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
- `246.png`–`252.png` — The Night counter frames

The previous incorrect mapping of `233.png` has been removed. `233.png` is a death-animation frame.

The menu arrow (`245.png`) is placed at the Fusion offsets from Frame 2
Events.txt ((-10,+16/17/19/19) from each item's top-left), adjusted for the
hotspot: Fusion positions the arrow by its pointing tip (right-center) while
SDL draws from the top-left, so the native renderer draws it with
`FNAE_ANCHOR_RIGHT_CENTER` (see `COORDINATES.md`).

## Functional controls

- Title: Up/Down or W/S, Enter, mouse menu selection
- Night: A/D doors, S camera, M mask, Z/Ctrl flashlight, 1–4 camera selection, E audio lure, R/mouse for music-box winding
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
  0 on signal loss); the scene shakes during the death wait
- Camera feeds auto-pan left <-> right on the Camera Center Object drift
  (+/-1 px per tick, clamped to the feed image ends so no black bars show),
  drawn cover-cropped so the full 1600px feed scrolls through the view
  instead of letterboxing

## Known limitations

The project is still a native reconstruction rather than a byte-for-byte Clickteam runtime replacement. Audio, save/INI persistence, the Customize-screen UI, and exact character animation still need further conversion work. Mask/flashlight/button/counter art is identified only where named in `src/fnae_assets.h`; unmapped objects render as nothing (no invented stand-ins).
