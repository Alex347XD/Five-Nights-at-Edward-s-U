#include "save.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <direct.h>
#define FNAE_MKDIR(p) _mkdir(p)
#else
#include <sys/stat.h>
#define FNAE_MKDIR(p) mkdir(p, 0755)
#endif

#define SAVE_FILE "Edward"
#define SAVE_DIR "MMFApplications"
#define SAVE_GROUP "Base"
/* Working-directory save from earlier builds: imported once when the
 * new location has no file yet. */
#define SAVE_LEGACY "Edward"

#ifdef __WIIU__
/* Installable-title save (NAND/USB): the OS mounts this title's save area
 * at fs:/vol/save when assets/wiiu/meta.xml declares a common_save_size,
 * so plain stdio lands on NAND (or USB, wherever the title is installed).
 * The common dir is shared across accounts, like the single desktop save,
 * and needs no per-account handling. */
#define SAVE_WIIU_PRIMARY "fs:/vol/save/common/Edward"
/* Fallbacks when vol/save is unavailable (HBL / .wuhb run, Cemu without a
 * mounted save dir): the homebrew app folder on the SD card, then the
 * working directory. Tried in order; the first readable file wins. */
#define SAVE_WIIU_SD "fs:/vol/external01/wiiu/apps/FNaE/Edward"
#endif

/* Resolved save path, computed once: %APPDATA%\MMFApplications\Edward
 * on Windows, fs:/vol/save/common/Edward on Wii U (title save on
 * NAND/USB), plain Edward wherever %APPDATA% is unavailable. */
static char save_path[512];
static int save_path_ready = 0;

static const char *resolve_path(void) {
 if (!save_path_ready) {
#ifdef __WIIU__
  snprintf(save_path, sizeof save_path, "%s", SAVE_WIIU_PRIMARY);
#else
  const char *app = getenv("APPDATA");
  if (app && *app)
   snprintf(save_path, sizeof save_path, "%s\\%s\\%s", app, SAVE_DIR, SAVE_FILE);
  else
   snprintf(save_path, sizeof save_path, "%s", SAVE_FILE);
#endif
  save_path[sizeof save_path - 1] = '\0';
  save_path_ready = 1;
 }
 return save_path;
}

const char *fnae_save_path(void) { return resolve_path(); }

/* Creates the save folders (everything before the final \ or /).
 * Missing folders are normal on first run; errors are ignored and
 * surface as a store failure instead. Each level is created in turn so
 * multi-level paths (the Wii U SD fallback) work even when several
 * folders are missing. */
static void ensure_parent_dir(const char *path) {
 char tmp[512];
 size_t n = strlen(path);
 if (n == 0 || n >= sizeof tmp) return;
 strcpy(tmp, path);
 char *sep = strrchr(tmp, '\\');
 char *fsep = strrchr(tmp, '/');
 if (fsep && (!sep || fsep > sep)) sep = fsep;
 if (!sep) return; /* bare filename: working directory exists */
 *sep = '\0';
 for (char *p = tmp + 1; *p; p++) {
  if (*p == '\\' || *p == '/') {
   char c = *p;
   *p = '\0';
   FNAE_MKDIR(tmp);
   *p = c;
  }
 }
 FNAE_MKDIR(tmp);
}

void fnae_save_default(FnaeSave *s) {
 memset(s, 0, sizeof *s);
 s->night = 1;
 s->progress = 0;
}

static int clamp_night(int n) { return n < 1 ? 1 : n > 7 ? 7 : n; }
static int clamp_progress(int p) { return p < 0 ? 0 : p > 3 ? 3 : p; }
static int clamp_flag(int f) { return f ? 1 : 0; }

/* Minimal INI reader: finds the [Base] group, then exact "Key=number"
 * lines (whitespace around the key tolerated, '#'/' ;' comments and
 * other groups ignored). Unknown keys are skipped. */
static int load_file(FnaeSave *s, const char *path) {
 FILE *f = fopen(path, "r");
 if (!f) return 1;
 int in_base = 0, ok = 0;
 char line[256];
 while (fgets(line, sizeof line, f)) {
  char *p = line;
  while (*p == ' ' || *p == '\t') p++;
  if (*p == '\0' || *p == '\n' || *p == '\r' || *p == '#' || *p == ';') continue;
  if (*p == '[') {
   size_t len = strlen(p);
   in_base = (len >= 6 && strncmp(p + 1, SAVE_GROUP, 4) == 0 && p[5] == ']');
   continue;
  }
  if (!in_base) continue;
  char *eq = strchr(p, '=');
  if (!eq) continue;
  *eq = '\0';
  char *key = p;
  char *end = eq - 1;
  while (end > key && (*end == ' ' || *end == '\t')) { *end = '\0'; end--; }
  int v = 0, n = 0;
  if (sscanf(eq + 1, "%d%n", &v, &n) < 1 || n <= 0) continue;
  if (strcmp(key, "Night") == 0) { s->night = clamp_night(v); ok = 1; }
  else if (strcmp(key, "Progress") == 0) { s->progress = clamp_progress(v); ok = 1; }
  else if (strcmp(key, "Challenge1") == 0) s->challenge[1] = clamp_flag(v);
  else if (strcmp(key, "Challenge2") == 0) s->challenge[2] = clamp_flag(v);
  else if (strcmp(key, "Challenge3") == 0) s->challenge[3] = clamp_flag(v);
 }
 fclose(f);
 return ok ? 0 : 1;
}

int fnae_save_load(FnaeSave *s) {
 if (!s) return 1;
 fnae_save_default(s);
 const char *path = resolve_path();
 if (load_file(s, path) == 0) return 0;
#ifdef __WIIU__
 /* Not installed to NAND/USB (HBL / .wuhb run, or Cemu without a mounted
  * save dir): fall back to the SD copy, then the working directory. */
 if (load_file(s, SAVE_WIIU_SD) == 0) return 0;
#endif
 /* One-time import of a working-directory save left by earlier builds;
  * the new location wins whenever both exist. */
 if (strcmp(path, SAVE_LEGACY) != 0 && load_file(s, SAVE_LEGACY) == 0) return 0;
 return 1;
}

static int store_file(const FnaeSave *s, const char *path) {
 ensure_parent_dir(path);
 FILE *f = fopen(path, "w");
 if (!f) return 1;
 fprintf(f, "[%s]\n", SAVE_GROUP);
 fprintf(f, "Night=%d\n", clamp_night(s->night));
 fprintf(f, "Progress=%d\n", clamp_progress(s->progress));
 for (int i = 1; i <= 3; i++)
  fprintf(f, "Challenge%d=%d\n", i, clamp_flag(s->challenge[i]));
 return (fclose(f) == 0) ? 0 : 1;
}

int fnae_save_store(const FnaeSave *s) {
 if (!s) return 1;
 if (store_file(s, resolve_path()) == 0) return 0;
#ifdef __WIIU__
 /* Title save unavailable: persist to the SD fallback, then CWD. */
 if (store_file(s, SAVE_WIIU_SD) == 0) return 0;
 if (store_file(s, SAVE_LEGACY) == 0) return 0;
#endif
 return 1;
}
