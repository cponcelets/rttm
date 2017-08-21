//
//  Symbol_table_parser.h
//  timedtracechecker
//
//  Created by Clement Poncelet on 16/05/2015.
//  Copyright (c) 2015 Clement Poncelet. All rights reserved.
//

#ifndef __timedtracechecker__Symbol_table_parser_h
#define __timedtracechecker__Symbol_table_parser_h

#include <iostream>
#include <fstream>
#include <set>
#include <vector>
#include <map>

#include <base/trace/tablesymbol/Symbols.h>

#define LABELSIZE 50
#define MAX_SIZE 1024

using namespace std;

/* ** ** ** ** ** **** ** ** ** ** **
 *  Symbol table parser ** ** ** ** ** **
 * ** ** ** ** ** **** ** ** ** ** **
 *  Parse a symbol table file and create the symbol map:
 *  string => string
 * [note: this table is not needed if labels are equals in actual/reference traces or input trace/score]
 */
class Symbol_table_parser
{
public:
  //Constructor / Destructor
    /**
      @param[in] file, filename of the symbol table (.symbol_table)
    **/
    Symbol_table_parser(const char* f);
    ~Symbol_table_parser() {_symbol_table_stream->close();}

  //Public methods
    /** Used for the trace generation (MODE T)
     *  Here we want to check doublon during the link of the two labels domain
     **/
    map <string, Symbol_phy*>* create_table();

    /** Link symbols of reference and output trace (symbols are updated)
     *  Used for comparizon of two traces (MODE V)
      @param[in/out] ref, reference trace
      @param[in/out] real, system output trace (to test)
    **/
    void link(Trace* ref, Trace* real);

private:

  //Private methods
    /*
     * Function used to read a line in the table symbol file
      @param[out] the pair* of <antescofo, model> labels (strings)
                  return null if the current line is not a symbol
     */
    pair<string, string>* get_current_symbols();
    /*
     * Read all the events of the symbol table file
     * in order to link and add missed labels for all the events in the two traces
      @param[in/out] ref, reference trace
      @param[in/out] real, system output trace (to test)
     */
    void link_events(Trace* ref, Trace* real);
    /*
     * Read all the actions of the symbol table file
     * in order to link and add missed labels for all the actions in the two traces
      @param[in/out] ref, reference trace
      @param[in/out] real, system output trace (to test)
     */
    void link_actions(Trace* ref, Trace* real);
    /*
     * Do the link with the corresponding labels
      @param[in]: current_symbol, the symbol to link <Antescofo, model> labels
      @param[in/out]: ref/real, iterators on the traces (the reference and the real)
      @param[out]: return true  if a link had been done
                          false otherwise
     */
    bool linkCurrentAction(pair<string, string>* current_symbol,
                           vector<Symbol_rel*>::const_iterator&, Trace* ref,
                           vector<Symbol_rel*>::const_iterator&, Trace* real);
    /*  note:
     * In complicate cases symbol actions of diff. traces are not on the same index
     * So a first pass considers this assumption storing missed/unexpected and unmatched action symbols
     * A second pass: terminates traces traversals (it means that some symbols were unmatched)
     *                and Check if some unexpected symbols are related to some missed ones.
     *
     * To do that we use: * two HashMap:
     *                             _missed and
     *                             _unexpected symbols (supposed by the first pass)
     *                    * one HashMap<model, Antescofo>:
     *                             unmatched symbols
      @param[in]: ref/real, the traces (the reference and the real)
      @param[in/out]: symbol_ref/symbol_real, iterators on the traces (the reference and the real)
     */
    void secondPass(Trace* ref, vector<Symbol_rel*>::const_iterator& symbol_ref,
                    Trace* real, vector<Symbol_rel*>::const_iterator& symbol_real);

  //Private Fields
    //V2 Fields
    map<string, string> _Ante_unmatched_symbols;  //keys Antescofo - reference string
    map<string, string> _Ref_unmatched_symbols;   //keys Model - reference string
    map<string, Symbol_rel*> _missed_symbols;     //in ref but no in real (keys Antescofo Lab)
    map<string, Symbol_rel*> _unexpected_symbols; //in real but no in ref (keys Model Lab)

    // Other fields
    ifstream*   _symbol_table_stream;   //istream of symbol table
    set<string> symbols_label;          //set of symbols used to check doublon

    bool is_event_read;           //boolean [used?]
    long _cuenum;                 //cuenum  [used?]

    //To stock and filtre real actions
    //set<string> _actionSet;
};

#endif
