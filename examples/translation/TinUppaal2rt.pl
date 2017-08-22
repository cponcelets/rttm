#!/usr/bin/perl -ls

# Clement Poncelet 22/08/17 *****
# Perl script launcher for rttm's toy examples *****
#
# Example2: We translate (as in the example1) an Uppaal trace into a suite of test cases
## [recall]
# --  Uppaal traces are output from the model-checker Uppaal after a covering request in order to test exhaustively a system against a model ---
# --- The Uppaal trace contains in general a suite of test cases (from an initial to a terminal model state)
# --- Time is relative (in model time unit (mtu) or tick) and are encoded with integers (expressed with d, a duration in a number of ticks)
## [recall]
# ~~~~ ~~~~ ~~~~~~~~~~~~ ~~~~ ~~~~~~~~ ~~~~~~~~
# However, we donnot use some table symbols (i.e score and model traces (Uppaal here) should have the same symbols)
# Moreover, we want to produce performance traces (and delete this flag treco [see the rttm command run for the example1: variable $trans_cmd]).
#
# ~~~~ ~~~~ ~~~~~~~~~~~~ ~~~~ ~~~~~~~~ ~~~~~~~~ two input trace types:
# Relation between performance and recognition traces.
## [recall]
# --- The input trace is into a generalized format of relative trace, expressed with a suite of triples <s, t, p>:
#     - s, the symbol label (event or action)
#     - t, its timestamp in relative time (or score time) into beat
#     - p, the related pace (or tempo)
#
# Note: Inspired by our music context, our time is timestamped in relative dates (beats) [dated from the start] and translated into physical time thanks to a tempo. For example, a simple trace can have the form:
# <e0, 0, 60> ° <e1, 0.5, 60> ° <e2, 1, 60>:
#         - first event at the beginning of the score 0beat with a tempo of 60 beat-per-minute (BPM)
#         - second event at time 0.5beat (a quarter after) with the same tempo, thus at 0.5 * 60bpm / 60. -> at the date 0.5seconds + 0(date e0)
#         - the third one at 1beat after the beginning (a half) or again a quarter after e1, thus at 0.5 * 60bpm / 60. -> 0.5seconds + 0.5(date e1) = 1s
# Which is not surprising since 60bpm is 1 beat per second.
## [recall]
#
# It isn't so easy ... this example assumes (implicitly) that pace values (the third field p) are interpreted as:
# "the tempo attached to the note to compute its 'physical' (in seconds) duration", that is a definition of a performance (in a musical sense) found in another formats like MIDI.
#
# The fact is that in general, Interactive Music Systems (IMS), when involved in live performances, have a specific module to mimic and follow interacting humans users (musicians or what ever) - the recognition (in our case called listening machine).
# The input traces representing the timed input sequence is not directly the musician performance but the detected (and often optimized) suite of input symbols - we called this trace a recognition trace (from here comes the -treco flag!).
#
# The manager handles these two input types (i guess at least for translation) that is the type of input trace printed in the output (.in files).
####

use Cwd;
#print STDERR "~~  From: " . cwd() . " ~~\n";

my $EXIT_SUCCESS = 0;
## !! !! Set the executable path (its my default nar-maven bin emplacement)
my $rttm = "../../target/nar/rttm-1.0-SNAPSHOT-x86_64-MacOSX-gpp-executable/bin/x86_64-MacOSX-gpp/rttm";
## !! !!

# ARGS *
my $U_trace = "../ressources/03_1.tr";                    #input (uppaal trace)
my $score = "../ressources/03_1.txt";                     #score
my $rt_trace = "../ressources/produced/ex2/03_1";         #output (suite of input trace form) [03_1.i.in]

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
my $trans_cmd = "$rttm -T --utrace $U_trace -s $score -p $rt_trace";
print "* " . $trans_cmd  . "\n";
`$trans_cmd`;
# TRANSLATION *

die "ERROR: rttm error during translation [TinUppaal2rt]: $!" if($?);
exit($EXIT_SUCCESS);
