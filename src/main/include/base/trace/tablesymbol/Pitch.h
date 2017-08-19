//
//  Pitch.h
//  timedtracechecker
//
//  Created by Clement Poncelet on 13/05/2015.
//  Copyright (c) 2015 Clement Poncelet. All rights reserved.
//

#ifndef __timedtracechecker__Pitch_h__
#define __timedtracechecker__Pitch_h__

/*
 *
 *  To store pitches and print them correctly in the input trace
 *  Data is read in the antescofo mixed score
 */

#include <iostream>
#include <fstream>
#include <vector>

using namespace std;

/* ** ** ** ** ** **** ** ** ** ** **
 *  A Pitch ** ** ** ** ** **
 * ** ** ** ** ** **** ** ** ** ** **
 */
class Pitch {
public:
  //Constructor / Destructor
    Pitch(){}
    virtual ~Pitch(){}

  //printer
    //print the name and pitches
    virtual void print(ostream& os) const = 0;
    //print only pitches
    virtual void printPitch(ostream& os) const = 0;
};

/* ** ** ** ** ** **** ** ** ** ** **
 *  A Note ** ** ** ** ** **
 * ** ** ** ** ** **** ** ** ** ** **
 */
class NOTE2 : public Pitch {
public:
  //Constructor / Destructor
    /*
      @param[in] value, the pitch
    */
    NOTE2(string value) : _pitch(value){}
    ~NOTE2() {}

    void printPitch(ostream& os) const override;
    void print(ostream& os) const override;

private:
    string _pitch; //value of pictch (A or LA or 64)
};

/* ** ** ** ** ** **** ** ** ** ** **
 *  A Chord ** ** ** ** ** **
 * ** ** ** ** ** **** ** ** ** ** **
 */
class CHORD2 : public Pitch {
public:
  //Constructor / Destructor
    CHORD2() : Pitch(), _pitches(){}
    ~CHORD2() {}

  //Setters
    //add pich
    void add_pitch(string p) {_pitches.push_back(p);}

    void printPitch(ostream& os) const override;
    void print(ostream& os) const override;

private:
    vector<string> _pitches;
};

/* ** ** ** ** ** **** ** ** ** ** **
 *  A Trill ** ** ** ** ** **
 * ** ** ** ** ** **** ** ** ** ** **
 */
class TRILL2 : public Pitch {
public:
  //Constructor / Destructor
    TRILL2() : Pitch(), _pitches(){}
    ~TRILL2() {}

  //Setters
    //add pich
    void add_pitch(string p) {_pitches.push_back(p);}

    void printPitch(ostream& os) const;
    void print(ostream& os) const;

private:
    vector<string> _pitches;
};

#endif
