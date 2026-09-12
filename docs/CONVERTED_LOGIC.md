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
- Office panning (`[ Office Panning ]`): pointer over the Left 1/2/3 zones
  (X 225/168/119) scrolls left at 2/4/6 px per tick, Right 1/2/3 zones
  (X 1025/1088/1143) scroll right at 2/4/6 px per tick; clamped so the
  1280-wide view stays inside the 1600-wide office scene. Desktop hover only
  (PC/Mobile = 0), office view only (View = 0), no pan while dead. Starts
  centered (Fusion starts at the left edge, X 640).
- Camera flip uses S; camera 1-4 are Hell, Mountain, Forest, Dinosaur Exhibit.
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

## Deliberately isolated

The exported text does not describe the exact visual animation durations of every Active animation. Those should be wired to the extracted PNG frame sequences rather than guessed. The native core therefore represents animation states separately from the event logic.
