/*************************************************************
 * lang.c - parser of interpreter language configuration files.
 * $Id$
 * Author: TAGA Yoshitaka <tagga@tsuda.ac.jp>
 *
 * WARNING: This still has many security problems!
 *************************************************************/
#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif /* HAVE_STDLIB_H */
#include <string.h>
#include <ctype.h>

#include "escm.h"

#define ESCM_LANGCFG_SIZE 512
static char buf[ESCM_LANGCFG_SIZE];
static struct escm_lang mylang;

/* parse_lang(langname, &interp) - parse a language configuration file.
 */
struct escm_lang *
parse_lang(const char *name, const char *interp)
{
  FILE *fp;
  size_t n;
  char *p;
  int leading_char;

  if (*name == '.' || *name == '/') {
    fp = fopen(name, "r");
  } else {
    strcpy(buf, ESCM_LANG_DIR);
    strcat(buf, name);
    fp = fopen(buf, "r");
  }
  if (fp == NULL) escm_error(PACKAGE, NULL);
  n = fread(buf, 1, ESCM_LANGCFG_SIZE - 1, fp);
  fclose(fp);
  if (n == 0) escm_error(PACKAGE, NULL);
  buf[n + 1] = '\0';
  leading_char = buf[0];
  p = strchr(buf, '\n');
  if (p == NULL) escm_error(PACKAGE, "an error in the language config file.");
  /* name */
  p = strstr(p, "<?");
  if (p == NULL) escm_error(PACKAGE, "an error in the language config file.");
  p += 2;
  mylang.name = p;
  p = strchr(p, ' ');
  if (p == NULL) escm_error(PACKAGE, "an error in the language config file.");
  *p = '\0';
  p++;
  p = strchr(p, '\n');
  if (p == NULL) escm_error(PACKAGE, "an error in the language config file.");
  p++;
  p = strchr(p, '\n');
  if (p == NULL) escm_error(PACKAGE, "an error in the language config file.");
  p++;
  /* interpreter */
  *interp = p;
  p = strchr(p, '\n');
  if (p == NULL) escm_error(PACKAGE, "an error in the language config file.");
  *p = '\0';
  p++;
  p = strchr(p, '\n');
  if (p == NULL) escm_error(PACKAGE, "an error in the language config file.");
  p++;
  /* initialization */
  if (*p == leading_char) {
    mylang.init = NULL;
  } else {
    mylang.init = p;
    while (*p != leading_char) {
      p = strchr(p, '\n');
      if (p == NULL) escm_error(PACKAGE, "an error in the language config file.");
      p++;
    }
    *p = '\0';
    p++;
  }
  p = strchr(p, '\n');
  if (p == NULL) escm_error(PACKAGE, "an error in the language config file.");
  p++;
  /* string */
  mylang.literal_prefix = p;
  p = strstr(p, "string");
  if (p == NULL) escm_error(PACKAGE, "an error in the language config file.");
  *p = '\0';
  p += 6;
  mylang.literal_suffix = p;
  p = strchr(p, '\n');
  if (p == NULL) escm_error(PACKAGE, "an error in the language config file.");
  *p = '\0';
  p++;
  /* expression */
  p = strchr(p, '\n');
  if (p == NULL) escm_error(PACKAGE, "an error in the language config file.");
  p++;
  mylang.display_prefix = p;
  p = strstr(p, "expression");
  if (p == NULL) escm_error(PACKAGE, "an error in the language config file.");
  *p = '\0';
  p += 10;
  mylang.display_suffix = p;
  p = strchr(p, '\n');
  if (p == NULL) escm_error(PACKAGE, "an error in the language config file.");
  *p = '\0';
  p++;
  p = strchr(p, '\n');
  if (p == NULL) escm_error(PACKAGE, "an error in the language config file.");
  /* finalization */
  p++;
  if (!*p) {
    mylang.finish = NULL;
  } else {
    mylang.finish = p;
  }
  return &mylang;
}

#ifdef ESCM_LANG_TEST
#define PRINT_FIELD(field) {\
  if (ptr->field) printf(#field " => \'%s\'\n", ptr->field);\
  else printf(#field " => NULL\n");\
}
void
dump_escm_lang(struct escm_lang *ptr)
{
  PRINT_FIELD(name);
  PRINT_FIELD(literal_prefix);
  PRINT_FIELD(literal_suffix);
  PRINT_FIELD(display_prefix);
  PRINT_FIELD(display_suffix);
  PRINT_FIELD(init);
  PRINT_FIELD(finish);
}
int main(int argc, char **argv)
{
  struct escm_lang *lang;
  char *interp = NULL;

  lang = parse_lang(argv[1], &interp);
  if (lang != NULL) dump_escm_lang(lang);
  if (interp != NULL) printf("interpreter => %s\n", interp);
  return 0;
}
#endif /* ESCM_LANG_TEST */
/* end of lang.c */
