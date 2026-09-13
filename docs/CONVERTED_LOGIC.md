# Actual MFA -> C conversion

This build is based on the **Fusion Export As Text** dump uploaded with the project.

## Frames converted

1. Warning
2. Title
3. Night
4. Death
5. Final
6. Which Night
7. Newspaper
8. Customize
9. 6 AM

## Directly translated Night systems

- 50-second hour clock; 12 -> 1 wrap; hour > 5 enters 6 AM.
- Power starts at 10000 hidden units.
- Power drain intervals: usage 1=2.00 s, 2=0.50 s, 3=0.25 s, 4=0.15 s, 5=0.10 s.
- Drain amount: `10 * ((Night / 5) + 1)`.
- Usage level: `1 + camera_up_check + left_door + right_door + flashlight + Ph Mangle Camera * 2`.
- Left door uses A/D; right door uses D, matching the exported events.
- Door animation (Left/Right Door Alterable Value A): 0=open (Stopped frame 0),
  1=closing (Close 0→15), 2=closed (hold 15), 3=opening (Open 15→0), over the
  existing 0.25 s transitions. Left art is `144.png`–`159.png` (223x720) at
  [119,0], right art is `160.png`–`175.png` (248x720) at [1263,0].
- Desk (`238.png`, 1066x511) draws at [266,177] in the office view (hidden
  while the camera is up); doors/desk pan with the office scroll.
- Title background flash: Random(50)=1 plays one RRandom(12,14) sequence —
  a single frame (`516.png`/`517.png`/`518.png`) held for the 0.2 s cut
  window — then back to Stopped (`515.png`). Static advances every 3rd tick.
- Office panning (`[ Office Panning ]`): pointer over the Left 1/2/3 zones
  (X 225/168/119) scrolls left at 2/4/6 px per tick, Right 1/2/3 zones
  (X 1025/1088/1143) scroll right at 2/4/6 px per tick; clamped so the
  1280-wide view stays inside the 1600-wide office scene. Desktop hover only
  (PC/Mobile = 0), office view only (View = 0), no pan while dead. Starts
  centered (Fusion starts at the left edge, X 640).
- Camera flip uses S; camera 1-4 are Hell, Mountain, Forest, Dinosaur Exhibit.
- Camera-feed auto-pan (`[ Camera Scrolling ]`): the Camera Center Object
  starts at Game Width / 2 - 120 and drifts +/-1 px per tick, bouncing
  between 520 and 1080 (display left edge -120..440, 120 px of overscan
  past each feed edge); the feed view follows it whenever a camera is up.
  Alterable B stays 1, so the drift runs unconditionally.
- Audio lure uses E on Cam 01.
- Music box uses the Dinosaur Exhibit camera (view 4).
- Music box starts at 2000, loses `Night * 2` every 0.07 s, and winding adds 100 every 0.35 s.
- Empty music box can trigger Puppet death while camera is up or mask is down.
- Freddy route: Cam 01 -> Cam 03 -> left door -> pending death / back to Cam 01.
- Foxy route: Cam 02 -> Cam 04 -> right door -> pending death / back to Cam 02.
- Freddy death id = 2; Foxy death id = 3.
- Springtrap death id = 4.
- Golden Freddy death id = 5.
- Puppet death id = 1.
- Phantom BB forces the camera down after its 80-tick camera state.
- Phantom Mangle forces the camera down after its 60-tick camera state.
- Golden Freddy uses the exported random/death-addup logic.
- Nights 1-7 difficulty values and hourly changes are translated from the event text.
- Night 7 loads AI from the Customize-screen globals (defaults 0, Puppet 7;
  see `fnae_set_custom`), and the All-20 star reads those globals, not the nightly rolls.
- Which Night (Frame 6): 0 = normal night, 1 = 6th, 2 = 7th/custom; the frame
  auto-advances to Night after 2 s (Return skips the wait).
- 6 AM routes to Final for nights 6/7 or Night Story >= 5, else increments to
  the next night via Which Night. Newspaper advances on any click.
- Springtrap steps between cameras on its move flag (Cam 01 -> Cam 03 kill
  room, Cam 02 <-> Cam 04 with the exported random branch); E places an audio
  lure on the viewed camera that can pull Springtrap to it after ~2 s.
- Camera Out ("Connection Lost"): 50% re-tune every 0.5 s, forced clear after
  2 s; camera static runs 150+Random(50) every 0.08 s while the feed is live,
  0 on signal loss.
- Music-box warnings (<600 low, <200 critical, <=0 empty) and the power-out
  fade (255 -> 0, death rolls start once it is gone) are tracked in core.
- Closed doors swing open on power loss; any death forces the cameras down,
  the mask off, and the flashlight off; the night scene shakes +/-5 during
  the death wait (GF +/-8).
- Doorway-figure flags (Freddy at left door, Foxy at right, Springtrap on its
  viewed cam) are exposed for the renderer.

## Deliberately isolated

The exported text does not describe the exact visual animation durations of every Active animation. Those should be wired to the extracted PNG frame sequences rather than guessed. The native core therefore represents animation states separately from the event logic.
