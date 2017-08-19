//
//  Trace_Parser.cpp
//  timedtracechecker
//
//  Created by Clement Poncelet on 11/05/2015.
//  Copyright (c) 2015 Clement Poncelet. All rights reserved.
//

#include <base/trace/parser/Trace_Parser.h>

/* ** ** ** ** ** **** ** ** ** ** **
 *  Uppaal_Suite_parser ** ** ** ** ** **
 * ** ** ** ** ** **** ** ** ** ** **
 **/

UpppalSuiteParser::UpppalSuiteParser(string fic, map <string, Symbol_phy*>* t):
    _Uppaal_trace_file(fic.c_str(), std::ios::in), test(NULL),
    trace_compter(0), _beat_cumulator(0.0), _phy_traces(false),
    symbol_table(t), _last_event(nullptr)
{
    string line;
    getline(_Uppaal_trace_file, line);
    while (true)
    {

        //preparation before parsing
        //keyword Trace is used to delimit test cases
        //We pass the first Trace (used as end-limitation)
        if (line.size() > 11 && !line.compare(6, 5, "Trace")) break;
        if (!line.compare(0, 5, "State")) break;

        getline(_Uppaal_trace_file, line);
        assert(!_Uppaal_trace_file.eof());
    }
}

UpppalSuiteParser::~UpppalSuiteParser()
{
    _Uppaal_trace_file.close();
}

Trace* UpppalSuiteParser::parseATrace(bool phy_symbol)
{
    _phy_traces = phy_symbol;
    test = new Trace();
    if(_phy_traces) test->set_physical();

    std::string ligne;

    trace_compter++;
    _beat_cumulator = 0.0;

    if (_Uppaal_trace_file)
    {
        while (getline(_Uppaal_trace_file, ligne))
        {
            if(ligne.size() > 5)
            {
                if (!ligne.compare(0, 5, "Delay"))
                {
                    int d;
                    sscanf(ligne.c_str(), "Delay: %d", &d);
                    // Automaton relative time into Score relative time
                    double dtmp = d;
                    dtmp /= CONVERTION;
                    _beat_cumulator += dtmp;
                }
                else if(ligne.size() > 11 && !ligne.compare(6, 5, "Trace"))
                {
                    //To free
                    return test;
                }
                else if (ligne.size() > 11 && !ligne.compare(0, 11, "Transitions"))
                {
                    parse_transition();
                }
            }
        }
    }
    else
    {
        std::cerr << "Erreur lors de l'ouverture du fichier " << file << std::endl;
        exit(1);
    }

    //To free
    return test;
}

void UpppalSuiteParser::parse_transition()
{
    std::string ligne;
    getline(_Uppaal_trace_file, ligne);

    do
    {
        if(!ligne.compare(2, 3, "Env"))
        {
            long itstr, itend;


            itstr = (long) ligne.find_first_of(",");
            ligne.erase(ligne.begin(), ligne.begin()+itstr+2);

            itend = (long) ligne.rfind("!");
            if(itend == std::string::npos) {return;}
            ligne.erase(ligne.begin()+itend, ligne.end());

            //cerr << " Rttime == " << _Rttime << endl;
            //To set laters (convertion phase)
            if(_phy_traces)
            {
                Event_phy* ref_symbol = new Event_phy(ligne, _beat_cumulator, -1, -1);

                map <string, Symbol_phy*>::iterator it_found =  symbol_table->find(ref_symbol->data_rel().label);
                assert(it_found != symbol_table->end());
                ref_symbol->link_related(it_found->second);

                test->add_symbol(ref_symbol);
                ///> next used in generation_R_to_S for physical traces
                if (nullptr != _last_event) _last_event->set_next(*ref_symbol);
                _last_event = ref_symbol;
            }
            else
            {
                Event_rel* ref_symbol = new Event_rel(ligne, _beat_cumulator);

                map <string, Symbol_phy*>::iterator it_found =  symbol_table->find(ref_symbol->data_rel().label);
                assert(it_found != symbol_table->end());
                ref_symbol->link_related(it_found->second);

                test->add_symbol(ref_symbol);
            }
           // else                tests->add_symbol(new Event_rel(ligne, _beat_cumulator));
        }
        else
        {
            unsigned long itact = ligne.rfind(" a");
            if (itact != std::string::npos)
            {
                unsigned long itstr, itend;
                itstr = ligne.find_first_of(",");
                ligne.erase(ligne.begin(), ligne.begin()+(long)itstr+2);

                itend = ligne.rfind("!");
                ligne.erase(ligne.begin()+(long)itend, ligne.end());

                //cerr << ligne << endl;
                if (_phy_traces)
                {
                    Action_phy* ref_symbol = (new Action_phy(ligne, _beat_cumulator, -1));

                    map <string, Symbol_phy*>::iterator it_found =  symbol_table->find(ref_symbol->data_rel().label);
                    assert(it_found != symbol_table->end());
                    ref_symbol->link_related(it_found->second);

                    test->add_symbol(ref_symbol);
                }
                else
                {
                    Action_rel* ref_symbol = (new Action_rel(ligne, _beat_cumulator));

                    map <string, Symbol_phy*>::iterator it_found =  symbol_table->find(ref_symbol->data_rel().label);
                    assert(it_found != symbol_table->end());
                    ref_symbol->link_related(it_found->second);

                    test->add_symbol(ref_symbol);
                }
            }
        }
        getline(_Uppaal_trace_file, ligne);

    } while(ligne.size() > 0);
}

/* ** ** ** ** ** **** ** ** ** ** **
 *  Antescofo output parser ** ** ** ** ** **
 * ** ** ** ** ** **** ** ** ** ** **
 **/
AntescofoOutputParser::AntescofoOutputParser(string fic):
    file(fic), _last_event(NULL) {}

Trace* AntescofoOutputParser::parse()
{
    real_trace = new Trace();

    std::ifstream f(file.c_str(), std::ios::in);
    char ligne[MAX_SIZE];

    if (f) {
        while (f.getline(ligne, MAX_SIZE)) {
            int n = 0;
            char* pligne = ligne;
            long cuenum;

            sscanf(ligne, "%ld%n", &cuenum, &n);
            if (n) {    //EVENT
                double now =-1;
                double rnow =-1;
                double tempo;
                char label[20];


                pligne += n;    //LABEL
                n = 0;
                sscanf(pligne, "%s%n", label,&n);
                assert(n);

                pligne += n;    //NOW
                n = 0;
                sscanf(pligne, "%lf%n", &now, &n);
                assert(n);

                pligne += n;    //RNOW  ignoring
                n = 0;
                //                sscanf(pligne, "%*lf%n", &n);
                sscanf(pligne, "%lf%n", &rnow, &n);
                assert(n);

                pligne += n;    //TEMPO
                n = 0;
                sscanf(pligne, "%lf%n", &tempo,&n);
                assert(n);

                assert(rnow >= 0);
                assert(now >= 0);
                //std::cerr << "Read :: EVENT "<< notenum << " " << label << " now=" << timestp << " tempo=" << tempo << endl;
                Event_phy* current_event = new Event_phy(label, rnow,  now, tempo, cuenum);
                real_trace->add_symbol(current_event);

                //compute durations:  de-1 = te - te-1 (in seconds)
                if(_last_event)
                {
                    _last_event->set_relDuration(rnow - _last_event->data_rel().trel);
                    _last_event->set_phyDuration(now - _last_event->data_phy().tphy);
                }
                _last_event = current_event;

            } else {
                char label[20];
                sscanf(pligne, "%s%n", label, &n);
                if(n && strncmp(label, "now", 3) && strncmp(label, "$", 1) && strncmp(label, "gamma", 5)) //(parse can eq at "now" if no action name (phantom trace))                              ///ACTION
                {
                    double now;
                    double rnow;

                    //if(strncmp(label, "a", 1)) continue; //if not beginning with a -> not an action
                    //All actions are considered (the filter must be in Antescofo trace)

                    pligne += n;    //NOW
                    n = 0;
                    sscanf(pligne, "\t%lf%n", &now, &n);
                    assert(n);

                    pligne += n;    //RNOW
                    n = 0;
                    sscanf(pligne, "\t%lf%n", &rnow, &n);
                    assert(n);
                    //std::cerr << "Read :: Action "<< label << " now=" << timestp  << endl;

                    //erase _msg
                    string s(label);
                    std::string lab = s.substr (0, s.find("_msg"));

                    Action_phy* current_action = new Action_phy(label, rnow,  now);
                    real_trace->add_symbol(current_action);

                    //set the delay after the last event
                    double last_event_relTimestamp = (_last_event ? _last_event->data_rel().trel : 0.0);
                    double last_event_phyTimestamp = (_last_event ? _last_event->data_phy().tphy : 0.0);
                    current_action->set_relDuration(rnow - last_event_relTimestamp);
                    current_action->set_phyDuration(now - last_event_phyTimestamp);
                }
            }
        }
    }
    else
    {
     std::cerr << "Erreur lors de l'ouverture du fichier " << file << std::endl;
     exit(1);
    }
    f.close();


    return real_trace;
}

/* ** ** ** ** ** **** ** ** ** ** **
 *  Reference trace parser ** ** ** ** ** **
 * ** ** ** ** ** **** ** ** ** ** **
 **/
ReferenceTraceParser::ReferenceTraceParser(string fic):
    file(fic), _last_event(nullptr), _rel_cursor(0.0) {}

Trace* ReferenceTraceParser::parse()
{
    ref_trace = new Trace();

    std::ifstream f(file.c_str(), std::ios::in);
    char ligne[MAX_SIZE];

    bool start = false;
    if (f)
    {
         while (f.getline(ligne, MAX_SIZE))
         {

             int n = 0;
             char* pligne = ligne;

             if(start)
             {
                 // caution size(\t) = 1
                 if(strlen(ligne) > 3)
                 {
                    if (!strncmp(pligne, "EVENT", 5))
                    {
                        uint cuenum =0;
                        double timestamp;
                        double tempo;
                        char label[20];

                        pligne += 6;    //cuenum
                        n = 0;
                        sscanf(pligne, "%d%n", &cuenum, &n);
                        assert(n);
                        assert(0 != cuenum);

                        pligne += n;    //label
                        n = 0;
                        sscanf(pligne, "%s%n", label, &n);
                        assert(n);

                        pligne += n;    //t_imestamp (relative or physical)
                        n = 0;
                        sscanf(pligne, "%lf%n", &timestamp, &n);
                        assert(n);

                        pligne += n;    //tempo
                        n = 0;
                        sscanf(pligne, "%lf%n", &tempo, &n);
                        if(0 == n)  ///> management of relative references
                        {
                            ref_trace->set_relative();
                            ref_trace->add_symbol(new Event_phy(label, timestamp, -1, -1)); ///>computation later
                        } else
                        {
                            ref_trace->set_physical(); ///> then timestamps are physical

                            ///> otherwise we read tpi and pi
                            ///> and can compute dpi-1, dri-1 and tri-1
                            Event_phy* ref_symbol =  new Event_phy(label, 0., timestamp, tempo); ///> ei
                            ref_trace->add_symbol(ref_symbol);

                            if(nullptr != _last_event)
                            {
                                double phy_duration = timestamp - _last_event->data_phy().tphy;
                                double rel_duration = S_to_R(phy_duration, _last_event->tempo()); ///> dri-1 = (dpi-1 / 60) * pi-1
                                                                                                  ///> (<s ,tp, p>)
                                _rel_cursor += rel_duration; ///> relative timestamp

                                _last_event->set_phyDuration(phy_duration);
                                _last_event->set_relDuration(rel_duration);
                                ref_symbol->set_relTimestamp(_rel_cursor);
                            }

                            _last_event = ref_symbol;   ///> We store the last event
                        }

                        //ok
                        //std::cerr << "J'ai lu " << "NOTE " << label << " " << timestp << " " << dur << " tempo " << tempo << std::endl;
                    }
                    else
                    {
                        ///> actions
                        double timestamp;
                        char label[20];

                        pligne += n;
                        n = 0;          //label (<s, t>)

                        sscanf(pligne, "%s%n", label, &n);
                        assert(n);

                        pligne += n;
                        n = 0;  //timestamp
                        sscanf(pligne, "%lf%n", &timestamp, &n);
                        assert(n);

                        if(ref_trace->is_relative())
                        {
                            ref_trace->add_symbol(new Action_phy(label, timestamp, -1));
                        } else
                        {
                            double phy_duration = timestamp - _last_event->data_phy().tphy;     ///> dp
                            double rel_duration = S_to_R(phy_duration, _last_event->tempo());   ///> dr
                            double rel_timestamp = _rel_cursor + rel_duration;                  ///> tr

                            Action_phy* ref_symbol =  new Action_phy(label, rel_timestamp, timestamp);

                            ref_symbol->set_phyDuration(phy_duration);
                            ref_symbol->set_relDuration(rel_duration);

                            ref_trace->add_symbol(ref_symbol);
                        }
                    }
                }
             }
             else
             {
                 if (!strncmp(pligne, "START", 5))  {start = true;}
             }
        }
    }
    else
    {
        std::cerr << "Erreur lors de l'ouverture du fichier " << file << std::endl;
        exit(1);
    }
    f.close();
    return ref_trace;
}

/* ** ** ** ** ** **** ** ** ** ** **
 *  Antescofo score parser ** ** ** ** ** **
 * ** ** ** ** ** **** ** ** ** ** **
 **/
AntescofoScoreParser::AntescofoScoreParser(std::string fic):
    file(fic), _score_tempo(-1) {}

void AntescofoScoreParser::add_pitches(map <string, Symbol_phy*>* table)
{
    //Very trivial algo.
    //Key labels are model labels (no Antescofo ones)

    std::ifstream f(file.c_str(), std::ios::in);
    string ligne;

    //variable declarations
    double relative_timestamp = 0.0;
    double next_relative_duration = 0.0;
    double action_timestamp = 0.0;

    if (f)
    {
        while (getline(f, ligne))
        {

            // caution size(\t) = 1
            if(ligne.size() > 3)
            {

                if (!ligne.compare(0, 4, "NOTE"))
                {
                    relative_timestamp += next_relative_duration;
                    action_timestamp = relative_timestamp;

                    double relative_duration =-1;
                    char label[20];
                    char pitch[5];

                    if(ligne.find_first_of("/") == string::npos)
                    {
                        sscanf(ligne.c_str(), "NOTE %s %lf %s", pitch, &relative_duration, label);
                    }
                    else    //gestion des fractions
                    {
                        char string_dur[10];
                        sscanf(ligne.c_str(), "NOTE %s %s %s", pitch, string_dur, label);

                        relative_duration = string2double(string_dur);
                    }

                    assert(relative_duration >= 0);

                    Pitch* p = new NOTE2(pitch);

                    //Delete "" ...
                    char key[] = "\"";
                    char* pch = strpbrk (label, key);
                    while (pch != NULL)
                    {
                        char* ppch = pch;
                        //printf ("%c %c\n" , *pch, label[ppch-label]);
                        while( label[ppch-label] != '\0') {label[ppch-label] = label[(ppch+1)-label]; ppch++;}
                        label[ppch-label+1] = label[(ppch+1)-label+1];
                        pch = strpbrk (pch+1,key);
                    }
                    if(strlen(label) > 0)
                    {
                        addData(table, label, relative_timestamp, p);
                    } else
                    {
                        assert(false); //> No label on event
                    }

                    //ok
                    //std::cerr << "J'ai lu " << "EVENT " << pitch << " - " << label << " - " << dur << std::endl;
                    next_relative_duration = relative_duration;
                }
                else
                    if ((!ligne.compare(0, 5, "CHORD")))
                    {
                        relative_timestamp += next_relative_duration;
                        action_timestamp = relative_timestamp;

                        double relative_duration;
                        char label[20];

                        unsigned long p1 = ligne.find_first_of("(");
                        unsigned long p2 = ligne.find_first_of(")");
                        string pitches = ligne.substr(p1+2, (p2-3)-p1);

                        CHORD2* p = new CHORD2();

                        unsigned long _cur_p = 0;
                        p->add_pitch(pitches.substr(_cur_p, 4));

                        while((_cur_p = pitches.find_first_of(" ")) != string::npos)
                        {
                            p->add_pitch(pitches.substr(_cur_p+1, 4));
                            pitches.erase(0, _cur_p+1);
                        }

                        if(ligne.find_first_of("/") == string::npos)
                        {
                            sscanf(ligne.c_str(), "CHORD (%*[^)]) %lf %s", &relative_duration, label);
                        }
                        else
                        {
                            char string_dur[10];
                            sscanf(ligne.c_str(), "CHORD (%*[^)]) %s %s", string_dur, label);

                            relative_duration = string2double(string_dur);
                        }

                        //Delete "" ...
                        char key[] = "\"";
                        char* pch = strpbrk (label, key);
                        while (pch != NULL)
                        {
                            char* ppch = pch;
                            //printf ("%c %c\n" , *pch, label[ppch-label]);
                            while( label[ppch-label] != '\0') {label[ppch-label] = label[(ppch+1)-label]; ppch++;}
                            label[ppch-label+1] = label[(ppch+1)-label+1];
                            pch = strpbrk (pch+1,key);
                        }
                        if(strlen(label) > 0)
                        {
                            addData(table, label, relative_timestamp, p);
                        }

                        //ok
                        //std::cerr << "J'ai lu " << "EVENT - ";
                        //for(vector<string>::const_iterator itp = pitch->begin(); itp != pitch->end(); itp++) std::cerr << (*itp) << " ";
                        //std::cerr << "- " << label << " - " << dur << std::endl;
                        next_relative_duration = relative_duration;

                    }
                else
                    if  ((!ligne.compare(0, 5, "TRILL")))
                    {
                        relative_timestamp += next_relative_duration;
                        action_timestamp = relative_timestamp;

                        double relative_duration;
                        char label[20];

                        unsigned long p1 = ligne.find_last_of("(");
                        unsigned long p2 = ligne.find_first_of(")");
                        string pitches = ligne.substr(p1+1, (p2-1)-p1);

                        TRILL2* p = new TRILL2();
                        unsigned long _cur_p = 0;
                        p->add_pitch(pitches.substr(_cur_p, 4)); //first

                        while((_cur_p = pitches.find_first_of(" ")) != string::npos)
                        {
                            p->add_pitch(pitches.substr(_cur_p+1, 4));
                            pitches.erase(0, _cur_p+1);
                        }

                        const char* pligne = ligne.c_str();
                        p2 = ligne.find_last_of(")");
                        const char* pcur = pligne + p2 + 1;

                        if(ligne.find_first_of("/") == string::npos)
                        {
                            sscanf(pcur, " %lf %s", &relative_duration, label);
                        }
                        else
                        {
                            char string_dur[10];
                            sscanf(pcur, " %s %s", string_dur, label);

                            relative_duration = string2double(string_dur);
                        }


                        //Delete "" ...
                        char key[] = "\"";
                        char* pch = strpbrk (label, key);

                        while (pch != NULL)
                        {
                            char* ppch = pch;
                            //printf ("%c %c\n" , *pch, label[ppch-label]);
                            while( label[ppch-label] != '\0') {label[ppch-label] = label[(ppch+1)-label]; ppch++;}
                            label[ppch-label+1] = label[(ppch+1)-label+1];
                            pch = strpbrk (pch+1,key);
                        }

                        if(strlen(label) > 0)
                        {
                            addData(table, label, relative_timestamp, p);
                        }

                        //ok
                        //std::cerr << "J'ai lu " << "EVENT - ";
                        //for(vector<string>::const_iterator itp = pitch->begin(); itp != pitch->end(); itp++) std::cerr << (*itp) << " ";
                        //std::cerr << "- " << label << " - " << dur << std::endl;

                        next_relative_duration = relative_duration;
                    }
                else
                    if ((!ligne.compare(0, 3, "BPM")) || (!ligne.compare(0, 3, "bpm")))
                    {
                        double tempo = -1;
                        sscanf(ligne.c_str(), "BPM %lf", &tempo);
                        if(-1 == tempo)
                        {
                            sscanf(ligne.c_str(), "bpm %lf", &tempo);
                        }

                        _score_tempo = tempo;
                    }
                else
                    {
                        double delay;
                        char label[20];
                        //std::remove(ligne.begin(), ligne.begin() + ligne.find_first_not_of(' '), ' ');      //ltrim

                        //std::cerr << "### LIGNE :: " << ligne << std::endl;

                        ///> Labels required so for translation
                        unsigned long p = ligne.find_first_of("@"); ///> erreur if @ in action
                        if(string::npos != p)
                        {
                            const char* pligne = ligne.c_str() + p;
                            if(sscanf(pligne, "@name %s", label))
                            {
                                if(sscanf(ligne.c_str(), "%lf ", &delay))
                                {
                                    addData(table, label, action_timestamp + delay);
                                    action_timestamp += delay;
                                }
                                else addData(table, label, action_timestamp);
                            }
                        // std::cerr << "### J'ai lu une action :: " << delay << " " << label << std::endl;
                        //else std::cerr << "### J'ai lu autre chose :: " << delay << " " << label << std::endl;
                        }
                    }
                }
            }
    }
    else
    {
        std::cerr << "Erreur lors de l'ouverture du fichier " << file << std::endl;
        exit(1);
    }
    f.close();
}

double AntescofoScoreParser::string2double(string fraction) const
{
    unsigned long delimiteur = fraction.find_first_of("/");

    double nomin = atol(fraction.substr(0,delimiteur).c_str());
    double denom = atol(fraction.substr(delimiteur+1).c_str());

    return nomin/denom;
}

void AntescofoScoreParser::addData(map <string, Symbol_phy*>* table, string lab, double timestamp, Pitch* p)
{
    assert(table);
    assert(0 <= timestamp);

    map <string, Symbol_phy*>::iterator it = table->begin();
    while(it != table->end())
    {
        if(!(*it).second->data_rel().label.compare(lab))
        {
            if((*it).second->is_event())
            {
                Event_phy* e = dynamic_cast<Event_phy*>((*it).second);
                assert(p);
                e->link_pitches(p);
                e->set_relTimestamp(timestamp);
                return;
            }
            else
            {
                (*it).second->set_relTimestamp(timestamp);
                return;
            }
        }
        it++;
    }
}
