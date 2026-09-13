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
- `343.png` / `312.png` — Cam 01 / Hell ride (empty / Freddy present)
- `347.png` / `348.png` — Cam 02 / Mountain (empty / Foxy present)
- `350.png` / `379.png` — Cam 03 / Forest (empty / Freddy present)
- `212.png` / `211.png` — Cam 04 / Dinosaur Exhibit (empty / Foxy present)
- `46.png`–`53.png` — TV-static animation (8-frame loop; title flickers
  alpha 100+Random(100), cameras run 150+Random(50) every 0.08 s while the
  feed is live, 0 on signal loss)
- `4.png` — 6 AM screen
- `1.png` — death screen
- `2.png` — GOOD JOB final screen
- `7.png` — newspaper screen
- `246.png`–`252.png` — night cards, reused centered on black for the
  Frame 6 Which Night interstitial (Fusion parks Which Night at (640,360))

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

The SDL-rectangle debug HUD is gone: no more power bar, door boxes, camera
label boxes, or mask overlay. Live state (night/hour/power, camera name,
doors, mask, music) is reported in the window title until the real counter /
button art is mapped. Warning (1) and Customize (8) stay black — their UI art
is unmapped and the title background was the wrong image there.

## Next exact-visual step

The remaining visual work is to bind the individual Active animation sequences
to their extracted image-bank entries (Freddy/Foxy/Springtrap/phantoms, camera
flip animation, mask + flashlight, cam flip/mask flip visuals, minimap + cam
labels, music-box UI, warnings, Rec, Connection Lost, Power Out, mute-call
button, time/night/power/usage counters). The event logic is already separated
from this rendering layer, and the doorway-figure / warning / signal-loss
state it needs is exposed on `FnaeGame`.
