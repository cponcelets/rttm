//
//  Ref_computation.h
//  timedtracechecker
//
//  Created by Clement Poncelet on 09/08/2017.
//  Copyright (c) 2017 Clement Poncelet. All rights reserved.
//

#ifndef __timedtracechecker__Ref_computation_h__
#define __timedtracechecker__Ref_computation_h__

#include <base/trace/tablesymbol/Symbols.h>
#include <base/AuxiliaireFunction.h>

/* ** ** ** ** ** **** ** ** ** ** **
 *  Computation rel to phy visitor ** ** ** ** ** **
 * ** ** ** ** ** **** ** ** ** ** **
 * Used to compute physical reference traces during the translation.
 * A flag is set to manageme the detected tempo, the initial score tempo is needed (from score)
 **/
class Computation_Rel_To_Phy_Visitor : public TModifior
{
public:
  //Constructor / Destructor
    /** For translation
     @param[in] is_treco, true if the input trace is a recognition trace
     @param[in] score_tempo, the score's tempo value

     * If we want treco the simulation phase is not managed, otherwise we simulate it.
     **/
    Computation_Rel_To_Phy_Visitor(bool is_treco, double score_tempo):
        _simulate_detection(!is_treco), _last_event(nullptr), _score_tempo(score_tempo), _detected_tempo(score_tempo) {}

    virtual ~Computation_Rel_To_Phy_Visitor(){}

    //visitor (unused)
    void visit(Test_suite*) override  {assert(false);}
    void visit(Action_rel*) override  {assert(false);}
    void visit(Event_rel*) override   {assert(false);}
    //visitor (used)
    void visit(Trace*) override;
    void visit(Event_phy*) override;
    void visit(Action_phy*) override;

private:
    bool _simulate_detection;       // enable LM mimimc ?
    Event_phy* _last_event;         // The stored last event
    double _score_tempo;            // the ideal tempo
    double _detected_tempo;         // the tempo used to compute durations
    double _last_tempo;             // needed to store pi-1
};

#endif
