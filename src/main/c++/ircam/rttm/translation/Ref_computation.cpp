//
//  Ref_computation.cpp
//  timedtracechecker
//
//  Created by Clement Poncelet on 09/08/2017.
//  Copyright (c) 2017 Clement Poncelet. All rights reserved.
//

#include <translation/Ref_computation.h>

/* ** ** ** ** ** **** ** ** ** ** **
 *  Generation rel to phy visitor ** ** ** ** ** **
 * ** ** ** ** ** **** ** ** ** ** **
 **/
void Computation_Rel_To_Phy_Visitor::visit(Trace* t)
{
    for(auto it = t->sequence()->begin(); it != t->sequence()->end(); it++) {(*it)->modify(this);}
}

void Computation_Rel_To_Phy_Visitor::visit(Event_phy* e)
{
    /**
        Strategy: compute with the next field (easier for detection vision)
        We visit ei and want to compute:
            tpi (physical timestamp for ei)
            with pi (the good tempo)
     ------
        We need so ti+1 (relative timestamp of ei+1)
        and its tempo in case of detection (pi+1)
     ------
     **/

    ///> compute: tpi = tpi-1 + dpi-1 [if first] (tr * 60) / score_tempo since tr = dr
    if(nullptr == _last_event)
    {
        e->set_timestamp(R_to_S(e->data_rel().trel, _score_tempo)); ///>_current_tempo init with score tempo
    }
    else
    {
        assert(_last_event->get_next() == e);
        e->set_timestamp(_last_event->data_phy().tphy + _last_event->data_phy().dphy);
    }

    ///> Computation of di (rel and phy) [if last] -1
    if(nullptr == e->get_next())
    {
        /** last event **/
        e->set_relDuration(-1);
        e->set_phyDuration(-1);
    }
    else
    {
        ///> !!!! ~~~ Important note: Compute the next event duration totally diff. than simulate Antescofo behavior ~~~ !!!!!
        ///> former is event_tempo and the latter _detected_tempo

        ///> update p for this event and the following actions
        ///> with delay or not (to compute dpi the physical duration of ei)
        double event_tempo;
        if(_simulate_detection)
        {
            event_tempo = e->tempo();                                                   ///> (di * 60) / pi    [case tin with LM mimic]
            _detected_tempo = ((nullptr == _last_event) ? _score_tempo : _last_tempo);  ///> but the detected tempo is the last one
        }
        else
        {
            event_tempo = e->get_next()->tempo();                       ///> (di * 60) / pi+1      [case treco (the tempo of ei is pi+1)]
            _detected_tempo = e->tempo();                               ///> pi
        }

        assert(event_tempo > 0.0);

        ///>computations
        assert(event_tempo > 0.0); ///> We are visiting the input trace, tempo values have to be present
        ///> drel i
        double drel = e->get_next()->data_rel().trel - e->data_rel().trel;   ///> di = ti+1 - ti
        double dphy = R_to_S(drel, event_tempo);
        e->set_relDuration(drel);
        e->set_phyDuration(dphy);

        ///> update the tempo in the trace for printing the references
        ///> It is a little mone tricky
        ///> for a reco case, pi is forced and used for compute delays after ei
        ///> for a musician LM mimic case, it is the pi-1 tempo that is printed !! (<> those of the event duration)
        if(_simulate_detection)
                                        {e->set_tempo(_detected_tempo);} ///>tin pi-1
        else                            {e->set_tempo(_detected_tempo);} ///> treco -> pi

         _last_tempo = event_tempo; ///> pi-1 (deleted when replacing the tempo of e)
    }
    _last_event = e;
}

void Computation_Rel_To_Phy_Visitor::visit(Action_phy* a)
{
    ///> computation for dpi for an action.
    ///> Since it is delays and no durations, the amount of time is related to the last event
    ///> And translated by the current tempo [the same thempo as the event duration]
    double delay_rel = a->data_rel().trel - _last_event->data_rel().trel;   ///> dri = ti - tei
    double delay_phy = R_to_S(delay_rel, _detected_tempo);

    a->set_timestamp(_last_event->data_phy().tphy + delay_phy);             ///> tpi = tpei + dpi
    a->set_relDuration(delay_rel);
    a->set_phyDuration(delay_phy);
}
