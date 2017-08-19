//
//  Pitch.cpp
//  timedtracechecker
//
//  Created by Clement Poncelet on 15/05/2015.
//  Copyright (c) 2015 Clement Poncelet. All rights reserved.
//

#include <stdio.h>
#include <base/trace/tablesymbol/Pitch.h>

void NOTE2::printPitch(ostream& os) const
{
    os << _pitch;
}

void NOTE2::print(ostream& os) const
{
    os << "NOTE ";
    printPitch(os);
}

void CHORD2::printPitch(std::ostream& os) const
{
    os << "( ";
    for(std::vector<std::string>::const_iterator it = _pitches.begin();
        it != _pitches.end();
        it++)
        os << (*it) << " ";

    os << ")";
}

void CHORD2::print(ostream& os) const
{
    os << "CHORD ";
    printPitch(os);
}

void TRILL2::printPitch(ostream& os) const
{
    os << "( ";
    for(std::vector<std::string>::const_iterator it = _pitches.begin();
        it != _pitches.end();
        it++)
        os << (*it) << " ";

    os << ")";
}

void TRILL2::print(ostream& os) const
{
    os << "TRILL ";
    printPitch(os);
}
