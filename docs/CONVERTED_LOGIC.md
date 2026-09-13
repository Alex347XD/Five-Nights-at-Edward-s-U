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
- Camera buttons (`[ Is Up ]` + `[ Cam 01 ]` groups): clicking any "CAM 01"
  box moves You onto it and the view follows the overlapped "Cam 0X Text".
  Native click zones are the Objects.txt button hotspots (60x40
  center-anchored: [1179,371], [953,469], [1161,505] verbatim, CAM 01 at
  [1016,307] — 32px above its [1016,339] hotspot per owner request);
  the feed follows `g->camera` in `fnae_update`, like the You-overlap events.
- Camera UI visibility (`[ Camera Buttons Visibility ]`): minimap, cam
  labels/texts, static, White Frame Camera, CAM 01 boxes, and Rec appear
  with View > 0 and hide at View = 0; the native White Frame is a 2px
  hollow border, and the text/counter HUD (time/night/power/usage/room)
  additionally stays up on the office screen.
- Camera-feed auto-pan (`[ Camera Scrolling ]`): the Camera Center Object
  drifts +/-1 px per tick and bounces direction at each end; the feed view
  follows it whenever a camera is up. The pan is clamped to the feed image
  ends (display left edge 0..320), so no black bars show. Alterable B
  stays 1, so the drift runs unconditionally.
- Audio lure: E or clicking the Lure button (`381.png` "Lure" at [744,296],
  camera-up only, hidden on the Cam 04 music-box view) places a Lure Area
  (gray circle `390.png`, drawn mostly transparent over the viewed cam's
  minimap button) on the viewed camera and starts the button's Animation 12
  cooldown (1 → 2 → 3 → 4 square dots: `313/338/334/341.png` over the ~2 s
  window); 2 s later it pulls Springtrap there on a 50% roll with a Camera
  Out static burst, and the Lure Area is destroyed. The cooldown is
  independent of the marker: destroying the area never ends it, and a new
  lure requires the cooldown to have fully elapsed.
- Music box uses the Dinosaur Exhibit camera (view 4).
- Music box starts at 2000, loses `Night * 2` every 0.07 s on every screen
  (the Fusion drain has no view gate — only the crank press needs View 4),
  and winding adds 100 every 0.35 s. The crank (Alterable A) is held-wound:
  pointer over the [569,497] button with the mouse down, or the R test key,
  evaluated every tick with `death = 0`, so releasing, leaving the button,
  or leaving Cam 04 stops the wind (no latch). Warnings: <600 low (Stopped
  badge), <200 critical (flashing Animation 12), <=0 empty (hidden).
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
- Warning (Frame 1): boots here; `Timer equals 05''` auto-advances to Title
  after 5 s, and any key skips (no click event in Fusion, so clicks don't
  advance it).
- 6 AM routes to Final for nights 6/7 or Night Story >= 5, else increments to
  the next night via Which Night. Newspaper advances on any click.
- Springtrap starts on Cam 02's box (Fusion Start-of-Frame placement) and
  steps between cameras on its move flag: Cam 01 -> Cam 03 (B=0) / Cam 02
  (B=1), Cam 02 -> Cam 04 (B=0) / Cam 01 (B=1), Cam 03 -> Cam 01,
  Cam 04 -> Cam 02, with B re-rolled on Cam 01/02 like the Fusion
  RRandom(0,1) sets. Cam 03 is the kill room (4 s watch -> 50% death id 4).
  The Stand figure reappears while viewing Springtrap's camera.
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
 - Audio (`src/audio.c`, mixer channels = Fusion Sound channels): night entry
  stops everything then loops fansound #1, In The Depths #2, Camera Audio #3,
  deepbreaths #6, stare #9, buzzlight #10, With_S2 #13, Music_Box_Melody #8;
  volumes follow the events (fan 30 office / 10 cams, cam-audio 0 / 50,
  melody 0/0/20/10/50 by view 0/1/2/3/4, close-ambience 0 / 50, deepbreaths
  50 masked / 0 unmasked, stare 0 live / 50 on signal loss, buzzlight 0 /
  70 while the flashlight is on; Change/flip/mask one-shots on ch #4/#5 at
  50). One-shots: lure echo1/3b/4b + stop (ch #14),
  doors SFXBible_12478 (#7), flip up/down STEREO_CASSETTE 704/701 (#5),
  mask on/off FENCING_43/42 (#5), cam change Change (#4), Freddy deep steps /
  Foxy metallic thud (#12, panned left), Ph BB scream3 (#18, on the 80-tick
  scare), Mangle garble1 loop while the annoy runs + breathing once it ends
  (#17), Springtrap walk1 (#15), windup2 every 0.50 s while winding (#11),
  jackinthebox loop on empty (#20), powerdown after stop-all (#19),
  jumpscares on ch #2 after stop-all (Puppet money-counter, Freddy XSCREAM,
  Foxy dino-roar, Springtrap scream3, GF XScream2), calls on #16 per night,
  title static (one-shot #1) + darkness loop (#2) at 50, Which-Night Change
  (one-shot #1) at 50, Final music box (one-shot #1) at 50,
  6 AM Clock Chimes (one-shot #1) at 50, Death goblin loop (#32).
- Phone calls (`[ Phone Calls ]`): each night's call starts ~3 s in
  (`current_call`, ch #16, MP3 for 1-3 / WAV for 4-6); the Mute Call button
  (`415.png` at [100,55], visible while ch #16 plays) stops the call on
  click via `FNAE_SND_CALL_STOP`.

## Deliberately isolated

The exported text does not describe the exact visual animation durations of every Active animation. Those should be wired to the extracted PNG frame sequences rather than guessed. The native core therefore represents animation states separately from the event logic.
