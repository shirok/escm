/*************************************************************
 * lang.c - parser of interpreter language configuration files.
 * $Id$
 * Author: TAGA Yoshitaka <tagga@tsuda.ac.jp>
 *************************************************************/
#ifdef HAVE_CONFIG_H
# include "config.h"
#endif /* HAVE_CONFIG_H */
#include <stdio.h>
#ifdef HAVE_STDLIB_H
# include <stdlib.h>
#endif /* HAVE_STDLIB_H */
#ifdef HAVE_STRING_H
# include <string.h>
#endif /* HAVE_STRING_H */
#include <ctype.h>

#include "escm.h"

#define HASH_KEY(a, b) ((a) * 128 + (b))

#define ESCM_LANGCFG_SIZE 512
char buffer[ESCM_LANGCFG_SIZE];
static struct escm_lang mylang;

#ifndef FALSE
# define FALSE 0
#endif /* FALSE */
#ifndef TRUE
# define TRUE !FALSE
#endif /* TRUE */

/* read_conf(lang)
 */
static int
read_conf(const char *lang)
{
  FILE *fp;
  size_t n;

  if (*lang == '.' || *lang == '/') {
    fp = fopen(lang, "r");
  } else {
    strncpy(buffer, ESCM_LANG_DIR, 256 + 128);
    strncat(buffer, lang, 127);
    fp = fopen(buffer, "r");
  }
  if (fp == NULL) return FALSE;
  n = fread(buffer, 1, ESCM_LANGCFG_SIZE - 1, fp);
  fclose(fp);
  buffer[n] = '\0';
  return TRUE;
}

/* ptr = get_data2(ptr, &name, &data)
 */
static char *
get_data2(char *p, char **pname, char **pdata)
{
  int leadchar;

  leadchar = *p;
  *pname = NULL;
  *pdata = NULL;
  if (leadchar == '\0') return NULL;
  p++;
  while (*p != ' ' && *p != '\t') {
    if (*p == '\0') return NULL;
    p++;
  }
  while (*p == ' ' || *p == '\t') {
    if (*p == '\0') return NULL;
    p++;
  }
  *pname = p;
  while (isalnum(*p)) {
    p++;
  }
  if (*p == '\0') return NULL;
  else if (*p != '\n') {
    *p = '\0';
    p++;
  }
  while (*p != '\n') {
    if (*p == '\0') return NULL;
    p++;
  }
  *p = '\0';
  p++;
  if (*p == '\0') return NULL;
  else if (*p == leadchar) return p;
  *pdata = p;
  for (;;) {
    while (*p != '\n') {
      if (*p == '\0') return NULL;
      p++;
    }
    p++;
    if (*p == '\0') return NULL;
    else if (*p == leadchar) break;
  }
  p[-1] = '\0';
  return p;
}

static int
parse2(char *data, char **prefix, char **suffix)
{
  char *prev, *this;

  *prefix = data;
  prev = strchr(data, '@');
  if (prev == NULL) return FALSE;
  for (;;) {
    this = strchr(prev + 1, '@');
    if (this == NULL) return FALSE;
    if (this - prev == 4) break;
    prev = this;
  }
  if (prev == data) *prefix = NULL;
  else *prev = '\0';
  *suffix = this + 1;
  if (!**suffix) *suffix = NULL;
  return TRUE;
}
static int
parse3(char *data, char **prefix, char **infix, char **suffix)
{
  int ret;

  ret = parse2(data, prefix, &data);
  if (!ret) return FALSE;
  return parse2(data, infix, suffix);
}

static int
parse_form2(char *data, struct escm_form_two *p)
{
  return parse2(data, &(p->prefix), &(p->suffix));
}

static int
parse_form3(char *data, struct escm_form_three *p)
{
  return parse3(data, &(p->prefix), &(p->infix), &(p->suffix));
}

struct escm_lang *
parse_lang(const char *name, const char **interp)
{
  char *ptr, *rname, *data;

  if (!read_conf(name)) escm_error(gettext("can't open - %s"), name);
  ptr = get_data2(buffer, &(mylang.name), (char **)interp);
  if (ptr == NULL || mylang.name == NULL || *interp == NULL)
    escm_error(gettext("broken config file - %s"), name);

  if (strcmp(mylang.name, "scm") == 0 || strcmp(mylang.name, "scheme") == 0)
    mylang.scm_p = 1;

  mylang.nil = "\"\"";

  while (ptr) {
    ptr = get_data2(ptr, &rname, &data);
    if (rname == NULL) escm_error(gettext("broken config file - %s"), name);
    switch (HASH_KEY(rname[0], rname[1])) {
    case HASH_KEY('b', 'i'): /* bind */
      if (!parse_form3(data, &(mylang.bind)))
	escm_error(gettext("broken config file - %s"), name);
      break;
    case HASH_KEY('a', 's'): /* assign */
      if (!parse_form3(data, &(mylang.assign)))
	escm_error(gettext("broken config file - %s"), name);
      break;
    case HASH_KEY('i', 'n'): /* init */
      mylang.init = data;
      break;
    case HASH_KEY('n', 'i'): /* nil */
      mylang.nil = data;
      break;
    case HASH_KEY('f', 'i'): /* finish */
      mylang.finish = data;
      break;
    case HASH_KEY('d', 'i'): /* display */
      if (data && !parse_form2(data, &(mylang.display)))
	escm_error(gettext("broken config file - %s"), name);
      break;
    case HASH_KEY('f', 'o'): /* format */
      if (data && !parse_form3(data, &(mylang.format)))
	escm_error(gettext("broken config file - %s"), name);
      break;
    case HASH_KEY('s', 't'): /* string */
      if (!parse_form2(data, &(mylang.literal)))
	escm_error(gettext("broken config file - %s"), name);
      break;
    default:
      escm_error(gettext("broken config file - %s"), name);
    }
  }
  if (!mylang.assign.infix)
    escm_error(gettext("broken config file - %s"), name);
  if (!mylang.literal.prefix || !mylang.literal.suffix)
    escm_error(gettext("broken config file - %s"), name);
  if (!mylang.bind.infix) mylang.bind = mylang.assign;
  return &mylang;
}
/* end of lang.c */
