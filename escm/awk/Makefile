.PHONY : copyright lang doc
ESCM = aescm

# Generate copyright.h from AUTHORS and COPYING
copyright : ../src/copyright.h

# Update if an document has been added
doc : ../doc/Makefile.am

../src/copyright.h : ../AUTHORS ../COPYING
	awk -f copyright.awk > $@

../doc/Makefile.am : ../doc/*.escm
	mv $@ $@~
	awk -f adddoc.awk -v sources='$^' $@~ > $@

