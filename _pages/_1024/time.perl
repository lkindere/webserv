#!/usr/bin/perl -wT
use strict;

print "Content-Type: text/html\n\n";

my $now = localtime;
print "The time is $now";