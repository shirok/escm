# src/Makefile.am
# $Id$

bin_PROGRAMS = aescm
aescm_SOURCES = main.c
aescm_SOURCES += cgi.h cgi.c
aescm_SOURCES += copyright.h
aescm_SOURCES += parse/parse.h parse/parse.c
aescm_SOURCES += escm/escm.h escm/escm.c escm/fork.c
aescm_SOURCES += langconf.h

aescm_LDADD = @EXTRA_OBJS@ 
aescm_DEPENDENCIES = @EXTRA_OBJS@ 

EXTRA_DIST = config.h.in data/

INCLUDE = -I . -I./escm

%.c : data/%
	$(AWK) -f ../awk/make-lang-c.awk $< > $@

AM_CFLAGS = -Wall -Werror
# AM_CFLAGS = -Wall

# end of src/Makefile.am
