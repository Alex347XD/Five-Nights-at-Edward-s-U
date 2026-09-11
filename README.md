# Five Nights at Edward's — actual-frame native conversion

This build now uses the extracted FNaE PNG bank instead of placeholder rectangles.

## Controls
- Enter/Space: start/retry
- C: cameras
- 1-4: camera feeds
- L/R: left/right door
- M: mask
- F: flashlight
- L in cameras: audio lure
- Esc/C: return to office

## Important conversion boundary
The supplied CTFAK dump contains the extracted assets and object/event-related project data, but not a complete human-readable event-sheet listing. CTFAK's own README confirms it can dump everything and read event data, but a literal one-to-one event translation still requires the event export/decompiler output. The native logic therefore keeps exact recovered concepts separate from provisional values.

The four camera scenes are wired to real extracted 1280x720 images. The image IDs are centralized in `game_render()` so the scene IDs can be corrected without rewriting gameplay.
