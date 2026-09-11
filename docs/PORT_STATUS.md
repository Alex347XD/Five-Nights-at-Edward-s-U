# FNaE Native Port Status

This build is a functional gameplay prototype, not yet a 1:1 Clickteam recreation.

## Fixed in this revision
- Camera flip now has a real transition and reaches CAM_UP.
- S toggles camera up/down.
- 1-4 select cameras while cameras are up.
- Door A/D controls now complete their close/open transitions.
- Flashlight Z/Ctrl is released correctly on key-up.
- Mouse clicks are wired to title buttons, doors, camera flip, camera selection and music-box winding.
- Night menu navigation works with keyboard and mouse.
- Title screen no longer uses the incorrect GOOD JOB CAPTAIN image.
- Title Template asset is layered at its exported position.
- Camera 02/03 mapping was corrected to use the extracted forest character/background assets.

## Still not 1:1
- Clickteam animation sequences are not all reconstructed.
- Fusion INI save persistence is not implemented.
- Exact audio channel mixing is not implemented.
- Exact camera UI/minimap/labels and animatronic sprite layers are incomplete.
- Customize Night is still a simplified entry point.
- Jumpscare/death presentation is simplified.
- Springtrap audio lure and several phantom/GF timing details are simplified.
