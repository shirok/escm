# funclen.awk -- show the lengthes of C functions
# Usage: awk -f funclen.awk *.c
# $Id$
/^[A-Za-z_][A-Za-z_0-9]*\(/ {
  sub(/\(.*/, "");
  filename[$0] = ARGV[ARGIND];
  count[$0] = 1;
  name = $0;
  flag = 1;
}
flag == 1 && /^\}/ {
  count[name]++;
  flag = 0;
}
flag == 1 && !/^\}/ {
  count[name]++;  
}
END {
  for (f in filename) {
    printf("%4d %s (%s)\n", count[f], f, filename[f]) | "sort -nr";
  }
}
EOF
