# copyright.awk -- extract the copyright holders and theire Email addresses
# from AUTHOR and COPYING
# $Id$
#
# Usage: awk -f copyright.awk > ../src/copyright.h
#
BEGIN {
  ARGV[1] = "../AUTHORS";
  ARGV[2] = "../COPYING";
  ARGC = 3;
  cnt = 1;
}
ARGIND == 1 { # AUTHORS
  name = $1;
  flag = 0;
  for (i = 2; i <= NF; i++) {
    if (match($i, /^</)) {
      flag = 1;
      break;
    }
    name = name " " $i;
  }
  if (flag) address[name] = $i;
}
ARGIND == 2 && $1 == "Copyright" {
  this_year = $3;
  sub(/^ *Copyright *\([Cc]\) *[-0-9]* */, "");
  sub(/, *All rights .*$/, "");
  year[$0] = this_year;
  all[cnt] = $0;
  cnt++;
}
END {
  print "#ifndef COPYRIGHT";
  print "#define COPYRIGHT \\";
  printf("\"");
  for (i = 1; i < cnt; i++) {
    name = all[i];
    printf("Copyright (c) %s %s %s\\n\\\n", year[name], name, address[name]);
  }
  print "\"";
  print "#endif /* COPYRIGHT */";
}
# end of copyright.awk
