# Finish.sh -*- sh -*-
# $Id$
# Author: TAGA Yoshitaka <tagga@tsuda.ac.jp>

# Check stdout
if ${DIFF} ${DIFF_OPTS} out out-ok > /dev/null; then
    : # nop
else
    echo "" >> 00check.log
    echo ">> stdout" >> 00check.log
    ${DIFF} ${DIFF_OPTS} out out-ok >> 00check.log
    cat out >> 00check.log
    ret="1"
fi

# Check stderr
if ${DIFF} ${DIFF_OPTS} err err-ok > /dev/null; then
    : # nop
else
    if test $ret = "0"; then
	echo "" >> 00check.log
    fi
    echo ">> stderr" >> 00check.log
    ${DIFF} ${DIFF_OPTS} err err-ok >> 00check.log
    cat err >> 00check.log
    ret="1"
fi

# Finalization
if test $ret = 0; then
   echo " -- OK" >> 00check.log
fi
exit $ret
# end of Finish.sh
