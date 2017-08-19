//
//  Symbols.cpp
//  timedtracechecker
//
//  Created by Clement Poncelet on 04/05/2015.
//  Copyright (c) 2015 Clement Poncelet. All rights reserved.
//

#include <base/trace/tablesymbol/Symbols.h>

/* ** ** ** ** ** **** ** ** ** ** **
 *  A Trace (Performance) ** ** ** ** ** **
 * ** ** ** ** ** **** ** ** ** ** **
 */
const vector<Symbol_rel*>::const_iterator Trace::get_last_evt() const
{
    vector<Symbol_rel*>::const_iterator it_search = _sequence.end();
    it_search --;
    while(!(*it_search)->is_event()) {it_search --;}
    return it_search;
}
