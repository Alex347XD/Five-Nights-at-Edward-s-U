#include "save.h"

#include <stdio.h>
#include <string.h>

#define SAVE_FILE "Edward.ini"
#define SAVE_GROUP "Base"

const char *fnae_save_path(void) { return SAVE_FILE; }

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
int fnae_save_load(FnaeSave *s) {
 if (!s) return 1;
 fnae_save_default(s);
 FILE *f = fopen(SAVE_FILE, "r");
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

int fnae_save_store(const FnaeSave *s) {
 if (!s) return 1;
 FILE *f = fopen(SAVE_FILE, "w");
 if (!f) return 1;
 fprintf(f, "[%s]\n", SAVE_GROUP);
 fprintf(f, "Night=%d\n", clamp_night(s->night));
 fprintf(f, "Progress=%d\n", clamp_progress(s->progress));
 for (int i = 1; i <= 3; i++)
  fprintf(f, "Challenge%d=%d\n", i, clamp_flag(s->challenge[i]));
 int rc = (fclose(f) == 0) ? 0 : 1;
 return rc;
}
