# Actual visual conversion

The renderer now uses real PNGs extracted from the supplied MFA/CTFAK dump.

## Scene assets wired

These match `visuals_init` in `src/visuals.c`:

- `227.png` — office scene (drawn cover-cropped to the 1280-wide view and
  panned by `office_scroll`; mouse at the screen edges reveals more of that
  side, per `[ Office Panning ]`)
- `144.png`–`159.png` — Left Door shutter (16 frames, open→closed) at [119,0]
- `160.png`–`175.png` — Right Door shutter (16 frames, open→closed) at [1263,0]
- `238.png` — desk scene (1066x511) at [266,177]; doors/desk pan with the office
- `211.png` — Cam 01 / Hell-volcano environment
- `379.png` — Cam 02 / Mountain-forest environment
- `350.png` — Cam 03 / Forest environment
- `312.png` — Cam 04 / Dinosaur Exhibit environment
- `46.png`–`53.png` — TV-static animation (8-frame loop; title flickers
  alpha 100+Random(100), cameras draw it at alpha 35)
- `4.png` — 6 AM screen
- `1.png` — death screen
- `2.png` — GOOD JOB final screen
- `7.png` — newspaper screen

The title screen is reconstructed from individual parts instead of a single
image (`2.png` is the GOOD JOB completed-night screen, not the title — see
`TITLE_CONVERSION.md` and `TITLE_ASSET_MAP.md`).

The source asset IDs are kept as numeric filenames because CTFAK's dump
renames image-bank entries by ID.

## Runtime

The C core controls:
- frame/state
- night/hour
- camera
- power
- doors
- mask
- animatronic AI

The visual layer only renders that state. This keeps the implementation portable
to Wii U SDL2.

## Next exact-visual step

The remaining visual work is to bind the individual Active animation sequences
to their extracted image-bank entries (Freddy/Foxy/Springtrap/phantoms, camera
flip animation, mask animation, doors, UI counters). The event logic is already
separated from this rendering layer.
