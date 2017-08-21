#!/usr/bin/perl -ls

# Clement Poncelet 19/08/17 *****
# Perl script launcher for rttm's toy examples *****
use Cwd;

my $EXIT_SUCCESS = 0;
print "-- -- -- -- -- -- -- -- Example1: Uppaal2rt -- -- -- -- -- -- -- -- ";
{
  chdir('translation'); #should avoid chdir ... but at least works ...
    print `perl Uppaal2rt.pl 2>&1`;
  chdir('..');
}

print "GoodBye World";
exit($EXIT_SUCCESS);
