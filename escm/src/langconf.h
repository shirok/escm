#ifndef LANGCONF_H
#define LANGCONF_H 1
#define ESCM_ARGV_SCM "/usr/bin/gosh", "-b"
extern struct escm_lang escm_scm;
#ifdef IN_MAIN_C
#ifdef ESCM_PLURISCRIPT
static struct escm_lang * const lang[] = {
  &escm_scm,
};
#endif /* ESCM_PLURISCRIPT */
#endif /* IN_MAIN_C */
#endif /* LANGCONF_H */
