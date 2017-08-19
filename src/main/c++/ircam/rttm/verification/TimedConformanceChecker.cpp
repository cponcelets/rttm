//
//  TimedConformanceChecker.cpp
//  timedtracechecker
//
//  Created by Clement Poncelet on 20/05/2015.
//  Copyright (c) 2015 Clement Poncelet. All rights reserved.
//

#include <verification/TimedConformanceChecker.h>

/* ** ** ** ** ** **** ** ** ** ** **
 *  Timed conformance checker ** ** ** ** ** **
 * ** ** ** ** ** **** ** ** ** ** **
 **/
bool TimedConformanceChecker::check()
{
    std::cout << "Check the timed conformance" << std::endl;

    // First phase
    if (!compute()) return false;
    //the second phase
    return is_conform();
}

/*
 Private methods
 */
bool TimedConformanceChecker::compute()
{
  ///> already in physical time ?
  if(!reference_trace->is_relative()) return true;

  Rel_To_Phy_Visitor computer;
  auto it_symbol = reference_trace->get_trace()->begin();

  ///> compute each symbol's absolute values
  while (reference_trace->get_trace()->end() != it_symbol)
  {
      (*it_symbol)->modify(&computer);
      if(_check_tempo && computer.is_wrong_tempo())   {return false;}
      it_symbol++;
  }

  return true;
}

bool TimedConformanceChecker::is_conform()
{
    Verdict_printer checker(reference_trace);
    std::vector<Symbol_rel*>::const_iterator it_symbol = real_trace->sequence()->begin();

    ///> verdict's header
    std::cout << "_____________________________________________________________________________" << std::endl;
    std::cout << "|-        Antescofo Trace               |          Expected Trace          -|" << std::endl;
    std::cout << "|-    label      now      [rnow]        |label  comp. timestamp [ref beat] -|" << std::endl;
    std::cout << "|---------------------------------------------------------------------------|" << std::endl;

    //verification for each real trace's symbol
    while (real_trace->sequence()->end() != it_symbol)
    {
        (*it_symbol)->accept(&checker);
        it_symbol++;
    }
    //terminate the verification (the label of the last real event is given)
    checker.terminate(real_trace->sequence()->at(real_trace->sequence()->size()-1)->data_rel().label);

    //footer
    std::cout << "|---------------------------------------------------------------------------|" << std::endl;

    return checker.is_conformed();
}

/* ** ** ** ** ** **** ** ** ** ** **
 *  Rel to phy visitor ** ** ** ** ** **
 * ** ** ** ** ** **** ** ** ** ** **
 **/
void Rel_To_Phy_Visitor::visit(Event_phy* e) //event computation
{
    if(nullptr == _last_event)
    {   //first event
        e->set_timestamp(e->data_rel().trel);
    }
    else
    {
        Event_phy* last_related_event = dynamic_cast<Event_phy*>(_last_event->data_rel().related_s);
        double tempo_value = last_related_event->tempo();

        //check
        if(round(tempo_value) != round(_last_event->tempo()))
        {
            cerr    << "Caution, the two traces have not the same tempo values (" << _last_event->data_rel().label
                    << "," << last_related_event->data_rel().label << ")" << endl;

            _error = true;
        }

        //computation
        double last_drel = e->data_rel().trel - _last_event->data_rel().trel; //[duration]drel_i-1
        double last_dphy = R_to_S(last_drel, tempo_value);                    //[duration]dphy_i-1

        _last_event->set_relDuration(last_drel);
        _last_event->set_phyDuration(last_dphy);
        e->set_timestamp(_last_event->data_phy().tphy + last_dphy);           //tphy_i
    }
    _last_event = e;
}

void Rel_To_Phy_Visitor::visit(Action_phy* a) //action computation
{
    Event_phy* last_related_symbol = dynamic_cast<Event_phy*>(_last_event->data_rel().related_s);
    double tempo_value = last_related_symbol->tempo();

    //computation
    double delay_rel = a->data_rel().trel - _last_event->data_rel().trel;   //[delay]drel_i
    double delay_phy = R_to_S(delay_rel, tempo_value);                      //[delay]dphy_i

    a->set_timestamp(_last_event->data_phy().tphy + delay_phy);             //tphy_i
    a->set_relDuration(delay_rel);
    a->set_phyDuration(delay_phy);
}

/* ** ** ** ** ** **** ** ** ** ** **
 *  Verdict_printer ** ** ** ** ** **
 * ** ** ** ** ** **** ** ** ** ** **
 **/
Verdict_printer::Verdict_printer(const Trace* ref):
    _refTrace(ref),
    _previous_ref_symbol(NULL),  _previous_real_symbol(NULL), _last_event(NULL),
    _test_finished(false), _conform(true)
{
    Symbol_phy* last_ref_symbol = dynamic_cast<Symbol_phy*>(_refTrace->sequence()->at(_refTrace->sequence()->size() - 1));
    _end_timestamp = last_ref_symbol->data_phy().tphy;

    _it_ref = _refTrace->sequence()->begin();
    _previous_ref_symbol = dynamic_cast<Symbol_phy*>(*_it_ref);
}

void Verdict_printer::visit(const Symbol_phy* s)
{
    Symbol_phy* reference = dynamic_cast<Symbol_phy*>(s->data_rel().related_s);

    if(!_previous_real_symbol)
    { //first symbol
        _previous_real_symbol = s;
    }
    else
    {                               //last timestamp
        if(epsilon_compare_double(_previous_real_symbol->data_phy().tphy,  s->data_phy().tphy)) //change instant
           {
               if (is_missing_symbols())                                                             //is_missing_symbols
               {
                   if(_conform  && !_test_finished) _conform = false;
               }
               print_flow_time(reference, s);
               _previous_real_symbol = s;      //current now                                                 //update
               if(epsilon_compare_double(_previous_real_symbol->data_phy().tphy, _end_timestamp) > 0)
               {
                   std::cout << "|-------------------------END TIMESTAMP OF REF TRACE------------------------|" << std::endl;
                   _test_finished = true;
               }
           }
    }
    if(is_unexepected(s))
    {
        if(_conform && !_test_finished) _conform = false;
    }
    else
    {
        if(epsilon_compare_double(s->data_phy().tphy, reference->data_phy().tphy))
        {
            print_timed_error(reference, s);
            if(_conform && !_test_finished) _conform = false;
        }
        else
        {
            print_conform(reference, s);
        }
    }
    if(s->is_event()) _last_event = dynamic_cast<const Event_phy*>(s);
}

//private
int Verdict_printer::epsilon_compare_double(double a, double b) const
{
    double res = a-b;
    if(res < -SEC_EPSILON)  return -1;
    if(res > SEC_EPSILON)   return 1;
    return 0;
}

bool Verdict_printer::is_missing_symbols()
{
    if(_refTrace->sequence()->end() == _it_ref) return false;
    Symbol_phy* ref_phy_symbol = dynamic_cast<Symbol_phy*>(*_it_ref);
    bool is_missing = false;

    //each symbol until the same logical instant (or the end)
    //we move forward in the ref trace (to check missed event)
    while( (_it_ref != _refTrace->sequence()->end()) && (!epsilon_compare_double(ref_phy_symbol->data_phy().tphy,
                                                                                 _previous_real_symbol->data_phy().tphy)) )
    {
        Symbol_phy* phy_symbol_related = dynamic_cast<Symbol_phy*>((*_it_ref)->data_rel().related_s);
        if(phy_symbol_related->is_erroneous())
        {
            print_missing_symbol(ref_phy_symbol, phy_symbol_related);
            is_missing = true;
        }
        _previous_ref_symbol = ref_phy_symbol; //save the previous ref symbol to compute time between instants

        _it_ref++;
        if(_it_ref != _refTrace->sequence()->end())
        {
            ref_phy_symbol = dynamic_cast<Symbol_phy*>(*_it_ref);
        }
    }

    return is_missing;
}

bool Verdict_printer::is_unexepected(const Symbol_phy* s) const
{
    Symbol_phy* phy_symbol_related = dynamic_cast<Symbol_phy*>(s->data_rel().related_s);
    if(phy_symbol_related->is_erroneous())
    {
        print_unexepected(s);
        return true;
    }
    return false;
}

void Verdict_printer::terminate(string last_label)
{
    if(_it_ref == _refTrace->sequence()->end()) return;

    Symbol_phy* ref_phy_symbol = dynamic_cast<Symbol_phy*>(*_it_ref);

    if(!_test_finished && (epsilon_compare_double(ref_phy_symbol->data_phy().tphy,
                                                  _end_timestamp)) ) _conform = false;

    while( (_it_ref != _refTrace->sequence()->end()) )
    {
        Symbol_phy* phy_symbol_related = dynamic_cast<Symbol_phy*>((*_it_ref)->data_rel().related_s);
        if(phy_symbol_related->is_erroneous())
        {
            print_missing_symbol(ref_phy_symbol, phy_symbol_related);
        }
        _it_ref++;
        if(_it_ref != _refTrace->sequence()->end())
        {
            ref_phy_symbol = dynamic_cast<Symbol_phy*>(*_it_ref);
        }
    }
}

//Print functions
void Verdict_printer::print_missing_symbol(const Symbol_phy* ref, const Symbol_phy* missed) const
{
    cout << "x " << setw(LENGTHLABEL) << missed->data_rel().label << "  missed     "
         << "x " << setw(LENGTHLABEL) << ref->data_rel().label << " " << setw(PRECISIONFLOAT) << ref->data_phy().tphy
                                      << "[" << setw(PRECISIONFLOAT) << ref->data_rel().trel << "] x" << endl;
}

void Verdict_printer::print_flow_time(const Symbol_phy* ref, const Symbol_phy* real) const
{
    double dphy_real = real->data_phy().tphy - _previous_real_symbol->data_phy().tphy;
    double drel_real = real->data_rel().trel - _previous_real_symbol->data_rel().trel;

    cout << " +" << setw(PRECISIONFLOAT) << dphy_real << " (" << setw(PRECISIONFLOAT) << drel_real << " * "
                                << setw(PRECISIONFLOAT) << _last_event->tempo() << ")          ";

    double dphy_ref = ref->data_phy().tphy - _previous_ref_symbol->data_phy().tphy;
    double drel_ref = ref->data_rel().trel - _previous_ref_symbol->data_rel().trel;

    //compare lasted duration in beat to mark time-shift
    int res = epsilon_compare_double(drel_real, drel_ref);

    if (res > 0)
    {
        cout << "> ";
    }
    else if (res == 0)
    {
        cout << "==";
    }
    else
    {
        cout << "< ";
    }

    cout << setw(PRECISIONFLOAT) << dphy_ref << " ("
         << setw(PRECISIONFLOAT) << drel_ref << " * "
         << setw(PRECISIONFLOAT) << _last_event->tempo() << ") " << endl;
}//last event ...

void Verdict_printer::print_unexepected(const Symbol_phy* s) const
{
    cout << "x " << setw(LENGTHLABEL) << s->data_rel().label << " " << setw(PRECISIONFLOAT) << s->data_phy().tphy
                 << setw(PRECISIONFLOAT) << "[" << s->data_rel().trel << "]  "

         << "x " << setw(LENGTHLABEL) << s->data_rel().related_s->data_rel().label << "  unexpected   x" << endl;
}

void Verdict_printer::print_timed_error(const Symbol_phy* ref, const Symbol_phy* real) const
{
    if(ref->is_event())
        print_timed_error((Event_phy*) ref, (Event_phy*)real);
    else
        print_timed_error((Action_phy*)ref, (Action_phy*)real);
}

void Verdict_printer::print_timed_error(const Event_phy* ref, const Event_phy* real) const
{
    double d = real->data_phy().tphy - ref->data_phy().tphy;
    cout << "x " << setw(LENGTHLABEL) << real->data_rel().label << " " << setw(PRECISIONFLOAT) << real->data_phy().tphy
                 << "       [" << setw(PRECISIONFLOAT) << real->data_rel().trel << "]   "

         << "x " << setw(LENGTHLABEL) << ref->data_rel().label << " " << setw(PRECISIONFLOAT) << ref->data_phy().tphy
                 << "[" << setw(PRECISIONFLOAT) << ref->data_rel().trel << "]       x " << setw(PRECISIONFLOAT) << real->tempo()
    << "BPM delta:" << d << endl;
}

void Verdict_printer::print_timed_error(const Action_phy* ref, const Action_phy* real) const
{
    double d = real->data_phy().tphy - ref->data_phy().tphy;
    cout << "x " << setw(LENGTHLABEL) << real->data_rel().label << " " << setw(PRECISIONFLOAT) << real->data_phy().tphy
                 << "       [" << setw(PRECISIONFLOAT) << real->data_rel().trel << "]   "

         << "x " << setw(LENGTHLABEL) << ref->data_rel().label << " " << setw(PRECISIONFLOAT) << ref->data_phy().tphy
                 << "[" << setw(PRECISIONFLOAT) << ref->data_rel().trel << "] "
         << "      x       delta:" << d << endl;
}

void Verdict_printer::print_conform(const Symbol_phy* ref, const Symbol_phy* real) const
{
    if(ref->is_event())
        print_conform((Event_phy*) ref, (Event_phy*)real);
    else
        print_conform((Action_phy*)ref, (Action_phy*)real);
}

void Verdict_printer::print_conform(const Event_phy* ref, const Event_phy* real) const
{
    cout << "| " << setw(LENGTHLABEL) << real->data_rel().label << " " << setw(PRECISIONFLOAT) << real->data_phy().tphy << "        ["
                 << setw(PRECISIONFLOAT) << real->data_rel().trel << "]   "

    << "| " << setw(LENGTHLABEL) << ref->data_rel().label << " " << setw(PRECISIONFLOAT) << ref->data_phy().tphy << "       ["
            << setw(PRECISIONFLOAT) << ref->data_rel().trel << "]|* " << setw(PRECISIONFLOAT) << real->tempo() << "BPM" << endl;
}

void Verdict_printer::print_conform(const Action_phy* ref, const Action_phy* real) const
{
    cout << "| " << setw(LENGTHLABEL) << real->data_rel().label << " " << setw(PRECISIONFLOAT) << real->data_phy().tphy << "        ["
                 << setw(PRECISIONFLOAT) << real->data_rel().trel << "]   "

    << "| " << setw(LENGTHLABEL) << ref->data_rel().label << " " << setw(PRECISIONFLOAT) << ref->data_phy().tphy << "       ["
            << setw(PRECISIONFLOAT) << ref->data_rel().trel << "]|" << endl;
}
