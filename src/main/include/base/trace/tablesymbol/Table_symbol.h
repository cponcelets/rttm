//
//  Symbol.h
//  timedtracechecker
//
//  Created by Clement Poncelet on 05/12/13.
//
//

#ifndef __timedtracechecker___Symbol__
#define __timedtracechecker___Symbol__

#include <iostream>
#include <fstream>

#include <map>
#include <cassert>

#include <base/trace/tablesymbol/Symbols.h>
#include <base/AuxiliaireFunction.h>

// ************************************
#define LABEL_LENGTH 20
#define MINUTE 60.0

using namespace std;

/* ** ** ** ** ** **** ** ** ** ** **
 *  Symbol table ** ** ** ** ** **
 * ** ** ** ** ** **** ** ** ** ** **
 *  The table symbol stores the relation between Antescofo's and automata's labels (system tested and model sides)
 *  The file is created during the model creation (by .automata [opt -test])
 *  The table type is V or T [Verification or translation]
 *
 *  T type (translation)  : automata -> antescofo : key -> val
 *  V type (Verif)        : antescofo -> automata : kay -> val
 **/
class Symbol_table {
public:
  //Constructor / Destructor
    /**
      @param[in] file, file containing the table symbol
      @param[in] type, T or V
    **/
    Symbol_table(const string file, char type);

    ~Symbol_table(){_table_act.clear();_table_evt.clear();}

  //Getters [TOCHECK]
    //get table (singleton design pattern)
    /**
      @param[in] file, file containing the table symbol
      @param[in] type, T or V
    **/
    static Symbol_table* get_table(const string file, char type);
    //get automaton label from antescofo one
    /**
      @param[in] l_antescofo, antescofo label
      @param[in] type, T or V
    **/
    string get_automata_lab(const string& l_antescofo, char type) const;
    //get antescofo label from automaton one
    /**
      @param[in] l_automata, automata label
      @param[in] type, T or V
    **/
    string get_antescofo_lab(const string& l_automata, char type) const;
    //get antescofo key label from automata one
    /**
      @param[in] l_automata, automata label
      @param[in] type, T or V
    **/
    string get_key_antescofo_lab(const string& l_automata, char type) const;
    //get automaton key label from antescofo one
    /**
      @param[in] l_antescofo, antescofo label
      @param[in] type, T or V
    **/
    string get_key_automata_lab(const string& l_antescofo, char type) const;
    //true if there is a next event
    /**
      @param[in] current, current event
      @param[in] evt_automata, automata evt ??
    **/
    bool is_next_evt(const long current, const string evt_automata) const;
    //true if it is an action
    /**
      @param[in] label, label symbol
    **/
    bool is_action(const string label) const;
    //true if it is an action ??
    /**
      @param[in] label, label symbol
    **/
    bool is_action2(const string label) const;
    //true if it is an event
    /**
      @param[in] label, label symbol
    **/
    bool is_event(const string label) const;
    //get cuenum ??
    /**
      @param[in] l_automata, automata label
    **/
    long get_eventNum(string l_automata) const;
    //get tempo
    double getTempo() const
      {assert(_score_tempo > 10 && _score_tempo < 300); return _score_tempo;}

    //debug
    void print_evt() const;

private:
  //Private methods
    /**
      @param[in] key, key label
      @param[in] type, ??
    **/
    string get(string key, char type) const;
    //the inversed map
    /**
      @param[in] val, val label
      @param[in] type, ??
    **/
    string val_to_key(string val, char type) const;

  //Private fields
    static Symbol_table* _Stable; //table symbol

    //the maps
    map <string,string> _table_act;                 //actions
    map <string, pair <string, long> > _table_evt;  //events

    //Table symbol is used for the translater (in case of jumb we need the ante-label) and the verification
    //The problem, we need as key label the automaton label in the first case or the antescofo ones in the other
    //the type change the key-value order to solve it
    char _type;

    double _score_tempo; //needded?
};
#endif /* defined(__timedtracechecker___Symbol__) */
