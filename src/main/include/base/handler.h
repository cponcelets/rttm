//
//  handler.h
//  timedtracechecker
//
//  Created by Clement Poncelet on 28/08/13.
//  Copyright (c) 2013 Clement Poncelet. All rights reserved.
//

#ifndef __timedtracechecker__handler__
#define __timedtracechecker__handler__

#include <iostream>
#include <assert.h>
#include <getopt.h> //command options
#include <stdarg.h> // for va_start

#include <translation/Ref_computation.h>
#include <translation/TempoCurve.h>

//new files
#include <base/trace/tablesymbol/Symbols.h>
#include <base/trace/parser/Trace_Parser.h>
#include <base/trace/printer/Trace_printer.h>

#include <base/trace/parser/Symbol_table_parser.h>
#include <verification/TimedConformanceChecker.h>

/**
  -- -- function declarations -- --
**/

//True main function
//perform paramuments and run features
int perform(int argc, char * argv[]);

//handle and check option consistency
bool check_and_complete_options();
//version printer
void version(ostream &out);
//error management
void error(const char *fmt, ...);
//usage
void usage(ostream &out);
//handle option with a flag
void set_flag_options(int option_index);

//free file management
void flush_files();

//main translation function
int run_translation();

//main verification function
int run_verification();

/**
  -- -- global declarations -- --
**/
//Automata format
enum o_mode {TRANSLAT, VERIF, NONE};

//common
static o_mode run_mode = NONE;
static const char* symboltable = NULL;

//Mode T
static const char* scorename = NULL;
static const char* uppaaltrace = NULL;

static const char* pathname = NULL;
static const char* input_file = NULL;

//Mode V
static const char* reftrace = NULL;
static const char* realtrace = NULL;

// --- Flags for options
static int f_verif = false;
static int f_tran = false;

static int f_rel = false;
static int f_kind = false;
static int f_tempo = false;

static int f_treco = false;

/** OPTION STRUCTURE    (getopt) **/
static struct option longoptions[] =
{
    // Options with an abbreviation

    /*  0 */  { "help",    no_argument, NULL,             'h' },
    /*  1 */  { "version", no_argument, NULL,             'v' },

    // Modes
    /*  2 */  { "V",        no_argument, &f_verif,        'V'},
    /*  3 */  { "T",        no_argument, &f_tran,         'T'},

    //common options
    /*  4 */  { "table",    required_argument, NULL,      't'},

    //Translation options
    /*  5 */  { "utrace",   required_argument, NULL,      'u'},
    /*  6 */  { "score",    required_argument, NULL,      's' },
    /*  7 */  { "rel",      no_argument, &f_rel,           0},
    /*  8 */  { "treco", no_argument, &f_treco,           'R'},

    /*  9 */  { "kind",     required_argument,  &f_kind,  'k'},
    /*  10 */  { "tin",     required_argument,   NULL,    'i'},

    /* 11 */ {"path",        required_argument,  NULL,    'p'},

    //Verification options
    /*  12 */  { "ref",      required_argument, NULL,     'r'},
    /*  13 */  { "totest",   required_argument, NULL,     'a'},
    /*  14 */  { "tempo",    no_argument, &f_tempo,       0},

    /* last */ { NULL, 0, NULL, 0}
};

//macro check index/name option's
#define check_index_is(NAME) \
assert(0 == strcmp(longoptions[option_index].name, NAME))

#endif /* defined(__timedtracechecker__handler__) */
