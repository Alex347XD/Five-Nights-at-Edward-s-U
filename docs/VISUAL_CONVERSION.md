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
- `520.png` — HELP WANTED newspaper screen (Frame 7, shown once on New Game
  before Which Night)
- `28.png` — camera minimap line art (372x322, YOU baked in) at [882,265],
  drawn only while a camera is up
- `29.png` / `30.png` — cam button boxes (60x40, gray Stopped / green
  Animation-12 selected), center-anchored on the Frame 3 "CAM 01"
  hotspots; CAM 01 draws 32px above its hotspot ([1016,307] instead of
  [1016,339]) per owner request
- `31.png`–`34.png` — "CAM 01".."CAM 04" button labels (31x25, top-left,
  CAM 01 label shifted with its box)
- `381.png` — "Lure" audio-lure button (128x64 Stopped frame, Layer #5 UI
  at [744,296] center-anchored, camera-up only, hidden on Cam 04)
- `313.png` / `338.png` / `334.png` / `341.png` — Lure Button Animation 12
  cooldown (128x64 each, transparent with 1 → 2 → 3 → 4 white square
  dots), played over the ~2 s lure window, then back to Stopped
- `390.png` — Lure Area marker (gray circle, 256x256, Layer #5) drawn
  center-anchored over the lured camera's minimap button until the lure
  resolves (Fusion spawns it at (0,0) from the viewed CAM 01 button)
- `236.png` — Springtrap Stand figure (full-body blue Edward with stars,
  299x715, Layer #2 at [416,-24], drawn over the feed while viewing
  Springtrap's camera)
- `133.png` / `178.png` — music-box crank box (156x65 each,
  center-anchored at [569,497], Cam 04 view only; 133 dark-slate
  Stopped = released, 178 olive Animation 12 = held)
- `210.png` — Music Box Wind Text ("Give $ To Business Edward",
  142x37, top-left at [497,475], drawn on the crank box)
- `180.png` — Click & Hold hint (154x14, top-left at [491,534],
  under the box)
- `181.png`–`202.png` — wind-gauge pie (22 frames, 54x54,
  empty→full disc), top-left at the Music Left counter spot [418,474];
  frame follows Music Left 0–2000 (fills while held, loses wedges
  released). Replaces the "MUSIC: n" bitmap readout (value stays in
  the window title)
- `37.png` / `38.png` — low-music warning badges (63x55 each,
  center-anchored; PROVISIONAL: 37 triangle = Stopped steady below 600,
  38 black = Animation 12 flash below 200, hidden when empty; out-of-cam
  at [1228,672] on the office screen, in-cam at [1215,506] on cameras)
- `415.png` — Mute Call button ("MUTE CALL", 121x31, center-anchored at
  [100,55] on office and camera screens while the night's call plays;
  clicking it stops ch #16)
- White Frame Camera ([-1,0], camera-up only) — no mapped PNG; drawn as a
  2px hollow white feed border inset 8px from the screen edge
- Night HUD (time of day [1186,65] + am [1200,37], Which Night? [759,85]
  + The Night [1245,101], Power [129,627] + Power Left [24,616] + Usage
  Text [24,632], Cam Labels [888,272]) — Fusion text/counter objects with
  no PNG frames; drawn with a built-in 5x7 bitmap font on office and
  camera views alike
- `351.png` — Frame 1 warning screen (fullscreen 1280x720 card at [0,0])
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
button art is mapped. Customize (8) stays black — its UI art is
unmapped.

## Next exact-visual step

The remaining visual work is to bind the individual Active animation sequences
to their extracted image-bank entries (Freddy/Foxy/phantoms, camera
  flip animation, mask + flashlight, cam flip/mask flip visuals, White Frame
  Camera, music-box UI,
  warnings, Rec, Connection Lost, Power Out,
mute-call button, time/night/power/usage counters, Cam Labels room-name
string). The event logic is already separated from this rendering layer, and
the doorway-figure / warning / signal-loss state it needs is exposed on
`FnaeGame`.
