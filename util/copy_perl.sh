#!/bin/ksh -e

list="$(mktemp /tmp/copy_prog.XXXXXXXX)"
perl copy_prog.pl /usr/bin/perl > "${list}"
tar -c -v -f - -I "${list}" | \
    xz -c > perl.tar.xz

rm "${list}"
