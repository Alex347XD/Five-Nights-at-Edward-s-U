# Native FNaE port status

## Current state

The native SDL2 build contains the recovered gameplay state machine and extracted visual assets.

### Frame 2 title screen
The title screen now uses the correct Frame 2 asset mapping:

- `179.png` — Background (Stopped sequence)
- `515.png`–`518.png` — Background flash animation (Random(50)=1 plays
  RRandom(12,14), cut back to Stopped after 0.2 s; played as one 4-frame flash)
- `46.png`–`53.png` — animated Static overlay (alpha flickers 100+Random(100))
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
- Night: A/D doors, S camera, M mask, Z/Ctrl flashlight, 1–4 camera selection, R/mouse for music-box winding
- Night: mouse position pans the office view — pointer in the left/right edge
  zones scrolls toward that side at the Fusion 2/4/6 px-per-tick speeds,
  clamped to the 1600px-wide office scene (see `CONVERTED_LOGIC.md`)
- Camera and door transition timers are implemented
- Night: doors render their 16-frame shutter animation (`160.png`–`175.png`
  left at [119,0], `144.png`–`159.png` right at [1263,0]), and the desk
  (`238.png`) sits at [266,177] — all panning with the office view
- Flashlight and music-box controls release correctly on key-up
- Title menu responds to physical Up/Down keys (SDL1-era keycodes `273`/`274`/`308` replaced with `SDLK_` constants; `308` was Left Alt for the flashlight)

## Known limitations

The project is still a native reconstruction rather than a byte-for-byte Clickteam runtime replacement. Audio, save/INI persistence, several secondary frames, and exact character animation still need further conversion work.
