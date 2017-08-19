//
//  TimedConformanceChecker.h
//  offline_trace_manager
//
//  Created by Clement Poncelet on 20/05/2015.
//  Copyright (c) 2015 Clement Poncelet. All rights reserved.
//

#ifndef __timedtracechecker__TimedConformanceChecker_h
#define __timedtracechecker__TimedConformanceChecker_h

#include <iomanip>
#include <math.h>

#include <base/trace/tablesymbol/Symbols.h>
#include <base/AuxiliaireFunction.h>

//to pretty print the verdict
#define PRECISIONFLOAT 7
#define LENGTHLABEL 10

//FROM PRE ANTESCOFO
//The fault tolerance during comparison (epsilon between same timestamp) [in seconds]
#define SEC_EPSILON 0.0001

/* ** ** ** ** ** **** ** ** ** ** **
 *  Timed conformance checker ** ** ** ** ** **
 * ** ** ** ** ** **** ** ** ** ** **
 *  Check two traces (a reference and a obtained trace).
 *  Is a trace conforms its reference (expected) trace.
 *  Note that ref is relative from the model and instantiated with tempi values (that explain the Symbol_phy)

 * The verifation is done in two phases Computation and Comparizon
 *
 * 1) Computation: transforms the relative ref trace in a physical one (instance tempi)
 * <tout_beat> -> <tout_phy>
 * Note: The tempo of the real trace is taken from antescofo
 * However the tempo values (if present in the input trace) can be put in the reference trace to check if Antescofo took the good values.
 * A verification of tempo values is so done during the computation.
 *
 *
 * 2) Comparizon: Does the conformance check
 * If <t_phy> !=_tolerance <t_phy'>
 * and print the Verdict.
 *
 *
 */
class TimedConformanceChecker
{
public:
  //Constructor / Destructor
    /**
      @param[in] ref, reference trace
      @param[in] real, actual output trace to test
      @param[in] tempo_managemnt, true if tempo values must be tested (ref/actual tempo must be identical)
    **/
    TimedConformanceChecker(Trace* ref, const Trace* real, bool tempo_managemnt):
        reference_trace(ref), real_trace(real), _check_tempo(tempo_managemnt) {}

  //Public methods
    /** Check the trace
      @param[out] true if the actual trace conforms its reference trace
                  false otherwise
    **/
    bool check();

private:
  //Private methods
    /** 1) Compute the reference trace
      @param[out]: can return false if related tempo is not equal to the ref tempo
     **/
    bool compute();
    /** 2) Compare the two traces
      @param[out] true if the actual trace conforms its reference trace
                  false otherwise
     **/
    bool is_conform();

  //Private fields
    Trace* reference_trace;   //the reference
    const Trace* real_trace;  //the actual trace
    bool _check_tempo;        //should we check tempo values ?
};

/* ** ** ** ** ** **** ** ** ** ** **
 *  Rel to phy visitor ** ** ** ** ** **
 * ** ** ** ** ** **** ** ** ** ** **
 ** 1)
 * We use a visitor that can modify the visited Symbol (-> modifior)
 * Since events are linked to its real symbol, just the ref trace is needed (the modified Trace)
 **/
class Rel_To_Phy_Visitor : public TModifior
{
public:

  //Constructor / Destructor
    /** the trace is visited **/
    Rel_To_Phy_Visitor():
        _error(false), _last_event(nullptr) {}

    virtual ~Rel_To_Phy_Visitor(){}

    //visitor (unused)
    void visit(Test_suite*) override {assert(false);}
    void visit(Trace*) override      {assert(false);}
    void visit(Action_rel*) override {assert(false);}
    void visit(Event_rel*) override  {assert(false);}
    //visitor (ref symbols should be absolute)
    void visit(Event_phy*) override;
    void visit(Action_phy*) override;

  //Getters
    //true if the tempo is not eq. to the ref trace
    bool is_wrong_tempo() const {return _error;}

private:
    bool _error;            //manage wrong tempi
    Event_phy* _last_event; //save the last event to manage
};

/* ** ** ** ** ** **** ** ** ** ** **
 *  Verdict_printer ** ** ** ** ** **
 * ** ** ** ** ** **** ** ** ** ** **
 * 2) (printer is merged with comparison)
 * We use a visitor to compare the reference and the real traces
 * Since Events are related to its reference symbol, just the ref trace is needed (the visited Trace)
 *
 * Verdict format:
 *   Real _ Trace                           Ref trace
    -- symbols
 * <l, t_phy, t_rel>                        <l, t_phy, t_rel>
 * (with t_rel : score time (rnow))         (with computed t_phy and ref value for t_rel)
 *
 *
    -- Step of time -- (note T is eq) [time elapse]
 * + <t_rel, T>                             + <t_rel, T>
 */
class Verdict_printer : public TVisitor
{
public:
  //Constructor / Destructor
    /**
     @param[in] ref, the reference trace
     **/
    Verdict_printer(const Trace* ref);
    ~Verdict_printer(){}

    //visitor (unused)
    void visit(const Test_suite*) override  {assert(false);}
    void visit(const Trace*) override       {assert(false);}
    void visit(const Action_rel*) override  {assert(false);}
    void visit(const Event_rel*) override   {assert(false);}
    //visitor (used)
    void visit(const Symbol_phy*);
    void visit(const Event_phy* e) override   {visit((Symbol_phy*) e);}
    void visit(const Action_phy* a) override  {visit((Symbol_phy*) a);}

  //Getters
    //true if real trace conforms its reference trace
    bool is_conformed() {return _conform;}

  //Public methods [TODO:TOCHECK]
    /** Indicate to the verdict that the real trace is ended check if it keep ref symbols (missing symbols)
     *
     @param[in] last_label, label of the last event
     **/
    void terminate(string last_label);

private:
  //Private methods
    /** A function that compare two doubles (using the fault tolerance)
     *  if a-b  :    <  - epsilon : -1 ( a lower than b)
     *               >    epsilon : 1  ( a greater than b)
     *              otherwize 0 equal  ( -epsilon < a-b < epsilon )
      @param[in] a, first double
      @param[in] b, second double
      @param[out]   1   if a>b
                    -1  if a<b
                    0   if a=b
     **/
    int epsilon_compare_double(double a, double b) const;

    /** Check if the reference has missed symbol.
     *  Recall that we put error symbol if we couldn't link to real symbol (cause of ixexistance)
      @param[out] true if some symbols had been missed
     **/
    bool is_missing_symbols();

    /** Check if the real symbol has an error related symbol
     *  Same as missing symbol but for an unexpected output symbol
      @param[in] s, the real symbol
      @param[out] true if the symbol is unexepected
     **/
    bool is_unexepected(const Symbol_phy* s) const;

  //*** Private methods: Print Verdict *** [TODO:->printer]
    //(All methods use the cout (standard output) flow)

    /** Print a missing symbol (expected in the ref but not present in the real trace)
      @param[in] ref, the reference symbol
      @param[in] real, its missed related and actual symbol
     **/
    void print_missing_symbol(const Symbol_phy* ref, const Symbol_phy* real) const;

    /** Print a step of time (time flowing) and precise the delay / or duration lasted
      @param[in] ref, current ref symbol which begins the next logical instant
      @param[in] real, current real symbol which begins the next logical instant
     **/
    void print_flow_time(const Symbol_phy* ref, const Symbol_phy* real) const;

    /** Print an unexpected symbol (present in the real trace but not in the ref)
      @param[in] real, the unexpected symbol
     **/
    void print_unexepected(const Symbol_phy*) const;

    /** Print a symbol and its reference in a faulty case
     *  Note:: We distinct event and action
     @param[in] ref, the reference symbol
     @param[in] real, its erroneous related and actual symbol
     **/
    void print_timed_error(const Symbol_phy*, const Symbol_phy*) const;
    void print_timed_error(const Event_phy*, const Event_phy*) const;
    void print_timed_error(const Action_phy*, const Action_phy*) const;

    /** Print a symbol and its reference in a conform case
     *  Note:: We distinct event and action
     @param[in] ref, the reference symbol
     @param[in] real, its related and actual symbol
     **/
    void print_conform(const Symbol_phy* rel, const Symbol_phy* real) const;
    void print_conform(const Event_phy*, const Event_phy*) const;
    void print_conform(const Action_phy*, const Action_phy*) const;

  //*** Private methods: Print Verdict *** [TODO:->printer]

  //Private fields
    const Trace* _refTrace;    //The reference trace used to check missed symbols
    //current iterator to avoid the entire parse each time
    vector<Symbol_rel*>::const_iterator _it_ref;
    //we store the last symbol to acess to the last timestamp (ref and real traces)
    const Symbol_phy* _previous_ref_symbol;
    const Symbol_phy* _previous_real_symbol;
    const Event_phy* _last_event; //we store the last event too

    double _end_timestamp;        //the last timestamp
    bool _test_finished;          //is the test finished ? (time >= _end_timestamp)
    bool _conform;                //is the real trace conform ?
};

#endif
