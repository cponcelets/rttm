//
//  Symbol_table_parser.cpp
//  timedtracechecker
//
//  Created by Clement Poncelet on 16/05/2015.
//  Copyright (c) 2015 Clement Poncelet. All rights reserved.
//

#include <base/trace/parser/Symbol_table_parser.h>

Symbol_table_parser::Symbol_table_parser(const string f):
    _symbol_table_file(f.c_str(), std::ios::in), symbols_label()
{
    if (!_symbol_table_file)
    {
            std::cerr << "Erreur lors de l'ouverture du fichier " << f << std::endl;
            exit(1);
    }
}

map <string, Symbol_phy*>* Symbol_table_parser::create_table()
{
    map <string, Symbol_phy*>* table = new map<string, Symbol_phy*>();


    while(!_symbol_table_file.eof())
    {

        pair<string, string>* current_label_symbol = NULL;
        while ( (!current_label_symbol))
        {
            current_label_symbol = get_current_symbols();
            if(_symbol_table_file.eof()) return table;
        }

        if(symbols_label.find(current_label_symbol->first) != symbols_label.end())
        {
            std::cerr << "Warning :: We 've got a duplic label \"" << current_label_symbol->first << "\" on the test score" << std::endl;
            if(is_event_read)
                {exit(EXIT_FAILURE);} ///> managing of duplicate symbols (for antescofo actions) [Id for models]

            ///> add number to distincts same labels + _::
            auto it1 = current_label_symbol->first.find_last_of("_::");
            if(string::npos == it1)
                {
                    current_label_symbol->first.append("_::1");
                }
            else
            {
                const char* point1 = current_label_symbol->first.c_str() + it1 + 3;
                char charnum[5];
                sprintf(charnum, "%d", atoi(point1));

                current_label_symbol->first.replace((unsigned long)*point1, string::npos, charnum);
            }
        }
        symbols_label.insert(current_label_symbol->first);
        if(is_event_read)
        {
                table->insert(make_pair(current_label_symbol->second, new Event_phy(current_label_symbol->first, -1, -1, -1, _cuenum)));
        }
        else
        {
                table->insert(make_pair(current_label_symbol->second, new Action_phy(current_label_symbol->first, -1, -1)));
        }

    }
    return table;
}

void Symbol_table_parser::link(Trace* ref, Trace* real)
{
    link_events(ref, real);
    link_actions(ref, real);
}

/* Private Methods */
pair<string, string>* Symbol_table_parser::get_current_symbols()
{
    is_event_read = false;
    pair<string, string>* labels = NULL;

    if (_symbol_table_file)
    {
        char ligne[MAX_SIZE];
        if(!_symbol_table_file.getline(ligne, MAX_SIZE)) return NULL;

        int n = 0;
        char* pointer_ligne = ligne;

        //first letter T (TEMPO)
        if ('T' == *pointer_ligne) return labels;

        //or E
        _cuenum = -1;
        if('E' == *pointer_ligne)
        {
            pointer_ligne += 2;

            sscanf(pointer_ligne, "%ld%n", &_cuenum, &n);
            assert(n);

            pointer_ligne += n;
            n=0;

            is_event_read = true;
        }//else A
        else
        {
            pointer_ligne += 2;
        }
        char info_labs[LABELSIZE];

        sscanf(pointer_ligne, "%s%n", info_labs, &n);
        assert(n);
        string labs(info_labs);
        unsigned long p = labs.find_last_of(",");
        //antescofo , model labels
        labels = new pair<string, string>(labs.substr(0, p),
                                          labs.substr(p+1));

        /*if(-1 == _cuenum)
        {
            _actionSet.insert(labels->first);
        }
         */
    }
    else std::cerr << "Erreur lors de la lecture du fichier " << std::endl;
    return labels;
}

//[TO MERGE]
void Symbol_table_parser::link_events(Trace* ref, Trace* real)
{
    assert(ref->sequence()->size());
    assert(real->sequence()->size());
    vector<Symbol_rel*>::const_iterator symbol_ref = ref->sequence()->begin();
    vector<Symbol_rel*>::const_iterator symbol_real = real->sequence()->begin();

    //initialisation
    while(!((*symbol_real)->is_event()) && (symbol_real != real->sequence()->end()) ) symbol_real++;
    while(!((*symbol_ref)->is_event()) && (symbol_ref != ref->sequence()->end()) ) symbol_ref++;

    while ((symbol_ref != ref->sequence()->end()) || (symbol_real != real->sequence()->end()))
    {
        //Read a new value from the table symbol file
        pair<string, string>* current_label_symbol = NULL;
        while ( (!current_label_symbol))
        {
            current_label_symbol = get_current_symbols();
            if(_symbol_table_file.eof()) assert(false); //unknown symbols
        }

        //compare current traces' symbols
        if((symbol_ref != ref->sequence()->end()) && ! current_label_symbol->second.compare((*symbol_ref)->data_rel().label) )
        {   //match expected trace (model lab in the second field of the pair)

            if((symbol_real != real->sequence()->end()) && ! current_label_symbol->first.compare((*symbol_real)->data_rel().label) )
            {   //both matches ! link !
                (*symbol_real)->link_related(*symbol_ref);
                (*symbol_ref)->link_related(*symbol_real);

                //next event in the real trace
                symbol_real++;
                while((symbol_real != real->sequence()->end()) && !((*symbol_real)->is_event()) ) symbol_real++;
            }
            else
            {   //a ref symbol but not an antescofo one
                Event_phy* missing_event = new Event_phy(current_label_symbol->first, -1, -1, -1);
                missing_event->set_error(true);
                (*symbol_ref)->link_related(missing_event);
            }

            //next event in the ref trace
            symbol_ref++;
            while((symbol_ref != ref->sequence()->end()) && !((*symbol_ref)->is_event()) ) symbol_ref++;
        }
        else
        {
            if((symbol_real != real->sequence()->end()) && !current_label_symbol->first.compare((*symbol_real)->data_rel().label) )
            {   //an Antescofo symbol but not a ref one
                Event_phy* added_event = new Event_phy(current_label_symbol->second, -1, -1, -1);
                added_event->set_error(true);
                (*symbol_real)->link_related(added_event);

                symbol_real++;
                while((symbol_real != real->sequence()->end()) && !((*symbol_real)->is_event()) ) symbol_real++;
            }
        }

        if(current_label_symbol)
        {
            delete current_label_symbol;
        }
    }
}

void Symbol_table_parser::link_actions(Trace* ref, Trace* real)
{
    vector<Symbol_rel*>::const_iterator symbol_ref = ref->sequence()->begin();
    vector<Symbol_rel*>::const_iterator symbol_real = real->sequence()->begin();

    //initialisation
    while((symbol_real != real->sequence()->end()) && ((*symbol_real)->is_event()) ) symbol_real++;
    while((symbol_ref != ref->sequence()->end()) && ((*symbol_ref)->is_event()) ) symbol_ref++;

    while ((symbol_ref != ref->sequence()->end()) || (symbol_real != real->sequence()->end()))
    {
        //Read a now value from the table symbol file
        pair<string, string>* current_label_symbol = NULL;
        while ( (is_event_read) || (!current_label_symbol))
        {
            current_label_symbol = get_current_symbols();
            if(_symbol_table_file.eof())
            {
                secondPass(ref, symbol_ref, real, symbol_real);
                return;
            }
        }

        linkCurrentAction(current_label_symbol, symbol_ref, ref,
                                                symbol_real, real);

        if(current_label_symbol)
        {
            delete current_label_symbol;
        }
    }
}

//[TO MERGE]
bool Symbol_table_parser::linkCurrentAction(pair<string, string>* current_label_symbol,
                                            vector<Symbol_rel*>::const_iterator& symbol_ref, Trace* ref,
                                            vector<Symbol_rel*>::const_iterator& symbol_real, Trace* real)
{
    //compare current traces symbols
    if((symbol_ref != ref->sequence()->end()) && (!current_label_symbol->second.compare((*symbol_ref)->data_rel().label)) )
    {   //match expected trace (model lab in the second field of the pair)

        if((symbol_real != real->sequence()->end()) && (!current_label_symbol->first.compare((*symbol_real)->data_rel().label)) )
        {   //both matches ! link !
            (*symbol_real)->link_related(*symbol_ref);
            (*symbol_ref)->link_related(*symbol_real);

            //next event in the real trace
            symbol_real++;
            while((symbol_real != real->sequence()->end()) && ((*symbol_real)->is_event()) ) symbol_real++;
        }
        else
        {   //a ref symbol but not the antescofo one
            Action_phy* missing_event = new Action_phy(current_label_symbol->first, -1, -1);
            missing_event->set_error(true);
            (*symbol_ref)->link_related(missing_event);

            //record for the second pass
            _missed_symbols.insert(make_pair(current_label_symbol->first,(*symbol_ref)));
        }

        //next action in the ref trace
        symbol_ref++;
        while((symbol_ref != ref->sequence()->end()) && ((*symbol_ref)->is_event()) ) symbol_ref++;
    }
    else
    {
        if((symbol_real != real->sequence()->end()) && (!current_label_symbol->first.compare((*symbol_real)->data_rel().label)) )
        {   //an Antescofo symbol but not the ref one
            Action_phy* added_event = new Action_phy(current_label_symbol->second, -1, -1);
            added_event->set_error(true);
            (*symbol_real)->link_related(added_event);

            //record for the second pass
            _unexpected_symbols.insert(make_pair(current_label_symbol->second,(*symbol_real)));

            symbol_real++;
            while((symbol_real != real->sequence()->end()) && ((*symbol_real)->is_event()) ) symbol_real++;
        }
        else
        {
            //record for the second pass
            _Ante_unmatched_symbols.insert(make_pair(current_label_symbol->first, current_label_symbol->second));
            _Ref_unmatched_symbols.insert(make_pair(current_label_symbol->second, current_label_symbol->first));
            return false;
        }
    }
    return true;
}

void Symbol_table_parser::secondPass(Trace* ref, vector<Symbol_rel*>::const_iterator& symbol_ref,
                                     Trace* real, vector<Symbol_rel*>::const_iterator& symbol_real)
{
    //terminate the ref trace if not
    while(symbol_ref != ref->sequence()->end())
    {
        map<string, string>::iterator it_found = _Ref_unmatched_symbols.find((*symbol_ref)->data_rel().label);

        //become a missed symbol (if not present in _unexpected_symbols)
        map<string, Symbol_rel*>::iterator it_unexpected_found = _unexpected_symbols.find((*symbol_ref)->data_rel().label);

        assert(it_found != _Ref_unmatched_symbols.end() || it_unexpected_found != _unexpected_symbols.end()); //symbol inexistant ?

        if(it_unexpected_found == _unexpected_symbols.end())
        { //maybe missed
            //a ref symbol but not the antescofo one
            Action_phy* missing_event = new Action_phy(it_found->second, -1, -1);
            missing_event->set_error(true);
            (*symbol_ref)->link_related(missing_event);

            //record for the second pass
            _missed_symbols.insert(make_pair(it_found->second,(*symbol_ref)));
        }
        else
        {
            // symbol in missed so, delete missed related event and link the two symbols
            delete it_unexpected_found->second->data_rel().related_s;

            //both matches ! link !
            (*symbol_ref)->link_related(it_unexpected_found->second);
            it_unexpected_found->second->link_related(*symbol_ref);

            _unexpected_symbols.erase(it_unexpected_found);

            if(it_found != _Ref_unmatched_symbols.end())
            {
                _Ref_unmatched_symbols.erase(it_found);
            }
        }
        //next action in the ref trace
        symbol_ref++;
        while((symbol_ref != ref->sequence()->end()) && ((*symbol_ref)->is_event()) ) symbol_ref++;
    }

    //terminate the real trace if not
    while(symbol_real != real->sequence()->end())
    {
        map<string, string>::iterator it_found = _Ante_unmatched_symbols.find((*symbol_real)->data_rel().label);

        //become an unexpected symbol (if not present in _missed_symbols)
        map<string, Symbol_rel*>::iterator it_missed_found = _missed_symbols.find((*symbol_real)->data_rel().label);

        assert(it_found != _Ante_unmatched_symbols.end() || it_missed_found != _missed_symbols.end()); //symbol inexistant ?

        if(it_missed_found == _missed_symbols.end())
        { //really unexpected
            Event_phy* added_event = new Event_phy(it_found->second, -1, -1, -1);
            added_event->set_error(true);
            (*symbol_real)->link_related(added_event);
        }
        else
        {
            // symbol in missed so, delete missed related event and link the two symbols
            delete it_missed_found->second->data_rel().related_s;

            //both matches ! link !
            (*symbol_real)->link_related(it_missed_found->second);
            it_missed_found->second->link_related(*symbol_real);

            _missed_symbols.erase(it_missed_found);

            if(it_found != _Ante_unmatched_symbols.end())
            {
                _Ante_unmatched_symbols.erase(it_found);
            }
        }
        symbol_real++;
        while((symbol_real != real->sequence()->end()) && ((*symbol_real)->is_event()) ) symbol_real++;
    }

    //checks if some unexpected symbols are too in missed symbols
    for(auto it = _missed_symbols.begin(); it != _missed_symbols.end(); it++)
    {
        //Cherche if the model label of missed symbols are present in the unexpected keys
        auto it_found = _unexpected_symbols.find((*it).second->data_rel().label);
        if(it_found != _unexpected_symbols.end())
        {//Yes !!
            delete it->second->data_rel().related_s;
            delete it_found->second->data_rel().related_s;

            //both matches ! link !
            it->second->link_related(it_found->second);
            it_found->second->link_related(it->second);
        }
        //else it is a really missed symbol
    }
}
