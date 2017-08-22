#!/usr/bin/perl -ls

# Clement Poncelet 19/08/17 *****
# Perl script launcher for rttm's toy examples *****
use Cwd;

my $EXIT_SUCCESS = 0;
{
  chdir('translation'); #should avoid chdir ... but at least works ...

#example1:
  print "-- -- -- -- -- -- -- -- Example1: Uppaal2rt -- -- -- -- -- -- -- -- ";
    print `perl Uppaal2rt.pl 2>&1`;

#example2:
  print "-- -- -- -- -- -- -- - Example2: TinUppaal2rt - -- -- -- -- -- -- - ";
    print `perl TinUppaal2rt.pl 2>&1`;

  chdir('..');
}

print "GoodBye World";
exit($EXIT_SUCCESS);
