//
//  Symbols.h
//  timedtracechecker
//
//  Created by Clement Poncelet on 30/04/2015.
//  Copyright (c) 2015 Clement Poncelet. All rights reserved.
//

#ifndef __timedtracechecker__Symbols_h__
#define __timedtracechecker__Symbols_h__

#include <list>
#include <string>
#include <cassert>

#include <base/trace/tablesymbol/Pitch.h>

/*  Symbol class defines symbol items used during the Translation / Verification
 *  From traces, events are timestamped (onsets) in relative (input & ref traces) or physical time unit (resp. in beat / s)
 *
 *  A symbol is an event or an action, in relative or physical time unit.
 *  It contains:
        = its label,
        = timestamp,
        = computed duration (absolutes symbols only) and
        = tempo (event only).
 *
 *  A performance, a trace or a test case, is a sequence of symbols.
 *  A Tsuite is a suite of test cases for Uppaal (i.e a suite of performances).

 ** General notes **
       ** Relative time unit **
 [still??]
 Warning : An automaton duration unit is *100 more then a score one [no float in Uppaal]
*/

using namespace std;

/* Pre-definitions for Visitor */
class Symbol_rel;
class Event_rel;
class Action_rel;
class Symbol_phy;
class Event_phy;
class Action_phy;
class Trace;
class Test_suite;

//      -- Our Test visitors --
//      To parse our test trees
class TVisitor {
public:
    virtual void visit(const Test_suite*) = 0;
    virtual void visit(const Trace*) = 0;
    virtual void visit(const Action_rel*) = 0;
    virtual void visit(const Event_rel*) = 0;
    virtual void visit(const Action_phy*) = 0;
    virtual void visit(const Event_phy*) = 0;
};

//      -- Our Test modifiors --
//      To parse AND modify our test trees
class TModifior {
public:
    virtual void visit(Test_suite*) = 0;
    virtual void visit(Trace*) = 0;
    virtual void visit(Action_rel*) = 0;
    virtual void visit(Event_rel*) = 0;
    virtual void visit(Action_phy*) = 0;
    virtual void visit(Event_phy*) = 0;
};

/* ** ** ** ** ** **** ** ** ** ** **
 *  Relative Symbol ** ** ** ** ** **
 * ** ** ** ** ** **** ** ** ** ** **
 *
 * A symbol has:
 * -> is optional
 */
struct data_symbol_rel
{
    double trel;            //a relative timestamp
    double drel;          //-> a relative duration
    string label;           //a label
    Symbol_rel* related_s;  //its related symbol (set with table symbol)
    Pitch* pitches;       //-> (a) pitch(es)
};

class Symbol_rel
{
public:
  //Constructor / Destructor
  /**
    @param[in] l, label
    @param[in] tr, relative timestamp
  **/
    Symbol_rel(string l, double tr)
    {_data.label = l; _data.trel = tr; _data.drel = 0; _data.related_s = NULL;_data.pitches = NULL;}

    virtual ~Symbol_rel(){}

  //Getters
    //get data
    data_symbol_rel data_rel() const {return _data;}
    //true if the symbol is an event
    virtual bool is_event() const =0;

  //Visitor/Modifior
    virtual void accept(TVisitor* v) const =0;
    virtual void modify(TModifior* v) =0;

  //Setters
    //set piches
    void link_pitches(Pitch* p) {_data.pitches = p;}
    //set related symbol
    void link_related(Symbol_rel* s) {_data.related_s = s;}
    //set relative timestamp
    void set_relTimestamp(double timestamp) {_data.trel = timestamp;}
    //set relative duration
    void set_relDuration(double d) {_data.drel = d;}

private:
    data_symbol_rel _data;
};

 /* ** ** ** ** ** **** ** ** ** ** **
  *  Relative Event ** ** ** ** ** **
  * ** ** ** ** ** **** ** ** ** ** **
  *
  * data: + cuenum if present
  */
class Event_rel : public Symbol_rel
{
public:
  //Constructor / Destructor
    /**
      @param[in] l, label
      @param[in] tr, relative timestamp
      @param[in] cnum, cue num (related to antescofo's score)
    **/
    Event_rel(string l, double tr, int cnum = -1) : Symbol_rel(l, tr), _cuenum(cnum) {}
    /**
      @param[in] l, label
      @param[in] tr, relative timestamp
      @param[in] pitches
    **/
    Event_rel(string l, double tr, Pitch* pitches) : Event_rel(l, tr) {link_pitches(pitches);}

    ~Event_rel(){}

  //Getters
    //true if the symbol is an event
    bool is_event() const override {return true;}
    //get cuenum
    int cuenum() const {return _cuenum;}

  //Visitor/Modifior
    void accept(TVisitor* v) const override {v->visit(this);}
    void modify(TModifior* m) override {m->visit(this);}

private:
    int _cuenum;
};

/* ** ** ** ** ** **** ** ** ** ** **
 *  Relative Action ** ** ** ** ** **
 * ** ** ** ** ** **** ** ** ** ** **
 */
class Action_rel : public Symbol_rel
{
public:
  //Constructor / Destructor
    /**
      @param[in] l, label
      @param[in] tr, relative timestamp
    **/
    Action_rel(string l, double tr) : Symbol_rel(l, tr) {}

    ~Action_rel(){}

  //Getters
    //true if the symbol is an event
    bool is_event() const override {return false;}

//Visitor/Modifior
    void accept(TVisitor* v) const override {v->visit(this);}
    void modify(TModifior* m) override {m->visit(this);}
};

/* ** ** ** ** ** **** ** ** ** ** **
 *  Physical Symbol ** ** ** ** ** **
 * ** ** ** ** ** **** ** ** ** ** **
 *
 * A physical symbol has:
      = an absolute timestamp,
      = an absolute duration
      = a flag to manage timed error
!! * We precise that dphy is:
      - the event's duration (between e end e+1)
!! *  - but action's delay (so between a-1 (or e) and a)
 */
struct data_symbol_phy
{
    double tphy;
    double dphy;
    bool error;
};

class Symbol_phy : public Symbol_rel
{
public:
  //Constructor / Destructor
    /**
      @param[in] l, label
      @param[in] tr, relative timestamp
      @param[in] tp, absolute timestamp
    **/
    Symbol_phy(string l, double tr, double tp) : Symbol_rel(l, tr)
      {_data.tphy = tp; _data.dphy = 0.0; _data.error = false;}

    virtual ~Symbol_phy(){}

  //Getters
    //get data
    const data_symbol_phy& data_phy() const {return _data;}
    //true if the symbol is wrong (compared to the reference trace)
    bool is_erroneous() const {return _data.error;}

  //Visitor/Modifior
    virtual void accept(TVisitor* v) const override =0;
    virtual void modify(TModifior* m) override =0;

  //Setters
    //set absolute timestamp
    void set_timestamp(double t) {_data.tphy = t;}
    //set absolute duration
    void set_phyDuration(double d) {_data.dphy = d;}
    //set error flag
    void set_error(const bool b) {_data.error = b;}

private:
    data_symbol_phy _data;
};

/* ** ** ** ** ** **** ** ** ** ** **
 *  Physical Event ** ** ** ** ** **
 * ** ** ** ** ** **** ** ** ** ** **
 *
 * data: + next event
 *       + cuenum
 *       + tempo
 */
class Event_phy : public Symbol_phy
{
public:
  //Constructor / Destructor
    /**
      @param[in] l, label
      @param[in] tr, relative timestamp
      @param[in] tp, absolute timestamp
      @param[in] T, tempo
      @param[in] cnum, cuenum
    **/
    Event_phy(string l, double tr, double tp, double T, long cnum = -1):
        Symbol_phy(l, tr, tp), _tempo(T), _cuenum(cnum), _next(nullptr) {}

    ~Event_phy(){}

  //Getters
    //get tempo
    double tempo() const {return _tempo;}
    //get cuenum
    long cuenum() const {return _cuenum;}
    //get next event
    Event_phy* get_next() const {return _next;}
    //true if the symbol is an event
    bool is_event() const override {return true;}

  //Visitor/Modifior
    void accept(TVisitor* v) const override {v->visit(this);}
    void modify(TModifior* m) override {m->visit(this);}

  //Setters
    //set tempo
    void set_tempo(double t) {_tempo = t;}
    //set next symbol
    void set_next(Event_phy& s) {_next = &s;}

private:
    double _tempo;    //tempo from this event
    long _cuenum;     //event's num (in antescofo score)
    Event_phy* _next; //pointer to its next event
};

 /* ** ** ** ** ** **** ** ** ** ** **
  *  Physical Action ** ** ** ** ** **
  * ** ** ** ** ** **** ** ** ** ** **
  */
class Action_phy : public Symbol_phy
{
public:
  //Constructor / Destructor
    /**
      @param[in] l, label
      @param[in] tr, relative timestamp
      @param[in] tp, absolute timestamp
    **/
    Action_phy(string l, double tr, double tp) : Symbol_phy(l, tr, tp) {}

    ~Action_phy(){}

  //Getters
    //true if the symbol is an event
    bool is_event() const override {return false;}

  //Visitor/Modifior
    void accept(TVisitor* v) const override {v->visit(this);}
    void modify(TModifior* m) override {m->visit(this);}
};

 /* ** ** ** ** ** **** ** ** ** ** **
  *  Trace (Performance) ** ** ** ** ** **
  * ** ** ** ** ** **** ** ** ** ** **
  *  A timed trace suite of timed symbols
  */
class Trace
{
public:
  //Constructor / Destructor
    Trace() : _sequence(), _is_relative(false) {}

    ~Trace()
    {   for(uint i = 0; i < _sequence.size(); i++) delete _sequence.at(i);
        _sequence.clear();
    }

  //Getters
    //get symbols' sequence (const)
    const vector<Symbol_rel*>* sequence() const {return &_sequence;}
    //get symbols' sequence
    vector<Symbol_rel*>* get_trace() {return &_sequence;}
    //get last event
    const vector<Symbol_rel*>::const_iterator get_last_evt() const;
    //get initial tempo
    double get_init_tempo() const {return _init_tempo;}
    //true if the symbol is relative (only)
    bool is_relative() const {return _is_relative;}

  //Visitor/Modifior
    void accept(TVisitor* v) const {v->visit(this);}
    void modify(TModifior* m) {m->visit(this);}

  //Setters
    //add a new symbol
    void add_symbol(Symbol_rel* s)
    {
        unsigned long size = _sequence.size();
        //cerr << "***** size " << size << " max " << _seuence.max_size() << endl;
        _sequence.push_back(s);
        assert(_sequence.size() == size + 1);
    }
    //set initial tempo
    void set_init_tempo(double t) {_init_tempo = t;}
    //set relative event (easy type management) [TOCHECK]
    void set_relative() {_is_relative = true;} ///> default
    void set_physical() {_is_relative = false;}

private:
    vector<Symbol_rel*> _sequence; //the symbols' sequence
    double _init_tempo;

    bool _is_relative; ///> true if the Trace is relative (only relative or absolute symbol)
};


/* ** ** ** ** ** **** ** ** ** ** **
 *  Test Suite  ** ** ** ** ** **
 * ** ** ** ** ** **** ** ** ** ** **
 *  A suite of traces
 * A scenario, a suite of test cases
 */
class Test_suite
{
public:
  //Constructor / Destructor
    Test_suite() : _suite() {}

    ~Test_suite()
    {
        list<Trace*>::iterator i = _suite.begin();
        for(; i != _suite.end(); i++) delete *i;
        _suite.clear();
    }

  //Getters
    //get traces (const)
    const list<Trace*>* getTraces() const {return &_suite;}
    //get traces
    list<Trace*>* getModifTraces() {return &_suite;}

  //Setters
    //add a next trace
    void next_trace() {_suite.push_back(new Trace());}
    //add a symbol
    void addSymbol(Symbol_rel* s) {_suite.back()->add_symbol(s);}

  //Visitor
    void accept(TVisitor* v) const {v->visit(this);}

private:
    list<Trace*> _suite;
};

#endif
