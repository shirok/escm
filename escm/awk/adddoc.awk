# adddoc.awk - Update doc/Makefile.am
# $Id$
#
# See Makefile for usage
#
BEGIN {
  gsub(/\.[^ ]*\//, "", sources);
  html = sources;
  gsub(/\.escm/, ".html", html);
}
/^noinst_DATA =/ {
  print "noinst_DATA = " html;
}
/^EXTRA_DIST =/{
  print "EXTRA_DIST = " sources;
}
!/^noinst_DATA =/ && !/^EXTRA_DIST =/ {
  print;
}
