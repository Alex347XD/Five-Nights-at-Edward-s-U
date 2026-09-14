# Frame 2 — Title asset map

Mapped from the exported Frame 2 object layout and the extracted image bank.

| Fusion object | Native asset | Size | Notes |
|---|---:|---:|---|
| Background | `515.png` | 1280x720 | Title background (Stopped sequence; sequences 12–14 are the single-frame flashes `516.png`/`517.png`/`518.png`, held 0.2 s) |
| Static | `46.png`–`53.png` | 1280x720 | 8-frame noise loop, alpha flickers 100+Random(100) |
| Template Title | `464.png` | 266x271 | "Five Nights at Edward's" text card at (64,96) |
| New | `239.png` | 203x33 | Menu item at (96,448) |
| Continue | `240.png` | 204x34 | Menu item at (96,512) |
| 6 Night | `241.png` | 227x44 | Menu item at (96,576) |
| Custom | `242.png` | 306x44 | Menu item at (96,640) |
| Arrow | `245.png` | 43x26 | Selected-item arrow; hotspot-anchored, see `COORDINATES.md` |
| Star / Star 2 / Star 3 | `232.png` | 57x55 | Same source image, unlocked by progress |
| The Night | (Counter, no PNG) | — | Saved night number at (326,545), shown while Continue is selected; rendered with the bitmap font like all other Fusion counters/strings |

## Important correction

`238.png` (1066x511) is an office desk scene, not the title card. The
600x507 devil cards `233.png` (sad eyes) / `460.png` (wide eyes) look like
two frames of one object but are currently unassigned — owner to confirm
which object/sequence they belong to. See `src/fnae_assets.h` for the
named-ID registry (`IMG_DEVIL_SAD`, `IMG_DEVIL_SHOCKED`).
