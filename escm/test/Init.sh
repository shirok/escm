# Init.sh -*- sh -*- 
# $Id$
# Author: TAGA Yoshitaka <tagga@tsuda.ac.jp>

# Initialization

# the program name and the test name
testname=`basename $0 .test`

# remove temporary files
test -f out && rm out
test -f out-ok && rm out-ok
test -f err && rm err
test -f err-ok && rm err-ok
tmp_all=`echo tmp*`
test "$tmp_all" = "tmp*" || rm $tmp_all

# programs used in test scripts.
DIFF="${DIFF-diff}"
DIFF_OPTS=

# the program to check
PROG=${PROG-${THIS_PROG}}

# return value
ret="0"

# Make empty temporary files.
touch out err out-ok err-ok

# Write out the name of this test.
echo -n "** $testname" >> 00check.log
# end of Init.sh
