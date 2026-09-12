# Native FNaE port status

## Current state

The native SDL2 build contains the recovered gameplay state machine and extracted visual assets.

### Frame 2 title screen

The title screen now uses the correct Frame 2 asset mapping:

- `179.png` — Background
- `238.png` — Template Title
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
SDL draws from the top-left, so the native renderer shifts by the arrow size
to keep it beside the menu text instead of overlapping it.

## Functional controls

- Title: Up/Down or W/S, Enter, mouse menu selection
- Night: A/D doors, S camera, M mask, Z/Ctrl flashlight, 1–4 camera selection, R/mouse for music-box winding
- Camera and door transition timers are implemented
- Flashlight and music-box controls release correctly on key-up
- Title menu responds to physical Up/Down keys (SDL1-era keycodes `273`/`274`/`308` replaced with `SDLK_` constants; `308` was Left Alt for the flashlight)

## Known limitations

The project is still a native reconstruction rather than a byte-for-byte Clickteam runtime replacement. Audio, save/INI persistence, several secondary frames, and exact character animation still need further conversion work.
