//
//  Trace_printer.cpp
//  offline_trace_manager
//
//  Created by Clement Poncelet on 04/05/2015.
//  Copyright (c) 2015 Clement Poncelet. All rights reserved.
//

#include <base/trace/printer/Trace_printer.h>

/* ** ** ** ** ** **** ** ** ** ** **
 *  Show   ** ** ** ** ** **
 * ** ** ** ** ** **** ** ** ** ** **
 **/
void Show::visit(const Test_suite* ts)
{
    _out << "// Start TestSuite"<< std::endl;
    for(
        list<Trace*>::const_iterator it = ts->getTraces()->begin();
        it != ts->getTraces()->end();
        it++
        )
        {(*it)->accept(this);}
    _out << "//       ... the end"<< std::endl;
}

void Show::visit(const Trace* t)
{
    _test_cpt ++;
    _out << "// Test #"<< _test_cpt << std::endl;

    for(
        vector<Symbol_rel*>::const_iterator it = t->sequence()->begin();
        it != t->sequence()->end();
        it++
        )
    {(*it)->accept(this);}
}

void Show::visit(const Action_rel* a)
{
    _out << "   " << a->data_rel().trel << " " << a->data_rel().label << std::endl;
    if(a->data_rel().related_s)
    {
        _out << " related to " << a->data_rel().related_s->data_rel().trel << " " << a->data_rel().related_s->data_rel().label << std::endl;
    }
}

void Show::visit(const Event_rel* e)
{
    _out << "EVENT ";
    _out << " " << e->data_rel().label  << " " << e->data_rel().trel << std::endl;
    //if(e->getPitch()) e->getPitch()->printPitch(_out); TODO

    if(e->data_rel().related_s)
    {
        _out << " related to " << e->data_rel().related_s->data_rel().trel << " " << e->data_rel().related_s->data_rel().label << std::endl;
    }
}

void Show::visit(const Event_phy* e)
{
    visit((Event_rel*) e);
    _out << "\t " << e->data_phy().tphy << " " << e->data_phy().dphy << "s " << e->tempo() << "BPM" << std::endl;
}

void Show::visit(const Action_phy* a)
{
    visit((Action_rel*) a);
    _out << "\t " << a->data_phy().tphy << " " << a->data_phy().dphy << "s " << std::endl;
}

/* ** ** ** ** ** **** ** ** ** ** **
 *  Print Tin   ** ** ** ** ** **
 * ** ** ** ** ** **** ** ** ** ** **
 **/
void Print_tin::visit(const Trace* t)
{
    _backup = _out.rdbuf();

    /** New tin file creation **/
    char cname[_pathbase.size() + 23];
    sprintf(cname, "%s.%d.in", _pathbase.c_str(), _trace_number);

    std::ofstream funder;
    funder.open(cname, std::ios::out);

    _out.rdbuf(funder.rdbuf());

    /* Header */
    _out << "\n\
    // Input trace number " << _trace_number << endl;
    _out << "\
    // Generated from trace_offline_manager v2.\n\
    // -------------------------------------------------------------------\n" << endl;

    /* Body (trace) */
    const auto current_seq = t->sequence();
    for (auto it = current_seq->begin(); it != current_seq->end(); it++)    {(*it)->accept(this);}

    /* Footer */
    _out << "\t//\t" << "End of trace" << std::endl << std::endl;
    funder.close();
    cout.rdbuf(_backup);
}

///> No used
void Print_tin::visit(const Event_rel* e)
{
    assert(false); //>input trace without tempo values [error?]
    const Event_phy* related_e = dynamic_cast<Event_phy*>(e->data_rel().related_s);
    assert(related_e && (related_e->cuenum() >= 0));
    string Antelab = related_e->data_rel().label;

    _out << related_e->cuenum() << " " << e->data_rel().trel << " \"" <<  Antelab  << "\" ";
    related_e->data_rel().pitches->printPitch(_out);
    _out <<  std::endl << std::endl << std::endl;
}

void Print_tin::visit(const Event_phy* e)
{
    const Event_phy* related_e = dynamic_cast<Event_phy*>(e->data_rel().related_s);
    assert(related_e && (related_e->cuenum() >= 0));

    string Antelab = related_e->data_rel().label;

    _out << related_e->cuenum() << " " << e->data_rel().trel << " " << e->tempo() << " \"" << Antelab << "\" ";
    related_e->data_rel().pitches->printPitch(_out);
    _out << std::endl << std::endl;
}

/* ** ** ** ** ** **** ** ** ** ** **
 *  Print tref   ** ** ** ** ** **
 * ** ** ** ** ** **** ** ** ** ** **
 **/
void Print_tref::visit(const Trace* t)
{
     _backup = _out.rdbuf();

    /** New tref file creation **/
    char cname[_pathbase.size() + 23];
    sprintf(cname, "%s.%d.ref", _pathbase.c_str(), _trace_number);

    std::ofstream funder;
    funder.open(cname, std::ios::out);

    _out.rdbuf(funder.rdbuf());

    /* Header */
    _out << "\n\
    // " << (t->is_relative() ? "[relative] " : "")  << "Reference trace number " << _trace_number << endl << "\
    // Generated from trace_offline_manager v2.\n\
    // Time stamps are already computed in physical time. The tempo used to translate them is in the events \n\
    // -------------------------------------------------------------------\n\n";

    _out << "START of traces" << std::endl << std::endl;

    /* trace */
    const auto current_seq = t->sequence();
    for (auto it = current_seq->begin(); it != current_seq->end(); it++) {(*it)->accept(this);}

    funder.close();
    cout.rdbuf(_backup);
}

void Print_tref::visit(const Action_rel* a)
{
    _out << "\t" << a->data_rel().label << " " << a->data_rel().trel << std::endl;      ///> <s, t>
}

void Print_tref::visit(const Event_rel* e)
{
    const Event_phy* related_e = dynamic_cast<Event_phy*>(e->data_rel().related_s);
    assert(related_e && (related_e->cuenum() >= 0));

    _out << "EVENT " << related_e->cuenum() << " " << e->data_rel().label << " " << e->data_rel().trel << std::endl;  ///> event <s, t>
}

void Print_tref::visit(const Event_phy* e)
{
    const Event_phy* related_e = dynamic_cast<Event_phy*>(e->data_rel().related_s);
    assert(related_e && (related_e->cuenum() >= 0));

    _out << "EVENT " << related_e->cuenum() << " " << e->data_rel().label << " " << e->data_phy().tphy << " " << e->tempo() << std::endl; ///> event <s, tp, p>
}

void Print_tref::visit(const Action_phy* a)
{
   _out << "\t" << a->data_rel().label << " " << a->data_phy().tphy << std::endl; ///> <s, tp>
}
