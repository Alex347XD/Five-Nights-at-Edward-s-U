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

## Functional controls

- Title: Up/Down or W/S, Enter, mouse menu selection
- Night: A/D doors, S camera, M mask, Z/Ctrl flashlight, 1–4 camera selection, R/mouse for music-box winding
- Camera and door transition timers are implemented
- Flashlight and music-box controls release correctly on key-up

## Known limitations

The project is still a native reconstruction rather than a byte-for-byte Clickteam runtime replacement. Audio, save/INI persistence, several secondary frames, and exact character animation still need further conversion work.
