//
//  Trace_Parser.h
//  timedtracechecker
//
//  Created by Clement Poncelet on 03/05/2015.
//  Copyright (c) 2015 Clement Poncelet. All rights reserved.
//

#ifndef __timedtracechecker__Trace_Parser_h__
#define __timedtracechecker__Trace_Parser_h__

#include <assert.h>
#include <string>
#include <fstream>
#include <vector>
#include <map>

#include <base/AuxiliaireFunction.h>
#include <base/trace/tablesymbol/Pitch.h>
#include <base/trace/tablesymbol/Symbols.h>

using namespace std;

/* ** ** ** ** ** **** ** ** ** ** **
 *  Uppaal_Suite_parser ** ** ** ** ** **
 * ** ** ** ** ** **** ** ** ** ** **
 *
 *  A parser of uppaal's suites
 *  It parses a coVer Test suite (<filename>.tr) and creates a test suite
 *  Use visitors to read informations
 *
 *  Uppaal trace is composed of suites with delays and I/O synchronization
 *  Since it is  not managing float numbers a convertion is done with CONVERSION
 **/
#define CONVERTION 100000

/**
 * It returns a suite of symbols (relativly timedstamped from the start (0.0))
 * <s, t> : model labels, beats timestamp (shifted from the score by uppaal)
 **/

class UpppalSuiteParser
{
public:
  //Constructor / Destructor
  /**
    @param[in] fic, filename containing the uppaal trace (.tr)
    @param[in] t, table symbol
  **/
    UpppalSuiteParser(string fic, map <string, Symbol_phy*>* t);

    ~UpppalSuiteParser();

  //Getters
    //get the number of trace (test cases)
    uint numberTrace() {return trace_compter;}
    //true if the end of file is read
    bool is_eof() const {return _Uppaal_trace_file.eof();}

  //public Methods
    //parse the file and the suite of test cases
    //@param[in] phy_symbol, true if the symbol are physical
    Trace* parseATrace(bool phy_symbol);

private:
  //private Methods
    //parse a transition
    void parse_transition();

  //private Fields
    string file;                    //filename (.tr)
    ifstream _Uppaal_trace_file;    //related input stream
    Trace* test;                    //data sturcture (test suite)

    uint trace_compter;             //trace counter
    double _beat_cumulator;         //beat Accumulator to save the time spent betw. symbols (Relative-time)
    bool _phy_traces;               //are the traces absolute ?

    map <string, Symbol_phy*>* symbol_table; //Used to link related symbols (error [needed])
    Event_phy* _last_event;         //the last event
};

/* ** ** ** ** ** **** ** ** ** ** **
 *  Antescofo output parser ** ** ** ** ** **
 * ** ** ** ** ** **** ** ** ** ** **
 *  Antescofo-Output Parser (real trace t'out)
 *  It parses an Anstecofo's output (<filename>-Output.aaa.out) and create an AST test.
 *
 * Antescofo:
 *  -- symbols with  NOW (physical timestamp), RNOW score position (beat), tempo detected
 *  trace format:
        <s, t, T> : s is the Antescofo (score) label, t the physical timestamp [effectif], T the tempo (for an event).
 **/
class AntescofoOutputParser {
public:
  //Constructor / Destructor
  /**
    @param[in] fic, filename containing the output trace (.out)
  **/
    AntescofoOutputParser(string fic);

    ~AntescofoOutputParser(){}

  //public Methods
    //parse the output file
    Trace* parse();

private:
    std::string file;       //file name (<>-Output.aaa.out)
    Trace* real_trace;      //AST
    Event_phy* _last_event; //the last event
};


/* ** ** ** ** ** **** ** ** ** ** **
 *  Reference trace parser ** ** ** ** ** **
 * ** ** ** ** ** **** ** ** ** ** **
 *  Reference trace Parser (tout)
 *  It parses a reference (<filename>-score.aaa.ref) and create an AST test.
 *
 *  Symbol in a reference trace are relative (without tempo) and the relative timestamps (beat) are dated from the trace start.
 *
 * trace format:
 *  <s, t> : s the Model label, t the relative timestamp
 **/
class ReferenceTraceParser
{
public:
  //Constructor / Destructor
    /**
    @param[in] fic, filename containing the output trace (.ref)
    **/
    ReferenceTraceParser(string fic);

    ~ReferenceTraceParser(){}

  //public Methods
    //parse the output file
    Trace* parse();

private:
    std::string file;           //file name (<>-Output.aaa.ref)
    Trace*     ref_trace;       //AST

    Event_phy* _last_event;     //> the last event
    double _rel_cursor;         //> relative cursor
};

/* ** ** ** ** ** **** ** ** ** ** **
 *  Antescofo score parser ** ** ** ** ** **
 * ** ** ** ** ** **** ** ** ** ** **
 *
 *  This class is the Antescofo DSL mixed score parser
 *  It parses a Score to get the ideal trace (used for coVer translater)
 *
 *  We want event durations, tempo and pitches.
 **/
class AntescofoScoreParser {
public:
  //Constructor / Destructor
    /**
    @param[in] fic, filename containing the output trace (.ref)
    **/
    AntescofoScoreParser(string fic);

  //Setters
    /** add pitches to the table symbols
    @param[in] map, symbol table
    **/
    void add_pitches(map <string, Symbol_phy*>*);

  //Getters
    //get the initial tempo of the score
    double get_score_tempo() const
      {assert(_score_tempo > -1);return _score_tempo;}

private:
    //Convert string n/d in double
    double string2double(string fraction) const;

    //modify the table with relative positions and pitch infos
    void addData(map <string, Symbol_phy*>*, string, double, Pitch* p = NULL);

    std::string file;       //file name (.txt .antescofo .antesc .ante)
    double _score_tempo;    //initial tempo
};

#endif
