#!/usr/bin/perl -ls

# Clement Poncelet 19/08/17 *****
# Perl script launcher for rttm's toy examples *****
#
# Example1: From Uppaal trace into a suite of relative test cases (input + reference traces)
# -- Uppaal traces are output from the model-checker Uppaal after a covering request in order to test exhaustively a system against a model ---
# --- The Uppaal trace contains in general a suite of test cases (from an initial to a terminal model state)
# --- Time is relative (in model time unit (mtu) or tick) and are encoded with integers (expressed with d, a duration in a number of ticks)
# ~~~~ ~~~~ ~~~~~~~~~~~~ ~~~~ ~~~~~~~~ ~~~~~~~~
#
# --- The input trace is into a generalized format of relative trace, expressed with a suite of triples <s, t, p>:
#     - s, the symbol label (event or action)
#     - t, its timestamp in relative time (or score time) into beat
#     - p, the related pace (or tempo)
#
# Note: Inspired by our music context, our time is timestamped in relative dates (beats) [dated from the start] and translated into physical time thanks to a tempo. For example, a simple trace can have the form:
# <e0, 0, 60> ° <e1, 0.5, 60> ° <e2, 1, 60>:
#         - first event at the beggining of the score 0beat with a tempo of 60 beat-per-minute (BPM)
#         - second event at time 0.5beat (a quarter after) with the same tempo, thus at 0.5 * 60bpm / 60. -> at the date 0.5seconds + 0(date e0)
#         - the third one at 1beat after the beginning (a half) or again a quarter after e1, thus at 0.5 * 60bpm / 60. -> 0.5seconds + 0.5(date e1) = 1s
# Which is not surprising since 60bpm is 1 beat per second.
#
####
# -- The only difference between input and reference traces is that reference traces have expected outputs with events --
####

use Cwd;
#print STDERR "~~  From: " . cwd() . " ~~\n";

my $EXIT_SUCCESS = 0;
## !! !! Set the executable path (its my default nar-maven bin emplacement)
my $rttm = "../../target/nar/rttm-1.0-SNAPSHOT-x86_64-MacOSX-gpp-executable/bin/x86_64-MacOSX-gpp/rttm";
## !! !!

# ARGS *
my $U_trace = "../ressources/03_1.tr";                    #input (uppaal trace)
my $symb_table = "../ressources/03_1.symbol_table";       #table symbol
my $score = "../ressources/03_1.txt";                     #score
my $rt_trace = "../ressources/produced/03_1";             #output (suite of input trace form) [03_1.i.in]

# CLEAN *
$old_inputs = `ls $rt_trace.*.in 2>/dev/null`; #(warnings into trash when no file)
$old_refs = `ls $rt_trace.*.ref 2>/dev/null`; #(warnings into trash when no file)
if($old_inputs)
{
  #in
  my $clean_cmd = "rm " . join(" ", split(/\s+/, $old_inputs));
  print $clean_cmd . "\n";
  `$clean_cmd`;
  #ref
  $clean_cmd = "rm " . join(" ", split(/\s+/, $old_refs));
  print $clean_cmd . "\n";
  `$clean_cmd`;
}
# CLEAN *

# TRANSLATION *
my $trans_cmd = "$rttm -T -t $symb_table --utrace $U_trace --treco -s $score -p $rt_trace";
print "* " . $trans_cmd  . "\n";
`$trans_cmd`;
# TRANSLATION *

die "ERROR: rttm error during translation [Uppaal2rt]: $!" if($?);
exit($EXIT_SUCCESS);
