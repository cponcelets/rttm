#!/usr/bin/perl -ls

# Clement Poncelet 19/08/17 *****
# Perl script compiles rttm and launches rttm's toy examples *****
use Cwd;

print "---  --- --- --- --- --- --- --- --- --- \n";
print "---  clean and compile maven project --- \n";
print "---  --- --- --- --- --- --- --- --- --- \n";

#verbose
print "---  --- --- --- clean --- --- --- --- \n";
print `mvn clean 2>&1`;
print "---  --- --- --- compile --- --- --- --- \n";
print `mvn compile 2>&1`;

print "---  --- --- --- --- --- --- --- --- --- \n";
print "---  ---   run rttm's examples   --- --- \n";
print "---  --- --- --- --- --- --- --- --- --- \n";

#Path info
chdir('examples'); #should avoid chdir ... but at least works ...
my $main_examples_script = "mainexample.pl";  #path to the main example script
#Test file presence
die "ERROR: main script $main_examples_script is missing" unless (-f "$main_examples_script");

#Run and test return
print `perl $main_examples_script`;
die "ERROR: on examples: $!" if($?);

print "---  --- --- --- --- --- --- --- --- --- \n";
print "---  ---         D O N E         --- --- \n";
print "---  --- --- --- --- --- --- --- --- --- \n";
