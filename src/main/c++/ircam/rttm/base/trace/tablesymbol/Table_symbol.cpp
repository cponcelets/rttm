//
//  Symbol.cpp
//  timedtracechecker
//
//  Created by Clement Poncelet on 05/12/13.
//
//

#include <base/trace/tablesymbol/Table_Symbol.h>

#define LABELSIZE 25

/** statics member and method **/
Symbol_table* Symbol_table::_Stable = NULL;
Symbol_table* Symbol_table::get_table(const string file, char t) {
    if(!Symbol_table::_Stable) Symbol_table::_Stable = new Symbol_table(file, t);

    assert(Symbol_table::_Stable);
    return Symbol_table::_Stable;
}
/****/

Symbol_table::Symbol_table(const string file, char t):
  _table_act(), _table_evt(),
  _type(t), _score_tempo(0)
  {
    assert(file != "");
    assert(!Symbol_table::_Stable);

    std::ifstream f(file.c_str(), std::ios::in);
    std::string ligne;


    if (f) {
        while (getline(f, ligne)) {

            unsigned long size = _table_evt.size();

            if(!ligne.compare(0, 6,"TEMPO=")) {

                sscanf(ligne.c_str(), "TEMPO=%lf", &_score_tempo);

            } else {

                char type;
                char labels_c[LABELSIZE*2+1];
                double eventnum = -1;
                sscanf(ligne.c_str(), "%c", &type);
                if(type=='E') sscanf(ligne.c_str(), "%c %lg %s", &type, &eventnum, labels_c);
                else sscanf(ligne.c_str(), "%c %s", &type, labels_c);

                assert( (type=='A' && (eventnum == -1)) || (type=='E' && (eventnum >= 0)) );

                string label_antescofo;
                string label_automata;
                string labels(labels_c);

                unsigned long p = labels.find_last_of(",");
                label_antescofo = labels.substr(0, p);
                label_automata = labels.substr(p+1);
                //sscanf(ligne.c_str(), "%s,%s", label_antescofo, label_automata);

                //trick key/value inversion
                if(_type == 'V') {

                    //Potential mistakes (Duplic names antescofo output label)
                    //assert(_table.find(label_antescofo) == _table.end());
                    if(_table_act.find(label_antescofo) != _table_act.end() || _table_evt.find(label_antescofo) != _table_evt.end()) {
                        std::cerr << "Error :: We 've got a duplic label \"" << label_antescofo << "\" on the test score" << std::endl;
                        exit(EXIT_FAILURE);
                    }

                    if (type == 'A')   _table_act.insert(pair<string, string>(label_antescofo, label_automata));
                    else
                    {
                        _table_evt.insert(pair<string, pair<string, long> >(label_antescofo,
                                                                        pair<string, long>(label_automata, eventnum)));
                        //std::cerr << "**** insert : " << label_antescofo << " " << label_automata << " " << eventnum <<endl;
                        assert(_table_evt.size() == size+1);
                    }
                }
                else
                {
                    // Potential mistakes (Duplic names antescofo output label)
                    //assert(_table.find(label_antescofo) == _table.end());
                    if(_table_act.find(label_automata) != _table_act.end() || _table_evt.find(label_automata) != _table_evt.end()) {
                        std::cerr << "Error :: We 've got a duplic label \"" << label_antescofo << "\" on the test score" << std::endl;
                        exit(EXIT_FAILURE);
                    }

                    if (type == 'A')   _table_act.insert(pair<string, string>(label_automata, label_antescofo));
                        else
                        {
                            _table_evt.insert(pair<string, pair<string, long> >(label_automata,
                                                                                        pair<string, long>(label_antescofo, eventnum)));
                            //std::cerr << "**** insert : " << label_antescofo << " " << label_automata << " " << eventnum <<endl;
                            assert(_table_evt.size() == size+1);
                        }
                }
            }
        }
    } else std::cerr << "Erreur lors de l'ouverture du fichier " << file << std::endl;

    f.close();
}

void Symbol_table::print_evt() const
{
    std::cerr << "event table " << std::endl;
    map<string, pair <string, long> >::const_iterator it = _table_evt.begin();
    while(it != _table_evt.end()) {std::cerr << (*it).first << " " << (*it).second.first << std::endl;it++;}
}

long Symbol_table::get_eventNum(string l_automata) const
{
    map<string, pair <string, long> >::const_iterator it = _table_evt.find(l_automata);
    assert(it != _table_evt.end());
  //Not work for maps  long index = std::distance(_table_evt.begin(), it);
    return (*it).second.second;
}

bool Symbol_table::is_next_evt(const long current, const string evt_automata) const {
    map<string, pair <string, long> >::const_iterator it = _table_evt.begin();
    for(long i = 0; i < current; i++) it ++;
    return it != _table_evt.end() && !(*it).first.compare(evt_automata);
}

//(attention aux sens, c'est la donnée lu (clef) sur le fichier qui doit etre passée en input)
bool Symbol_table::is_action(const string label) const {
    map<string, string>::const_iterator it = _table_act.find(label);
    return !(it == _table_act.end());
}

//(attention aux sens, c'est la donnée lu (clef) sur le fichier qui doit etre passée en input)
bool Symbol_table::is_action2(const string label) const {
    map <string,string>::const_iterator it = _table_act.begin();
    while(it != _table_act.end()) {
        if(! (*it).second.compare(label)) return true;
        it++;
    }
    return false;
}

//(attention aux sens, c'est la donnée lu (clef) sur le fichier qui doit etre passée en input)
bool Symbol_table::is_event(const string label) const {
    map<string, pair <string, long> >::const_iterator it = _table_evt.find(label);
    return !(it == _table_evt.end());
}

/*  Standard access */
string Symbol_table::get_automata_lab(const string& l_antescofo, char type) const {
    assert(_type == 'V');
    return get(l_antescofo, type);
}

string Symbol_table::get_antescofo_lab(const string& l_automata, char type) const {
    assert(_type == 'D');
    return get(l_automata, type);
}

string Symbol_table::get(string key, char type) const {
    if(type == 'A')     {map<string, string>::const_iterator it = _table_act.find(key);
                         return (it == _table_act.end()) ? NULL : it->second;}
    map<string, pair <string, long> >::const_iterator it = _table_evt.find(key);
    return (it == _table_evt.end()) ? NULL : it->second.first;
}

/* Inversed access*/
string Symbol_table::get_key_antescofo_lab(const string& l_automata, char type) const {
    assert(_type == 'V');
    return val_to_key(l_automata, type);
}

string Symbol_table::get_key_automata_lab(const string& l_antescofo, char type) const {
    assert(_type == 'D');
    return val_to_key(l_antescofo, type);
}

/* Impossible to optimize :: To use rarely */
string Symbol_table::val_to_key(string val, char type) const {
    assert(&val);
    if(type == 'A')     {
        map<string, string>::const_iterator it = _table_act.begin();
        while(it->second.compare(val)) it++;
        assert(it != _table_act.end());
        return it->first;
    }
    map<string, pair <string, long> >::const_iterator it = _table_evt.begin();
    //cerr << " **** " << val <<  endl;
    while(it != _table_evt.end() && it->second.first.compare(val)) it++;
    assert(it != _table_evt.end());
    return it->first;
}
