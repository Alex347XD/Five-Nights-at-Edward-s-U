# Title screen conversion

The native renderer now reconstructs Frame 2 (Title) from the exported Fusion object layout instead of using `2.png`, which is the GOOD JOB CAPTAIN completed-night screen.

Mapped title assets:
- Background: `515.png` (Stopped; `516.png`–`518.png` are the flash frames)
- New: `239.png`
- Continue: `240.png`
- 6th Night: `241.png`
- Custom Night: `242.png`
- Arrow: `245.png`
- Star / Star 2 / Star 3: `232.png`

Positions are taken directly from Frame 2 (Title)/Objects.txt.

See `TITLE_ASSET_MAP.md` for the full asset table (with sizes and the
`233.png`-vs-`238.png` template correction) and `COORDINATES.md` for how
Fusion hotspot positions translate to SDL draw origins (matters for the
menu arrow).
