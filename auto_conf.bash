#!/bin/bash

cat > Makefile.am <<\
"--------------"
bin_PROGRAMS=wtrace
wtrace_SOURCES=wtrace.c
--------------

autoscan

sed -e 's/FULL-PACKAGE-NAME/wtrace/' \
    -e 's/VERSION/0.0.1/' \
    -e 's|BUG-REPORT-ADDRESS|s.takei.dev@gmail.com|' \
    -e '10i\
AM_INIT_AUTOMAKE\
' \
    < configure.scan > configure.ac

touch NEWS README AUTHORS ChangeLog

autoreconf -iv

exit

./configure
make distcheck
