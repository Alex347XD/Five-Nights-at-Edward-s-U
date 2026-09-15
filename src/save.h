#pragma once

/* Native replacement for the Fusion INI object + savestring pair.
 *
 * Fusion names the file from the savestring ("Edward") under group
 * "Base" with items Night / Progress / Challenge<N>. The native file is
 * Edward inside %APPDATA%\MMFApplications (the classic MMF save
 * location), written in plain INI form:
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
 * - Challenge1..3: custom-night challenge completion flags, read (Check
 *   marks) and written (unmodified-preset night-7 clear) by the
 *   Customize screen.
 *
 * The MMFApplications folder is created on first store. When %APPDATA%
 * is unavailable (non-Windows / future ports), the file falls back to
 * the working directory. A leftover working-directory Edward from
 * earlier builds is imported once (new location wins when both exist).
 *
 * Wii U (__WIIU__): the primary file is fs:/vol/save/common/Edward, the
 * title's common save dir, so a NAND/USB install (WUP Installer) keeps
 * progress on the console like a proper title (SaveMii-compatible). This
 * needs the common_save_size declared in assets/wiiu/meta.xml, which the
 * OS uses to allocate the save area at install. When vol/save is
 * unavailable (HBL / .wuhb run, Cemu without a mounted save dir), load
 * and store fall back to the SD copy
 * (fs:/vol/external01/wiiu/apps/FNaE/Edward), then the working
 * directory. Moving from a fallback layout to an installed title starts
 * a fresh save unless the old file is injected (e.g. SaveMii).
 *
 * All values are clamped on load; a missing/corrupt file yields defaults.
 * Store failures are silent (never block gameplay). C11 plus mkdir only,
 * plain stdio on every platform (Wii U fs: paths included), so no
 * platform SDK includes are needed here.
 */

typedef struct {
 int night;        /* 1-5 story night (Continue target) */
 int progress;     /* 0-3 star unlocks */
 int challenge[4]; /* [1..3] completion flags ([0] unused) */
} FnaeSave;

void fnae_save_default(FnaeSave *s);
/* Loads the save into s (defaults first). Returns 0 on success,
 * nonzero when no readable file exists (s keeps defaults). */
int fnae_save_load(FnaeSave *s);
/* Writes s to the save file. Returns 0 on success, nonzero on IO error. */
int fnae_save_store(const FnaeSave *s);
/* Resolved save filename (%APPDATA%\MMFApplications\Edward, or
 * fs:/vol/save/common/Edward on Wii U, or a working-directory Edward
 * fallback). Pointer stays valid. */
const char *fnae_save_path(void);
