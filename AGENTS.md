# Agents — Five Nights at Edward's (Native C/SDL2)

Canonical instructions for AI agents working in this repo.

## Stack

C11, CMake 3.16+, Ninja, SDL2 + SDL2_image. Canonical toolchain is
Windows 10/11 + MSYS2 UCRT64 (see README.md).

Canonical modules (what actually builds — see `CMakeLists.txt`):
`src/main.c` (entry, interactive loop), `src/fnae_core.c` (game state
machine), `src/visuals.c` (SDL renderer), `src/headless.c` (scripted
headless test runs). `src/game.c` / `src/assets.c` / `src/fnae.h` are an
unbuilt parallel API — do not mix them in; ask before deleting.

## Build (MSYS2 UCRT64)

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

This produces the self-contained bundle `build/FNaE/` (`FNaE_Native.exe`
+ `assets/` + SDL DLLs). Run it from the repo root (resolves `assets/`)
or from inside `build/FNaE/`:

```bash
./build/FNaE/FNaE_Native.exe
```

`cmake *` is pre-approved in `opencode.json`; anything destructive still
asks first.

## Testing (headless + screenshots)

No display or real input needed. Headless runs fixed 1/60 steps, honors
`SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy`, and saves screenshots:

```bash
./build/FNaE/FNaE_Native.exe --headless --frames 60 \
  --script scripts/headless/title.txt
```

- Script format (`<frame> <key|keyup|click|shot> <args>`, `#` comments):
  see `scripts/headless/example.txt`. Keys are true SDL keycodes
  (`return/enter, esc/escape, space, up/down/left/right`, single chars).
- Tracked scripts: `scripts/headless/example.txt` (title→night→camera),
  `scripts/headless/title.txt` (menu-arrow alignment). Scratch work goes
  in `scripts/headless/local.txt` (git-ignored).
- Screenshots land under `screenshots/` (git-ignored, `.gitkeep` kept).
  Clean with `cmake --build build --target clean-screenshots`.
- Loop: write/tweak a script → run → read the PNGs → patch → rebuild →
  re-shoot. `title.txt` is the template for visual-alignment checks.

## Docs map (`docs/`)

- `PORT_STATUS.md` — current port state: title asset mapping, working
  controls, known limitations. Update when converting Fusion logic.
- `CONVERTED_LOGIC.md` — night/AI/power/music-box systems translated from
  the MFA event text, with exact values.
- `VISUAL_CONVERSION.md` — renderer architecture and wired scene assets
  (mirrors `visuals_init`); core-owns-state, visuals-only-render.
- `TITLE_CONVERSION.md` — how Frame 2 title is reconstructed from parts.
- `TITLE_ASSET_MAP.md` — title asset table with sizes/positions (source
  of truth for title layout; includes the `233`-vs-`238` correction).
- `COORDINATES.md` — Fusion hotspot coordinates vs SDL top-left draws,
  the `FnaeAnchor` utility, and the verification rule. Read before
  porting any positioned object.
- `mfa_text/` — Fusion "Export As Text" dumps, the source of truth for
  positions (`Objects.txt`) and behavior (`Events.txt`).

## Rules

- Keep to C11 + SDL2 public API. Update `PORT_STATUS.md` / `docs/`
  when converting Fusion logic.
- Do not commit MSYS2 absolute paths (`C:/msys64...`); keep
  `CMakeLists.txt` portable.
- Fusion positions are hotspot-anchored, SDL draws top-left: use
  `visuals_draw_anchored()` with an explicit `FnaeAnchor` and verify with
  a headless screenshot — never eyeball numbers (see `COORDINATES.md`).
- Use `SDLK_` constants for keycodes, never raw numbers (SDL1 codes like
  `273`/`274`/`308` silently break under SDL2).
- Host may be Win11 Home: no Sandbox/Hyper-V. Prefer CI for
  untrusted builds; do not run opencode with `--auto`.
