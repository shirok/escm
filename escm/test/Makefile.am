# test/Makefile.am
# $Id$
# Author: TAGA Yoshitaka <tagga@tsuda.ac.jp>

EXTRA_DIST = Init.sh Finish.sh Template.test $(TESTS)

TESTS_ENVIRONMENT = THIS_PROG="$(top_builddir)/src/$(PACKAGE)" ESCM_DEFAULT="$(top_srcdir)/src/lang/scm"

# Do not forget to add new tests.
TESTS = t010_nothing.test t020_something.test
TESTS += t030_write.test t040_display.test
TESTS += t050_wrong_name.test t060_wrong_subname.test
TESTS += t070_blocks.test t080_displays.test t090_alternate.test
TESTS += t100_span.test t110_string.test
TESTS += t120_awk.test t130_preset.test t140_prst_awk.test t150_comment.test 

CLEANFILES = input out err out-ok err-ok 00check.log

clean-local :
	-rm tmp*

# end of test/Makefile.am
