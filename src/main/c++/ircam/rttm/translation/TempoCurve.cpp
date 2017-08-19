//
//  TempoCurve.cpp
//  timedtracechecker
//
//  Created by Clement Poncelet on 15/01/14.
//
//

#include <translation/TempoCurve.h>

void TempoCurve::visit(Test_suite* ts)
{
    const std::list<Trace*> *lt = ts->getTraces();
    for (std::list<Trace*>::const_iterator it_l = lt->begin(); it_l != lt->end(); it_l++)
    {
        (*it_l)->modify(this);
    }
}

void TempoCurve::visit(Trace* t)
{
    vector<Symbol_rel*>* vs = t->get_trace();
    for (
         vector<Symbol_rel*>::const_iterator it_v = vs->begin();
         it_v != vs->end();
         it_v++
         )
    {
        (*it_v)->modify(this);
    }
}

/**
 e Abs Tempo (the tempo is related to the absolute value but calculated with the next received event (impossible to know the duration before)

 event    e0        d0      e1          ..  ei                  ek        0 END
 tempo    Tscore           tempo[D0]        tempo[Di-1]         tempo[ek-1]

 Input trace ::     e0  d0  Tscore ---->  e1 d1 T[D0] ...

 ** _currenttempo is the tempo computed with the last event. But needed at the current step
**/
void TempoCurve::visit(Event_phy* e)
{
    //We translate the relative time in absolute at the current relative stamp of event (_Relative_Position)
    //We update the relative foward, and compute the related tempo in the curve

    assert(e->data_rel().drel >= 0);
    double phy_duration = (e->data_rel().drel / _current_tempo) *  MINUTE;

    //We set the tempo at the current event
    e->set_tempo(_current_tempo);
    e->set_phyDuration(phy_duration);

    //tempo[di-1] --> tempo[di]
    _current_tempo = _next_tempo;
}

/************** Equation TempoCurve **************/
void Equation_TempoCurve::visit(Event_phy* e)
{
    //In this curve version, we compute the tempo with an equation (a real 'curve')
    //  f(Relative_time) = tempo

     assert(e->data_rel().drel >= 0);

    //last note (duration 0)
    if(0. == e->data_rel().drel)
    {
        e->set_tempo(_current_tempo);
        e->set_phyDuration(0);
    }
    else
    {
        _rel_counter += e->data_rel().trel;
        _next_tempo = f(_rel_counter);
        TempoCurve::visit(e);
    }
}

double Equation_TempoCurve::f(double _t) const {
    return (_a * (_t * _t * _t)) + (_b * (_t * _t)) + (_c * _t) + _d;
}

/************** CoVer TempoCurve **************/

void Cover_TempoCurve::visit(Trace* t) {
    vector<Symbol_rel*> *vs = t->get_trace();

    /** Initialise at test begging **/
    _last_cover_trel = 0.0;
    _last_score_trel = 0.0;
    /*  **  *   **  *   **  *   **  */
    //cerr << "*** test" << endl << endl;

    for (vector<Symbol_rel*>::iterator it_v = vs->begin(); it_v != vs->end(); it_v++) {
        (*it_v)->modify(this);
    }

}

void Cover_TempoCurve::visit(Event_phy* e)
{
    ///> the related symbol is the score ideal trace !
    ///> [] values
    ///> e->data_rel().related_s;

    ///> A Cover input trace is a time-shifted only trace
    ///> To translate this same interpretation in an input trace with tempo-curve (compute delays with the good fluctation)
    ///> We have to compute: [ti] pi' (score time and tempo changed)
    ///> with pi' = [di] * 60 / di'
    ///> ([di] * 60 / di'(s))

    ///> !! jumps: managed with last !!
    auto next_event = e->get_next();

    if(nullptr != next_event)
    {
        double score_duration = next_event->data_rel().related_s->data_rel().trel - e->data_rel().related_s->data_rel().trel; ///> [di]
        double cover_duration = next_event->data_rel().trel - e->data_rel().trel;    ///>di'

        double duration_phy = R_to_S(cover_duration, _current_tempo);

        double tempo_changed = -1;
        if (0. == duration_phy)
        {
            tempo_changed = _last_tempo;
        } else
        {
            assert(0. != score_duration); ///> We don't want interpret a grace note
            tempo_changed = (score_duration * 60) / duration_phy;

        }
        assert(tempo_changed >= 0);

        ///> debug
        ///>cerr << "### d'" << cover_duration << " d" << score_duration << " t'" << tempo_changed << endl;

        e->set_relTimestamp(e->data_rel().related_s->data_rel().trel);
        e->set_tempo(tempo_changed);

        _last_tempo = tempo_changed;

    } else
    {
        e->set_tempo(_last_tempo);
    }

    ///-------- [TO_UPDATE]

    //We compute the equiv. tempo for the coVer time-scaling
    //We want so : The tempo * corresponding to the time scaled duration
    // D* / T = D / T*
    // D is the ideal duration (BEAT) for tempo T of the score
    // D* is the time-scaled duration (BEAT) from coVer
    // T* = D * T/ D*
    // T* is stocked in _next_tempo

    //INSTANTIATION
    //Here we know the equiv. tempo and want the same scale-time with score RELATIVE durations
    //We so instantiate score durations R with the tempo T* computed to have scaled Absolute durations of each note (in s).

    // D(ei) = [ei] * T[ei]
    //Caution those computations are done in a "predictive context" : that mean we re compute the abs. val and know the tempo and the duration
    //Input trace is <>. We re in Antescofo context, the abs. is computed and the tempo is the tempo[di-1] (antescofo detected tempo)
    //Because Antescofo know the duration and the tempo estimation, only when it's effectively elapsed

    //last note (duration 0) FALSE ===== LAST NOTE LABELED STOP
    //Another STOP note is added in the case where last event isn't played (just for input trace)

//    if(!Score->getTrace()->getInputs()->at(num-1)->getLabel().compare("END"))
//    {
//         e->setTranslatedValues(0, _Score_RTime, _last_tempo);
//    }
//    else
//    {
//        _Score_RTime += _last_rt_event_duration;

        //[COMPUTE TEMPO] Computation **
//        double D = e-> Score->getTrace()->getInputs()->at(num-1)->getRelative();
//        double T = Score->getTempo();
//        //see actions (Absolute timestamp)
//        _coVer_Abs = e->At();
//
//        //compute the tempo change for next event
//        _next_tempo = (T * D) / e->getRelative();
//
//        //Dbug
//        //cerr << "eq: D* / T = D / T* : " << e->getDelay() << "/" << T << "=" << D << "/" << _next_tempo << endl;
//        //cerr <<  e->getDelay()/T << "=" << D/_next_tempo << "min, " << e->getDelay()/(T/MINUTE) << "s" << endl;
//        //cerr << "Instantiation [AnteMode]: d* = [d*] / T* " << d << "=" << e->getDelay() << "/(" << _current_tempo << "/" << MINUTE << ")" << endl;
//
//        //END [COMPUTE TEMPO] Computation **
//
//        //[Instantiation]
//        //Mimic Antescofo :: With the tempo (T[e-1])
//        //We are related to the musician clock :: So the elapsed time, has to be instantiated on the last tempo T(e-1)
//        //compute the instantiated value
//        double d = R_to_S(e->getRelative(), _current_tempo);
//
//        //Update event values
//        e->setTranslatedValues(d, _Score_RTime, _current_tempo);
//
//        //Dbug
//        //cerr << "We have " << e->getLabel() << " " << d << " " << _Score_RTime << " " << _current_tempo << endl;
//
//
//        //END [Instantiation]
//
//        //tempo[di-1] --> tempo[di]
//        _last_tempo = _current_tempo;
//
//        //gestion de saut de note ::
//
//        _current_tempo = _next_tempo;
//
//        //Comp. of RTime
//        _last_rt_event_duration = D;
//
//        /**
//         Beware : We have two references :
//            The coVer trace in slice RTime (That we read)
//            The Score RTime that we want to put <>.
//                Compute action° ref. from coVer
//                And next put [act] + ScoreEventTime
//         **/
//
//    }
}
//
void Cover_TempoCurve::visit(Action_phy* a)
{
    double trel = a->data_rel().related_s->data_rel().trel;
    assert(0 <= trel);
    a->set_relTimestamp(trel);

//    //COMPUTE CONTEXT **
//    string score_lab = Symbol_table::_Stable->get_antescofo_lab(a->getLabel(), 'A');
//    double da = a->At();
//    double d = fabs(_coVer_Abs - da);
//
//    //EXPECTED CONTEXT **
//    // !!! WARNING We HAVE to Save TIME CONTRAINTS FROM AUTOMATA !!!
//    //We set the Relative delay between event and action on coVer (with time contraints) d = d[e]-d[a] and set with the score event time D + d
//    a->setTranslatedValues(_Score_RTime + d); //from the beggining of the Relative time score (this action may not be attached of the previous event)
//
//    //Dbug
//    //cerr << "de - da " << _coVer_RTime - da << ":: since score start " <<  _Score_RTime + d <<  " " << score_lab << endl;

}

/************** Random TempoCurve **************/
//Interpolation TODO

void Random_TempoCurve::visit(Trace* t)
{
    std::vector<Symbol_rel*> *vs = t->get_trace();

    _size = vs->size();
    _cur_size = 0;

    for (
         vector<Symbol_rel*>::const_iterator it_v = vs->begin();
         it_v != vs->end();
         it_v++
         )
    {
        (*it_v)->modify(this);
        _cur_size++;
    }
}

void Random_TempoCurve::visit(Event_phy* e)
{

    //last note (duration 0)
//    if(!e->getRelative())
//    {
//       e->setTranslatedValues(0, _current_tempo);
//    }
//    else
//    {
        if(_cur_size < (_size/2) )
        {   //first part
            double p = (_second - _first)/(_size/2);
            _next_tempo = p * (_cur_size) + _first;

        } else
        {
             //second part
            double p = (_third - _second)/(_size - _size/2);
            _next_tempo = p * (_cur_size - _size/2) + _second;
        }
        TempoCurve::visit(e);
//    }
}

/************** Input TempoCurve **************/
void Input_Curve::read_next_tempo()
{
    char ligne[MAX_SIZE];
    while (input_stream.getline(ligne, MAX_SIZE)) {
        int n = 0;

        sscanf(ligne, "%*ld %*f %lf%n", &_current_tempo, &n);
        if (n)
        {
            //cerr << "### " << _current_tempo << " line " << ligne << endl;
            return;
        }
    }
    return;
    assert(false); //> End of file
}

void Input_Curve::visit(Trace* t)
{
    const std::vector<Symbol_rel*> *vs = t->get_trace();

    for (std::vector<Symbol_rel*>::const_iterator it_v = vs->begin(); it_v != vs->end(); it_v++) {
        (*it_v)->modify(this);
    }
}

void Input_Curve::visit(Event_phy* e)
{

    TempoCurve::visit(e);
    read_next_tempo();
}
