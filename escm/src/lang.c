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

#include "escm.h"

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

#define HASH_KEY(a, b) ((a) * 128 + (b))
/* hash_key = get_data(lead_char, &ptr, &data)
 */
static int 
get_data(int lead, char **pptr, char **pdata)
{
  char *ptr;
  int c;
  int i;

  ptr = *pptr;
  if (*ptr != lead) return -1; /* error */
  for (/**/; *ptr != ' ' && *ptr != '\t'; ptr++) {
    if (!*ptr) return -1;
  }
  for (/**/; *ptr == ' ' || *ptr == '\t'; ptr++) {
    if (!*ptr) return -1;
  }
  for (i = 0; i < 2; i++, ptr++) {
    if (!*ptr || *ptr == '\n' || *ptr == ' ' || *ptr == '\t')
      return -1; /* error */
  }
  c = HASH_KEY(ptr[-2], ptr[-1]);
  for (/**/; *ptr != '\n'; ptr++) {
    if (!*ptr) return -1; /* error */
  }
  ptr++;
  if (!*ptr) {
    *pptr = NULL;
    *pdata = NULL;
  } else if (*ptr == lead) {
    *pptr = ptr;
    *pdata = NULL;
  } else {
    *pdata = ptr;
    for (/**/; *ptr != lead || *(ptr - 1) != '\n'; ptr++) {
      if (!*ptr) {
	*pptr = NULL;
	return c;
      }
    }
    *(ptr-1) = '\0';
    *pptr = ptr;
  }
  return c;
}

static int
parse_name(char *p, char **pname)
{
  if (*p != '<') return FALSE;
  p++;
  if (*p != '?') return FALSE;
  p++;
  *pname = p;
  for (/**/; *p != ' ' && *p != '\t'; p++) {
    if (*p == '\n' || *p == '\0') return FALSE;
  }
  *p = '\0';
  return TRUE;
}
static int
parse_bind(char *data, char **prefix, char **infix, char **suffix, int *flag)
{
  char *p;

  *prefix = data;
  *flag = 1;
  p = strstr(data, "variable-name");
  if (p == NULL) {
    *flag = 0;
    p = strstr(data, "variable_name");
  }
  if (p == NULL) return FALSE;
  if (p == data) *prefix = NULL;
  *p = '\0';
  p += 13;
  *infix = p;
  p = strstr(p, "value");
  if (p == NULL) return FALSE;
  *p = '\0';
  p += 5;
  *suffix = p;
  if (!*p) *suffix = NULL;
  return TRUE;
}
static int
parse_expression(char *data, char **prefix, char **suffix)
{
  char *p;
  *prefix = data;
  p = strstr(data, "expression");
  if (p == NULL) return FALSE;
  *p = '\0';
  p += 10;
  *suffix = p;
  return TRUE;
}
static int
parse_string(char *data, char **prefix, char **suffix)
{
  char *p;
  *prefix = data;
  p = strstr(data, "string");
  if (p == NULL) return FALSE;
  *p = '\0';
  p += 6;
  *suffix = p;
  return TRUE;
}

struct escm_lang *
parse_lang(const char *name, const char **interp)
{
  int c;
  char *ptr, *data;
  int flag1, flag2;

  if (!read_conf(name)) escm_error("Can't open config file for %s", name);
  ptr = buffer;
  c = get_data(buffer[0], &ptr, &data);
  if (c == -1) escm_error("syntax error for %s", name);
  c = buffer[0];
  /* name */
  if (!parse_name(data, &(mylang.name)))
    escm_error("syntax error for %s", name);
  while (ptr != NULL) {
    switch (get_data(c, &ptr, &data)) {
    case -1:
      escm_error("syntax error for %s", name);
      /* not reached */
    case HASH_KEY('b', 'i'): /* bind */
      if (!parse_bind(data, &(mylang.bind_prefix), &(mylang.bind_infix), &(mylang.bind_suffix), &flag1))
	escm_error("syntax error for %s", name);
      break;
    case HASH_KEY('a', 's'): /* assign */
      if (!parse_bind(data, &(mylang.assign_prefix), &(mylang.assign_infix), &(mylang.assign_suffix), &flag2))
	escm_error("syntax error for %s", name);
      break;
    case HASH_KEY('i', 'n'): /* initialization */
      mylang.init = data;
      break;
    case HASH_KEY('n', 'i'): /* nil */
      mylang.nil = data;
      break;
    case HASH_KEY('c', 'o'): /* command */
      *interp = data;
      break;
    case HASH_KEY('f', 'i'): /* finalization */
      mylang.finish = data;
      break;
    case HASH_KEY('e', 'x'): /* expression */
      if (!parse_expression(data, &(mylang.display_prefix), &(mylang.display_suffix)))
	escm_error("syntax error for %s", name);
      break;
    case HASH_KEY('s', 't'): /* string */
      if (!parse_string(data, &(mylang.literal_prefix), &(mylang.literal_suffix)))
	escm_error("syntax error for %s", name);
      break;
    default:
      escm_error("syntax error for %s", name);
    }
  }
  if (!mylang.bind_infix) {
    mylang.bind_prefix = mylang.assign_prefix;
    mylang.bind_infix = mylang.assign_infix;
    mylang.bind_suffix = mylang.assign_suffix;
    flag1 = flag2;
  }
  if (!mylang.assign_infix) {
    mylang.assign_prefix = mylang.bind_prefix;
    mylang.assign_infix = mylang.bind_infix;
    mylang.assign_suffix = mylang.bind_suffix;
    flag2 = flag1;
  }
  mylang.use_hyphen = flag1;
  return &mylang;
}
/* end of lang.c */
