//
//  TempoCurve.h
//  timedtracechecker
//
//  Created by Clement Poncelet on 15/01/14.
//
//

#ifndef __timedtracechecker___TempoCurve__
#define __timedtracechecker___TempoCurve__

#include <iostream>
#include <math.h>

#include <base/trace/tablesymbol/Symbols.h>
#include <base/AuxiliaireFunction.h>

/**** ****************************************************************************************
    Tempo Curve

 This class instantiates relative input trace durations to absolute trace duration
    -(defaut) constant tempo (The one on the score)                     type C
    -         linear movement (accelerate or decelerate)                type E (Equation)
              linear random
    -         sinus movement                                            type S
              random movement                                           type R
              random random
    -         coVer values                                              type V
****************************************************************************************  ***/

class TempoCurve : public TModifior
{
public:
    /** Constant (default) **/
    TempoCurve(double tempo): _current_tempo(tempo), _rel_counter(0.0), _next_tempo(tempo) {}
    virtual ~TempoCurve(){}

    virtual void visit(Test_suite* ts);
    virtual void visit(Trace* t);

    virtual void visit(Event_rel* e){assert(false);}
    virtual void visit(Action_rel* a){assert(false);}

    virtual void visit(Event_phy* e);
    virtual void visit(Action_phy* a){}

protected:
    double _current_tempo;
    double _rel_counter;

    double _next_tempo;
    double _last_tempo;
};

    /** Equation **/
class Equation_TempoCurve : public TempoCurve
{
public:                                                         //next tempo will be computed
    Equation_TempoCurve(float a, float b, float c, float d, double init_tempo) :
      TempoCurve(init_tempo), _a(a), _b(b), _c(c), _d(d) {}

    void visit(Event_phy* e);

private:
    //Equation _a * t^3 + _b * _t^2 + _c * _t + _d
    float _a, _b, _c, _d;
    double f(double _t) const;
};

/** Sinus **/
class Sinus_TempoCurve : public TempoCurve
{
public:
    Sinus_TempoCurve(float a, float b, float c, double init_tempo) : TempoCurve(init_tempo), _a(a), _b(b), _c(c) {assert(false && "TODO");}

    void visit(Event_phy* e);

private:
    //Equation sin(x) + _a + _b * x + _c * x^2
    float _a, _b, _c;
};

    /** coVer **/
///> A Cover input trace is a time-shifted only trace
///> We translate this same interpretation in an input trace with tempo-curve (to compute delays with good fluctation)
///> [ti] pi' (score time and tempo changed)
///> with pi' = [di] * 60 / di'
class Cover_TempoCurve : public TempoCurve
{
public:

    //coVer needs the ideal trace (the Score value durations)
    //To compute the difference between cover time-scales and original durations
    Cover_TempoCurve(double init_tempo) : TempoCurve(init_tempo),
    _last_cover_trel(0.0), _last_score_trel(0.0), _last_tempo(init_tempo)    {}

    void visit(Trace* t);
    void visit(Event_phy* e);

    /*  [TOCHECK]! There is a modification of tempo
        ! relative time of actions must map this modification !
     */
    void visit(Action_phy* a);

private:
    double _last_cover_trel;
    double _last_score_trel;
    double _last_tempo;
};

/** Random **/
class Random_TempoCurve : public TempoCurve
{
public:
    Random_TempoCurve(double init_tempo) : TempoCurve(init_tempo)
    {
        //Generate 3 random tempi
        //From 20 to 250 BPM

        srand ((unsigned int)time(NULL));
        _first = rand() % 250 + 20;
        _second = rand() % 250 + 20;
        _third = rand() % 250 + 20;

        //dbug
        std::cerr << "values " << _first << " " << _second << " " << _third << endl;
    }

    void visit(Trace* t);
    void visit(Event_phy* e);

private:
    double _first, _second, _third;
    unsigned long _size;
    unsigned long _cur_size;
};

/* Input File */
class Input_Curve : public TempoCurve
{
public:

    Input_Curve(const char* input_file, double init_tempo) : TempoCurve(init_tempo), input_stream(input_file, ios::in)
    {read_next_tempo();}

    virtual ~Input_Curve(){input_stream.close();}

    void visit(Trace* t);
    void visit(Event_phy* e);

private:
    std::ifstream input_stream;
    void read_next_tempo();
};


#endif /* defined(__timedtracechecker___TempoCurve__) */
