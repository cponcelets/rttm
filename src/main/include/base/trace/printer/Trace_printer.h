//
//  Trace_printer.h
//  timedtracechecker
//
//  Created by Clement Poncelet on 03/05/2015.
//  Copyright (c) 2015 Clement Poncelet. All rights reserved.
//

#ifndef __timedtracechecker__Trace_printer_h__
#define __timedtracechecker__Trace_printer_h__

#include <fstream>
#include <map>

#include <base/trace/tablesymbol/Symbols.h>

/* ** ** ** ** ** **** ** ** ** ** **
 *  Show   ** ** ** ** ** **
 * ** ** ** ** ** **** ** ** ** ** **
 * Print test score with events and actions (debug)
 **/
class Show : public TVisitor {
public:
  //Constructor / Destructor
    /**
      @param[in] oss, output stream
    **/
    Show(std::ostream& oss) : _out(oss), _test_cpt(0) {}

    void visit(const Test_suite*) override;
    void visit(const Trace*) override;
    void visit(const Action_rel*) override;
    void visit(const Event_rel*) override;
    void visit(const Event_phy*) override;
    void visit(const Action_phy*) override;

private:
    std::ostream& _out; //output stream
    int _test_cpt;      //test counter
};

/* ** ** ** ** ** **** ** ** ** ** **
 *  Print Tin   ** ** ** ** ** **
 * ** ** ** ** ** **** ** ** ** ** **
 * print input timed traces, format:
    <i, t [, T]> -->    i: Identifier (cuenum)
                        t: timestamp (t_rel in relative time)
                        p: Tempo (BPM), the tempo used to play the event_i (i.e those Antescofo estimates after the duration of event_i, i.e the NEXT event)
 + datas for tests:
                        label: the Antescofo label (for readability)
                        pitches: to translate a tin into MIDI and play it
 ********************************************************************************/
class Print_tin : public TVisitor
{
public:
  //Constructor / Destructor
    /** Generate a file per test case named : basename.<test_cpt>.in
      @param[in] f: string, the name of the file testsuite
      @param[in] number: uint, test case number
    **/
    Print_tin(std::string f, uint number): _out(std::cout), _pathbase(f), _trace_number(number) {}

    ~Print_tin(){}

    void visit(const Test_suite*){assert(false);}
    void visit(const Trace*);
    void visit(const Action_rel*){} //skip actions
    void visit(const Event_rel*);
    void visit(const Event_phy*);
    void visit(const Action_phy*){}

private: /** Fields **/

    std::ostream& _out;       // The output stream
    std::string _pathbase;    // The base path to the input traceS
    std::streambuf* _backup;  // The backup (old-bad method)

    unsigned int _trace_number;    // id number of tin
};


/* ** ** ** ** ** **** ** ** ** ** **
 *  Print tref   ** ** ** ** ** **
 * ** ** ** ** ** **** ** ** ** ** **
 *  Print reference trace (print input/output timed traces [the expected ones])
 <i, t [, T]> -->       i: Identifier (model's label)
                        t: timestamp (t_PHY in physical time)
                        p: Tempo (BPM), the tempo used to play the event_i (i.e those Antescofo estimates after the duration of event_i, i.e the NEXT event)

 (to detect an event easier EVENT keyword is inserted before an event)
  ********************************************************************************/
class Print_tref : public TVisitor
{
public:
  //Constructor / Destructor
    /** It will generate a file per test case named : basename.<test_cpt>.ref
      @param[in] f, the name of the file testsuite
      @param[in] number: uint, to force a certain number of tref trace
      @param[in] sting: e, The last event (used in comparison?) put to terminate the reference trace

     The detected tempo is needed to compute the good output trace from a musician input trace.
     Antescofo delays the tempo information or not (this fact appears because we can bypass the listening machine and so «mimic» it).
     We add a bool to manage or not the recognition delay.
     **/
    Print_tref(string f, uint number, string e):
        _out(std::cout), _pathbase(f), _trace_number(number), _last_event(e)
    {}

    ~Print_tref(){}

    void visit(const Test_suite*){assert(false);}
    void visit(const Trace*);
    void visit(const Action_rel*);
    void visit(const Event_rel*);
    void visit(const Event_phy*);
    void visit(const Action_phy*);

private: /** Fields **/

    std::ostream& _out;       // The output stream
    std::string _pathbase;    // The base path to the reference traceS
    std::streambuf* _backup;  // The backup (old-bad method)

    unsigned int _trace_number;     // counter of tref
    string _last_event;             // The last visited event
};

/* ** ** ** ** ** **** ** ** ** ** **
 *  Print tmidi   ** ** ** ** ** **
 * ** ** ** ** ** **** ** ** ** ** **
 * [TODO]
 * This printer translates a timed trace (absolute) into a midi midi.
 * Using the lib. midifile.
 *
 **/
#endif
