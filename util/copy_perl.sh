#!/bin/ksh -e

perl copy_prog.pl /usr/bin/perl | \
    tar -c -v -f - -I - | \
    xz -c > perl.tar.xz
