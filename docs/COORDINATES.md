# Fusion coordinates vs SDL coordinates

Read this before porting any positioned object from `docs/mfa_text/`.

## The mismatch

Fusion object coordinates address the object's **hotspot** (action point),
not the image's top-left corner. SDL (`SDL_RenderCopy`, and our
`draw_texture` in `src/visuals.c`) always draws from the **top-left**.

Copying Fusion numbers literally as SDL draw origins is only correct when
the hotspot happens to be top-left. For anything else the sprite lands
shifted by up to its full size — and yes, we will keep running into this:
every positioned object in every frame's `Objects.txt` needs the same
interpretation (title arrow, Customize-screen arrows, stars, counters, …).

## What we know

- The text export (`docs/mfa_text/*/Objects.txt`) lists positions but **not**
  hotspot locations, so the hotspot must be inferred per object.
- Static/scenery objects (Background `[0, 0]`, Template Title `[64, 96]`)
  behave as top-left: their Fusion numbers work verbatim in SDL.
- Pointer/indicator graphics use a tip hotspot. Proven case: the Frame 2
  menu Arrow parks at `[-22, 461]` — fully offscreen only if its 43px width
  extends *left* of the hotspot (right-center anchor). Drawn top-left from
  the same numbers it overlapped the menu text (see below).

## The utility

`src/visuals.h` provides `FnaeAnchor` + `visuals_draw_anchored()` so ported
code keeps **verbatim Fusion numbers** and states the anchor explicitly:

| Anchor | Use for | SDL origin = |
|---|---|---|
| `FNAE_ANCHOR_TOP_LEFT` | Scenery, panels, menu items (default) | `(x, y)` |
| `FNAE_ANCHOR_CENTER` | Centered sprites, spinning/rotating objects | `(x - w/2, y - h/2)` |
| `FNAE_ANCHOR_RIGHT_CENTER` | Right-pointing indicators (menu arrow) | `(x - w, y - h/2)` |

Texture sizes are queried at draw time, so asset swaps don't silently
re-break alignment.

## Worked example: title arrow

Frame 2 `Events.txt` places the arrow at `(-10,+16/17/19/19)` from each menu
item's top-left, i.e. hotspot positions `(86,464/529/595/659)`:

```c
static const int arrow_x[4] = {86, 86, 86, 86};
static const int arrow_y[4] = {464, 529, 595, 659};
visuals_draw_anchored(r, v->title_arrow, arrow_x[a], arrow_y[a],
                      FNAE_ANCHOR_RIGHT_CENTER);
```

Drawing those numbers top-left put the 43px-wide `>>` on top of the menu
text; the anchor lands its tip at `(86,464)`, i.e. beside the text at
`(43,451)`. Verified with `scripts/headless/title.txt` before/after
screenshots.

## Rule of thumb for new ports

1. Take the position from `Objects.txt` / `Events.txt` verbatim.
2. Ask what the hotspot is: scenery → `TOP_LEFT`; centered/rotating →
   `CENTER`; something that *points at* another object → the tip edge.
   The parked/offscreen position in `Objects.txt` is a good hint — it
   should look deliberately hidden under your assumed anchor.
3. Verify with a headless screenshot script (`scripts/headless/`) — never
   by eyeballing numbers. `title.txt` is the template to copy.
