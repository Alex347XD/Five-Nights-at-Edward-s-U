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
- Door buttons (Button Left [105,500] / Button Right [1489,500], Layer #2
  world objects): clicking one toggles its door exactly like the A/D keys
  (Fusion `[ Doors ]` click events, same death/power/view gates as the keys
  via the shared toggle path). Native click zones are the 51x56 button
  frames (center-anchored) plus 2px grace: left fx 77–133, right fx
  1461–1517, y 470–530 in frame space (screen x + office scroll). The button
  shows Stopped (`176.png` dark red) while A is 0/3 and Animation 12
  (`177.png` olive) while A is 1/2; buttons hide with the doors when the
  cameras are up (office branch only).
- Doorway figures: Freddy at your door (`213.png` @1.1, center-anchored,
  drawn at [230,360], centered in the left doorway at Foxy's height)
  reappears while Freddy Collision overlaps
  Left Door Collision on the office screen and hides otherwise; Foxy Stand
  (`228.png` @1.1, drawn at [1387,331], centered in the right doorway)
  mirrors it for the right
  door (Fusion `[ Freddy ]` / `[ Foxy ]` visibility events, View > 0 hides
  both). Each figure draws behind its door shutter in Layer #2 order, so a
  closed door covers the character. Freddy's verbatim Objects.txt spot [260,788] sits below the 720
  screen and showed antennae only, so he is centered on the left door
  (door `[119,0]` is 223 wide, center x~230) per owner request. The native
  `freddy_door` / `foxy_stand` flags reuse the route positions (freddy
  pos 6, foxy pos 5) with the same office-view + power gates.
- Wii U GamePad (`src/wiiu.h`, hardware only): ZL = S (cameras), L/R = A/D
  (doors), Y = M (mask), B-hold = Z (flashlight), X = E (lure),
  ZR-hold = R (wind), A/Plus = Return, Minus = Mute-Call click zone,
  D-pad Up/Down = menu arrows, D-pad Left/Right = camera switch while
  viewing (title/customize arrows otherwise), left stick = office pan via
  the mouse zones, touchscreen = press/click/release in 1280x720 space.
  Dual screen: TV window renders with cameras forced down (office always),
  GamePad window renders the live camera UI while `CAM_UP`, black
  otherwise (owner: cams on gamepad, everything else on tv, black DRC
  when closed).
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
  (story nights 1-6; night 7/custom drains `Puppet AI * 2`, so Puppet 0
  disables the drain — custom-night only, per Frame 3 `[ Music Box ]`),
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
- Phantom Mangle forces the camera down after its 60-tick camera state, then
  runs the office annoy (A 0→224 while B==0, B+1 every 1 s at A>=224, A→0
  once B>=7, then C clears); ch17 stays silent through the camera phase so
  cam open/close keeps its stereo-cassette flip. Both phantoms roll
  Alterable A once per camera open/switch (stable overlay while viewing,
  no per-tick re-roll); leaving the cameras clears A/B silently.
- Phantom BB camera render state: the camera overlay (`352.png` x2.7,
  Layer #6 top) shows while A==1 on any Frame 3 screen; clicking any cam
  button while the view is up dismisses it (A/B = 0, Fusion "User clicks
  with left button on CAM 01"). The B>80 event additionally resets the
  Scare overlay alpha to 0, which then fades +7/tick to 255 with no view
  gate (`349.png` x2.7, Layer #6 top, gated on the first trigger so night
  start — where the Fusion initial A=255 has unrecoverable visibility —
  draws nothing). The Mangle Annoy (`380.png`, Layer #3 above the desk)
  draws at (508 − scroll, 720 − A) while A>0.
- Golden Freddy uses the exported random/death-addup logic: roll on the
  camera-down anim while AI > 0, GF Sit (`310.png`, 150x200, Layer #2 at
  [440,240]) reappears in the office while GF Random == 1, cam-up-anim or
  mask-down clears it, +1 Death Addup per tick while shown, >90 (~1.5 s
  of staring) kills (death 5, `458.png` still jumpscare). The whole group
  runs through a tick accumulator, so the roll rate and the 1.5 s stare
  last the same real time at any refresh rate.
- Nights 1-7 difficulty values and hourly changes are translated from the event text.
- Night 7 loads AI from the Customize-screen globals (defaults all 0,
  range 0-20 / Puppet 0-7; see `fnae_set_custom`), and the All-20 star reads
  those globals, not the nightly rolls (still requires Puppet == 7).
- Customize screen (Frame 8): hover a portrait to select its column (arrows
  appear at select+(36,88)/(36,152) @1.3 scale); hold an arrow to repeat
  +-1 every 0.10 s (0-20, Puppet 0-7 — owner request, Fusion clamps
  Puppet 1-7); Set 20 / Add 1 bump the selected column (Set 20 on Puppet
  clamps to 7); Left/Right Challenge cycle 0-3 with wraparound, reset the
  board to 0 and apply the preset while B>0 (1 = The Classics: Freddy 20 /
  Foxy 5 / Golden 20; 2 = Broken Down: Golden 10 / BB 20 / Springtrap 20 /
  Mangle 20; 3 = Soy Sauce Edward: Mangle 15 / Golden 20 / Foxy 10 /
  Freddy 15 / BB 20 / Springtrap 8 / Puppet 7); any manual edit clears B
  (label reappears, preset stops holding). GO! / Return starts night 7 via
  Which Night; Escape returns to Title. Check marks show while the selected
  challenge's save flag is set; beating night 7 with an unmodified preset
  writes Challenge<N>=1 (Frame 5 Final), which also feeds the 6 AM
  custom-night Progress star path.
- Which Night (Frame 6): 0 = normal night, 1 = 6th, 2 = 7th/custom; the frame
  auto-advances to Night after 2 s (Return skips the wait).
- Warning (Frame 1): boots here; `Timer equals 05''` auto-advances to Title
  after 5 s, and any key skips (no click event in Fusion, so clicks don't
  advance it).
- 6 AM (Frame 9) is black with the "which AM" odometer at [533,324]:
  Stopped "5" (`389.png`), rolling to "6" at 20fps from 3 s in and holding
  the last frame (`492.png`); the win screens never show here. It routes
  to Final for nights 6/7 or Night Story >= 5, else increments to
  the next night via Which Night. Final shows the per-night win screen
  (Frame 5 Events.txt creates one of Night 5/6/7 from the "6th or 7th
  night" counter): night 5 -> weekly paycheck (`2.png`), night 6 ->
  overtime paycheck (`4.png`), night 7/custom -> termination notice
  (`7.png`). Newspaper advances on any click.
- Mask (Frame 3 "[ Mask ]", office view only, Layer #5 UI above the HUD):
  putting on plays Flip Mask Down (`134.png`-`140.png` at [0,0]), the worn
  mask (`129.png`, 1480x870 at [-100,-66]) shows while down, taking off
  plays Flip Mask Up (`141.png`-`143.png` at [0,0]); frames run
  proportionally over the 0.45 s transitions with per-pixel alpha
  (transparent eye holes / fade edges).
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
- Jumpscares (Frame 3 "[ Jumpscares ]"): `death==N` spawns the character's
  art at (0,0) x2.8 fullscreen over the shaking office for the 60-tick
  wait (runs: Springtrap 314-331, Freddy 353-363+365, Puppet 493-510,
  Foxy 536-539+562-572, GF still 458; frames loop at 20fps). Death Addup
  >= 60 jumps to the Death frame. The 60-tick wait, the GF stare addup,
  and the Death-frame fades below all run through tick accumulators
  (`death_tick_acc` / `gf_tick_acc`): Fusion logic is per-tick but the
  native update gets real-time dt, so at 144 Hz+ the death sequence used
   to fly by in ~2 s; it now lasts the same ~3.9 s (1.0 s wait + ~2.9 s
   Death frame) at any refresh rate, identical to before at fixed 1/60.
- Death frame (Frame 4): fullscreen Red Fade In flashes +7/tick to 255
 then drains back to 0 (owner: a brief flash, not a permanent overlay —
 the native latches the first full-opacity moment so the text below keeps
 running as the flash clears); the devil-card Death Anim and the RIP Text
 stay hidden until that moment, so the red shows alone first. RIP Text
 fades -7/tick out (B==0, showing the 404 RIP frame), waits the 0.5 s
 gates (B 0->1->2, switching to the GAME OVER frame; Fusion uses 1 s,
 halved per owner for a snappier pause), fades +7/tick back in (B>1),
 then jumps to Title;
 the devil-card Death Anim (233/460) cycles underneath at [630,390] for
 the whole screen and the goblin loops ch #32 until the Title entry
 stops it.
- Doorway-figure flags (Freddy at left door, Foxy at right, Springtrap on its
  viewed cam) are exposed for the renderer.
 - Audio (`src/audio.c`, mixer channels = Fusion Sound channels): night entry
  stops everything then loops fansound #1, In The Depths #2, Camera Audio #3,
  deepbreaths #6, stare #9, buzzlight #10, With_S2 #13, Music_Box_Melody #8;
  deepbreaths/stare/buzzlight start silent (the mask-down / signal-loss /
  flashlight edges raise them); volumes follow the events (fan 30 office /
  10 cams, cam-audio 0 / 50, melody 0/0/20/10/50 by view 0/1/2/3/4,
  close-ambience 0 / 50, deepbreaths 50 masked / 0 unmasked, stare 0 live /
  50 on signal loss, buzzlight 0 / 70 while the flashlight is on;
  Change/flip/mask one-shots on ch #4/#5 at 50, title menu blips on ch #3).
  One-shots: lure echo1/3b/4b + stop (ch #14),
  doors SFXBible_12478 (#7), flip up/down STEREO_CASSETTE 704/701 (#5),
  mask on/off FENCING_43/42 (#5), cam Change (#4) on camera switch, camera
  flip-up, and Connection Lost, Freddy deep steps / Foxy metallic thud (#12,
  panned left, fired on the AI reaching the door with no view gate),
  Ph BB scream3 (#18, on the 80-tick scare), Mangle garble1 loop while the
  office annoy descends (C==1, Annoy A>0/B<7) + breathing once B hits 7
  (#17), Springtrap walk1 (#15),
  windup2 every 0.50 s while winding (#11), jackinthebox loop on empty (#20),
  powerdown after stop-all (#19), jumpscares on ch #2 after stop-all
  (Puppet money-counter, Freddy XSCREAM, Foxy dino-roar, Springtrap scream3,
  GF XScream2), calls on #16 per night, title static (one-shot #1) +
  darkness loop (#2) at 50 plus the title-entry Change blip (ch #3),
  Which-Night Change (one-shot #1) at 50, Final music box (one-shot #1) at
  50, 6 AM Clock Chimes (one-shot #1) at 50, Death goblin loop (#32).
- Phone calls (`[ Phone Calls ]`): each night's call starts ~3 s in
  (`current_call`, ch #16, MP3 for 1-3 / WAV for 4-6); the Mute Call button
  (`415.png` at [100,55], visible while ch #16 plays) stops the call on
  click via `FNAE_SND_CALL_STOP`.
- Saving (Fusion INI object + `savestring` "Edward", group "Base"):
  native `Edward` save in %APPDATA%\MMFApplications (`src/save.c`) with items
  `Night` (story night 1-5), `Progress` (star unlocks 0-3), and
  `Challenge1..3` (custom-night completion flags, round-tripped until
  the Customize UI reads them). Boot loads Night/Progress (missing or
  corrupt file = night 1, no stars); New Game (Newspaper start) resets
  Night to 1; 6 AM story advance writes Night+1; Final writes Progress 1
  (night 5), 2 (6th), or 3 (7th/custom all-20, never downgraded); Which
  Night / Night start re-read Night in normal mode (6/7 forced by the
  `six_or_seven` mode, like the Fusion Start-of-Frame events).

## Deliberately isolated

The exported text does not describe the exact visual animation durations of every Active animation. Those should be wired to the extracted PNG frame sequences rather than guessed. The native core therefore represents animation states separately from the event logic.
