#!/usr/bin/perl

use strict;
use warnings;

# Deduplicates list contents
# https://stackoverflow.com/a/7657
sub dedup {
    my %s;
    grep !$s{$_}++, @_;
}

sub get_libs {
    my ($prog) = @_;
    my @libs = ();

    if (! -f $prog) {
        printf STDERR "Couldn't find '$prog'";
        return \@libs;
    }

    # Push program itself into libs array
    push @libs, $prog;

    # Invoke ldd
    my $pid = open(my $proc, "-|", "/usr/bin/ldd $prog") or die("wtf");
    while(my $line = <$proc>) {
        # Trim left side
        $line =~ s/^\s+//;
        chomp $line;

        my @sp = split(/\s+/, $line);
        if(($sp[0] eq "$prog:") or ($sp[0] eq "Start")) {
            next;
        }

        my $lib = $sp[6];

        # Skip itself
        if($lib eq $prog) {
            next;
        }

        push @libs, $lib;
        
        # Get dependencies of dependency
        foreach my $l (get_libs($lib)) {
            push @libs, $l;
        }
    }

    return dedup(@libs);
}

my ($prog) = @ARGV;

if(!defined($prog)) {
    printf STDERR "Usage: $0 <path-to-program-or-library>\n";
    exit 1;
}

foreach my $l (get_libs($prog)) {
    print $l . "\n";
}
