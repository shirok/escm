# Init.sh -*- sh -*- 
# $Id$
# Author: TAGA Yoshitaka <tagga@tsuda.ac.jp>

# Initialization

# the program name and the test name
progname=`echo "$0" | sed -e 's,^.*/,,'`
testname=`basename ${progname} .test`

# programs used in test scripts.
DIFF="${DIFF-diff}"
DIFF_OPTS=

# the program to check
PROG=${PROG-${THIS_PROG}}

# return value
ret="0"

# These files must have been removed by the previous call
# of finish.sh.
touch out err out-ok err-ok

# Write out the name of this test.
echo -n "** $testname" >> 00check.log
# end of Init.sh
