# histo.awk - print a histograph
# $Id$
#
# $1 is a non-negative number.
#
BEGIN {
  max = 0;
  if (!width) {
    width = 5;
  }
}
// {
  class = int(($1 + 1) / width);
  star[class] = star[class] "*";
  if (class > max) {
    max = class;
  }
}
END {
  for (i = max; i >= 0; i--) {
    printf "%4d-  %s\n", width * i, star[i];
  }
}
