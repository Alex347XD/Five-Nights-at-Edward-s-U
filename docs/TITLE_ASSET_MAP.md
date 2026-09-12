# Frame 2 — Title asset map

Mapped from the exported Frame 2 object layout and the extracted image bank.

| Fusion object | Native asset | Size | Notes |
|---|---:|---:|---|
| Background | `179.png` | 1280x720 | Title background |
| Template Title | `233.png` | 600x507 | Red-devil title card, positioned at (64,96) |
| New | `239.png` | 203x33 | Menu item at (96,448) |
| Continue | `240.png` | 204x34 | Menu item at (96,512) |
| 6 Night | `241.png` | 227x44 | Menu item at (96,576) |
| Custom | `242.png` | 306x44 | Menu item at (96,640) |
| Arrow | `245.png` | 43x26 | Selected-item arrow; hotspot-anchored, see `COORDINATES.md` |
| Star / Star 2 / Star 3 | `232.png` | 57x55 | Same source image, unlocked by progress |
| The Night | `246.png`–`252.png` | ~231x97 | 1st–7th Night counter frames at (326,545) |

## Important correction

`233.png` (600x507, red devil on white card) **is** the Title Template —
it is what `visuals_init` loads. `238.png` (1066x511) is an office desk
scene, not the title card; `460.png`, `540.png`, and `541.png` are
death-animation frames. None of those belong on the title screen.
