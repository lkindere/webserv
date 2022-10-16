#!/usr/bin/perl -wT
use strict;

print "Content-type: text/html\n\n";
my $now = localtime;
print "The time is $now";