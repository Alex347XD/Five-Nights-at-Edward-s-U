# Actual visual conversion

The renderer now uses real PNGs extracted from the supplied MFA/CTFAK dump.

## Scene assets wired

- `227.png` — office scene
- `211.png` — camera scene 1 / Hell-volcano environment
- `350.png` — camera scene 2 / Forest environment
- `227.png` — camera scene 3 / office feed
- `312.png` — camera scene 4 / Dinosaur Exhibit environment
- `46.png` — camera static overlay
- `2.png` — title-sized screen
- `4.png` — 6 AM-sized screen
- `7.png` — newspaper/final-sized screen

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
