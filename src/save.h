#pragma once

/* Native replacement for the Fusion INI object + savestring pair.
 *
 * Fusion names the file from the savestring ("Edward") under group
 * "Base" with items Night / Progress / Challenge<N>. The native file
 * is "Edward.ini" in the working directory (same place assets/ resolves
 * from), written in plain INI form:
 *
 *   [Base]
 *   Night=1
 *   Progress=0
 *   Challenge1=0
 *   Challenge2=0
 *   Challenge3=0
 *
 * - Night: story night 1-5 (Continue resumes it; New Game resets it).
 * - Progress: star unlocks 0-3 (1 = 6th night, 2 = custom, 3 = all-20).
 * - Challenge1..3: custom-night challenge completion flags. Nothing
 *   reads them yet (Customize UI is unmapped), but they round-trip so
 *   the file keeps whatever a future pass stores.
 *
 * All values are clamped on load; a missing/corrupt file yields defaults.
 * Store failures are silent (never block gameplay). C11, no dependencies,
 * so a future port (Wii U) can swap the file backend in one place.
 */

typedef struct {
 int night;        /* 1-5 story night (Continue target) */
 int progress;     /* 0-3 star unlocks */
 int challenge[4]; /* [1..3] completion flags ([0] unused) */
} FnaeSave;

void fnae_save_default(FnaeSave *s);
/* Loads Edward.ini into s (defaults first). Returns 0 on success,
 * nonzero when the file is missing or unparsable (s keeps defaults). */
int fnae_save_load(FnaeSave *s);
/* Writes s to Edward.ini. Returns 0 on success, nonzero on IO error. */
int fnae_save_store(const FnaeSave *s);
/* Save filename ("Edward.ini", working-directory relative). */
const char *fnae_save_path(void);
