#!/usr/bin/perl

use strict;
use warnings;

use CGI;
use Data::Dumper;
use JSON;

sub parse_usernum {
    my ($num) = @_;

    my $pid = open(my $proc, "-|", "/htdocs/vmm/cgi/conv_scaled $num") or die("wtf");
    my $out = join "", <$proc>;
    chomp $out;

    return (length $out) ? $out + 0 : -1;
}

binmode STDOUT, ':utf8';
my $q = CGI->new;

print $q->header(-type => 'application/json', -charset => 'utf-8');

my %res = ();

my $vm_count = 0;
my @vminfo = ();

my $pid = open(my $proc, "-|", "doas /usr/sbin/vmctl status") or die("wtf");
while(my $line = <$proc>) {
    # Trim left side
    $line =~ s/^\s+//;

    my @sp = split(/\s+/, $line);
    if ($sp[0] eq "ID") {
        next
    }
    
    # Increase vm counter
    $vm_count++;

    # Grab info
    my %vm = (
        "id" => $sp[0] + 0,
        "name" => $sp[7],
        "pid" => $sp[1] eq "-" ? undef : ($sp[1] + 0),
        "running" => $sp[1] eq "-" ? JSON::false : JSON::true,
        "memory" => {
            "max" => parse_usernum($sp[3]),
            "used" => $sp[4] eq "-" ? 0 : parse_usernum($sp[4])
        },
        "owner" => {}
    );

    ($vm{"owner"}{"user"}, $vm{"owner"}{"group"}) = split(/:/, $sp[6]);

    # Append to vminfo array
    push(@vminfo, \%vm);
}

# Format res
$res{"vm_count"} = $vm_count;
$res{"vms"} = \@vminfo;

my $_json = JSON->new;
$_json->canonical(1);
print $_json->utf8->encode(\%res);
